#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>
#include <time.h>
#include <math.h>
#include "tsfi_fourier.h"
#include "tsfi_c_math.h"
#include "lau_memory.h"

// Define the old strided layout and gather-based reconstruction function for comparison
typedef struct {
    float data[TSFI_FOURIER_SAMPLES][TSFI_FOURIER_HARMONICS * 2];
} TSFiFourierBasisOld;

static void init_basis_old(TSFiFourierBasisOld *basis) {
    for (int k = 0; k < TSFI_FOURIER_SAMPLES; k++) {
        float t = (2.0f * 3.14159265f * k) / TSFI_FOURIER_SAMPLES;
        for (int n = 0; n < TSFI_FOURIER_HARMONICS; n++) {
            int harmonic = n + 1;
            basis->data[k][n*2 + 0] = cosf(harmonic * t);
            basis->data[k][n*2 + 1] = sinf(harmonic * t);
        }
    }
}

static void fourier_reconstruct_gather(float *output_voxels, 
                                      const TSFiFourierBasisOld *basis, 
                                      const TSFiFourierGlyph *glyph) 
{
    float acc_x[TSFI_FOURIER_SAMPLES] = {0};
    float acc_y[TSFI_FOURIER_SAMPLES] = {0};
    
    for (int n = 0; n < TSFI_FOURIER_HARMONICS; n++) {
        __m512 v_an = _mm512_set1_ps(glyph->coeffs[n][0]);
        __m512 v_bn = _mm512_set1_ps(glyph->coeffs[n][1]);
        __m512 v_cn = _mm512_set1_ps(glyph->coeffs[n][2]);
        __m512 v_dn = _mm512_set1_ps(glyph->coeffs[n][3]);

        for (int k = 0; k < TSFI_FOURIER_SAMPLES; k += 16) {
            __m512i v_idx = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
            v_idx = _mm512_mullo_epi32(v_idx, _mm512_set1_epi32(TSFI_FOURIER_HARMONICS * 2));
            
            __m512 v_cos = _mm512_i32gather_ps(v_idx, &basis->data[k][n*2+0], 4);
            __m512 v_sin = _mm512_i32gather_ps(v_idx, &basis->data[k][n*2+1], 4);

            __m512 v_ax = _mm512_loadu_ps(&acc_x[k]);
            __m512 v_ay = _mm512_loadu_ps(&acc_y[k]);

            v_ax = _mm512_fmadd_ps(v_an, v_cos, v_ax);
            v_ax = _mm512_fmadd_ps(v_bn, v_sin, v_ax);
            
            v_ay = _mm512_fmadd_ps(v_cn, v_cos, v_ay);
            v_ay = _mm512_fmadd_ps(v_dn, v_sin, v_ay);

            _mm512_storeu_ps(&acc_x[k], v_ax);
            _mm512_storeu_ps(&acc_y[k], v_ay);
        }
    }

    for(int i = 0; i < TSFI_FOURIER_SAMPLES; i++) {
        output_voxels[i*2+0] = acc_x[i];
        output_voxels[i*2+1] = acc_y[i];
    }
}

static inline uint64_t rdtsc() {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

int main() {
    printf("Starting Fourier Voxel Reconstruction Benchmark...\n");
    
    // Allocate glyph
    TSFiFourierGlyph *glyph = (TSFiFourierGlyph*)lau_malloc(sizeof(TSFiFourierGlyph));
    for (int n = 0; n < TSFI_FOURIER_HARMONICS; n++) {
        glyph->coeffs[n][0] = (float)n * 0.01f;
        glyph->coeffs[n][1] = (float)n * -0.005f;
        glyph->coeffs[n][2] = (float)n * 0.02f;
        glyph->coeffs[n][3] = (float)n * -0.015f;
    }

    // Benchmark Gather-based Version
    printf("[1] Initializing Gather-based Basis...\n");
    TSFiFourierBasisOld *basis_old = (TSFiFourierBasisOld*)lau_malloc(sizeof(TSFiFourierBasisOld));
    init_basis_old(basis_old);
    
    float output_old[TSFI_FOURIER_SAMPLES * 2];
    
    printf("[2] Benchmarking Gather-based reconstruction (1000 runs)...\n");
    uint64_t start = rdtsc();
    for (int i = 0; i < 1000; i++) {
        fourier_reconstruct_gather(output_old, basis_old, glyph);
        __asm__ volatile("" : : "r"(output_old));
    }
    uint64_t end = rdtsc();
    uint64_t cycles_gather = end - start;
    printf("    -> %lu cycles total (avg %lu per run)\n", cycles_gather, cycles_gather / 1000);

    // Benchmark Aligned Transposed version
    printf("[3] Initializing Aligned Transposed Basis...\n");
    TSFiFourierBasis *basis_new = (TSFiFourierBasis*)lau_malloc_wired(sizeof(TSFiFourierBasis));
    tsfi_fourier_init_basis(basis_new);
    
    float output_new[TSFI_FOURIER_SAMPLES * 2];
    
    printf("[4] Benchmarking Aligned Transposed reconstruction (1000 runs)...\n");
    start = rdtsc();
    for (int i = 0; i < 1000; i++) {
        tsfi_fourier_reconstruct_avx512(output_new, basis_new, glyph);
        __asm__ volatile("" : : "r"(output_new));
    }
    end = rdtsc();
    uint64_t cycles_new = end - start;
    printf("    -> %lu cycles total (avg %lu per run)\n", cycles_new, cycles_new / 1000);

    double speedup = (double)cycles_gather / (double)cycles_new;
    printf("=== Benchmarking Results ===\n");
    printf("  Gather-based Cycles:   %lu\n", cycles_gather / 1000);
    printf("  Aligned-load Cycles:   %lu\n", cycles_new / 1000);
    printf("  Reconstruction Speedup: %.2fx\n", speedup);
    
    // Quick verification
    float diff = 0;
    for (int i = 0; i < TSFI_FOURIER_SAMPLES * 2; i++) {
        diff += fabs(output_old[i] - output_new[i]);
    }
    printf("  Validation (Accumulated absolute diff): %f\n", diff);

    lau_free(glyph);
    lau_free(basis_old);
    lau_free(basis_new);
    return 0;
}
