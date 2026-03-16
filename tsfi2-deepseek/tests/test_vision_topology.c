#include "tsfi_vision.h"
#include "lau_memory.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_holes(int holes_to_draw) {
    int W = 128, H = 128;
    uint32_t *pixels = (uint32_t*)lau_malloc(W * H * sizeof(uint32_t));
    memset(pixels, 0, W * H * sizeof(uint32_t));

    // Draw a solid square (The "Glyph")
    for (int y = 20; y < 100; y++) {
        for (int x = 20; x < 100; x++) {
            pixels[y * W + x] = 0xFFFFFFFF;
        }
    }

    // Carve out holes
    if (holes_to_draw >= 1) {
        for (int y = 30; y < 50; y++) {
            for (int x = 30; x < 90; x++) {
                pixels[y * W + x] = 0xFF000000;
            }
        }
    }
    if (holes_to_draw >= 2) {
        for (int y = 70; y < 90; y++) {
            for (int x = 30; x < 90; x++) {
                pixels[y * W + x] = 0xFF000000;
            }
        }
    }

    TSFiGlyphPhenotype p;
    memset(&p, 0, sizeof(p));
    p.target_holes = holes_to_draw;
    
    TSFiResonanceAnalysis vis;
    tsfi_vision_analyze_glyph(pixels, W, H, &p, &vis);

    printf("[TOPO] Drawing %d holes -> Detected: %.0f\n", holes_to_draw, vis.topo_hole_count);
    assert((int)vis.topo_hole_count == holes_to_draw);

    lau_free(pixels);
}


void test_islands(int islands_to_draw) {
    int W = 128, H = 128;
    uint32_t *pixels = (uint32_t*)lau_malloc(W * H * sizeof(uint32_t));
    memset(pixels, 0, W * H * sizeof(uint32_t));

    // Draw disconnected squares (Islands)
    for (int i = 0; i < islands_to_draw; i++) {
        int start_x = 10 + (i * 30);
        int start_y = 10;
        if (start_x > 100) { start_x = 10 + ((i-3) * 30); start_y = 50; }
        
        for (int y = start_y; y < start_y + 20; y++) {
            for (int x = start_x; x < start_x + 20; x++) {
                if (x < W && y < H) pixels[y * W + x] = 0xFFFFFFFF;
            }
        }
    }

    TSFiGlyphPhenotype p;
    memset(&p, 0, sizeof(p));
    
    TSFiResonanceAnalysis vis;
    tsfi_vision_analyze_glyph(pixels, W, H, &p, &vis);

    printf("[TOPO] Drawing %d islands -> Detected: %.0f\n", islands_to_draw, vis.island_count);
    
    // Depending on the vision engine's implementation, it might measure exactly or probabilistically.
    // We will assert if it's exact.
    assert((int)vis.island_count == islands_to_draw);

    lau_free(pixels);
}


void test_symmetry_and_coverage() {
    int W = 128, H = 128;
    uint32_t *pixels = (uint32_t*)lau_malloc(W * H * sizeof(uint32_t));
    
    // 1. PERFECT SYMMETRY (A solid centered rectangle)
    memset(pixels, 0, W * H * sizeof(uint32_t));
    for (int y = 20; y < 100; y++) {
        for (int x = 40; x < 88; x++) {
            pixels[y * W + x] = 0xFFFFFFFF;
        }
    }
    TSFiGlyphPhenotype p;
    memset(&p, 0, sizeof(p));
    TSFiResonanceAnalysis vis1;
    tsfi_vision_analyze_glyph(pixels, W, H, &p, &vis1);
    
    printf("[TOPO] Symmetric Shape -> Symmetry: %.3f, Coverage: %.3f\n", vis1.glyph_symmetry, vis1.coverage);
    assert(vis1.glyph_symmetry > 0.85f); // High symmetry expected for rectangle // Should be highly symmetrical
    assert(vis1.coverage > 0.1f && vis1.coverage < 0.5f);

    // 2. ASYMMETRICAL SHAPE (An 'L' shape pushed to the left)
    memset(pixels, 0, W * H * sizeof(uint32_t));
    for (int y = 20; y < 100; y++) {
        for (int x = 20; x < 40; x++) {
            pixels[y * W + x] = 0xFFFFFFFF; // Vertical bar
        }
    }
    for (int y = 80; y < 100; y++) {
        for (int x = 40; x < 100; x++) {
            pixels[y * W + x] = 0xFFFFFFFF; // Horizontal bar
        }
    }
    TSFiResonanceAnalysis vis2;
    tsfi_vision_analyze_glyph(pixels, W, H, &p, &vis2);
    
    printf("[TOPO] Asymmetric Shape -> Symmetry: %.3f, Coverage: %.3f\n", vis2.glyph_symmetry, vis2.coverage);
    assert(vis2.glyph_symmetry < 0.5f); // Should be poorly symmetrical

    lau_free(pixels);
}

int main() {
    printf("[TEST] Starting Vision Topology Verification...\n");

    printf("[CASE] 0 Holes (Solid Square)\n");
    test_holes(0);

    printf("[CASE] 1 Hole (O-shape)\n");
    test_holes(1);

    printf("[CASE] 2 Holes (B-shape proxy)\n");
    test_holes(2);

    
    printf("[CASE] 1 Island (Single block)\n");
    test_islands(1);

    printf("[CASE] 2 Islands (Disconnected blocks)\n");
    test_islands(2);

    printf("[CASE] 3 Islands (Fragmented)\n");
    test_islands(3);

    
    printf("[CASE] Symmetry & Coverage Field\n");
    test_symmetry_and_coverage();

    printf("[SUCCESS] Vision Topology Verification PASSED.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}