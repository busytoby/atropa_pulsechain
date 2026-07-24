#define _POSIX_C_SOURCE 199309L
#include "auncient_sdk.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

static void benchmark_recovery_callback(sdk_cics_context_t *ctx, sdk_status_code_t error_code) {
    if (error_code == SDK_STATUS_ERR_SECURITY) {
        ctx->security_clearance = 3;
    }
}

static double get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1.0e9 + (double)ts.tv_nsec;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SDK DESIGN-BY-CONTRACT LATENCY BENCHMARKS\n");
    printf("=============================================================\n");
    fflush(stdout);

    sdk_coaxial_env_t env;
    bool ok = auncient_sdk_init_coaxial(&env);
    assert(ok == true);

    sdk_kermit_cache_t cache = { .cached_value = 100, .cached_ts = { 1, 1 }, .is_warm = true };
    sdk_cics_context_t ctx = {
        .env = &env,
        .cache = &cache,
        .quorum_type = SDK_QUORUM_MAJORITY,
        .writer_id = 99,
        .security_clearance = 3,
        .has_lock = true,
        .state = SDK_STATE_EXECUTING,
        .is_contract_checking = false,
        .last_blame = SDK_BLAME_NONE,
        .recovery_handler = NULL
    };

    const int iterations = 20000;
    bool approvals[SDK_NUM_NODES] = { true, true, true, false };

    // 1. Benchmark Cache Hit vs Cache Miss
    printf("[BENCHMARK] Running %d iterations of Cache Hits...\n", iterations);
    fflush(stdout);
    double start = get_time_ns();
    for (int i = 0; i < iterations; i++) {
        uint32_t val;
        auncient_sdk_alu_execute(&ctx, ALU_OP_READ_KERMIT, 0, approvals, &val);
    }
    double end = get_time_ns();
    double cache_hit_avg = (end - start) / iterations;
    printf("   ✓ Cache Hit Average Latency: %.2f ns/run\n", cache_hit_avg);
    fflush(stdout);

    printf("[BENCHMARK] Running %d iterations of Cache Misses...\n", iterations);
    fflush(stdout);
    start = get_time_ns();
    for (int i = 0; i < iterations; i++) {
        uint32_t val;
        ctx.cache->is_warm = false; // Force miss
        auncient_sdk_alu_execute(&ctx, ALU_OP_READ_KERMIT, 0, approvals, &val);
    }
    end = get_time_ns();
    double cache_miss_avg = (end - start) / iterations;
    printf("   ✓ Cache Miss Average Latency: %.2f ns/run\n", cache_miss_avg);
    fflush(stdout);

    // 2. Benchmark Behavioral Subtyping Verification
    printf("[BENCHMARK] Running %d iterations of Refinement & Subtyping...\n", iterations);
    fflush(stdout);
    sdk_cics_context_t parent = ctx;
    parent.quorum_type = SDK_QUORUM_WEIGHTED;
    parent.security_clearance = 3;

    sdk_cics_context_t child = ctx;
    child.quorum_type = SDK_QUORUM_WEIGHTED;
    child.security_clearance = 2; // Compliant weakened clearance

    start = get_time_ns();
    for (int i = 0; i < iterations; i++) {
        auncient_sdk_validate_contract_refinement(&parent, &child);
        auncient_sdk_validate_invariant_strengthening(&parent, &child);
    }
    end = get_time_ns();
    double subtyping_avg = (end - start) / iterations;
    printf("   ✓ Subtyping/Refinement Average Latency: %.2f ns/run\n", subtyping_avg);
    fflush(stdout);

    // 3. Benchmark Resilient Contract Recovery Handlers
    printf("[BENCHMARK] Running %d iterations of Recovery Handlers...\n", iterations);
    fflush(stdout);
    ctx.recovery_handler = benchmark_recovery_callback;

    start = get_time_ns();
    for (int i = 0; i < iterations; i++) {
        uint32_t val;
        ctx.security_clearance = 1; // Low clearance to trigger failure
        auncient_sdk_alu_execute(&ctx, ALU_OP_WRITE_ABD, 950000, approvals, &val);
    }
    end = get_time_ns();
    double recovery_avg = (end - start) / iterations;
    printf("   ✓ Resilient Recovery Average Latency: %.2f ns/run\n", recovery_avg);
    fflush(stdout);

    auncient_sdk_close_coaxial(&env);

    printf("=============================================================\n");
    printf("BENCHMARKS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);

    return 0;
}
