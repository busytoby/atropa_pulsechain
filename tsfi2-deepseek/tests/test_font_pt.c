#include "lau_memory.h"
#include "tsfi_font_vectors.h"
#include "tsfi_k0rn_ops.h"
#include "tsfi_genetic.h"
#include "tsfi_io.h"
#include "vulkan/vulkan_render.h"
#include "vulkan/vulkan_main.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "tsfi_raw.h"

int main() {
    printf("[FONT] Starting Point-Size (pt) Rendering Test...\n");

    VulkanSystem *s = create_vulkan_system();
    if (!s) return 1;
    s->external_render_enabled = true;

    // Load Phenotypes
    TSFiGlyphPhenotype p_A, p_B;
    tsfi_glyph_phenotype_load("assets/universal_font/0x41.glyph", &p_A);
    tsfi_glyph_phenotype_load("assets/universal_font/0x42.glyph", &p_B);

    // Load DNA Assets
    K0RnStream *stream_A = tsfi_k0rn_load("0x41");
    K0RnStream *stream_B = tsfi_k0rn_load("0x42");
    // Compile Ieung (Procedural Ideal)
    K0RnStream *stream_I = tsfi_k0rn_compile_char(0x327E); 
    
    if (!stream_A || !stream_B || !stream_I) {
        fprintf(stderr, "[ERROR] Failed to load/compile DNA assets.\n");
        destroy_vulkan_system(s);
        return 1;
    }
    
    K0RnStream *streams[3] = { stream_A, stream_B, stream_I };

    // Standardized analysis tile (128x128)
    uint32_t *analysis_tile = (uint32_t*)lau_malloc(128 * 128 * sizeof(uint32_t));
    void *wired_obj = lau_malloc_wired(sizeof(uint32_t) + sizeof(TSFiResonanceAnalysis));
    uint32_t *magic = (uint32_t*)wired_obj;
    *magic = 0x41534956; // 'VISA'
    TSFiResonanceAnalysis *vis_ptr = (TSFiResonanceAnalysis*)(magic + 1);

    int frame = 0;
    float sizes[] = { 18.0f, 36.0f, 72.0f, 120.0f };
    uint32_t colors[] = { 0xFFFFFFFF, 0xFF00FF00, 0xFF00FFFF, 0xFFFF00FF };

    // Morphing State
    // Allocate max capacity (Ieung is largest ~42 ops)
    K0RnStream *morph_stream = tsfi_k0rn_interpolate(stream_A, stream_I, 0.0f);
    int prev_class = TSFI_CLASS_LATIN_A;

    while (s->running && frame < 600) { // Extended runtime for 3 states
        wl_display_dispatch_pending(s->display);
        if (!s->paint_buffer || !s->paint_buffer->data) { draw_frame(s); continue; }

        uint32_t *pixels = (uint32_t*)s->paint_buffer->data;
        int W = s->paint_buffer->width;
        int H = s->paint_buffer->height;

        memset(pixels, 0, W * H * sizeof(uint32_t));

        // 3-State Loop
        float cycle_len = 100.0f;
        float t_global = (float)frame / cycle_len;
        int idx = ((int)t_global) % 3;
        int next_idx = (idx + 1) % 3;
        float t = t_global - (int)t_global; // 0.0 to 1.0
        
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

        // --- Standard Analysis Tile ---
        memset(analysis_tile, 0, 128 * 128 * sizeof(uint32_t));
        tsfi_font_render_k0rn_pt(morph_stream, analysis_tile, 128, 128, 72.0f, 64.0f, 64.0f, 0xFFFFFFFF);
        tsfi_vision_analyze_glyph(analysis_tile, 128, 128, &p_B, vis_ptr); // Use 'B' as reference for holes
        int current_class = tsfi_vision_classify_thunk(wired_obj);

        if (current_class != prev_class) {
            printf("[TRANSITION] Frame %d: DNA shifted from %d to %d (t=%.2f, Holes=%.0f)\n", 
                   frame, prev_class, current_class, t, vis_ptr->topo_hole_count);
            prev_class = current_class;
        }

        for (int i = 0; i < 4; i++) {
            float x_off = 100.0f + i * 160.0f;
            float y_off = 150.0f + i * 100.0f;
            
            tsfi_font_render_k0rn_pt(morph_stream, pixels, W, H, sizes[i], x_off, y_off, colors[i]);
            
            if (frame % 30 == 0) {
                printf("[STATUS] t=%.2f | Class=%d | Holes=%.0f | Corr=%.2f\n", 
                       t, current_class, vis_ptr->topo_hole_count, vis_ptr->target_correlation);
            }
        }

        draw_frame(s);
        tsfi_raw_usleep(16000);
        frame++;
    }

        lau_free(analysis_tile);

        lau_free(wired_obj);

        destroy_vulkan_system(s);

            extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;

    }

    