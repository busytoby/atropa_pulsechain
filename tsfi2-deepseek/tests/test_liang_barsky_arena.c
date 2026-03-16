#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <immintrin.h>
#include "tsfi_liang_barsky.h"

void test_alligator_static_survival() {
    printf("[ALLIGATOR TEST] Static Survival Verification...\n");
    
    TsfiClippingVolume vol = {
        .x_min = -1.0f, .x_max = 1.0f,
        .y_min = -1.0f, .y_max = 1.0f,
        .z_min = -1.0f, .z_max = 1.0f,
        .w_min = -1.0f, .w_max = 1.0f
    };

    // Vector inside the box (16 floats)
    float inside[16];
    for(int i=0; i<16; i++) inside[i] = 0.5f;
    __m512 v_inside = _mm512_loadu_ps(inside);

    __mmask16 mask = tsfi_liang_barsky_clip_zmm(&v_inside, &vol);
    assert(mask == 0xFFFF);
    printf("  [PASS] All lanes survived inside the arena.\n");
}

void test_alligator_freedom_cull() {
    printf("[ALLIGATOR TEST] Freedom/Culling Verification...\n");
    
    TsfiClippingVolume vol = {
        .x_min = -1.0f, .x_max = 1.0f,
        .y_min = -1.0f, .y_max = 1.0f,
        .z_min = -1.0f, .z_max = 1.0f,
        .w_min = -1.0f, .w_max = 1.0f
    };

    // Vector with one group outside (Lanes 4-7 outside)
    float drift[16];
    for(int i=0; i<16; i++) drift[i] = 0.5f;
    drift[4] = 2.5f; // X > x_max

    __m512 v_drift = _mm512_loadu_ps(drift);
    __mmask16 mask = tsfi_liang_barsky_clip_zmm(&v_drift, &vol);

    // Group 1 (0-3) survives (0x000F)
    // Group 2 (4-7) culled   (0x00F0)
    // Group 3 (8-11) survives (0x0F00)
    // Group 4 (12-15) survives (0xF000)
    // Result: 0xF000 | 0x0F00 | 0x000F = 0xFF0F
    assert(mask == 0xFF0F);

    float result[16];
    _mm512_storeu_ps(result, v_drift);
    assert(result[4] == 0.0f);
    assert(result[5] == 0.0f);
    assert(result[0] == 0.5f);

    printf("  [PASS] Drift detected. Alligator culled the fugitive group.\n");
}

void test_alligator_boundary_survival() {
    printf("[ALLIGATOR TEST] Boundary Survival Verification...\n");
    
    TsfiClippingVolume vol = {
        .x_min = -1.0f, .x_max = 1.0f,
        .y_min = -1.0f, .y_max = 1.0f,
        .z_min = -1.0f, .z_max = 1.0f,
        .w_min = -1.0f, .w_max = 1.0f
    };

    // Vector exactly on the boundaries
    float boundary[16];
    for(int i=0; i<16; i+=4) {
        boundary[i]   = -1.0f; // x_min
        boundary[i+1] =  1.0f; // y_max
        boundary[i+2] = -1.0f; // z_min
        boundary[i+3] =  1.0f; // w_max
    }
    __m512 v_boundary = _mm512_loadu_ps(boundary);

    __mmask16 mask = tsfi_liang_barsky_clip_zmm(&v_boundary, &vol);
    assert(mask == 0xFFFF);
    printf("  [PASS] Boundary points (On the line) survived.\n");
}

void test_alligator_nan_cull() {
    printf("[ALLIGATOR TEST] NaN/Inf Protection Verification...\n");
    
    TsfiClippingVolume vol = {
        .x_min = -1.0f, .x_max = 1.0f,
        .y_min = -1.0f, .y_max = 1.0f,
        .z_min = -1.0f, .z_max = 1.0f,
        .w_min = -1.0f, .w_max = 1.0f
    };

    float unstable[16];
    for(int i=0; i<16; i++) unstable[i] = 0.0f;
    unstable[0] = 0.0f / 0.0f; // NaN
    unstable[4] = 1.0f / 0.0f; // Inf

    __m512 v_unstable = _mm512_loadu_ps(unstable);
    __mmask16 mask = tsfi_liang_barsky_clip_zmm(&v_unstable, &vol);

    // Group 1 (0-3) has NaN -> culled (0x000F mask bit 0 is 0)
    // Group 2 (4-7) has Inf -> culled (0x00F0 mask bit 4 is 0)
    // Surviving: 0xFF00
    assert((mask & 0x00FF) == 0);
    
    float result[16];
    _mm512_storeu_ps(result, v_unstable);
    assert(result[0] == 0.0f);
    assert(result[4] == 0.0f);

    printf("  [PASS] Alligator culled the unstable math (NaN/Inf). PPO is safe.\n");
}

int main() {
    printf("=== TSFi Liang-Barsky Alligator Unit Test ===\n");
    test_alligator_static_survival();
    test_alligator_boundary_survival();
    test_alligator_freedom_cull();
    test_alligator_nan_cull();
    printf("[SUCCESS] Alligator's head is rigid and fully vetted.\n");
    return 0;
}
