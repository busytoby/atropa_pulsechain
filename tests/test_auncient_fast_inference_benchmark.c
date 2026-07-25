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
    printf("AUNCIENT TRANSFLUXOR FAST INFERENCE BENCHMARK\n");
    printf("=============================================================\n");

    auncient_transfluxor_registry_t registry;
    auncient_sdk_init_transfluxor_registry(&registry);

    // Setup base words for iterative run
    auncient_transfluxor_word_t lock_word;
    auncient_sdk_compile_transfluxor_word(&lock_word, "SPK_LOCK", 1, 440.0, 880.0, 0.4, 0x10, 0x00);
    
    auncient_transfluxor_word_t write_word;
    auncient_sdk_compile_transfluxor_word(&write_word, "SPK_WRITE", 2, 350.0, 700.0, 0.2, 0x00, 0x20);

    auncient_sdk_register_transfluxor_word(&registry, &lock_word);
    auncient_sdk_register_transfluxor_word(&registry, &write_word);

    // Setup composite word for compacted run
    auncient_transfluxor_word_t compound_word;
    auncient_sdk_compile_transfluxor_word(&compound_word, "SPK_COMP", 3, 500.0, 1000.0, 0.3, 0x10, 0x20);
    auncient_sdk_register_transfluxor_word(&registry, &compound_word);

    // 1. Benchmarking Iterative (Slow) Mode
    printf("[BENCHMARK] Running %d iterations in Iterative (2 dispatches/op) Mode...\n", BENCHMARK_RUNS);
    double start_time = get_time_us();
    double feedback = 0.0;
    
    for (int i = 0; i < BENCHMARK_RUNS; i++) {
        // Dispatch lock and write sequentially
        bool ok1 = auncient_sdk_dispatch_transfluxor_word(&registry, &lock_word, &feedback);
        bool ok2 = auncient_sdk_dispatch_transfluxor_word(&registry, &write_word, &feedback);
        assert(ok1 && ok2);
    }
    double end_time = get_time_us();
    double iterative_duration = end_time - start_time;
    double iterative_ns_per_op = (iterative_duration * 1000.0) / (double)BENCHMARK_RUNS;

    // 2. Benchmarking Compacted (Fast) Mode
    printf("[BENCHMARK] Running %d iterations in Compacted (1 compound dispatch/op) Mode...\n", BENCHMARK_RUNS);
    start_time = get_time_us();
    
    for (int i = 0; i < BENCHMARK_RUNS; i++) {
        // Dispatch compound word once
        bool ok = auncient_sdk_dispatch_transfluxor_word(&registry, &compound_word, &feedback);
        assert(ok);
    }
    end_time = get_time_us();
    double compacted_duration = end_time - start_time;
    double compacted_ns_per_op = (compacted_duration * 1000.0) / (double)BENCHMARK_RUNS;

    // Output performance metrics
    double speedup = iterative_duration / compacted_duration;
    
    printf("\n=============================================================\n");
    printf("PERFORMANCE BENCHMARK RESULTS:\n");
    printf("-------------------------------------------------------------\n");
    printf("Mode            | Total Time (us) | Latency (ns / dispatch operation)\n");
    printf("-------------------------------------------------------------\n");
    printf("Iterative (Slow)| %15.2f | %15.2f\n", iterative_duration, iterative_ns_per_op);
    printf("Compacted (Fast)| %15.2f | %15.2f\n", compacted_duration, compacted_ns_per_op);
    printf("-------------------------------------------------------------\n");
    printf("Auncient Speedup Factor: %.2fx faster\n", speedup);
    printf("=============================================================\n");

    return 0;
}
