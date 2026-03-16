#include "tsfi_zhao.h"
#include "lau_memory.h"
#include "tsfi_raw.h"
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <stdatomic.h>
#include <immintrin.h>
#include <stdalign.h>

#define STACK_SIZE (1024 * 1024)

#ifndef CLONE_VM
#define CLONE_VM      0x00000100
#define CLONE_FS      0x00000200
#define CLONE_FILES   0x00000400
#define CLONE_SIGHAND 0x00000800
#define CLONE_THREAD  0x00010000
#define CLONE_SYSVSEM 0x00040000
#endif

static ZhaoScheduler g_zhao;
static void **g_stacks = NULL;

static int tsfi_zhao_worker_entry(void *arg) {
    ZhaoRing *ring = (ZhaoRing*)arg;
    
    // Core Pinning
    tsfi_cpu_set_t cpuset;
    TSFI_CPU_ZERO(&cpuset);
    TSFI_CPU_SET(ring->cpu_id % 1024, &cpuset);
    tsfi_raw_sched_setaffinity(0, sizeof(tsfi_cpu_set_t), &cpuset);

    atomic_fetch_add(&g_zhao.active_workers, 1);
    
    while (!atomic_load_explicit(&g_zhao.shutdown, memory_order_relaxed)) {
        uint32_t h = atomic_load_explicit(&ring->head, memory_order_relaxed);
        uint32_t t = atomic_load_explicit(&ring->tail, memory_order_acquire);
        
        if (h != t) {
            ZhaoTask *task = &ring->tasks[h & ZHAO_RING_MASK];
            task->func(task->ctx);
            
            atomic_fetch_sub_explicit(&g_zhao.tasks_inflight, 1, memory_order_release);
            atomic_store_explicit(&ring->head, h + 1, memory_order_release);
        } else {
            _mm_pause();
        }
    }
    
    atomic_fetch_sub(&g_zhao.active_workers, 1);
    return 0;
}

void tsfi_zhao_init(int thread_count) {
    if (g_zhao.worker_count > 0) return;
    
    g_zhao.worker_count = thread_count;
    g_zhao.rings = (ZhaoRing*)lau_memalign(128, sizeof(ZhaoRing) * thread_count);
    memset(g_zhao.rings, 0, sizeof(ZhaoRing) * thread_count);
    
    atomic_init(&g_zhao.active_workers, 0);
    atomic_init(&g_zhao.shutdown, 0);
    atomic_init(&g_zhao.tasks_inflight, 0);
    atomic_init(&g_zhao.submission_idx, 0);
    
    g_stacks = (void**)lau_malloc(thread_count * sizeof(void*));
    
    for (int i = 0; i < thread_count; i++) {
        g_zhao.rings[i].cpu_id = i % sysconf(_SC_NPROCESSORS_ONLN);
        atomic_init(&g_zhao.rings[i].head, 0);
        atomic_init(&g_zhao.rings[i].tail, 0);
        
        g_stacks[i] = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, 
                           MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
        
        int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM;
        // Align stack top to 64 bytes for AVX-512/ZMM safety
        void *stack_top = (void*)(((uintptr_t)g_stacks[i] + STACK_SIZE) & ~63);
        
        if (tsfi_raw_clone(tsfi_zhao_worker_entry, stack_top, flags, &g_zhao.rings[i]) == -1) {
            perror("zhao clone");
            abort();
        }
    }
    
    while (atomic_load(&g_zhao.active_workers) < thread_count) _mm_pause();
}

// Occupancy-Aware Submission (Least Loaded Ring)
void tsfi_zhao_submit(void (*func)(void*), void *ctx) {
    int best_ring = 0;
    uint32_t min_load = 0xFFFFFFFF;
    
    // Scan rings for least occupied (wait-free query)
    // We only scan a subset (e.g. 4) to keep submission latency sub-microsecond
    uint32_t start_idx = atomic_fetch_add_explicit(&g_zhao.submission_idx, 1, memory_order_relaxed);
    
    for (int i = 0; i < 4; i++) {
        int r_idx = (start_idx + i) % g_zhao.worker_count;
        ZhaoRing *r = &g_zhao.rings[r_idx];
        uint32_t h = atomic_load_explicit(&r->head, memory_order_relaxed);
        uint32_t t = atomic_load_explicit(&r->tail, memory_order_relaxed);
        uint32_t load = t - h;
        
        if (load < min_load) {
            min_load = load;
            best_ring = r_idx;
            if (load == 0) break; // Perfect match
        }
    }

    ZhaoRing *ring = &g_zhao.rings[best_ring];
    uint32_t t = atomic_load_explicit(&ring->tail, memory_order_relaxed);
    
    // Spin if local ring full
    while ((t + 1 - atomic_load_explicit(&ring->head, memory_order_acquire)) >= ZHAO_RING_SIZE) {
        _mm_pause();
    }
    
    ZhaoTask *task = &ring->tasks[t & ZHAO_RING_MASK];
    task->func = func;
    task->ctx = ctx;
    
    atomic_fetch_add_explicit(&g_zhao.tasks_inflight, 1, memory_order_relaxed);
    atomic_store_explicit(&ring->tail, t + 1, memory_order_release);
}

void tsfi_zhao_sync(void) {
    while (atomic_load_explicit(&g_zhao.tasks_inflight, memory_order_acquire) > 0) {
        _mm_pause();
    }
}

void tsfi_zhao_shutdown(void) {
    atomic_store(&g_zhao.shutdown, 1);
    while (atomic_load(&g_zhao.active_workers) > 0) _mm_pause();
    
    for (int i = 0; i < g_zhao.worker_count; i++) munmap(g_stacks[i], STACK_SIZE);
    lau_free(g_stacks);
    lau_free(g_zhao.rings);
    g_zhao.worker_count = 0;
}
