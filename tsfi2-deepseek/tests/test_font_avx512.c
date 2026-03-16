#define _POSIX_C_SOURCE 200809L
#include "tsfi_font_vectors.h"
#include "lau_memory.h"
#include "tsfi_wire_firmware.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main() {
    printf("[TEST] AVX-512 Font Rasterizer Geometric Stress Matrix...\n");
    TSFiFontSystem *fs = tsfi_font_init();
    tsfi_font_generate_default(fs);

    // 1. Sustained Sequential Batch Rasterization Bound
    printf("[TEST] Sweeping full ASCII character spectrum (94 glyphs)...\n");
    uint8_t buffer[64 * 64];
    
    int successful_renders = 0;
    for (uint32_t cp = 0x20; cp <= 0x7E; cp++) {
        uint32_t gid = fs->unicode_map[cp];
        if (gid > 0) {
            memset(buffer, 0, sizeof(buffer));
            tsfi_font_rasterize_glyph_avx512(buffer, &fs->registry[gid], fs->segments);
            successful_renders++;
        }
    }
    
    printf("[TEST] Synthesized %d discrete geometric character profiles.\n", successful_renders);
    assert(successful_renders > 50); // Ensure fundamental default characters are instantiated

    // 2. Explicit Visual Validation Boundary ('A')
    uint32_t gid = fs->unicode_map['A'];
    assert(gid > 0);
    memset(buffer, 0, sizeof(buffer));
    tsfi_font_rasterize_glyph_avx512(buffer, &fs->registry[gid], fs->segments);
    
    printf("\n--- Visual Verification ('A') ---\n");
    for (int y = 0; y < 64; y += 2) {
        for (int x = 0; x < 64; x++) {
            uint8_t val = buffer[y * 64 + x];
            if (val > 200) printf("#");
            else if (val > 100) printf(":");
            else if (val > 0) printf(".");
            else printf(" ");
        }
        printf("\n");
    }
    printf("---------------------------------\n");
    
    tsfi_font_destroy(fs);
    printf("[TEST] Structural rasterization loop passed.\n");
    
    // Mathematically dissolve the hidden firmware bindings spawned by the font initialization
    extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    
    // Dissolve topological tracking barriers
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    
    // Explicitly audit final unmapped variables
    printf("\n[TEST] Expecting PERFECTLY ZERO LEAKS...\n");
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    
    
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
