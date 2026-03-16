#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <immintrin.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <stdalign.h>
#include "lau_memory.h"
#include "lau_registry.h"
#include "lau_thunk.h"
#include "tsfi_io.h"
#include "tsfi_time.h"
#include "tsfi_crash_handler.h"
#include "tsfi_ballet.h"

static LauTelemetryState *g_sys_telem = NULL;

void plier_benchmark() {
    const int iterations = 50000; 
    unsigned long long start = get_time_ns();
    
    if (g_sys_telem) {
        snprintf(g_sys_telem->last_directive_str, sizeof(g_sys_telem->last_directive_str), "Validating Plier Foundation");
    }
    
    for (int i = 0; i < iterations; i++) {
        void *tmp = lau_malloc(64);
        lau_free(tmp);
        
        if (g_sys_telem && (i % 2500 == 0)) {
            atomic_store(&g_sys_telem->exec_steps, i);
            g_sys_telem->current_intensity = (float)i / (float)iterations;
        }
    }
    unsigned long long end = get_time_ns();
    
    if (g_sys_telem) {
        atomic_store(&g_sys_telem->exec_steps, iterations);
        g_sys_telem->current_intensity = 1.0f;
    }
    
    double per_bend_us = (double)(end - start) / (double)iterations / 1000.0;
    printf("[BENCH] Plier Benchmark: %.4f us/bend.\n", per_bend_us);
}

void __attribute__((noinline)) zmm_alignment_check() {
    register uintptr_t rsp __asm__("rsp");
    // Upon entry to a function, RSP is 8 bytes off a 16-byte (or 64-byte) boundary 
    // because the 'call' instruction pushed the return address.
    if ((rsp + 8) & 63) {
        printf("[FAIL] Stack pointer %p is NOT correctly aligned (rsp+8 %% 64 != 0)!\n", (void*)rsp);
        exit(1);
    }
    
    float data[16] __attribute__((aligned(64)));
    memset(data, 0, sizeof(data));
    __m512 zero = _mm512_setzero_ps();
    _mm512_store_ps(data, zero); 
    
    printf("[PASS] Stack verified at %p (Aligned). ZMM operation successful.\n", (void*)rsp);
}

void test_relever_logic() {
    printf("[SUB-TEST] Relever (Stack Alignment) - Hardware Safety Routine\n");
    size_t cp = BALLET_CHECKPOINT();
    
    printf("  [ROUTINE] 1. Creating Thunk Proxy for Alignment...\n");
    ThunkProxy *p = ThunkProxy_create();
    
    printf("  [ROUTINE] 2. Emitting Stack Re-alignment (Relever) Instructions...\n");
    void *aligned_entry = ThunkProxy_emit_relever_alignment(p, (void*)zmm_alignment_check);
    
    printf("  [ROUTINE] 3. Sealing Proxy for AVX-512 Execution...\n");
    ThunkProxy_seal(p);

    void (*relever_call)(void) = (void (*)(void))aligned_entry;

    printf("[INFO] Rising to hardware-safe alignment...\n");
    relever_call();

    if (atomic_load(&g_sys_telem->system_integrity_fault)) {
        printf("[CRITICAL] Relever failed: System integrity fractured.\n");
        exit(1);
    }

    printf("[INFO] Relever Task is Finished. System is GOOD.\n");

    printf("\n=== FINAL VALIDATION: Validating Plier foundation ===\n");
    plier_benchmark();

    printf("  [ROUTINE] 4. Reclaiming Relever Thunk Manifold...\n");
    ThunkProxy_destroy(p);
    
    BALLET_AUDIT(cp, "Relever Logic");
}

int main() {
    tsfi_install_crash_handler();
    printf("=== TSFi Ballet Step 3: Relever (Rising) ===\n");
    
    int fd = tsfi_shm_open("/tsfi_ballet_step3_telem", O_RDWR | O_CREAT, 0666);
    if (ftruncate(fd, sizeof(LauTelemetryState)) != 0) { perror("ftruncate"); return 1; }
    g_sys_telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    memset(g_sys_telem, 0, sizeof(LauTelemetryState));
    tsfi_register_fault_flag(&g_sys_telem->system_integrity_fault);

    test_relever_logic();

    printf("=== Relever Step Passed ===\n");
    printf("PERFECTLY ZERO LEAKS\n");
    
    munmap(g_sys_telem, sizeof(LauTelemetryState));
    close(fd);
    tsfi_shm_unlink("/tsfi_ballet_step3_telem");
    return 0;
}
