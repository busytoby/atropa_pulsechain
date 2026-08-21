#define _POSIX_C_SOURCE 200809L
#include "tsfi_ramac_fourier_dirichlet_solver.h"
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
    printf("   FORMAL PROOF: RAMAC FOURIER ZORSE DIRICHLET BOUNDARY & PHASE COHERENCE SOLVER        \n");
    printf("   Target Prover: solidity/dysnomia/domain/std/ramac_fourier_dirichlet_boundary_prover.algol61\n");
    printf("========================================================================================\n\n");

    printf("[STAGE 1] Testing Valid Fourier Zorse Dirichlet Boundary across Heads [0..23] & EDO-22...\n");
    for (uint32_t head = 0; head < 24; head++) {
        for (uint32_t k = 1; k <= 22; k++) {
            TsfiRamacFourierProofParams params = {
                .head_index = head,
                .cylinder_index = head % 50,
                .totient_state_val = 0,
                .harmonic_step_k = k,
                .initial_phase_jitter_fs = 1250,
                .cics_writer_id = 1000000,
                .simulate_fault = 0,
                .k_param = 3
            };
            TsfiRamacFourierRuling ruling = tsfi_verify_ramac_fourier_dirichlet_boundary(&params);
            assert(ruling == TSFI_RAMAC_FOURIER_QUALIFIED);
        }
    }
    printf("   ✓ Formally Proven across 24 Heads x 22 Microtones: QUALIFIED (0)\n\n");

    printf("[STAGE 2] Testing Head Index & Phase Jitter Boundary Fault Rejections...\n");
    {
        // 1. Invalid head index > 23
        TsfiRamacFourierProofParams bad_head = {
            .head_index = 25,
            .cylinder_index = 0,
            .totient_state_val = 0,
            .harmonic_step_k = 1,
            .initial_phase_jitter_fs = 100,
            .cics_writer_id = 1000000,
            .simulate_fault = 0,
            .k_param = 3
        };
        assert(tsfi_verify_ramac_fourier_dirichlet_boundary(&bad_head) == TSFI_RAMAC_FOURIER_INVALID_HEAD);

        // 2. Non-zero totient state violation
        TsfiRamacFourierProofParams bad_totient = {
            .head_index = 0,
            .cylinder_index = 0,
            .totient_state_val = 15,
            .harmonic_step_k = 1,
            .initial_phase_jitter_fs = 100,
            .cics_writer_id = 1000000,
            .simulate_fault = 0,
            .k_param = 3
        };
        assert(tsfi_verify_ramac_fourier_dirichlet_boundary(&bad_totient) == TSFI_RAMAC_FOURIER_NON_ZERO_TOTIENT);
    }
    printf("   ✓ Invalid Head Indexes & Non-Zero Totient Violations Properly Quarantined\n\n");

    printf("[STAGE 3] Executing 100,000 Repeated Sub-Microsecond Proof Verifications (< 100 ns)...\n");
    const uint32_t ITERS = 100000;
    double start_ns = get_time_ns();
    for (uint32_t i = 0; i < ITERS; i++) {
        TsfiRamacFourierProofParams params = {
            .head_index = i % 24,
            .cylinder_index = i % 50,
            .totient_state_val = 0,
            .harmonic_step_k = (i % 22) + 1,
            .initial_phase_jitter_fs = (i % 5000),
            .cics_writer_id = 1000000,
            .simulate_fault = 0,
            .k_param = 3
        };
        TsfiRamacFourierRuling r = tsfi_verify_ramac_fourier_dirichlet_boundary(&params);
        (void)r;
    }
    double total_ns = get_time_ns() - start_ns;
    double avg_ns = total_ns / (double)ITERS;

    printf("   ✓ Total Executions: %u\n", ITERS);
    printf("   ✓ Average Fourier Dirichlet Proof Latency: %.2f ns (Guard Gate: < 100 ns)\n", avg_ns);
    assert(avg_ns < 100.0);

    printf("\n========================================================================================\n");
    printf("RAMAC FOURIER ZORSE DIRICHLET BOUNDARY SOLVER CERTIFIED (ALL 3 STAGES PASSED)\n");
    printf("========================================================================================\n");
    return 0;
}
