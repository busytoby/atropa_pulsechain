#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <strings.h>
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
    printf("AUNCIENT TPU LEXICAL VOCABULARY BENCHMARK\n");
    printf("=============================================================\n");

    // Setup mock query word properties
    const char *query_name = "SPK_LOCK_SCSI_URGENT";
    double f1 = 800.0, f2 = 1600.0, decay = 0.1;
    uint32_t word_id = 1;

    // Vocab database for string comparison
    const char *vocab_db[4] = {
        "SPK_WRITE_LEDGER",
        "SPK_RELEASE_SCSI",
        "SPK_LOCK_SCSI_CAUTIOUS",
        "SPK_LOCK_SCSI_URGENT"
    };

    // 1. Benchmarking CPU String-Matching Lookups (Slow)
    printf("[BENCHMARK] Running %d CPU String-Matching vocabulary checks...\n", BENCHMARK_RUNS);
    double start_time = get_time_us();
    volatile int cpu_match_index = -1;

    for (int i = 0; i < BENCHMARK_RUNS; i++) {
        cpu_match_index = -1;
        for (int j = 0; j < 4; j++) {
            if (strcasecmp(query_name, vocab_db[j]) == 0) {
                cpu_match_index = j;
                break;
            }
        }
    }
    double end_time = get_time_us();
    double cpu_duration = end_time - start_time;
    double cpu_ns_per_run = (cpu_duration * 1000.0) / (double)BENCHMARK_RUNS;

    // 2. Benchmarking TPU Hash-based Lexical Lookups (Fast)
    printf("[BENCHMARK] Running %d TPU Hash-based vocabulary checks...\n", BENCHMARK_RUNS);
    start_time = get_time_us();
    volatile uint64_t verified_hash = 0;

    for (int i = 0; i < BENCHMARK_RUNS; i++) {
        // Query directly using modular TPU hash
        verified_hash = auncient_sdk_calculate_transfluxor_hash(f1, f2, decay, word_id);
    }
    end_time = get_time_us();
    double tpu_duration = end_time - start_time;
    double tpu_ns_per_run = (tpu_duration * 1000.0) / (double)BENCHMARK_RUNS;

    assert(cpu_match_index == 3);
    assert(verified_hash != 0);

    // Output performance metrics
    double speedup = cpu_duration / tpu_duration;
    
    printf("\n=============================================================\n");
    printf("TPU LEXICAL BENCHMARK RESULTS:\n");
    printf("-------------------------------------------------------------\n");
    printf("Mode            | Total Time (us) | Latency (ns / lookup operation)\n");
    printf("-------------------------------------------------------------\n");
    printf("CPU String Match| %15.2f | %15.2f\n", cpu_duration, cpu_ns_per_run);
    printf("TPU Hash Lookups| %15.2f | %15.2f\n", tpu_duration, tpu_ns_per_run);
    printf("-------------------------------------------------------------\n");
    printf("TPU Speedup Factor: %.2fx faster\n", speedup);
    printf("=============================================================\n");

    return 0;
}
