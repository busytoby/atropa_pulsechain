#define _POSIX_C_SOURCE 200809L
#include "tsfi_vaesen_sight_solver.h"
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
    printf("   FORMAL PROOF: VAESEN THE SIGHT (TROLLSYNT) & PERCEPTION SOLVER                       \n");
    printf("   Target Prover: solidity/dysnomia/domain/std/vaesen_sight_perception_prover.algol61   \n");
    printf("========================================================================================\n\n");

    printf("[STAGE 1] Testing Valid The Sight Perception across 22-EDO Microtones & Origins [1..5]...\n");
    for (uint32_t origin = 1; origin <= 5; origin++) {
        for (uint32_t k = 1; k <= 22; k++) {
            TsfiVaesenSightProofParams params = {
                .sight_active_flag = 1,
                .trauma_origin_code = origin,
                .creature_invisibility_level = 5,
                .microtone_bin_k = k,
                .accumulator_energy = 1500,
                .cics_writer_id = 1000000,
                .simulate_fault = 0,
                .k_param = 3
            };
            TsfiVaesenSightRuling ruling = tsfi_verify_vaesen_sight_perception(&params);
            assert(ruling == TSFI_VAESEN_SIGHT_QUALIFIED);
        }
    }
    printf("   ✓ Formally Proven across 5 Trauma Origins x 22 Microtones: QUALIFIED (0)\n\n");

    printf("[STAGE 2] Testing Invalid Trauma Origin & Invisibility Overflow Fault Rejection...\n");
    {
        // 1. Invalid trauma origin (must be in [1..5] when sight is active)
        TsfiVaesenSightProofParams bad_origin = {
            .sight_active_flag = 1,
            .trauma_origin_code = 0, // Invalid origin
            .creature_invisibility_level = 3,
            .microtone_bin_k = 1,
            .accumulator_energy = 500,
            .cics_writer_id = 1000000,
            .simulate_fault = 0,
            .k_param = 3
        };
        assert(tsfi_verify_vaesen_sight_perception(&bad_origin) == TSFI_VAESEN_SIGHT_INVALID_ORIGIN);

        // 2. Invisibility level > 10 overflow
        TsfiVaesenSightProofParams bad_invis = {
            .sight_active_flag = 1,
            .trauma_origin_code = 2,
            .creature_invisibility_level = 15, // Overflow
            .microtone_bin_k = 1,
            .accumulator_energy = 500,
            .cics_writer_id = 1000000,
            .simulate_fault = 0,
            .k_param = 3
        };
        assert(tsfi_verify_vaesen_sight_perception(&bad_invis) == TSFI_VAESEN_SIGHT_INVALID_INVISIBILITY);
    }
    printf("   ✓ Invalid Origins & Invisibility Overflows Properly Quarantined\n\n");

    printf("[STAGE 3] Executing 100,000 Repeated Sub-Microsecond Proof Verifications (< 100 ns)...\n");
    const uint32_t ITERS = 100000;
    double start_ns = get_time_ns();
    for (uint32_t i = 0; i < ITERS; i++) {
        TsfiVaesenSightProofParams params = {
            .sight_active_flag = (i % 2),
            .trauma_origin_code = (i % 5) + 1,
            .creature_invisibility_level = (i % 11),
            .microtone_bin_k = (i % 22) + 1,
            .accumulator_energy = (i % 10000),
            .cics_writer_id = 1000000,
            .simulate_fault = 0,
            .k_param = 3
        };
        TsfiVaesenSightRuling r = tsfi_verify_vaesen_sight_perception(&params);
        (void)r;
    }
    double total_ns = get_time_ns() - start_ns;
    double avg_ns = total_ns / (double)ITERS;

    printf("   ✓ Total Executions: %u\n", ITERS);
    printf("   ✓ Average The Sight Proof Latency: %.2f ns (Guard Gate: < 100 ns)\n", avg_ns);
    assert(avg_ns < 100.0);

    printf("\n========================================================================================\n");
    printf("VAESEN THE SIGHT (TROLLSYNT) PERCEPTION SOLVER CERTIFIED (ALL 3 STAGES PASSED)\n");
    printf("========================================================================================\n");
    return 0;
}
