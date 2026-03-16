#include "tsfi_font_vectors.h"
#include "lau_memory.h" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_ottype.h"


void test_mongolian_joining() {
    printf("[TEST] Testing Mongolian Joining Logic...\n");

    // Mongolian word: "ᠮ蒙古" (Simplified test)
    // U+1820 (A), U+1821 (E), U+1822 (I)
    uint32_t cps[] = {0x1820, 0x1821, 0x1822};
    int count = 3;

    tsfi_ottype_t ot;
    memset(&ot, 0, sizeof(ot));
    
    tsfi_shaped_glyph_t shaped[32];
    int out_count = tsfi_ottype_shape(&ot, cps, count, shaped, 32, NULL, 0);
    
    printf("  Shaped count: %d (Input: %d)\n", out_count, count);
    assert(out_count == count); 

    // Verify indices are tracked
    assert((int)shaped[0].cluster_index == 0);
    assert((int)shaped[1].cluster_index == 1);
    assert((int)shaped[2].cluster_index == 2);

    printf("[TEST] Mongolian Joining Logic passed.\n");
}


#include "tsfi_io.h"

void test_cmap_marchen() {
    printf("[TEST] Evaluating CMAP Marchen Execution Bounds...\n");
    size_t size;
    uint8_t *font_data = (uint8_t*)tsfi_io_map_file("dummy.ttf", &size);
    if (!font_data) {
        printf("  [WARN] dummy.ttf unavailable, skipping topological parse.\n");
        return;
    }

    tsfi_ottype_t ot;
    if (!tsfi_ottype_init(&ot, font_data, size, 0)) {
        printf("  [WARN] tsfi_ottype_init safely rejected non-compliant font matrix.\n");
        extern void lau_unseal_object(void *ptr);
        lau_unseal_object(font_data);
        lau_free(font_data);
        return;
    }

    uint32_t gid = tsfi_ottype_get_glyph_index(&ot, 0x11C70, 0);
    printf("  GID for 0x11C70: %d\n", gid);
    
    // Physically decompose tracking boundaries
    extern void lau_unseal_object(void *ptr);
    lau_unseal_object(font_data);
    lau_free(font_data);
    printf("[TEST] CMAP matrix successfully evaluated and unmapped.\n");
}

int main() {
    printf("[TEST] Initializing Font Engine...\n");
    TSFiFontSystem *fs = tsfi_font_init();
    assert(fs != NULL);
    assert(fs->unicode_map != NULL);
    
    printf("[TEST] Generating Default Font...\n");
    tsfi_font_generate_default(fs);
    
    test_mongolian_joining();
    
    test_cmap_marchen();
    
    // Verify 'A' (ASCII 65)
    uint32_t gid_A = fs->unicode_map[65];
    printf("[TEST] 'A' Glyph ID: %u\n", gid_A);
    assert(gid_A > 0);
    
    TSFiGlyphRegistryPacked reg = fs->registry[gid_A];
    printf("[TEST] 'A' Segments: %u, Start: %u\n", reg.num_segments, reg.start_index);
    assert(reg.num_segments > 0);
    
    // Verify Vector Data
    TSFiPackedSegment seg0 = fs->segments[reg.start_index];
    printf("[TEST] 'A' Seg 0: (%d,%d) -> (%d,%d)\n", seg0.p0_x, seg0.p0_y, seg0.p2_x, seg0.p2_y);
    assert(seg0.p0_x == -6000); 
    assert(seg0.p0_y == -12000); 
    
    tsfi_font_destroy(fs);
    printf("[TEST] Passed.\n");
    
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