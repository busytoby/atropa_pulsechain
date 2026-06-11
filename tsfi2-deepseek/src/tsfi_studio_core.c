#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <immintrin.h>
#include <sys/mman.h>
#include <unistd.h>

#include "tsfi_zhao.h"
#include "tsfi_fourier.h"
#include "lau_memory.h"
#include "tsfi_resonance.h"

#define COMPETITIVE_BUFFER_SIZE (8 * 1024 * 1024) // 8MB

typedef struct {
    uint8_t *buffer;
    int ccx_id;
    bool active;
} StudioCompetitor;

typedef struct {
    StudioCompetitor competitors[2];
    TSFiFourierBasis basis;
    void *manifold_shm;
    bool running;
} TsfiStudio;

static TsfiStudio g_studio;

// Core Complex Pinning Utility
static void pin_to_ccx(pthread_t thread, int ccx_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    if (ccx_id == 0) {
        // CCX0 (CCD 0): Cores 0-7 (Threads 0-7, 16-23)
        for (int i = 0; i < 8; i++) {
            CPU_SET(i, &cpuset);
            CPU_SET(i + 16, &cpuset);
        }
    } else {
        // CCX1 (CCD 1): Cores 8-15 (Threads 8-15, 24-31)
        for (int i = 8; i < 16; i++) {
            CPU_SET(i, &cpuset);
            CPU_SET(i + 16, &cpuset);
        }
    }
    pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
}

// DeepSeek L3-Resident Simulation (quantized inference emulation using VNNI)
static void simulate_deepseek_inference(StudioCompetitor *comp) {
    // Quantized VNNI inner loop simulation
    __m512i acc = _mm512_setzero_si512();
    __m512i act = _mm512_set1_epi32(0x01010101); // Simulated activations
    __m512i wgt = _mm512_set1_epi32(0x5A5A5A5A); // Simulated INT8 weights
    
    // Simulate processing of a small 1.5B quantized parameter layer (30MB)
    // Run entirely in local L3/L2 cache
    for (int i = 0; i < 10000; i++) {
        acc = _mm512_dpbusd_epi32(acc, act, wgt);
    }
    
    // Convert to target seed coordinate modifier
    float val = (float)_mm512_reduce_add_epi32(acc) * 1e-10f;
    comp->buffer[0] = (uint8_t)(val * 255.0f);
}

// Candidate Evolution Thread
static void *studio_evolution_thread(void *arg) {
    StudioCompetitor *comp = (StudioCompetitor*)arg;
    pin_to_ccx(pthread_self(), comp->ccx_id);

    printf("[STUDIO] Candidate Evolution Thread Active on CCX %d\n", comp->ccx_id);

    while (g_studio.running) {
        // 1. Grade 2 Unlock
        mprotect(comp->buffer, COMPETITIVE_BUFFER_SIZE, PROT_READ | PROT_WRITE);

        // 2. Local DeepSeek validation / seed generation (VNNI)
        simulate_deepseek_inference(comp);

        // 3. Coordinate synthesis (AVX-512)
        // Perform low-resolution coordinate evaluation on the 8MB buffer
        __m512i pattern = _mm512_set1_epi32(comp->buffer[0]);
        size_t chunks = COMPETITIVE_BUFFER_SIZE / 64;
        for (size_t i = 0; i < chunks; i++) {
            _mm512_storeu_si512((void*)(comp->buffer + (i * 64)), pattern);
        }

        // Manual coherence write-back: flush cache lines to memory controller
        for (size_t i = 0; i < COMPETITIVE_BUFFER_SIZE; i += 64) {
            _mm_clwb(comp->buffer + i);
        }
        _mm_sfence();

        // 4. Grade 2 Lock (Lock down memory for zero-copy GPU upscaling)
        mprotect(comp->buffer, COMPETITIVE_BUFFER_SIZE, PROT_READ);

        // Sleep to throttle frame generation to match standard target rates
        usleep(8333); // ~120 Hz
    }
    return NULL;
}

void tsfi_studio_init(void *vk_ctx, void *shm_ptr) {
    printf("[STUDIO] Formalizing Unified Studio Model...\n");
    g_studio.manifold_shm = shm_ptr;
    g_studio.running = true;

    // Initialize Fourier Constant ROM Basis
    tsfi_fourier_init_basis(&g_studio.basis);

    // Initialize Zhao CCX-aware scheduler
    tsfi_zhao_init(32);

    // Allocate page-aligned competitive 8MB buffers
    g_studio.competitors[0].buffer = (uint8_t*)aligned_alloc(4096, COMPETITIVE_BUFFER_SIZE);
    g_studio.competitors[0].ccx_id = 0;
    g_studio.competitors[0].active = true;

    g_studio.competitors[1].buffer = (uint8_t*)aligned_alloc(4096, COMPETITIVE_BUFFER_SIZE);
    g_studio.competitors[1].ccx_id = 1;
    g_studio.competitors[1].active = true;

    // Spawn CCX-isolated evolution workers
    pthread_t t0, t1;
    pthread_create(&t0, NULL, studio_evolution_thread, &g_studio.competitors[0]);
    pthread_create(&t1, NULL, studio_evolution_thread, &g_studio.competitors[1]);

    pthread_detach(t0);
    pthread_detach(t1);

    printf("[STUDIO] Unified Studio Model Running (CCX0/CCX1 Active).\n");
    (void)vk_ctx;
}

void tsfi_studio_shutdown(void) {
    g_studio.running = false;
    usleep(20000); // Allow threads to exit loop

    mprotect(g_studio.competitors[0].buffer, COMPETITIVE_BUFFER_SIZE, PROT_READ | PROT_WRITE);
    mprotect(g_studio.competitors[1].buffer, COMPETITIVE_BUFFER_SIZE, PROT_READ | PROT_WRITE);

    free(g_studio.competitors[0].buffer);
    free(g_studio.competitors[1].buffer);

    tsfi_zhao_shutdown();
    printf("[STUDIO] Unified Studio Model Closed.\n");
}
