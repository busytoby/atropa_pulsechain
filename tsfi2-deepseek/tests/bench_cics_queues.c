#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define NUM_RUNS 2000

extern bool tsfi_xplos_shell_cbt_cics(const char *cmd);

int main(void) {
    printf("=== RUNNING CICS QUEUES PERFORMANCE BENCHMARK ===\n");

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < NUM_RUNS; i++) {
        // Write record to TSQ
        tsfi_xplos_shell_cbt_cics("cbtcicsts write Q1 TESTVAL");
        // Read record from TSQ
        tsfi_xplos_shell_cbt_cics("cbtcicsts read Q1");
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double rate = (NUM_RUNS * 2.0) / elapsed; // 2 operations per loop iteration
    double latency = (elapsed / (NUM_RUNS * 2.0)) * 1e6; // in microseconds

    printf("CICS Queue Benchmark Metrics:\n");
    printf("  Throughput: %f ops/sec\n", rate);
    printf("  Commit/Access Latency: %f us\n", latency);

    return 0;
}
