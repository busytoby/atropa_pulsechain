#include "lau_memory.h"
#include "tsfi_vision.h"
#include "tsfi_k0rn_ops.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main() {
    printf("[TEST] Starting Mapped classify() Verification...\n");

    // 1. Genome Verification (Capital A)
    K0RnStream *s = tsfi_k0rn_load("0x41"); 
    assert(s != NULL);
    assert(s->classify != NULL);
    int class_genome = s->classify();
    printf("[RESULT] Genome Class: %d (Expected: %d)\n", class_genome, TSFI_CLASS_GLYPH);
    assert(class_genome == TSFI_CLASS_GLYPH);

    // 2. Teddy Bear Verification (The Original K0Rn)
    K0RnStream *teddy = tsfi_k0rn_compile_teddy();
    assert(teddy != NULL);
    assert(teddy->classify != NULL);
    int class_teddy = teddy->classify();
    printf("[RESULT] Teddy Class: %d (Expected: %d)\n", class_teddy, TSFI_CLASS_TEDDY);
    assert(class_teddy == TSFI_CLASS_TEDDY);

    // 3. Manifestation Verification (VISA Tagged)
    void *vis_buf = lau_malloc_wired(1024);
    uint32_t *magic = (uint32_t*)vis_buf;
    *magic = 0x41534956; 
    TSFiResonanceAnalysis *vis_ptr = (TSFiResonanceAnalysis*)(magic + 1);
    *vis_ptr = (TSFiResonanceAnalysis){0};
    
    vis_ptr->topo_hole_count = 1.0f;
    vis_ptr->glyph_symmetry = 0.95f;
    vis_ptr->coverage = 0.15f;
    vis_ptr->specular_contrast = 0.7f; 
    for (int i=0; i<8; i++) vis_ptr->captured_radial_profile[i] = 0.1f;
    vis_ptr->captured_radial_profile[4] = 1.0f;

    int class_macro = tsfi_vision_classify_thunk(vis_buf);
    printf("[RESULT] Manifestation Class: %d (Expected: %d)\n", class_macro, TSFI_CLASS_LATIN_A);
    assert(class_macro == TSFI_CLASS_LATIN_A);

    printf("[SUCCESS] Mapped classify() and Teddy Identity verified.\n");

    lau_free(s);
    lau_free(teddy);
    lau_free(vis_buf);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}