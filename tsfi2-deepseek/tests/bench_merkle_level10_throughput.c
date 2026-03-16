#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tsfi_merkle.h"
#include "tsfi_helmholtz.h"
#include "tsfi_io.h"
#include "lau_memory.h"

static double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

int main() {
    printf("=== TSFi Level 10 Helmholtz Reduction Throughput Benchmark ===\n");
    tsfi_helmholtz_init();
    uint64_t resonance_k = 2026;

    // Allocate 256KB Manifold (1024 leaves * 256 bytes)
    void *manifold = lau_malloc(256 * 1024);
    memset(manifold, 0, 256 * 1024);

    uint8_t root[32];
    uint8_t receipt[32];
    
    int iterations = 1000;
    printf("[ACT] Running %d Level 10 reductions in a loop...\n", iterations);
    
    double start = get_time_ms();
    for (int i = 0; i < iterations; i++) {
        tsfi_helmholtz_reduce_10(root, receipt, NULL, NULL, manifold, i, resonance_k, NULL);
    }
    double end = get_time_ms();
    
    double total_ms = end - start;
    double avg_ms = total_ms / iterations;
    double reductions_per_sec = 1000.0 / avg_ms;
    
    printf("\n=== BENCHMARK RESULTS ===\n");
    printf("Total Time:     %.2f ms\n", total_ms);
    printf("Average Time:   %.4f ms per reduction\n", avg_ms);
    printf("Throughput:     %.2f reductions/sec\n", reductions_per_sec);
    printf("State Density:  %.2f MiB/sec reduced\n", (reductions_per_sec * 256.0 * 1024.0) / (1024.0 * 1024.0));

    lau_free(manifold);
    tsfi_helmholtz_cleanup();
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
