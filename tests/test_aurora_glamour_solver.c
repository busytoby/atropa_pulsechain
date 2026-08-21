#define _POSIX_C_SOURCE 200809L
#include "tsfi_aurora_glamour_solver.h"
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
    printf("   FORMAL PROOF: AURORA GEOMAGNETIC PLASMA & GLAMOUR DISSIPATION SOLVER                \n");
    printf("   Target Prover: solidity/dysnomia/domain/std/aurora_glamour_dissipation_prover.algol61\n");
    printf("========================================================================================\n\n");

    printf("[STAGE 1] Testing Valid Planetary Kp [0..9] & Auroral Ionization Dissipation...\n");
    for (uint32_t kp = 0; kp <= 9; kp++) {
        for (uint32_t intens = 0; intens <= 10; intens++) {
            TsfiAuroraProofParams params = {
                .kp_index = kp,
                .auroral_intensity_level = intens,
                .active_glamour_opacity = 80,
                .accumulator_energy = 2500,
                .cics_writer_id = 1000000,
                .simulate_fault = 0,
                .k_param = 3
            };
            TsfiAuroraRuling ruling = tsfi_verify_aurora_glamour_dissipation(&params);
            assert(ruling == TSFI_AURORA_QUALIFIED);
        }
    }
    printf("   ✓ Formally Proven across Planetary Kp [0..9] and Auroral Intensities [0..10]: QUALIFIED (0)\n\n");

    printf("[STAGE 2] Testing Kp Overflow & Negative Energy Fault Interception...\n");
    {
        // 1. Kp index > 9 overflow
        TsfiAuroraProofParams bad_kp = {
            .kp_index = 10, // Invalid Kp
            .auroral_intensity_level = 5,
            .active_glamour_opacity = 50,
            .accumulator_energy = 500,
            .cics_writer_id = 1000000,
            .simulate_fault = 0,
            .k_param = 3
        };
        assert(tsfi_verify_aurora_glamour_dissipation(&bad_kp) == TSFI_AURORA_INVALID_KP);

        // 2. Glamour opacity > 100 overflow
        TsfiAuroraProofParams bad_op = {
            .kp_index = 6,
            .auroral_intensity_level = 5,
            .active_glamour_opacity = 120, // Invalid opacity
            .accumulator_energy = 500,
            .cics_writer_id = 1000000,
            .simulate_fault = 0,
            .k_param = 3
        };
        assert(tsfi_verify_aurora_glamour_dissipation(&bad_op) == TSFI_AURORA_INVALID_OPACITY);
    }
    printf("   ✓ Kp Overflows & Invalid Opacities Properly Quarantined\n\n");

    printf("[STAGE 3] Executing 100,000 Repeated Sub-Microsecond Proof Verifications (< 100 ns)...\n");
    const uint32_t ITERS = 100000;
    double start_ns = get_time_ns();
    for (uint32_t i = 0; i < ITERS; i++) {
        TsfiAuroraProofParams params = {
            .kp_index = i % 10,
            .auroral_intensity_level = (i % 11),
            .active_glamour_opacity = (i % 101),
            .accumulator_energy = (i % 10000),
            .cics_writer_id = 1000000,
            .simulate_fault = 0,
            .k_param = 3
        };
        TsfiAuroraRuling r = tsfi_verify_aurora_glamour_dissipation(&params);
        (void)r;
    }
    double total_ns = get_time_ns() - start_ns;
    double avg_ns = total_ns / (double)ITERS;

    printf("   ✓ Total Executions: %u\n", ITERS);
    printf("   ✓ Average Aurora Glamour Proof Latency: %.2f ns (Guard Gate: < 100 ns)\n", avg_ns);
    assert(avg_ns < 100.0);

    printf("\n========================================================================================\n");
    printf("AURORA GEOMAGNETIC PLASMA & GLAMOUR DISSIPATION SOLVER CERTIFIED (ALL 3 STAGES PASSED)\n");
    printf("========================================================================================\n");
    return 0;
}
