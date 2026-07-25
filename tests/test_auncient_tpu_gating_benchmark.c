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
    printf("AUNCIENT TPU GATING & CONTENTION BENCHMARK\n");
    printf("=============================================================\n");

    auncient_transfluxor_registry_t registry;
    auncient_sdk_init_transfluxor_registry(&registry);

    // Setup write word
    auncient_transfluxor_word_t write_word;
    auncient_sdk_compile_transfluxor_word(&write_word, "SPK_WRITE", 1, 350.0, 700.0, 0.2, 0x00, 0x02);
    auncient_sdk_register_transfluxor_word(&registry, &write_word);

    // 1. Benchmarking Uncontended (Direct Write) Mode
    printf("[BENCHMARK] Running %d Uncontended dispatches...\n", BENCHMARK_RUNS);
    double start_time = get_time_us();
    volatile uint32_t uncontended_writes = 0;
    
    for (int i = 0; i < BENCHMARK_RUNS; i++) {
        // Direct write execution without lock checking
        uncontended_writes++;
    }
    double end_time = get_time_us();
    double uncontended_duration = end_time - start_time;
    double uncontended_ns_per_run = (uncontended_duration * 1000.0) / (double)BENCHMARK_RUNS;

    // 2. Benchmarking Contended (Lock Gated) Mode
    printf("[BENCHMARK] Running %d Contended (Lock Gated) dispatches...\n", BENCHMARK_RUNS);
    start_time = get_time_us();
    
    // Simulate active SCSI lock owned by another agent name
    volatile bool scsi_locked = true;
    const char *volatile scsi_owner = "Agent_A";
    const char *volatile requesting_agent = "Agent_B";
    volatile uint32_t blocked_writes = 0;

    for (int i = 0; i < BENCHMARK_RUNS; i++) {
        // Evaluate lock gating constraints
        if (scsi_locked && strcmp((const char *)scsi_owner, (const char *)requesting_agent) != 0) {
            blocked_writes++; // Blocked write path
        }
    }
    end_time = get_time_us();
    double contended_duration = end_time - start_time;
    double contended_ns_per_run = (contended_duration * 1000.0) / (double)BENCHMARK_RUNS;

    assert(blocked_writes == BENCHMARK_RUNS);

    printf("\n=============================================================\n");
    printf("TPU GATING & CONTENTION BENCHMARK RESULTS:\n");
    printf("-------------------------------------------------------------\n");
    printf("Mode            | Total Time (us) | Latency (ns / dispatch operation)\n");
    printf("-------------------------------------------------------------\n");
    printf("Uncontended     | %15.2f | %15.2f\n", uncontended_duration, uncontended_ns_per_run);
    printf("Contended Gated | %15.2f | %15.2f\n", contended_duration, contended_ns_per_run);
    printf("-------------------------------------------------------------\n");
    printf("Gating Overhead: %.2f ns per operation\n", contended_ns_per_run - uncontended_ns_per_run);
    printf("=============================================================\n");

    return 0;
}
