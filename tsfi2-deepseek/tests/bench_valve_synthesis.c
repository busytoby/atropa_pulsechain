#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "tsfi_valve.h"
#include "lau_memory.h"


#define BENCH_SAMPLES 160000 // 160k samples (multiple of 16 for AVX-512)


int main() {
    printf("=== TSFi2 Thermionic Valve Performance Benchmark ===\n");
    printf("[BENCHMARK] Warm-up and allocating %d samples (approx %.2f MB)...\n", 
           BENCH_SAMPLES, (double)(BENCH_SAMPLES * sizeof(float)) / (1024.0 * 1024.0));

    float *input = (float*)lau_memalign(64, BENCH_SAMPLES * sizeof(float));
    float *output_avx = (float*)lau_memalign(64, BENCH_SAMPLES * sizeof(float));
    float *output_seq = (float*)lau_memalign(64, BENCH_SAMPLES * sizeof(float));

    for (int i = 0; i < BENCH_SAMPLES; i++) {
        input[i] = 0.5f * sinf((float)i * 0.01f);
    }

    TsfiValveTriode valve;
    tsfi_valve_init(&valve, 100.0, 0.00002, 250.0, -1.5);

    // 1. Benchmark AVX-512 Process
    printf("[BENCHMARK] Running AVX-512 Vectorized Triode Processing...\n");
    struct timespec start_avx, end_avx;
    clock_gettime(CLOCK_MONOTONIC, &start_avx);

    tsfi_valve_process_avx512(&valve, input, output_avx, BENCH_SAMPLES, 0.2, 1.0);

    clock_gettime(CLOCK_MONOTONIC, &end_avx);
    double time_avx = (end_avx.tv_sec - start_avx.tv_sec) + 
                      (end_avx.tv_nsec - start_avx.tv_nsec) * 1e-9;

    printf("  AVX-512 Time: %.6f seconds\n", time_avx);
    printf("  AVX-512 Throughput: %.2f MSamples/sec\n", (BENCH_SAMPLES / 1e6) / time_avx);

    // 2. Benchmark Sequential Regenerative Process (with beta = 0) to compare algorithmic baseline
    printf("[BENCHMARK] Running Sequential Regenerative Processing (beta = 0.0)...\n");
    struct timespec start_seq, end_seq;
    clock_gettime(CLOCK_MONOTONIC, &start_seq);

    tsfi_valve_process_regenerative(&valve, input, output_seq, BENCH_SAMPLES, 0.2, 1.0, 0.0);

    clock_gettime(CLOCK_MONOTONIC, &end_seq);
    double time_seq = (end_seq.tv_sec - start_seq.tv_sec) + 
                      (end_seq.tv_nsec - start_seq.tv_nsec) * 1e-9;

    printf("  Sequential Time: %.6f seconds\n", time_seq);
    printf("  Sequential Throughput: %.2f MSamples/sec\n", (BENCH_SAMPLES / 1e6) / time_seq);

    // Speedup
    printf("[SUMMARY] AVX-512 Speedup: %.2fx faster than sequential\n", time_seq / time_avx);

    // Cleanup
    lau_free(input);
    lau_free(output_avx);
    lau_free(output_seq);

    printf("[SUCCESS] Valve performance benchmarking complete!\n");
    return 0;
}
