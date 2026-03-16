#include "tsfi_font_ai.h"
#include "lau_memory.h"
#include "lau_thunk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Removed TSFiFontAiContext, using TSFiAiMethodTable directly

static void nop_evolve(void *ctx, float intensity) {
    (void)ctx; (void)intensity;
}

static void nop_evolve_wave(void *ctx, float *wave, size_t count) {
    (void)ctx; (void)wave; (void)count;
}

static void nop_evolve_sparse_wave(void *ctx, float *wave, uint64_t *mask, size_t count) {
    (void)ctx; (void)wave; (void)mask; (void)count;
}

void tsfi_font_ai_init(TSFiFontSystem *fs) {
    // Allocate the method table and store it in the system struct
    if (fs->ai_methods) return; // Already init
    
    TSFiAiMethodTable *table = (TSFiAiMethodTable*)lau_malloc(sizeof(TSFiAiMethodTable));
    memset(table, 0, sizeof(TSFiAiMethodTable));
    fs->ai_methods = table;
    
    // Create pools
    table->evolve = ThunkProxy_create();
    table->evolve_wave = ThunkProxy_create();
    table->evolve_sparse_wave = ThunkProxy_create();
    
    // Default NOPs
    table->evolve_entry = ThunkProxy_emit_baked(table->evolve, (void*)nop_evolve, 1, fs);
    table->evolve_wave_entry = ThunkProxy_emit_forwarding(table->evolve_wave, (void*)nop_evolve_wave, fs);
    table->evolve_sparse_wave_entry = ThunkProxy_emit_forwarding(table->evolve_sparse_wave, (void*)nop_evolve_sparse_wave, fs);
}

void tsfi_font_ai_destroy(TSFiFontSystem *fs) {
    if (!fs->ai_methods) return;
    TSFiAiMethodTable *table = (TSFiAiMethodTable*)fs->ai_methods;
    
    if (table->evolve) ThunkProxy_destroy(table->evolve);
    if (table->evolve_wave) ThunkProxy_destroy(table->evolve_wave);
    if (table->evolve_sparse_wave) ThunkProxy_destroy(table->evolve_sparse_wave);
    
    lau_free(table);
    fs->ai_methods = NULL;
}

void tsfi_font_ai_bind_evolve(TSFiFontSystem *fs, void *func_ptr, void *context) {
    if (!fs->ai_methods) tsfi_font_ai_init(fs);
    TSFiAiMethodTable *table = (TSFiAiMethodTable*)fs->ai_methods;
    
    // Emit new thunk
    table->evolve_entry = ThunkProxy_emit_baked(table->evolve, func_ptr, 1, context);
}

void tsfi_font_ai_bind_evolve_wave(TSFiFontSystem *fs, void *func_ptr, void *context) {
    if (!fs->ai_methods) tsfi_font_ai_init(fs);
    TSFiAiMethodTable *table = (TSFiAiMethodTable*)fs->ai_methods;
    
    // Emit new thunk
    table->evolve_wave_entry = ThunkProxy_emit_forwarding(table->evolve_wave, func_ptr, context);
}

void tsfi_font_ai_bind_evolve_sparse_wave(TSFiFontSystem *fs, void *func_ptr, void *context) {
    if (!fs->ai_methods) tsfi_font_ai_init(fs);
    TSFiAiMethodTable *table = (TSFiAiMethodTable*)fs->ai_methods;
    
    // Emit new thunk
    table->evolve_sparse_wave_entry = ThunkProxy_emit_forwarding(table->evolve_sparse_wave, func_ptr, context);
}

void tsfi_font_ai_inherit(TSFiFontSystem *child, TSFiFontSystem *parent) {
    if (!parent->ai_methods) return;
    TSFiAiMethodTable *p_table = (TSFiAiMethodTable*)parent->ai_methods;
    
    // Ensure child has storage
    if (!child->ai_methods) {
        child->ai_methods = lau_malloc(sizeof(TSFiAiMethodTable));
        memset(child->ai_methods, 0, sizeof(TSFiAiMethodTable));
    }
    TSFiAiMethodTable *c_table = (TSFiAiMethodTable*)child->ai_methods;
    
    // Copy table
    *c_table = *p_table;
    
    // Give child its own pools for future overrides (Copy-on-write semantics essentially)
    // Note: This detaches the pool pointer but keeps the ENTRY point valid.
    c_table->evolve = ThunkProxy_create();
    c_table->evolve_wave = ThunkProxy_create();
    c_table->evolve_sparse_wave = ThunkProxy_create();
}

// --- Export Wrapper for Plugins ---
void tsfi_font_ai_invoke_evolve_export(void *fs, float intensity) {
    tsfi_font_ai_invoke_evolve((TSFiFontSystem*)fs, intensity);
}