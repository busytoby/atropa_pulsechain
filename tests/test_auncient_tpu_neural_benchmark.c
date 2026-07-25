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
#define NUM_INPUTS 4

// Helper to get time in microseconds
double get_time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000000.0 + (double)ts.tv_nsec / 1000.0;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TPU NEURAL LAYER BENCHMARK\n");
    printf("=============================================================\n");

    auncient_transfluxor_registry_t registry;
    auncient_sdk_init_transfluxor_registry(&registry);

    // Setup input node waves
    auncient_transfluxor_word_t inputs[NUM_INPUTS];
    for (int i = 0; i < NUM_INPUTS; i++) {
        auncient_sdk_compile_transfluxor_word(&inputs[i], "IN_NODE", i, 150.0, 300.0, 0.4, 0x00, 0x00);
    }

    // 1. Benchmarking CPU-Iterative Summation & Evaluation
    printf("[BENCHMARK] Running %d CPU-Iterative neural gate simulations...\n", BENCHMARK_RUNS);
    double start_time = get_time_us();
    
    for (int i = 0; i < BENCHMARK_RUNS; i++) {
        volatile double sum_f1 = 0.0;
        volatile double sum_f2 = 0.0;
        for (int j = 0; j < NUM_INPUTS; j++) {
            sum_f1 = sum_f1 + inputs[j].f1;
            sum_f2 = sum_f2 + inputs[j].f2;
        }
        // Apply activation logic on CPU
        volatile double act_f1 = (sum_f1 > 500.0) ? 900.0 : 100.0;
        volatile double act_f2 = (sum_f2 > 1000.0) ? 1800.0 : 200.0;
        assert(act_f1 == 900.0 && act_f2 == 1800.0);
    }

    double end_time = get_time_us();
    double cpu_duration = end_time - start_time;
    double cpu_ns_per_run = (cpu_duration * 1000.0) / (double)BENCHMARK_RUNS;

    // 2. Benchmarking TPU Analog Layer Gating API
    printf("[BENCHMARK] Running %d TPU Analog Layer dispatches...\n", BENCHMARK_RUNS);
    auncient_transfluxor_word_t output;
    start_time = get_time_us();
    
    for (int i = 0; i < BENCHMARK_RUNS; i++) {
        bool ok = auncient_sdk_tpu_execute_layer(&registry, inputs, NUM_INPUTS, &output);
        assert(ok);
    }
    end_time = get_time_us();
    double tpu_duration = end_time - start_time;
    double tpu_ns_per_run = (tpu_duration * 1000.0) / (double)BENCHMARK_RUNS;

    // Output performance metrics
    double speedup = cpu_duration / tpu_duration;
    
    printf("\n=============================================================\n");
    printf("TPU NEURAL BENCHMARK RESULTS (4-Input Layer):\n");
    printf("-------------------------------------------------------------\n");
    printf("Mode            | Total Time (us) | Latency (ns / layer operation)\n");
    printf("-------------------------------------------------------------\n");
    printf("CPU-Iterative   | %15.2f | %15.2f\n", cpu_duration, cpu_ns_per_run);
    printf("TPU Analog Layer| %15.2f | %15.2f\n", tpu_duration, tpu_ns_per_run);
    printf("-------------------------------------------------------------\n");
    printf("TPU Speedup Factor: %.2fx faster\n", speedup);
    printf("=============================================================\n");

    return 0;
}
