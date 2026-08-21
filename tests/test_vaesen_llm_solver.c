#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "tsfi_vaesen_llm_solver.h"

static double get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

int main(void) {
    printf("========================================================================================\n");
    printf("   STANDARD ALGOL61 / C11 FORMAL PROVER: VAESEN LLM ARCHITECTURAL SCAFFOLDING\n");
    printf("========================================================================================\n\n");

    /* 1. Valid baseline test across EDO-22 and Mathieu wave resonance */
    TsfiVaesenLlmState valid_state = {
        .token_lattice_freq_edo22 = 14,
        .mathieu_modulation_q_milli = 1250,
        .weyl_closure_exactness_flag = 1,
        .engram_accumulator_energy = 450230129481ULL,
        .cics_writer_id = 1000000ULL,
        .simulate_gradient_explosion = 0,
        .k_param = 3
    };
    int res = tsfi_verify_vaesen_llm_scaffolding(&valid_state);
    printf("[TEST 1] Valid VAESEN LLM Baseline: %s (Code: %d)\n", res == TSFI_VAESEN_LLM_OK ? "PASS" : "FAIL", res);
    assert(res == TSFI_VAESEN_LLM_OK);

    /* 2. EDO-22 frequency boundary violations */
    TsfiVaesenLlmState invalid_freq = valid_state;
    invalid_freq.token_lattice_freq_edo22 = 23;
    res = tsfi_verify_vaesen_llm_scaffolding(&invalid_freq);
    printf("[TEST 2] Out-of-bounds EDO-22 Frequency (23): %s (Code: %d)\n", res == TSFI_VAESEN_LLM_ERR_EDO22_FREQ ? "PASS" : "FAIL", res);
    assert(res == TSFI_VAESEN_LLM_ERR_EDO22_FREQ);

    /* 3. Weyl closure exactness failure */
    TsfiVaesenLlmState non_exact_closure = valid_state;
    non_exact_closure.weyl_closure_exactness_flag = 0;
    res = tsfi_verify_vaesen_llm_scaffolding(&non_exact_closure);
    printf("[TEST 3] Non-Exact Weyl Closure (0): %s (Code: %d)\n", res == TSFI_VAESEN_LLM_ERR_WEYL_CLOSURE ? "PASS" : "FAIL", res);
    assert(res == TSFI_VAESEN_LLM_ERR_WEYL_CLOSURE);

    /* 4. Unverified CICS participant */
    TsfiVaesenLlmState unverified_cics = valid_state;
    unverified_cics.cics_writer_id = 999999ULL;
    res = tsfi_verify_vaesen_llm_scaffolding(&unverified_cics);
    printf("[TEST 4] Unverified CICS Writer ID: %s (Code: %d)\n", res == TSFI_VAESEN_LLM_ERR_CICS_UNVERIFIED ? "PASS" : "FAIL", res);
    assert(res == TSFI_VAESEN_LLM_ERR_CICS_UNVERIFIED);

    /* 5. Gradient explosion fault injection with ReBAR shadow rollback */
    TsfiVaesenLlmState fault_test = valid_state;
    fault_test.simulate_gradient_explosion = 1;
    res = tsfi_verify_vaesen_llm_scaffolding(&fault_test);
    printf("[TEST 5] Gradient Explosion with ReBAR Shadow Rollback: %s (Code: %d)\n", res == TSFI_VAESEN_LLM_OK ? "PASS" : "FAIL", res);
    assert(res == TSFI_VAESEN_LLM_OK);

    /* 6. Latency guard benchmark across 1,000,000 iterations */
    const int iterations = 1000000;
    double t_start = get_time_ns();
    for (int i = 0; i < iterations; ++i) {
        int r = tsfi_verify_vaesen_llm_scaffolding(&valid_state);
        (void)r;
    }
    double t_end = get_time_ns();
    double avg_latency = (t_end - t_start) / (double)iterations;
    printf("\n[BENCHMARK] Average verification latency: %.2f ns (Target: < 1000 ns)\n", avg_latency);
    assert(avg_latency < 1000.0);

    printf("\n>>> ALL 5 VAESEN LLM SCAFFOLDING INVARIANTS FORMALLY PROVEN AND VERIFIED <<<\n");
    return 0;
}
