#include "lau_memory.h"
#include "tsfi_font_vectors.h"
#include "tsfi_k0rn_ops.h"
#include "tsfi_genetic.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

// External
void tsfi_font_render_k0rn_pt(const K0RnStream *stream, uint32_t *pixels, int width, int height, float pt_size, float x, float y, uint32_t color);
void tsfi_video_write_frame(void *pixels, size_t size);

int main() {
    fprintf(stderr, "[VIDEO] Rendering A/B/Ieung Transition (22s)...\n");

    // Load Assets
    K0RnStream *stream_A = tsfi_k0rn_load("0x41");
    K0RnStream *stream_B = tsfi_k0rn_load("0x42");
    K0RnStream *stream_I = tsfi_k0rn_compile_char(0x327E); 
    
    if (!stream_A || !stream_B || !stream_I) {
        fprintf(stderr, "[ERROR] Failed to load/compile DNA assets.\n");
        return 1;
    }
    
    K0RnStream *streams[3] = { stream_A, stream_B, stream_I };

    // Buffer
    int W = 800, H = 600;
    uint32_t *pixels = (uint32_t*)lau_memalign(64, W * H * 4);
    
    // Interpolation Target
    K0RnStream *morph_stream = tsfi_k0rn_interpolate(stream_A, stream_I, 0.0f);

    float sizes[] = { 18.0f, 36.0f, 72.0f, 120.0f };
    uint32_t colors[] = { 0xFFFFFFFF, 0xFF00FF00, 0xFF00FFFF, 0xFFFF00FF };

    int total_frames = 60 * 22; // 22 seconds at 60fps
    float cycle_len = 120.0f; // Slower transition for visibility

    for (int frame = 0; frame < total_frames; frame++) {
        memset(pixels, 0, W * H * 4);

        // 3-State Loop
        float t_global = (float)frame / cycle_len;
        int idx = ((int)t_global) % 3;
        int next_idx = (idx + 1) % 3;
        float t = t_global - (int)t_global;
        
        // Smooth sine ease
        t = (1.0f - cosf(t * 3.14159f)) * 0.5f;

        K0RnStream *s1 = streams[idx];
        K0RnStream *s2 = streams[next_idx];
        
        // Update DNA
        for (uint32_t i = 0; i < morph_stream->op_count; i++) {
            K0RnOp opA = (i < s1->op_count) ? s1->ops[i] : (K0RnOp){.type=K0RN_OP_SPHERE, .p1=0};
            K0RnOp opB = (i < s2->op_count) ? s2->ops[i] : (K0RnOp){.type=K0RN_OP_SPHERE, .p1=0};
            morph_stream->ops[i].type = (t < 0.5f) ? opA.type : opB.type;
            morph_stream->ops[i].p1 = opA.p1 + (opB.p1 - opA.p1) * t;
            morph_stream->ops[i].x  = opA.x  + (opB.x  - opA.x)  * t;
            morph_stream->ops[i].y  = opA.y  + (opB.y  - opA.y)  * t;
            morph_stream->ops[i].z  = opA.z  + (opB.z  - opA.z)  * t;
            morph_stream->ops[i].w  = opA.w  + (opB.w  - opA.w)  * t;
        }

        // Render Multi-PT
        for (int i = 0; i < 4; i++) {
            float x_off = 100.0f + i * 160.0f;
            float y_off = 150.0f + i * 100.0f;
            tsfi_font_render_k0rn_pt(morph_stream, pixels, W, H, sizes[i], x_off, y_off, colors[i]);
        }

        // Write Frame
        tsfi_video_write_frame(pixels, W * H * 4);
        
        if (frame % 60 == 0) fprintf(stderr, "[VIDEO] Frame %d/%d\r", frame, total_frames);
    }

    lau_free(pixels);
    lau_free(morph_stream->ops); lau_free(morph_stream);
    lau_free(stream_I->ops); lau_free(stream_I);
    lau_free(stream_A->ops); lau_free(stream_A);
    lau_free(stream_B->ops); lau_free(stream_B);
    fprintf(stderr, "\n[VIDEO] Done.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}