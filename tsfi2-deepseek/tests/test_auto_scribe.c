#include <sys/mman.h>
#include <signal.h>
#include <time.h>
#include "tsfi_svdag.h"
#include "tsfi_opt_zmm.h"
#include "lau_memory.h"
#include "lau_thunk.h"
#include "tsfi_hilbert.h"
#include "tsfi_vision.h"
#include "tsfi_io.h"
#include "tsfi_wave512.h"
#include "vulkan/vulkan_render.h" 
#include "vulkan/vulkan_logic.h"
#include "vulkan/vulkan_main.h"
#include "tsfi_logic.h"
#include "tsfi_k0rn_ops.h"
#include "tsfi_genetic.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>
#include <immintrin.h>
#include <stdalign.h>

#define LOG(...) fprintf(stderr, __VA_ARGS__)

// External Rasterizer Declaration
void tsfi_font_render_k0rn_pt(const K0RnStream *stream, uint32_t *pixels, int width, int height, float pt_size, float x, float y, uint32_t color);

int main(int argc, char **argv) {
    signal(SIGPIPE, SIG_IGN);
    int current_seed = 42; 
    bool auto_test = false;
    char glyph_file[128] = "assets/universal_font/0x41.glyph";
    for(int i=1; i<argc; i++) {
        if(strcmp(argv[i], "--seed") == 0 && i+1 < argc) current_seed = atoi(argv[++i]);
        if(strcmp(argv[i], "--test") == 0) auto_test = true;
        if(strcmp(argv[i], "--glyph") == 0 && i+1 < argc) {
            snprintf(glyph_file, 127, "assets/universal_font/%s.glyph", argv[++i]);
        }
    }
    
    TSFiGlyphPhenotype p;
    if (!tsfi_glyph_phenotype_load(glyph_file, &p)) {
        LOG("[ERROR] Failed to load glyph phenotype: %s\n", glyph_file);
        return 1;
    }

    LOG("[SCRIBE] Target: %s | Codepoint: 0x%X | Holes: %d | Seed: %d\n", p.name, p.codepoint, p.target_holes, current_seed);
    VulkanSystem *s = create_vulkan_system();
    if (!s) return 1;
    s->external_render_enabled = true;

    char dna_name[32]; snprintf(dna_name, 31, "0x%X", p.codepoint);
    K0RnStream *root_sample = tsfi_k0rn_load(dna_name);
    if (!root_sample) {
        LOG("[WARNING] DNA asset not found for %s, compiling procedural root.\n", dna_name);
        root_sample = tsfi_k0rn_compile_char(p.codepoint);
    }

    K0RnOp *best_ops = (K0RnOp*)lau_malloc(64 * sizeof(K0RnOp));
    K0RnOp *mutant_ops = (K0RnOp*)lau_malloc(64 * sizeof(K0RnOp));
    K0RnStream stream; memset(&stream, 0, sizeof(stream));
    stream.op_count = root_sample->op_count; stream.ops = mutant_ops;
    stream.stream_capacity = 64;
    memcpy(mutant_ops, root_sample->ops, root_sample->op_count * sizeof(K0RnOp));
    memcpy(best_ops, mutant_ops, 64 * sizeof(K0RnOp));

    srand(current_seed);
    float best_score = -10000.0f;
    float temp = 1.0f; int gen = 0;
    alignas(64) TSFiResonanceAnalysis last_vis; memset(&last_vis, 0, sizeof(last_vis));
    uint32_t *pixels = NULL;
    int W = 0, H = 0;

    while (s->running) {
        wl_display_dispatch_pending(s->display);
        if (s->last_key_scancode == KEY_R) { // Reset
            current_seed = rand(); srand(current_seed); temp = 1.0f; best_score = -10000.0f;
            memcpy(mutant_ops, root_sample->ops, root_sample->op_count * sizeof(K0RnOp));
            memcpy(best_ops, mutant_ops, 64 * sizeof(K0RnOp)); // Adjusted capacity
            s->last_key_scancode = 0; 
        }
        if (s->last_key_scancode == KEY_N) { // No / Reject
            LOG("[ABORT] User Disapproved.\n");
            LOG("[DIAGNOSTIC] Vision State at Rejection:\n");
            LOG("  Stem: %.2f | Bar: %.2f | Inner: %.2f | Outer: %.2f\n", 
                last_vis.feature_stem, last_vis.feature_bar, last_vis.feature_inner_circle, last_vis.feature_outer_circle);
            LOG("  Holes: %.0f | Islands: %.0f | Sym: %.2f\n", last_vis.topo_hole_count, last_vis.island_count, last_vis.glyph_symmetry);
            
            destroy_vulkan_system(s); return 1;
        }
        if (s->last_key_scancode == KEY_Y) { 
            LOG("[SAVE] Approved. Saving Secret Checkpoint for %s...\n", p.name);
            if (pixels) tsfi_vision_save_glyph_checkpoint(p.name, &last_vis, pixels, W, H);
            break; 
        }

        if (!s->paint_buffer || !s->paint_buffer->data) { draw_frame(s); continue; }
        pixels = (uint32_t*)s->paint_buffer->data;
        W = s->paint_buffer->width; H = s->paint_buffer->height;

        // 1. Evolve
        // tsfi_k0rn_evolve_vision(&stream, &last_vis, 0.5f * temp); // DISABLED for Stability
        tsfi_k0rn_symmetrize(&stream); // Keep symmetry enforcement
        
        // 1b. Apply Geometric Constraints for Ieung
        if (p.codepoint == 0x327E && stream.op_count >= 42) {
            // Force Horizontal Bar (Index 40) - Fixed Position & Length
            stream.ops[40].x = -0.25f; stream.ops[40].z = 0.25f;
            stream.ops[40].y = -0.25f; stream.ops[40].w = -0.25f;
            
            // Force Vertical Stem (Index 41) - Fixed Position & Length
            stream.ops[41].x = 0.0f;   stream.ops[41].z = 0.0f;
            stream.ops[41].y = -0.25f; stream.ops[41].w = -0.6f;
        }
        
        // 2. Clear
        memset(pixels, 0, W * H * 4); 
        
        // 3. Render High-Fidelity Glyph (White on Black)
        // Center: W/2, H/2. Size: H * 0.6 (Matches Vision Scale 0.4)
        float pt_size = (float)H * 0.6f;
        tsfi_font_render_k0rn_pt(&stream, pixels, W, H, pt_size, (float)W*0.5f, (float)H*0.5f, 0xFFFFFFFF);

        // 4. Analyze
        alignas(64) TSFiResonanceAnalysis vis;
        tsfi_vision_analyze_glyph(pixels, W, H, &p, &vis);
        
        float hole_score = (fabsf(vis.topo_hole_count - (float)p.target_holes) < 0.1f) ? 50.0f : 0.0f;
        float score = (vis.target_correlation * 100.0f) + (vis.glyph_symmetry * 20.0f) + hole_score;
        
        draw_frame(s);

        if (score > best_score || (temp > 0.1f && ((float)rand()/RAND_MAX) < expf((score - best_score) / temp))) {
            if (score > best_score) {
                best_score = score; memcpy(best_ops, mutant_ops, 32 * sizeof(K0RnOp));
                LOG("[%s] Gen %d | Corr: %.3f | Sym: %.2f | Holes: %.0f | Score: %.1f\n", p.name, gen, vis.target_correlation, vis.glyph_symmetry, vis.topo_hole_count, score);
            }
        } else { memcpy(mutant_ops, best_ops, 32 * sizeof(K0RnOp)); }
        
        temp *= 0.999f; if (temp < 0.05f) temp = 0.05f;
        last_vis = vis; gen++;

        if (auto_test && gen >= 100 && hole_score > 0) {
            LOG("[AUTO] Gen %d reached with valid topology. Saving %s and exiting.\n", gen, p.name);
            tsfi_vision_save_glyph_checkpoint(p.name, &last_vis, pixels, W, H);
            break;
        }
    }
    lau_free(best_ops); lau_free(mutant_ops);
    destroy_vulkan_system(s);     extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}