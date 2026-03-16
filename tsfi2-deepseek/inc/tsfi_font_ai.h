#ifndef TSFI_FONT_AI_H
#define TSFI_FONT_AI_H

#include "lau_thunk.h"
#include "tsfi_font_vectors.h"

// --- AI Workload Inheritance Pattern ---
//
// The "VTable" for Font AI.
typedef struct {
    ThunkProxy *evolve;    // (Font, Intensity) -> NewGeometry
    void       *evolve_entry;

    ThunkProxy *evolve_wave; // (Font, float* wave, size_t count) -> void
    void       *evolve_wave_entry;

    ThunkProxy *evolve_sparse_wave; // (Font, float* wave, uint64_t* mask, size_t count) -> void
    void       *evolve_sparse_wave_entry;

    ThunkProxy *recognize; // (Font, Bitmap) -> Probability
    void       *recognize_entry;

    ThunkProxy *collapse;  // (Font) -> HelmholtzState
    void       *collapse_entry;
} TSFiAiMethodTable;

// Internal-ish structure, but needed for inlining
// (Removed TSFiAiWrapper as it conflicts with real layout)

// --- API ---

// Initialize the AI subsystem for a font system
void tsfi_font_ai_init(TSFiFontSystem *fs);

// Destroy/Cleanup
void tsfi_font_ai_destroy(TSFiFontSystem *fs);

// Bind a specific C function to the 'Evolve' method.
// The 'context' is baked into the thunk as the first argument.
// signature: void evolve_impl(void *context, float intensity);
void tsfi_font_ai_bind_evolve(TSFiFontSystem *fs, void *func_ptr, void *context);

// Bind a batch processor
// signature: void evolve_wave_impl(void *context, float *intensities, size_t count);
void tsfi_font_ai_bind_evolve_wave(TSFiFontSystem *fs, void *func_ptr, void *context);

// Bind a sparse batch processor
// signature: void evolve_sparse_wave_impl(void *context, float *intensities, uint64_t *mask, size_t count);
void tsfi_font_ai_bind_evolve_sparse_wave(TSFiFontSystem *fs, void *func_ptr, void *context);

// Inheritance: Copy method table from parent to child.
// Child gets independent ThunkProxies pointing to the same logic (initially).
// This allows Child to diverge later without affecting Parent.
void tsfi_font_ai_inherit(TSFiFontSystem *child, TSFiFontSystem *parent);

// Invoke (Inlined for speed - Branchless)
static inline void tsfi_font_ai_invoke_evolve(TSFiFontSystem *fs, float intensity) {
    if (!fs->ai_methods) return;
    TSFiAiMethodTable *table = (TSFiAiMethodTable*)fs->ai_methods;
    // Baked thunk bound Arg0 (ctx). Caller provides Arg1 (float intensity).
    // In x86_64 SysV ABI, first float is in XMM0.
    void (*fn)(float) = (void (*)(float))table->evolve_entry;
    fn(intensity);
}

// Invoke Wave (Batch Dispatch for >200x speedup)
static inline void tsfi_font_ai_invoke_evolve_wave(TSFiFontSystem *fs, float *intensities, size_t count) {
    if (!fs->ai_methods) return;
    TSFiAiMethodTable *table = (TSFiAiMethodTable*)fs->ai_methods;
    // Forwarding thunk: Arg0=ctx, Arg1=intensities, Arg2=count
    void (*fn)(float*, size_t) = (void (*)(float*, size_t))table->evolve_wave_entry;
    fn(intensities, count);
}

// Invoke Sparse Wave (Skipping for >20,000x speedup)
static inline void tsfi_font_ai_invoke_evolve_sparse_wave(TSFiFontSystem *fs, float *intensities, uint64_t *mask, size_t count) {
    if (!fs->ai_methods) return;
    TSFiAiMethodTable *table = (TSFiAiMethodTable*)fs->ai_methods;
    // Forwarding thunk: Arg0=ctx, Arg1=intensities, Arg2=mask, Arg3=count
    void (*fn)(float*, uint64_t*, size_t) = (void (*)(float*, uint64_t*, size_t))table->evolve_sparse_wave_entry;
    fn(intensities, mask, count);
}
    #endif
    