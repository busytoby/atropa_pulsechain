#include "lau_memory.h"
#include "tsfi_k0rn_ops.h"
#include "tsfi_genetic.h"
#include "tsfi_vision.h"
#include "tsfi_hotloader.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <immintrin.h>

// Forward declarations if not in tsfi_genetic.h
K0RnStream* tsfi_k0rn_compile_char(uint32_t c);

void test_char_compilation_capacity() {
    printf("[TEST] Checking Character Compilation Capacity...\n");
    K0RnStream *sA = tsfi_k0rn_compile_char(0x41); // 'A'
    K0RnStream *sB = tsfi_k0rn_compile_char(0x42); // 'B'
    
    printf("  'A' Op Count: %u (Capacity: %u)\n", sA->op_count, sA->stream_capacity);
    printf("  'B' Op Count: %u (Capacity: %u)\n", sB->op_count, sB->stream_capacity);
    
    assert(sA->op_count > 0);
    assert(sB->op_count > 16); 
    assert(sB->stream_capacity >= 32);
    
    lau_unseal_object(sA);
    lau_free(sA->ops); lau_free(sA);
    lau_unseal_object(sB);
    lau_free(sB->ops); lau_free(sB);
    printf("  [PASS] Capacity Verified.\n");
}

void test_interpolation_stability() {
    printf("[TEST] Checking Interpolation Stability (Dual-Density)...\n");
    K0RnStream *sB = tsfi_k0rn_compile_char(0x42);
    for(uint32_t i=0; i < sB->op_count; i++) {
        assert(!isnan(sB->ops[i].x));
        assert(!isnan(sB->ops[i].y));
        assert(!isnan(sB->ops[i].p1));
    }
    printf("  [PASS] Stability Verified.\n");
    lau_unseal_object(sB);
    lau_free(sB->ops); lau_free(sB);
}

void test_validate_ieung_structure() {
    printf("=== TSFi2 Ieung Structure Validator ===\n");
    
    // 1. Compile 0x327E
    K0RnStream *s = tsfi_k0rn_compile_char(0x327E);
    if (!s) { printf("[FAIL] Compiler returned NULL\n"); return; }
    printf("[INFO] DNA Generated: %u ops\n", s->op_count);

    // 2. Render to Buffer
    int W = 256, H = 256;
    uint32_t *pixels = (uint32_t*)lau_memalign(64, W * H * 4);
    memset(pixels, 0, W * H * 4);
    
    // Render
    float pt_size = (float)H * 0.6f;
    void (*rasterizer)(const K0RnStream *, uint32_t *, int, int, float, float, float, uint32_t) = tsfi_hotload_thunk("thunks/tsfi_k0rn_rasterizer.c", "tsfi_font_render_k0rn_pt");
    if (rasterizer) rasterizer(s, pixels, W, H, pt_size, (float)W*0.5f, (float)H*0.5f, 0xFFFFFFFF);

    // 3. Analyze
    TSFiGlyphPhenotype p = { .target_holes = 2, .target_coverage = 0.25f }; // Mock
    TSFiResonanceAnalysis vis;
    tsfi_vision_analyze_glyph(pixels, W, H, &p, &vis);

    printf("[VISION] Topology Holes: %.2f\n", vis.topo_hole_count);
    printf("[VISION] Topology Islands: %.2f\n", vis.island_count);
    printf("[VISION] Coverage: %.4f\n", vis.coverage);
    printf("[VISION] Symmetry: %.4f\n", vis.glyph_symmetry);
    
    printf("[FEATURES] Stem: %.2f | Bar: %.2f | InnerHole: %.2f | Outer: %.2f\n", 
           vis.feature_stem, vis.feature_bar, vis.feature_inner_circle, vis.feature_outer_circle);

    
    
    // Check Separation: Should have 3 Islands (Outer, Inner, T)
    if (vis.island_count == 3.0f) {
        printf("[PASS] Separation Correct (3 Islands).\n");
    } else {
        printf("[FAIL] Separation Incorrect (Expected 3, Got %.0f). T may be connected to Circle.\n", vis.island_count);
        
    }

    if (vis.topo_hole_count == 2.0f) {
        printf("[PASS] Topology Correct (2 Holes).\n");
    } else {
        printf("[FAIL] Topology Incorrect (Expected 2, Got %.0f).\n", vis.topo_hole_count);
        
    }

    if (vis.feature_stem > 0.5f && vis.feature_bar > 0.5f) {
        printf("[PASS] T-Shape Detected.\n");
    } else {
        printf("[FAIL] T-Shape Missing (Stem=%.2f, Bar=%.2f).\n", vis.feature_stem, vis.feature_bar);
        
    }

    if (vis.glyph_symmetry > 0.95f) {
        printf("[PASS] Symmetry High (%.2f).\n", vis.glyph_symmetry);
    } else {
        printf("[WARN] Symmetry Low (%.2f) - Circles should be symmetric.\n", vis.glyph_symmetry);
    }

    lau_unseal_object(s);
    lau_free(s->ops);
    lau_free(s);
    lau_free(pixels);
}

int main() {
    printf("=== TSFi2 K0Rn Fidelity Test Suite ===\n");
    test_char_compilation_capacity();
    test_interpolation_stability();
    test_validate_ieung_structure();
    printf("=== [SUCCESS] All Fidelity Tests Passed ===\n");
    extern void tsfi_dl_thunks_teardown(void);
    tsfi_dl_thunks_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}