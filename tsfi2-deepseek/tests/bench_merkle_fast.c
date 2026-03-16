#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "tsfi_helmholtz.h"

// Prototype for the new fast kernel
void tsfi_helmholtz_reduce_8way_avx512(uint8_t *out, const uint8_t *leaves_bank);

static double get_ms(struct timespec start, struct timespec end) {
    return (double)(end.tv_sec - start.tv_sec) * 1000.0 + (double)(end.tv_nsec - start.tv_nsec) / 1000000.0;
}

int main() {
    printf("=== TSFi Benchmark: Fast 8-Way Helmholtz Reduction ===\n");
    
    uint8_t *leaves_bank = (uint8_t*)aligned_alloc(64, 8 * 256);
    uint8_t out[32];
    struct timespec t0, t1;

    // Hot-cache Pre-warm
    for(int i=0; i<1000; i++) tsfi_helmholtz_reduce_8way_avx512(out, leaves_bank);

    clock_gettime(CLOCK_MONOTONIC, &t0);
    for(int i=0; i<100000; i++) {
        tsfi_helmholtz_reduce_8way_avx512(out, leaves_bank);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);

    double total_ms = get_ms(t0, t1);
    printf("[BENCH] 100k Reductions: %.4f ms\n", total_ms);
    printf("[BENCH] Throughput: %.2f M-Reductions/sec\n", 100.0 / total_ms);

    free(leaves_bank);
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
