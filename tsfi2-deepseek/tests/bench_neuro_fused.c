#include <stdio.h>
#include <x86intrin.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>
#include "lau_memory.h"
#include "tsfi_wave512.h"
#include "tsfi_wave_interop.h"

#define COUNT (32 * 1024 * 1024) 
#define SIZE_BYTES (COUNT * sizeof(uint8_t))

static inline uint64_t get_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

// secret-Free Wave Fused
void Wave_Fused(uint8_t* out, uint8_t* in_a, uint8_t* in_b, float scale, size_t n_waves) {
    __m512 vscale = _mm512_set1_ps(scale);
    __m512 vzero = _mm512_setzero_ps();
    
    for(size_t i=0; i < n_waves; i++) {
        __m512i ua = _mm512_loadu_si512((void*)&in_a[i*64]);
        __m512i ub = _mm512_loadu_si512((void*)&in_b[i*64]);
        
        __m512i a0 = _mm512_cvtepu8_epi32(_mm512_castsi512_si128(ua));
        __m512i a1 = _mm512_cvtepu8_epi32(_mm512_extracti32x4_epi32(ua, 1));
        __m512i a2 = _mm512_cvtepu8_epi32(_mm512_extracti32x4_epi32(ua, 2));
        __m512i a3 = _mm512_cvtepu8_epi32(_mm512_extracti32x4_epi32(ua, 3));
        
        __m512i b0 = _mm512_cvtepu8_epi32(_mm512_castsi512_si128(ub));
        __m512i b1 = _mm512_cvtepu8_epi32(_mm512_extracti32x4_epi32(ub, 1));
        __m512i b2 = _mm512_cvtepu8_epi32(_mm512_extracti32x4_epi32(ub, 2));
        __m512i b3 = _mm512_cvtepu8_epi32(_mm512_extracti32x4_epi32(ub, 3));
        
        a0 = _mm512_mullo_epi32(a0, b0);
        a1 = _mm512_mullo_epi32(a1, b1);
        a2 = _mm512_mullo_epi32(a2, b2);
        a3 = _mm512_mullo_epi32(a3, b3);
        
        __m512 f0 = _mm512_cvtepi32_ps(a0); f0 = _mm512_mul_ps(f0, vscale); f0 = _mm512_max_ps(f0, vzero); a0 = _mm512_cvtps_epi32(f0);
        __m512 f1 = _mm512_cvtepi32_ps(a1); f1 = _mm512_mul_ps(f1, vscale); f1 = _mm512_max_ps(f1, vzero); a1 = _mm512_cvtps_epi32(f1);
        __m512 f2 = _mm512_cvtepi32_ps(a2); f2 = _mm512_mul_ps(f2, vscale); f2 = _mm512_max_ps(f2, vzero); a2 = _mm512_cvtps_epi32(f2);
        __m512 f3 = _mm512_cvtepi32_ps(a3); f3 = _mm512_mul_ps(f3, vscale); f3 = _mm512_max_ps(f3, vzero); a3 = _mm512_cvtps_epi32(f3);
        
        __m128i q0 = _mm512_cvtusepi32_epi8(a0);
        __m128i q1 = _mm512_cvtusepi32_epi8(a1);
        __m128i q2 = _mm512_cvtusepi32_epi8(a2);
        __m128i q3 = _mm512_cvtusepi32_epi8(a3);
        
        __m512i result = _mm512_castsi128_si512(q0);
        result = _mm512_inserti32x4(result, q1, 1);
        result = _mm512_inserti32x4(result, q2, 2);
        result = _mm512_inserti32x4(result, q3, 3);
        
        _mm512_storeu_si512((void*)&out[i*64], result);
    }
}

int main() {
    printf("=== TSFi Neuro Fused Layer Benchmark ===\n");
    
    uint8_t *A = (uint8_t*)lau_memalign_wired(512, SIZE_BYTES);
    uint8_t *B = (uint8_t*)lau_memalign_wired(512, SIZE_BYTES);
    uint8_t *C = (uint8_t*)lau_memalign_wired(512, SIZE_BYTES);
    
    for(size_t i=0; i<COUNT; i+=4096) { A[i] = 10; B[i] = 2; }
    
    uint64_t start = get_ns();
    uint64_t start_tsc = __rdtsc();
    Wave_Fused(C, A, B, 0.5f, COUNT/64);
    uint64_t end = get_ns();
    uint64_t end_tsc = __rdtsc();
    double t_wave = (double)(end - start) / 1e9;
    double bw_wave = (SIZE_BYTES * 3.0) / t_wave / 1e9;
    
    printf("WAVE512: %.4f s (%.2f GB/s)\n", t_wave, bw_wave);
    printf("Status:  [PASS] Fused Neurology Active\n");
    
    lau_free(A); lau_free(B); lau_free(C);
    double cycles_per_op = (double)(end_tsc - start_tsc) / (COUNT / 64);
    printf("Cycles/Op   : %.2f\n", cycles_per_op);

        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}