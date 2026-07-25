#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "auncient_sdk.h"

#define BENCHMARK_RUNS 50000

// Helper to get time in microseconds
double get_time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000000.0 + (double)ts.tv_nsec / 1000.0;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TPU RECURRENT STATE BENCHMARK\n");
    printf("=============================================================\n");

    auncient_transfluxor_registry_t registry;
    auncient_sdk_init_transfluxor_registry(&registry);

    // Setup input and recurrent states
    auncient_transfluxor_word_t input;
    auncient_sdk_compile_transfluxor_word(&input, "INPUT_VAL", 10, 400.0, 800.0, 0.4, 0x00, 0x00);

    auncient_transfluxor_word_t state;
    auncient_sdk_compile_transfluxor_word(&state, "STATE_VAL", 888, 100.0, 200.0, 0.2, 0x00, 0x00);

    // 1. Benchmarking CPU-Iterative Recurrent state updates
    printf("[BENCHMARK] Running %d CPU-Iterative recurrent updates...\n", BENCHMARK_RUNS);
    double start_time = get_time_us();
    
    volatile double cpu_f1 = state.f1;
    volatile double cpu_f2 = state.f2;
    volatile double cpu_decay = state.decay;

    for (int i = 0; i < BENCHMARK_RUNS; i++) {
        cpu_f1 = (input.f1 * 0.7) + (cpu_f1 * 0.3);
        cpu_f2 = (input.f2 * 0.7) + (cpu_f2 * 0.3);
        cpu_decay = (input.decay * 0.5) + (cpu_decay * 0.5);
    }
    double end_time = get_time_us();
    double cpu_duration = end_time - start_time;
    double cpu_ns_per_run = (cpu_duration * 1000.0) / (double)BENCHMARK_RUNS;

    // 2. Benchmarking TPU Recurrent Gating step API
    printf("[BENCHMARK] Running %d TPU Recurrent Gating step dispatches...\n", BENCHMARK_RUNS);
    start_time = get_time_us();
    
    for (int i = 0; i < BENCHMARK_RUNS; i++) {
        bool ok = auncient_sdk_tpu_execute_recurrent_step(&registry, &input, &state);
        assert(ok);
    }
    end_time = get_time_us();
    double tpu_duration = end_time - start_time;
    double tpu_ns_per_run = (tpu_duration * 1000.0) / (double)BENCHMARK_RUNS;

    // Output performance metrics
    double speedup = cpu_duration / tpu_duration;
    
    printf("\n=============================================================\n");
    printf("TPU RECURRENT BENCHMARK RESULTS:\n");
    printf("-------------------------------------------------------------\n");
    printf("Mode            | Total Time (us) | Latency (ns / recurrent step)\n");
    printf("-------------------------------------------------------------\n");
    printf("CPU-Iterative   | %15.2f | %15.2f\n", cpu_duration, cpu_ns_per_run);
    printf("TPU RTPU Gating | %15.2f | %15.2f\n", tpu_duration, tpu_ns_per_run);
    printf("-------------------------------------------------------------\n");
    printf("TPU Speedup Factor: %.2fx faster\n", speedup);
    printf("=============================================================\n");

    return 0;
}
