#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "../tsfi2-deepseek/inc/tsfi_shadowmap.h"

// Basic benchmark measuring Cinefex Reyes shadowmap lookup and serialization performance
int main(void) {
    printf("=== AUNCIENT CINEFEX SHADOWMAP BENCHMARK ===\n");
    
    TSFiShadowMap sm;
    tsfi_shadowmap_init(&sm);
    
    // Set some mock depth values
    for (int y = 0; y < TSFI_SHADOW_SIZE; y++) {
        for (int x = 0; x < TSFI_SHADOW_SIZE; x++) {
            tsfi_shadowmap_set_depth(&sm, x, y, (double)(x + y));
        }
    }
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Perform 1,000,000 constant-time shadow lookup operations
    volatile bool shadow_check = false;
    for (int i = 0; i < 1000000; i++) {
        int x = i % TSFI_SHADOW_SIZE;
        int y = (i / TSFI_SHADOW_SIZE) % TSFI_SHADOW_SIZE;
        shadow_check = tsfi_shadowmap_is_in_shadow(&sm, x, y, (double)(x + y + 1.0));
    }
    (void)shadow_check;
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_sec = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    double lookups_per_sec = 1000000.0 / elapsed_sec;
    
    printf("   ✓ Shadow lookups: %f per second\n", lookups_per_sec);
    
    // Perform 100,000 shadow map serializations to 8-bit XPLSM format
    uint8_t dest_xpl[TSFI_SHADOW_SIZE * TSFI_SHADOW_SIZE];
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 100000; i++) {
        tsfi_shadowmap_pack_xplsm(&sm, dest_xpl);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    elapsed_sec = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    double packs_per_sec = 100000.0 / elapsed_sec;
    
    printf("   ✓ XPLSM pack serialization: %f per second\n", packs_per_sec);
    
    // Print JSON output for parsing by the manifest generator
    printf("JSON_METRICS: {\"cinefex_lookup_rate\": %f, \"cinefex_pack_rate\": %f}\n", lookups_per_sec, packs_per_sec);
    
    printf("=== AUNCIENT CINEFEX BENCHMARK COMPLETE ===\n");
    return 0;
}
