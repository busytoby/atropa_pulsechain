#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "tsfi_synth_perf.h"

TSFiSynthPerfEngine* tsfi_synth_perf_create(tsfi_dat *disk_dat, tsfi_trie_node *trie_root) {
    if (!trie_root) return NULL;
    TSFiSynthPerfEngine *engine = calloc(1, sizeof(TSFiSynthPerfEngine));
    if (!engine) return NULL;

    engine->synth = tsfi_synth_akb_create(disk_dat, trie_root);
    engine->audio_queue.write_idx = 0;
    engine->audio_queue.read_idx = 0;
    engine->gpu_buffer.is_mapped = 0;

    return engine;
}

void tsfi_synth_perf_push_audio(TSFiSynthPerfEngine *engine, float sample) {
    if (!engine) return;
    TSFiAudioRingBuffer *q = &engine->audio_queue;
    int next_write = (q->write_idx + 1) % AUDIO_RING_BUFFER_SIZE;
    
    // Check if queue is full (if so, drop oldest sample)
    if (next_write == q->read_idx) {
        q->read_idx = (q->read_idx + 1) % AUDIO_RING_BUFFER_SIZE;
    }

    q->buffer[q->write_idx] = sample;
    q->write_idx = next_write;
}

float tsfi_synth_perf_pop_audio(TSFiSynthPerfEngine *engine) {
    if (!engine) return 0.0f;
    TSFiAudioRingBuffer *q = &engine->audio_queue;
    if (q->read_idx == q->write_idx) return 0.0f; // Empty

    float val = q->buffer[q->read_idx];
    q->read_idx = (q->read_idx + 1) % AUDIO_RING_BUFFER_SIZE;
    return val;
}

void tsfi_synth_perf_map_vram(TSFiSynthPerfEngine *engine, float *vram_buffer, size_t size) {
    if (!engine || !vram_buffer) return;
    engine->gpu_buffer.vram_ptr = vram_buffer;
    engine->gpu_buffer.size = size;
    engine->gpu_buffer.is_mapped = 1;
}

void tsfi_synth_perf_sync_vram(TSFiSynthPerfEngine *engine) {
    if (!engine || !engine->gpu_buffer.is_mapped || !engine->gpu_buffer.vram_ptr) return;

    TSFiAKB *akb = engine->synth->orchestrator->akb;
    size_t copy_limit = (akb->cache_count < (int)engine->gpu_buffer.size) ? 
                         akb->cache_count : (int)engine->gpu_buffer.size;

    // Direct memory write to the mapped GPU memory pointer
    for (size_t i = 0; i < copy_limit; i++) {
        const char *val = akb->cache[i].value;
        if (strncmp(val, "TEMP_", 5) == 0) {
            val += 5;
        }
        engine->gpu_buffer.vram_ptr[i] = atof(val);
    }
}

void tsfi_synth_perf_parallel_solve(TSFiSynthPerfEngine *engine, const int *bytecode, size_t len, int branch_count, int *results) {
    if (!engine || !bytecode || !results || branch_count <= 0) return;

    // Execute multiple logical deduction branches concurrently using OpenMP threads
    #pragma omp parallel for
    for (int i = 0; i < branch_count; i++) {
        TSFiMarkovVM local_vm;
        memset(&local_vm, 0, sizeof(TSFiMarkovVM));
        
        // Spawn local solver execution
        tsfi_markov_vm_execute(&local_vm, bytecode, len);
        
        results[i] = (local_vm.stack_len > 0) ? local_vm.stack[0] : 0;
    }
}

void tsfi_synth_perf_destroy(TSFiSynthPerfEngine *engine) {
    if (!engine) return;
    if (engine->synth) tsfi_synth_akb_destroy(engine->synth);
    free(engine);
}
