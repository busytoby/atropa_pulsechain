#ifndef TSFI_ZHAO_H
#define TSFI_ZHAO_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdatomic.h>

// Zhao: Zero-Copy Hybrid Asynchronous Orchestrator
// Multi-Ring Low-Latency implementation for high core counts.

#define ZHAO_RING_SIZE 256
#define ZHAO_RING_MASK (ZHAO_RING_SIZE - 1)

typedef struct {
    void (*func)(void *ctx);
    void *ctx;
} ZhaoTask;

typedef struct {
    ZhaoTask tasks[ZHAO_RING_SIZE];
    _Atomic uint32_t head;
    _Atomic uint32_t tail;
    int cpu_id;
    char padding[128]; // Explicit padding for cache-line isolation
} ZhaoRing;

// Global state for orchestration
typedef struct {
    ZhaoRing *rings;
    int worker_count;
    _Atomic int active_workers;
    _Atomic int shutdown;
    _Atomic int tasks_inflight;
    _Atomic uint32_t submission_idx;
} ZhaoScheduler;

void tsfi_zhao_init(int thread_count);
void tsfi_zhao_submit(void (*func)(void*), void *ctx);
void tsfi_zhao_sync(void);
void tsfi_zhao_shutdown(void);

#endif