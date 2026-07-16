#ifndef TSFI_SYNTH_PERF_H
#define TSFI_SYNTH_PERF_H

#include "tsfi_synth_akb.h"

#define AUDIO_RING_BUFFER_SIZE 256

// Lock-free Audio Ring Buffer
typedef struct {
    float buffer[AUDIO_RING_BUFFER_SIZE];
    volatile int write_idx;
    volatile int read_idx;
} TSFiAudioRingBuffer;

// Vulkan VRAM Direct Map representation (Simulated device storage buffer)
typedef struct {
    float *vram_ptr;
    size_t size;
    int is_mapped;
} TSFiVramDirectMap;

// Performance Engine Context
typedef struct {
    TSFiSynthAKB *synth;
    TSFiAudioRingBuffer audio_queue;
    TSFiVramDirectMap gpu_buffer;
} TSFiSynthPerfEngine;

// Create Performance Engine
TSFiSynthPerfEngine* tsfi_synth_perf_create(tsfi_dat *disk_dat, tsfi_trie_node *trie_root);

// Push audio sample to lock-free ring buffer
void tsfi_synth_perf_push_audio(TSFiSynthPerfEngine *engine, float sample);

// Pop audio sample from lock-free ring buffer
float tsfi_synth_perf_pop_audio(TSFiSynthPerfEngine *engine);

// Map AKB cache probabilities to GPU VRAM pointer
void tsfi_synth_perf_map_vram(TSFiSynthPerfEngine *engine, float *vram_buffer, size_t size);

// Sync AKB probabilities directly to mapped VRAM
void tsfi_synth_perf_sync_vram(TSFiSynthPerfEngine *engine);

// Run multi-threaded parallel branch resolution using OpenMP
void tsfi_synth_perf_parallel_solve(TSFiSynthPerfEngine *engine, const int *bytecode, size_t len, int branch_count, int *results);

// Destroy Performance Engine
void tsfi_synth_perf_destroy(TSFiSynthPerfEngine *engine);

#endif // TSFI_SYNTH_PERF_H
