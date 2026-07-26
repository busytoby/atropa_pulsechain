#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#include "../tsfi2-deepseek/inc/tsfi_shadowmap.h"

// Improved benchmark measuring Cinefex shadowmap lookups, OpenMP multi-threading, and quantization loss profiles
int main(void) {
    printf("=== IMPROVED AUNCIENT CINEFEX BENCHMARK ===\n");
    
    TSFiShadowMap sm;
    tsfi_shadowmap_init(&sm);
    
    // Set mock depth values
    for (int y = 0; y < TSFI_SHADOW_SIZE; y++) {
        for (int x = 0; x < TSFI_SHADOW_SIZE; x++) {
            tsfi_shadowmap_set_depth(&sm, x, y, (double)(x + y) * 1.5);
        }
    }
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Perform 2,000,000 parallel constant-time shadow lookup operations using OpenMP
    int num_threads = omp_get_max_threads();
    printf("   [OPENMP] Running parallel lookups across %d threads...\n", num_threads);
    
    #pragma omp parallel for
    for (int i = 0; i < 2000000; i++) {
        int x = i % TSFI_SHADOW_SIZE;
        int y = (i / TSFI_SHADOW_SIZE) % TSFI_SHADOW_SIZE;
        tsfi_shadowmap_is_in_shadow(&sm, x, y, (double)(x + y + 1.0));
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_sec = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    double lookups_per_sec = 2000000.0 / elapsed_sec;
    
    printf("   ✓ Parallel shadow lookups: %f per second\n", lookups_per_sec);
    
    // Estimate quantization precision loss
    uint8_t dest_xpl[TSFI_SHADOW_SIZE * TSFI_SHADOW_SIZE];
    tsfi_shadowmap_pack_xplsm(&sm, dest_xpl);
    
    double total_loss = 0.0;
    for (int y = 0; y < TSFI_SHADOW_SIZE; y++) {
        for (int x = 0; x < TSFI_SHADOW_SIZE; x++) {
            double original = sm.depth_grid[y][x];
            double quantized = (double)dest_xpl[y * TSFI_SHADOW_SIZE + x];
            total_loss += fabs(original - quantized);
        }
    }
    double avg_loss = total_loss / (TSFI_SHADOW_SIZE * TSFI_SHADOW_SIZE);
    printf("   ✓ Average 8-bit XPLSM quantization precision loss: %f depth units\n", avg_loss);
    
    // Perform 200,000 shadow map serializations
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 200000; i++) {
        tsfi_shadowmap_pack_xplsm(&sm, dest_xpl);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    elapsed_sec = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    double packs_per_sec = 200000.0 / elapsed_sec;
    
    printf("   ✓ XPLSM pack serialization: %f per second\n", packs_per_sec);
    
    // Print JSON output for parsing by the manifest generator
    printf("JSON_METRICS: {\"cinefex_lookup_rate\": %f, \"cinefex_pack_rate\": %f, \"quantization_loss\": %f}\n", 
           lookups_per_sec, packs_per_sec, avg_loss);
    
    printf("=== AUNCIENT CINEFEX BENCHMARK COMPLETE ===\n");
    return 0;
}
