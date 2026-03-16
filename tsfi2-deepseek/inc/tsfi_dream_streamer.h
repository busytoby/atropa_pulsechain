#ifndef TSFI_DREAM_STREAMER_H
#define TSFI_DREAM_STREAMER_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "tsfi_svdag.h"
#include "tsfi_pbr.h"

// --- TSFi Deep Moon Dream Streamer Architecture ---
// A continuous, real-time 3D generative pipeline.

// 1. The DeepSeek Geometric Mutation Stream
// Streams structural SVDAG deltas (mutations) to the ReBAR manifold.
typedef struct {
    void (*stream_mutations)(TSFiHelmholtzSVDAG *target_dag, void *deep_data);
    void *deep_data; // E.g., LLM context pointers
    uint32_t mutation_rate_hz;
    float current_entropy;
} TsfiDeepStreamer;

// 2. The Moondream Verification Stream (Moon Chuuker)
// Analyzes the live path-traced output buffer and streams back physical parameter adjustments.
typedef struct {
    void (*verify_output)(const uint32_t *frame_buffer, int w, int h, float *sov_melanin, float *sov_roughness, float *sov_irid);
    void *moon_data; // E.g., VLM context pointers
    uint32_t verify_rate_hz;
} TsfiMoonStreamer;

// 3. The Unified 3D Dream System
typedef struct {
    TsfiDeepStreamer *deep_module;
    TsfiMoonStreamer *moon_module;
    
    // Target Manifolds
    TSFiHelmholtzSVDAG *live_dag_flower;
    TSFiHelmholtzSVDAG *live_dag_bear;
    uint32_t *live_frame_buffer;
    
    // Sovereign PBR state
    float sov_melanin;
    float sov_roughness;
    float sov_irid;

    // Concurrency Controls
    pthread_mutex_t manifold_lock;
    pthread_t streamer_thread;
    bool is_active;
} TsfiDreamStreamerSystem;

// API
void tsfi_dream_streamer_init(TsfiDreamStreamerSystem *system);
void tsfi_dream_streamer_start(TsfiDreamStreamerSystem *system);
void tsfi_dream_streamer_stop(TsfiDreamStreamerSystem *system);

// The Core Background Loop
// 1. DeepSeek applies geometric mutations.
// 2. System Path-Traces the new SVDAG state.
// 3. Moondream analyzes the frame and updates Sovereign PBR secrets.
void* tsfi_dream_streamer_loop(void *system_ptr);

#endif // TSFI_DREAM_STREAMER_H
