#include "lau_memory.h"
#include "tsfi_vision.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main() {
    printf("[TEST] Starting Dynamic Classification Test...\n");

    // 1. Allocate a wired object
    // We allocate enough space for Magic + TSFiResonanceAnalysis
    size_t obj_size = sizeof(uint32_t) + sizeof(TSFiResonanceAnalysis);
    void *obj = lau_malloc_wired(obj_size);
    assert(obj != NULL);

    // 2. Setup the "Analyzed" object data
    uint32_t *magic = (uint32_t*)obj;
    *magic = 0x41534956; // 'VISA'
    
    TSFiResonanceAnalysis *a = (TSFiResonanceAnalysis*)(magic + 1);
    memset(a, 0, sizeof(TSFiResonanceAnalysis));
    
    // Setup metrics to match Latin_Capital_A (Class 3)
    a->glyph_symmetry = 0.95f;      // High symmetry
    a->topo_hole_count = 1.0f;     // 1 hole
    a->specular_contrast = 0.5f;    // Bottom heavy (Top/Bot ratio < 0.85)
    a->complexity = 0.8f;

    // 3. Classify via thunk
    int class_id = tsfi_vision_classify_thunk(obj);
    printf("[TEST] Object classified as: %d\n", class_id);

    assert(class_id == TSFI_CLASS_LATIN_A);
    printf("[TEST] Match confirmed: %d == TSFI_CLASS_LATIN_A\n", class_id);

    // 4. Test fallback (Flower Entry)
    void *obj2 = lau_malloc_wired(sizeof(TSFiFlowerEntry));
    TSFiFlowerEntry *e = (TSFiFlowerEntry*)obj2;
    memset(e, 0, sizeof(TSFiFlowerEntry));
    strcpy(e->phenotype.name, "Xenostegia");
    
    // Setup metrics for Floral (Class 1)
    e->last_analysis.radial_score = 0.9f;
    e->last_analysis.symmetry_stability = 0.9f;
    e->last_analysis.complexity = 0.5f;
    
    int class_id2 = tsfi_vision_classify_thunk(obj2);
    printf("[TEST] Flower object classified as: %d\n", class_id2);
    assert(class_id2 == TSFI_CLASS_FLORAL);

    lau_free(obj);
    lau_free(obj2);
    
    printf("[TEST] Dynamic Classification Test PASSED.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}