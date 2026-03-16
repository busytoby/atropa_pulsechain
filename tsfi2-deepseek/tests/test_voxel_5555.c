#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "lau_memory.h"

// Test 5555 Voxel Symmetry (Shader vs Thunk)
// Note: We simulate the shader logic in C for verification,
// as running the actual shader requires the full Vulkan runtime.

typedef struct {
    float global_intensity;
    uint32_t flags;
} VoxelUniforms;

#include "../thunks/tsfi_voxel_5555.c"

// Half-float decoder helper (simplified)
float half_to_float(uint16_t h) {
    uint32_t s = (h >> 15) & 0x00000001;
    uint32_t e = (h >> 10) & 0x0000001f;
    uint32_t m = h & 0x000003ff;
    if (e == 0) {
        if (m == 0) return s ? -0.0f : 0.0f;
        return (s ? -1.0f : 1.0f) * powf(2.0f, -14.0f) * (m / 1024.0f);
    } else if (e == 31) {
        return m == 0 ? (s ? -INFINITY : INFINITY) : NAN;
    }
    return (s ? -1.0f : 1.0f) * powf(2.0f, e - 15.0f) * (1.0f + m / 1024.0f);
}

int main() {
    printf("=== TSFi Voxel 5555 Symmetry Test ===\n");
    
    size_t count = 1024;
    uint32_t* input = lau_malloc(count * sizeof(uint32_t));
    uint16_t* output = lau_memalign(64, count * 4 * sizeof(uint16_t)); // RGBA16
    
    // Fill Input
    for (size_t i = 0; i < count; i++) {
        // R=31, G=0, B=15, S=16
        input[i] = (31) | (0 << 5) | (15 << 10) | (16 << 15);
    }
    
    VoxelUniforms u = { .global_intensity = 1.0f, .flags = 1 };
    
    tsfi_voxel_5555_thunk(input, output, count, &u);
    
    // Verify first pixel
    float r = half_to_float(output[0]);
    float g = half_to_float(output[1]);
    float b = half_to_float(output[2]);
    float a = half_to_float(output[3]);
    
    printf("Input: R31 G0 B15 S16\n");
    printf("Thunk: R%.4f G%.4f B%.4f A%.4f\n", r, g, b, a);
    
    // Expected:
    // S_norm = 16/31 ~= 0.516
    // R = 1.0 * S = 0.516
    // G = 0.0
    // B = (15/31) * S = 0.483 * 0.516 ~= 0.249
    // A = S = 0.516
    
    if (fabs(r - 0.516) < 0.01 && g == 0.0f && fabs(b - 0.249) < 0.01) {
        printf("[PASS] Thunk Output within tolerance.\n");
    } else {
        printf("[FAIL] Thunk Output incorrect.\n");
        return 1;
    }
    
    lau_free(input);
    lau_free(output);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}