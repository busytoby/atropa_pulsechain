#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include "vulkan_main.h"
#include "vulkan_render.h"
#include "tsfi_vision.h"
#include "lau_memory.h"
#include "tsfi_wave_any.h"
#include "tsfi_svdag.h"
#include "tsfi_dysnomia_crc.h"
#include "tsfi_raw.h"

// Uniforms for Wave Shaders
typedef struct { float intensity; float decay; } WaveUniforms;
// Uniforms for Voxel expansion
typedef struct { float global_intensity; uint32_t flags; } VoxelUniforms;

// Thunk Prototypes (declared in thunks/*.c)
extern void wave_shader_main(const WaveStream* stream, const WaveUniforms* u);
extern void wave_morph_main(const WaveStream* stream, const WaveUniforms* u);
#include "../thunks/tsfi_voxel_5555.c"
#include "../thunks/vrs_raymarch.c"
#include "../thunks/dysnomia_reactive_crc.c"

int main() {
    printf("[GALLERY] Initializing Unified Thunk Gallery...\n");
    
    VulkanSystem *s = create_vulkan_system();
    if (!s) return 1;

    int W = s->width;
    int H = s->height;
    int halfW = W / 2;
    int halfH = H / 2;

    // Allocate persistent buffers for thunks
    WaveStream stream;
    stream.count = halfW * halfH; // Scalar-like stream for manual partitioning
    stream.atom_size = 4;
    stream.stride = 4;
    stream.data = lau_memalign(64, halfW * halfH * 4 + 1024);
    stream.flags = 0;

    TSFiHelmholtzSVDAG* dag = tsfi_svdag_create(1024);
    DysnomiaReactiveState crc_state;
    memset(&crc_state, 0, sizeof(crc_state));

    VoxelUniforms v_u = { 1.0f, 1 };
    WaveUniforms w_u = { 1.0f, 0.5f };

    uint32_t* voxels = lau_malloc(halfW * halfH * sizeof(uint32_t));
    uint16_t* voxel_expanded = lau_memalign(64, halfW * halfH * 4 * sizeof(uint16_t));

    printf("[GALLERY] Starting Animation Loop (600 frames)...\n");
    for (int frame = 0; frame < 600; frame++) {
        float time_val = frame * 0.016f;
        float pulse = (sinf(time_val) + 1.0f) * 0.5f;
        uint32_t* screen = (uint32_t*)s->paint_buffer->data;
        
        // --- Region 1: Wave Shader (Top-Left) ---
        w_u.intensity = pulse * 2.0f;
        wave_shader_main(&stream, &w_u);
        float* f_data = (float*)stream.data;
        for(int y=0; y<halfH; y++) {
            for(int x=0; x<halfW; x++) {
                uint32_t v = (uint32_t)(f_data[y*halfW + x] * 255.0f);
                if(v > 255) v = 255;
                screen[y*W + x] = 0xFF000000 | (v << 8); // Green
            }
        }

        // --- Region 2: Wave Morph (Top-Right) ---
        w_u.decay = 1.0f - pulse;
        wave_morph_main(&stream, &w_u);
        f_data = (float*)stream.data;
        for(int y=0; y<halfH; y++) {
            for(int x=0; x<halfW; x++) {
                uint32_t v = (uint32_t)(f_data[y*halfW + x] * 255.0f);
                if(v > 255) v = 255;
                screen[y*W + (x + halfW)] = 0xFF000000 | (v << 16); // Red
            }
        }

        // --- Region 3: Voxel 5555 (Bottom-Left) ---
        for(int y=0; y<halfH; y++) {
            for(int x=0; x<halfW; x++) {
                uint32_t r5 = (uint32_t)(pulse * 31);
                uint32_t g5 = (x * 31 / halfW);
                uint32_t b5 = (y * 31 / halfH);
                voxels[y*halfW + x] = r5 | (g5 << 5) | (b5 << 10) | (31 << 15);
            }
        }
        tsfi_voxel_5555_thunk(voxels, voxel_expanded, halfW * halfH, &v_u);
        for(int y=0; y<halfH; y++) {
            for(int x=0; x<halfW; x++) {
                uint16_t* p = &voxel_expanded[(y*halfW + x)*4];
                uint32_t ir = p[0] >> 8; uint32_t ig = p[1] >> 8; uint32_t ib = p[2] >> 8;
                screen[(y + halfH)*W + x] = 0xFF000000 | (ib << 16) | (ig << 8) | ir;
            }
        }

        // --- Region 4: CRC Reactive (Bottom-Right) ---
        crc_state.op_counter = frame;
        wave_shader_main(&stream, &w_u); // Fill with data to CRC
        dysnomia_reactive_crc_thunk(&stream, &crc_state);
        float* crc_f = (float*)&crc_state.state_vector;
        for(int y=0; y<halfH; y++) {
            for(int x=0; x<halfW; x++) {
                uint32_t v = (uint32_t)(crc_f[(x+y)%16] * 255.0f);
                screen[(y + halfH)*W + (x + halfW)] = 0xFF000000 | v; // Blueish noise
            }
        }

        // Overlay Cockpit Labels
        draw_debug_text(s->paint_buffer, 10, 10, "THUNK: WAVE_SHADER (GREEN)", 0xFF00FF00, true);
        draw_debug_text(s->paint_buffer, halfW + 10, 10, "THUNK: WAVE_MORPH (RED)", 0xFF0000FF, true);
        draw_debug_text(s->paint_buffer, 10, halfH + 10, "THUNK: VOXEL_5555 (CMYK)", 0xFFFFFF00, true);
        draw_debug_text(s->paint_buffer, halfW + 10, halfH + 10, "THUNK: DYSNOMIA_CRC", 0xFFFF00FF, true);

        if (s->telem) {
            snprintf((char*)s->telem->last_directive_str, 64, "THUNK_GALLERY_ACTIVE");
            s->telem->current_intensity = pulse;
        }

        draw_frame(s);
        tsfi_raw_usleep(16000);
    }

    printf("[GALLERY] Complete.\n");
    
    lau_free(voxels);
    lau_free(voxel_expanded);
    lau_free(stream.data);
    tsfi_svdag_destroy(dag);
    destroy_vulkan_system(s);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}