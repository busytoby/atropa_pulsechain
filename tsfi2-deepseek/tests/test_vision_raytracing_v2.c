#include "tsfi_font_vectors.h"
#include "tsfi_font_ai.h"
#include "tsfi_vision.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// Simulating GFX12 ReBAR Upload (Linear CPU -> GPU write)
void simulate_gfx12_rebar_upload(uint8_t *screen, int sw, int sh, uint8_t *glyph, int gx, int gy, int gw, int gh) {
    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            uint8_t pixel = glyph[y * gw + x];
            if (pixel > 0) {
                int screen_x = gx + x;
                int screen_y = gy + y;
                if (screen_x < sw && screen_y < sh) {
                    screen[screen_y * sw + screen_x] = pixel;
                }
            }
        }
    }
}

// Helper: Symmetry Analysis
int analyze_x_symmetry(uint8_t *glyph, int w, int h) {
    int diff = 0;
    for(int y=0; y<h; y++) {
        for(int x=0; x<w/2; x++) {
            uint8_t left = glyph[y*w + x];
            uint8_t right = glyph[y*w + (w - 1 - x)];
            diff += abs(left - right);
        }
    }
    return diff;
}

int main() {
    printf("[TEST] Vision Raytracing V2 (Bijective + Symmetry)...\n");

    TSFiFontSystem *fs = tsfi_font_init();
    assert(fs != NULL);
    WaveSystem *ws = (WaveSystem*)fs;
    
    printf("[CHECK] Bijective Mapping Active (scramble=%p)\n", ws->scramble);
    tsfi_font_generate_default(fs);

    // --- PHASE 1: Identification ---
    printf("[TEST] Phase 1: Identification...\n");
    
    // Rasterize 'A'
    uint32_t gid_A = fs->unicode_map['A'];
    uint8_t glyph_A[64 * 64];
    tsfi_font_rasterize_glyph_avx512(glyph_A, &fs->registry[gid_A], fs->segments);

    // Rasterize 'B'
    uint32_t gid_B = fs->unicode_map['B'];
    uint8_t glyph_B[64 * 64];
    tsfi_font_rasterize_glyph_avx512(glyph_B, &fs->registry[gid_B], fs->segments);

    // Render 'A' to Screen
    int width = 256;
    int height = 256;
    uint8_t *screen_buffer = (uint8_t*)lau_malloc(width * height);
    memset(screen_buffer, 0, width * height);
    simulate_gfx12_rebar_upload(screen_buffer, width, height, glyph_A, 100, 100, 64, 64);

    // Extract ROI
    uint8_t roi[64 * 64];
    for(int y=0; y<64; y++) {
        for(int x=0; x<64; x++) {
            roi[y*64 + x] = screen_buffer[(100+y)*width + (100+x)];
        }
    }

    // Verify Identification
    bool is_A = tsfi_vision_compare_raw(roi, glyph_A, 64*64);
    assert(is_A == true);
    bool is_B = tsfi_vision_compare_raw(roi, glyph_B, 64*64);
    assert(is_B == false);
    printf("[PASS] Identification verified.\n");

    // --- PHASE 2: Symmetry ---
    printf("[TEST] Phase 2: Symmetry Analysis...\n");
    
    // Define 'O' (Symmetric Diamond) - Centered at X=-256 (Pixel 31.5)
    int idx_O = 0;
    TSFiPackedSegment segs_O[4];
    add_line(segs_O, &idx_O, -256, 12000, 5744, 0);
    add_line(segs_O, &idx_O, 5744, 0, -256, -12000);
    add_line(segs_O, &idx_O, -256, -12000, -6256, 0);
    add_line(segs_O, &idx_O, -6256, 0, -256, 12000);
    tsfi_font_set_glyph(fs, 'O', segs_O, 4, 12000, -6256, -12000, 5744, 12000);

    uint32_t gid_O = fs->unicode_map['O'];
    uint8_t glyph_O[64*64];
    tsfi_font_rasterize_glyph_avx512(glyph_O, &fs->registry[gid_O], fs->segments);

    int diff_O = analyze_x_symmetry(glyph_O, 64, 64);
    printf("      'O' Symmetry Diff: %d\n", diff_O);
    assert(diff_O < 2000);

    // Define 'L' (Asymmetric)
    int idx_L = 0;
    TSFiPackedSegment segs_L[2];
    add_line(segs_L, &idx_L, -6256, 12000, -6256, -12000);
    add_line(segs_L, &idx_L, -6256, -12000, 5744, -12000);
    tsfi_font_set_glyph(fs, 'L', segs_L, 2, 12000, -6256, -12000, 5744, 12000);
    
    uint32_t gid_L = fs->unicode_map['L'];
    uint8_t glyph_L[64*64];
    tsfi_font_rasterize_glyph_avx512(glyph_L, &fs->registry[gid_L], fs->segments);

    int diff_L = analyze_x_symmetry(glyph_L, 64, 64);
    printf("      'L' Symmetry Diff: %d\n", diff_L);
    assert(diff_L > 5000);
    printf("[PASS] Symmetry Analysis verified.\n");

    // --- PHASE 3: Bijective Scramble ---
    printf("[TEST] Phase 3: Bijective Scramble...\n");
    lau_free(screen_buffer); // Cleanup buffer before breaking system
    
    ws->scramble();
    LauSystemHeader *h = (LauSystemHeader *)((char *)fs - sizeof(LauSystemHeader));
    printf("      Resonance: %s\n", h->resonance_as_status ? h->resonance_as_status : "NULL");
    assert(h->resonance_as_status != NULL);
    
    lau_free(fs); // Raw free, as pointers are scrambled
    printf("[PASS] Scramble verified.\n");

        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
