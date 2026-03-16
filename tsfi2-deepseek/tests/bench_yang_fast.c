#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "tsfi_c_math.h"

void tsfi_yang_synthesis_avx512(float *out, const float *in, size_t count);

static double get_ms(struct timespec start, struct timespec end) {
    return (double)(end.tv_sec - start.tv_sec) * 1000.0 + (double)(end.tv_nsec - start.tv_nsec) / 1000000.0;
}

int main() {
    printf("=== TSFi Benchmark: Fast AVX-512 Yang Synthesis ===\n");
    const size_t N = 100000;
    float *in = (float*)aligned_alloc(64, N * sizeof(float));
    float *out = (float*)aligned_alloc(64, N * sizeof(float));
    struct timespec t0, t1;

    for(size_t i=0; i<N; i++) in[i] = (float)i * 0.01f;

    // Hot-cache Pre-warm
    for(int i=0; i<1000; i++) tsfi_yang_synthesis_avx512(out, in, N);

    clock_gettime(CLOCK_MONOTONIC, &t0);
    for(int i=0; i<100000; i++) {
        tsfi_yang_synthesis_avx512(out, in, N);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);

    double total_ms = get_ms(t0, t1);
    printf("[BENCH] 10B Waveforms: %.4f ms\n", total_ms);
    printf("[BENCH] Throughput: %.2f M-Waveforms/sec\n", 10000.0 / (total_ms / 1000.0));

    free(in); free(out);
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
