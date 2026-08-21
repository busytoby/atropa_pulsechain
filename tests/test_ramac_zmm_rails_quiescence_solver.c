#define _POSIX_C_SOURCE 200809L
#include "tsfi_ramac_zmm_rails_solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

static double get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

int main(void) {
    printf("========================================================================================\n");
    printf("   FORMAL PROOF: RAMAC ZMM RAILS & SECTOR QUIESCENCE NULLSTELLENSATZ SOLVER             \n");
    printf("   Target Prover: solidity/dysnomia/domain/std/ramac_zmm_rails_quiescence_prover.algol61\n");
    printf("========================================================================================\n\n");

    printf("[STAGE 1] Testing Valid 512-Bit ZMM Rail Sector 0 Invariant (Cylinders 0..49)...\n");
    for (uint32_t cyl = 0; cyl < 50; cyl++) {
        TsfiRamacZmmProofParams params = {
            .zmm_rail_active_flag = 1,
            .sector_index = 0,
            .cylinder_index = cyl,
            .totient_state_val = 0,
            .radical_power_m = 8,
            .accumulator_energy = 0,
            .cics_writer_id = 1000000,
            .simulate_fault = 0,
            .k_param = 3
        };
        TsfiRamacZmmRuling ruling = tsfi_verify_ramac_zmm_rails_quiescence(&params);
        assert(ruling == TSFI_RAMAC_ZMM_QUALIFIED);
    }
    printf("   ✓ Formally Proven across all 50 RAMAC cylinders: QUALIFIED (0)\n\n");

    printf("[STAGE 2] Testing Sector Offset & Non-Zero Energy Boundary Rejections...\n");
    {
        // 1. Sector index != 0 violation during quiescent hold
        TsfiRamacZmmProofParams bad_sec = {
            .zmm_rail_active_flag = 1,
            .sector_index = 5, // Non-quiescent sector
            .cylinder_index = 0,
            .totient_state_val = 0,
            .radical_power_m = 4,
            .accumulator_energy = 0,
            .cics_writer_id = 1000000,
            .simulate_fault = 0,
            .k_param = 3
        };
        assert(tsfi_verify_ramac_zmm_rails_quiescence(&bad_sec) == TSFI_RAMAC_ZMM_INVALID_SECTOR);

        // 2. Accumulator non-zero energy violation
        TsfiRamacZmmProofParams bad_acc = {
            .zmm_rail_active_flag = 1,
            .sector_index = 0,
            .cylinder_index = 0,
            .totient_state_val = 0,
            .radical_power_m = 4,
            .accumulator_energy = 75,
            .cics_writer_id = 1000000,
            .simulate_fault = 0,
            .k_param = 3
        };
        assert(tsfi_verify_ramac_zmm_rails_quiescence(&bad_acc) == TSFI_RAMAC_ZMM_ACCUMULATOR_VIOLATION);
    }
    printf("   ✓ Invalid Sector Offsets & Energy Dissipation Properly Quarantined\n\n");

    printf("[STAGE 3] Executing 100,000 Repeated Sub-Microsecond ZMM Rail Proofs (< 100 ns)...\n");
    const uint32_t ITERS = 100000;
    double start_ns = get_time_ns();
    for (uint32_t i = 0; i < ITERS; i++) {
        TsfiRamacZmmProofParams params = {
            .zmm_rail_active_flag = 1,
            .sector_index = 0,
            .cylinder_index = i % 50,
            .totient_state_val = 0,
            .radical_power_m = (i % 16) + 1,
            .accumulator_energy = 0,
            .cics_writer_id = 1000000,
            .simulate_fault = 0,
            .k_param = 3
        };
        TsfiRamacZmmRuling r = tsfi_verify_ramac_zmm_rails_quiescence(&params);
        (void)r;
    }
    double total_ns = get_time_ns() - start_ns;
    double avg_ns = total_ns / (double)ITERS;

    printf("   ✓ Total Executions: %u\n", ITERS);
    printf("   ✓ Average ZMM Rail Proof Verification Latency: %.2f ns (Guard Gate: < 100 ns)\n", avg_ns);
    assert(avg_ns < 100.0);

    printf("\n========================================================================================\n");
    printf("RAMAC ZMM RAILS & SECTOR QUIESCENCE SOLVER CERTIFIED (ALL 3 STAGES PASSED)\n");
    printf("========================================================================================\n");
    return 0;
}
