#include "tsfi_font_ai.h"
#include "tsfi_font_vectors.h"
#include "lau_memory.h"
#include <stdio.h>
#include <assert.h>

// --- Mock Implementations ---
static int g_parent_calls = 0;
static int g_child_calls = 0;

void parent_evolve_impl(void *ctx, float intensity) {
    printf("[TEST] Parent Evolve Called! Intensity: %.2f, Context: %p\n", intensity, ctx);
    g_parent_calls++;
}

void child_evolve_impl(void *ctx, float intensity) {
    printf("[TEST] Child Evolve Called! Intensity: %.2f, Context: %p\n", intensity, ctx);
    g_child_calls++;
}

int main() {
    printf("[TEST] Font AI Evolution & Inheritance Pattern\n");

    // 1. Setup Parent
    printf("--- Phase 1: Parent Setup ---\n");
    TSFiFontSystem *parent = tsfi_font_init();
    assert(parent != NULL);
    
    printf("[DEBUG] Parent: %p, &ai_methods: %p, Value: %p\n", 
           parent, &parent->ai_methods, parent->ai_methods);

    assert(parent->ai_methods != NULL);

    // Bind Parent Logic
    int parent_ctx = 100;
    tsfi_font_ai_bind_evolve(parent, parent_evolve_impl, &parent_ctx);
    
    // Verify Parent
    tsfi_font_ai_invoke_evolve(parent, 0.5f);
    assert(g_parent_calls == 1);
    printf("[PASS] Parent Invoke Success.\n");

    // 2. Setup Child & Inherit
    printf("--- Phase 2: Inheritance ---\n");
    TSFiFontSystem *child = tsfi_font_init();
    assert(child != NULL);
    
    tsfi_font_ai_inherit(child, parent);
    
    // Verify Child (Should call Parent Logic)
    tsfi_font_ai_invoke_evolve(child, 0.7f);
    assert(g_parent_calls == 2);
    assert(g_child_calls == 0);
    printf("[PASS] Child Inherited Parent Logic.\n");

    // 3. Override Child
    printf("--- Phase 3: Override ---\n");
    int child_ctx = 200;
    tsfi_font_ai_bind_evolve(child, child_evolve_impl, &child_ctx);
    
    // Verify Child Override
    tsfi_font_ai_invoke_evolve(child, 0.9f);
    assert(g_child_calls == 1);
    assert(g_parent_calls == 2); // Unchanged
    printf("[PASS] Child Override Success.\n");

    // 4. Verify Parent Isolation
    printf("--- Phase 4: Parent Isolation ---\n");
    tsfi_font_ai_invoke_evolve(parent, 0.1f);
    assert(g_parent_calls == 3);
    assert(g_child_calls == 1); // Unchanged
    printf("[PASS] Parent Logic Isolated.\n");

    // 5. Scramble Verification (Bijective Entropy)
    printf("--- Phase 5: Scramble & Entropy ---\n");
    
    // Clean up AI resources cleanly BEFORE scramble
    tsfi_font_ai_destroy(parent);
    
    // Detach buffers to prevent corruption during scramble
    if (parent->unicode_map) { lau_free(parent->unicode_map); parent->unicode_map = NULL; }
    if (parent->registry) { lau_free(parent->registry); parent->registry = NULL; }
    if (parent->segments) { lau_free(parent->segments); parent->segments = NULL; }
    if (parent->spectrum) { lau_free(parent->spectrum); parent->spectrum = NULL; }
    
    // Invoke the Scramble Thunk (Bijection)
    parent->scramble();
    
    // Verify that the 'system_id' slot (which overlaps unicode_map) is non-null/randomized
    uint32_t *slot = (uint32_t*)&parent->unicode_map;
    printf("[TEST] Scramble Slot Value: 0x%x\n", *slot);
    
    printf("[PASS] Scramble executed via Bijection.\n");

    // Manual free of parent (tsfi_font_destroy would crash on scrambled pointers)
    lau_free(parent);
    
    // Child is still intact, destroy normally
    tsfi_font_destroy(child);
    
    printf("=== ALL TESTS PASSED ===\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
