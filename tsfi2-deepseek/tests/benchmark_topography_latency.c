#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "tsfi_merkle.h"
#include "tsfi_helmholtz.h"

static double get_ms(struct timespec start, struct timespec end) {
    return (double)(end.tv_sec - start.tv_sec) * 1000.0 + (double)(end.tv_nsec - start.tv_nsec) / 1000000.0;
}

int main() {
    printf("=== TSFi Topographical Latency Benchmark [L11 vs L12] ===\n");
    tsfi_helmholtz_init();
    void *manifold = calloc(1, 2 * 1024 * 1024);
    uint8_t s[32], r[32], h[32];
    struct timespec t0, t1;

    for(int i=0; i<5; i++) tsfi_helmholtz_reduce_11(s, r, NULL, NULL, manifold, 0, 2026, NULL);

    clock_gettime(CLOCK_MONOTONIC, &t0);
    tsfi_helmholtz_reduce_11(s, r, NULL, NULL, manifold, 1, 2026, NULL);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("[L11] 2048-Leaf Reduction: %.4f ms\n", get_ms(t0, t1));

    clock_gettime(CLOCK_MONOTONIC, &t0);
    tsfi_helmholtz_reduce_12(s, r, h, NULL, NULL, manifold, 1, 2026, NULL);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("[L12] 4096-Leaf Reduction: %.4f ms\n", get_ms(t0, t1));

    free(manifold);
    tsfi_helmholtz_cleanup();
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
