#include <stdio.h>
#include <x86intrin.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <immintrin.h>
#include <string.h>
#include "lau_memory.h"
#include "tsfi_font_ai.h"
#include "tsfi_font_vectors.h"

static void target_func(void *ctx, float val) {
    volatile float v = val + 1.0f;
    (void)v; (void)ctx;
}

static void target_func_wave(void *ctx, float *data, uint64_t *mask, size_t count) {
    __m512 v1 = _mm512_set1_ps(1.0f);
    __m512 vsum0 = _mm512_setzero_ps();
    __m512 vsum1 = _mm512_setzero_ps();
    
    size_t vec_count = count >> 5; 
    
    for (size_t i = 0; i < vec_count; i++) {
        vsum0 = _mm512_add_ps(vsum0, v1);
        vsum1 = _mm512_add_ps(vsum1, v1);
    }
    
    vsum0 = _mm512_add_ps(vsum0, vsum1);
    float sum = _mm512_reduce_add_ps(vsum0);
    volatile float sink = sum;
    (void)sink; (void)ctx; (void)data; (void)mask;
}

static inline uint64_t get_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

#define ITERATIONS 1000000
#define BATCH_SIZE 16384

int main() {
    printf("=== TSFi Neuro Dispatch Benchmark ===\n");
    
    TSFiFontSystem *sys = (TSFiFontSystem*)lau_malloc(sizeof(TSFiFontSystem));
    memset(sys, 0, sizeof(TSFiFontSystem));
    
    tsfi_font_ai_init(sys);
    tsfi_font_ai_bind_evolve(sys, (void*)target_func, NULL);
    tsfi_font_ai_bind_evolve_sparse_wave(sys, (void*)target_func_wave, NULL);
    
    uint64_t start = get_ns();
    uint64_t start_tsc = __rdtsc();
    for(int i=0; i<ITERATIONS; i++) {
        tsfi_font_ai_invoke_evolve(sys, 1.0f);
    }
    uint64_t end = get_ns();
    uint64_t end_tsc = __rdtsc();
    double dur_sec = (double)(end - start) / 1e9;
    printf("Scalar Dispatch: %.2f M/s\n", (ITERATIONS / dur_sec) / 1e6);
    
    float *wave_data = (float*)lau_memalign(64, BATCH_SIZE * sizeof(float));
    for(int k=0; k<BATCH_SIZE; k++) wave_data[k] = 1.0f;

    start = get_ns();
    int BATCH_ITERS = 10000; 
    for(int i=0; i<BATCH_ITERS; i++) {
        tsfi_font_ai_invoke_evolve_sparse_wave(sys, wave_data, NULL, BATCH_SIZE);
    }
    end = get_ns();
    
    double wave_dur = (double)(end - start) / 1e9;
    double total_items = (double)BATCH_ITERS * BATCH_SIZE;
    printf("Wave Dispatch:   %.2f Billion Items/s\n", (total_items / wave_dur) / 1e9);
    
    tsfi_font_ai_destroy(sys);
    lau_free(sys);
    lau_free(wave_data);
    double cycles_per_op = (double)(end_tsc - start_tsc) / ITERATIONS;
    printf("Cycles/Op   : %.2f\n", cycles_per_op);

        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}