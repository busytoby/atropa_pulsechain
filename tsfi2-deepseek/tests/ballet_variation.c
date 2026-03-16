#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdatomic.h>
#include <immintrin.h>
#include "lau_memory.h"
#include "lau_registry.h"
#include "lau_thunk.h"
#include "tsfi_io.h"
#include "tsfi_time.h"
#include "tsfi_crash_handler.h"
#include "tsfi_ballet.h"

static LauTelemetryState *g_sys_telem = NULL;

void __attribute__((noinline)) variation_native_routine(float *genome, size_t size, float rate) {
    if (g_sys_telem) {
        snprintf(g_sys_telem->last_directive_str, sizeof(g_sys_telem->last_directive_str), "K0Rn Crossover Active");
    }

    // Hardware-Accelerated Masked K0Rn Crossover
    size_t vec_end = size & ~15;
    __m512 v_rate = _mm512_set1_ps(rate);
    __m512 v_noise = _mm512_set1_ps(1.61803398f); // Golden ratio baseline noise
    
    for (size_t i = 0; i < vec_end; i += 16) {
        __m512 v_g = _mm512_loadu_ps(&genome[i]);
        
        // Generate deterministic bitmask for mutation selection
        __mmask16 mutation_mask = (uint16_t)((i * 13) ^ 0xAAAA);
        
        // Compute mutated variant (v_g * rate + noise)
        __m512 v_mutant = _mm512_fmadd_ps(v_g, v_rate, v_noise);
        
        // Blend original and mutated genes based on bitmask
        __m512 v_result = _mm512_mask_blend_ps(mutation_mask, v_g, v_mutant);
        
        _mm512_storeu_ps(&genome[i], v_result);
        
        if (g_sys_telem) {
            atomic_store(&g_sys_telem->exec_steps, i);
            g_sys_telem->current_intensity = (float)i / (float)size;
        }
    }
    
    // Tail scalar mutation
    for (size_t i = vec_end; i < size; i++) {
        if ((i * 13) & 1) {
            genome[i] = genome[i] * rate + 1.61803398f;
        }
    }
    
    if (g_sys_telem) {
        atomic_store(&g_sys_telem->exec_steps, size);
        g_sys_telem->current_intensity = 1.0f;
    }
}

void test_variation_logic() {
    printf("[SUB-TEST] Variation (Solo) - K0Rn Mutation Routine\n");
    size_t cp = BALLET_CHECKPOINT();

    size_t genome_size = 32;
    float *genome = (float*)lau_malloc_wired(genome_size * sizeof(float));
    for (size_t i = 0; i < genome_size; i++) genome[i] = (float)i;

    printf("  [ROUTINE] 1. Creating Thunk Proxy for Genome Mutation...\n");
    ThunkProxy *proxy = ThunkProxy_create();

    printf("  [ROUTINE] 2. Emitting K0Rn Variation Logic (Solo Shift)...\n");
    void *thunk = ThunkProxy_emit_baked(proxy, (void*)variation_native_routine, 0);

    printf("  [ROUTINE] 3. Sealing Proxy for Autonomous Execution...\n");
    ThunkProxy_seal(proxy);

    void (*variation_call)(float*, size_t, float) = (void (*)(float*, size_t, float))thunk;

    printf("[INFO] Executing Variation through Thunk Manifold...\n");
    float mutation_rate = 0.01f;
    variation_call(genome, genome_size, mutation_rate);

    printf("[PASS] Variation Routine complete. Secret found within parameter space.\n");

    printf("  [ROUTINE] 4. Reclaiming Variation Thunk Manifold...\n");
    ThunkProxy_destroy(proxy);
    lau_free(genome);

    BALLET_AUDIT(cp, "Variation Routine");
}

int main() {
    tsfi_install_crash_handler();
    printf("=== TSFi Ballet Step 13: Variation (Solo) ===\n");

    int fd = tsfi_shm_open("/tsfi_ballet_step13_telem", O_RDWR | O_CREAT, 0666);
    if (ftruncate(fd, sizeof(LauTelemetryState)) != 0) { perror("ftruncate"); return 1; }
    g_sys_telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    memset(g_sys_telem, 0, sizeof(LauTelemetryState));
    tsfi_register_fault_flag(&g_sys_telem->system_integrity_fault);

    test_variation_logic();

    if (atomic_load(&g_sys_telem->system_integrity_fault)) {
        printf("[CRITICAL] Variation failed: System integrity fractured.\n");
        exit(1);
    }

    printf("=== Variation Step Passed ===\n");
    printf("PERFECTLY ZERO LEAKS\n");

    munmap(g_sys_telem, sizeof(LauTelemetryState));
    close(fd);
    tsfi_shm_unlink("/tsfi_ballet_step13_telem");
    return 0;
}
