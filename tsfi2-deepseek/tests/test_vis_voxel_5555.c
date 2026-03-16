#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "vulkan_main.h"
#include "vulkan_render.h"
#include "tsfi_vision.h"
#include "lau_memory.h"
#include "tsfi_raw.h"

// Define Voxel Uniforms matching the thunk
typedef struct {
    float global_intensity;
    uint32_t flags;
} VoxelUniforms;

#include "../thunks/tsfi_voxel_5555.c"

// Half-float decoder helper (for better tonemapping)
static float h2f(uint16_t h) {
    uint32_t s = (h >> 15) & 0x00000001;
    uint32_t e = (h >> 10) & 0x0000001f;
    uint32_t m = h & 0x000003ff;
    if (e == 0) return (s ? -1.0f : 1.0f) * powf(2.0f, -14.0f) * (m / 1024.0f);
    return (s ? -1.0f : 1.0f) * powf(2.0f, e - 15.0f) * (1.0f + m / 1024.0f);
}

static void tonemap_rgba16_to_8(const uint16_t* src, uint32_t* dst, size_t count) {
    for (size_t i = 0; i < count; i++) {
        float r = h2f(src[i*4 + 0]);
        float g = h2f(src[i*4 + 1]);
        float b = h2f(src[i*4 + 2]);
        
        uint32_t ir = (uint32_t)(r * 255.0f);
        uint32_t ig = (uint32_t)(g * 255.0f);
        uint32_t ib = (uint32_t)(b * 255.0f);
        
        if (ir > 255) ir = 255;
        if (ig > 255) ig = 255;
        if (ib > 255) ib = 255;

        dst[i] = 0xFF000000 | (ib << 16) | (ig << 8) | ir;
    }
}

int main() {
    printf("[VOXEL] Initializing 5-Bit Voxel Unity Test...\n");
    
    VulkanSystem *s = create_vulkan_system();
    if (!s) return 1;

    size_t voxel_count = s->width * s->height;
    uint32_t* voxels = lau_malloc(voxel_count * sizeof(uint32_t));
    uint16_t* expanded = lau_memalign(64, voxel_count * 4 * sizeof(uint16_t));
    
    VoxelUniforms u = { .global_intensity = 1.0f, .flags = 1 };

    printf("[VOXEL] Generating Categorical Gradient (5555 Basis)...\n");
    for (int y = 0; y < (int)s->height; y++) {
        for (int x = 0; x < (int)s->width; x++) {
            uint32_t r5 = (x * 31 / s->width) & 0x1F;
            uint32_t g5 = (y * 31 / s->height) & 0x1F;
            uint32_t b5 = ((x+y) * 31 / (s->width + s->height)) & 0x1F;
            uint32_t s5 = 31; // Max smoothing
            
            voxels[y * s->width + x] = r5 | (g5 << 5) | (b5 << 10) | (s5 << 15);
        }
    }

    printf("[VOXEL] Running 60-frame visual verification...\n");
    for (int frame = 0; frame < 60; frame++) {
        tsfi_voxel_5555_thunk(voxels, expanded, voxel_count, &u);
        tonemap_rgba16_to_8(expanded, (uint32_t*)s->paint_buffer->data, voxel_count);
        
        if (s->telem) {
            snprintf((char*)s->telem->last_directive_str, 64, "VOXEL_5555_UNITY_TEST");
            s->telem->current_intensity = 1.0f;
            s->telem->recip_symmetry = 1.0f;
        }

        draw_frame(s);
        tsfi_raw_usleep(16000);
    }

    printf("[VOXEL] Unity Test Complete.\n");
    
    lau_free(voxels);
    lau_free(expanded);
    destroy_vulkan_system(s);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}