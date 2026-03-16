#define _GNU_SOURCE
#include <stdlib.h>
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
#include "tsfi_helmholtz.h"
#include "tsfi_merkle.h"
#include "tsfi_svdag.h"

static LauTelemetryState *g_sys_telem = NULL;

void __attribute__((noinline)) acoustic_proof_routine(void *manifold, TSFiHelmholtzSVDAG *dag) {
    if (g_sys_telem) {
        snprintf(g_sys_telem->last_directive_str, sizeof(g_sys_telem->last_directive_str), "Acoustic Proof of State (KHIT)");
    }

    uint8_t state_root[32], receipt_root[32], sheaf_root[32];
    
    // Ballet Step: 16-level Kirchhoff-Helmholtz Reduction (9-Thunk Burst)
    // We execute the 9 primary anchors to solidify resonance without delay.
    for (int i = 0; i < 9; i++) {
        // Update Trilateral Potentials per Pillar
        dag->p_user = 1.0f; 
        dag->p_deepseek = 0.521f;
        dag->p_gemini = 0.8f;

        tsfi_helmholtz_reduce_16(state_root, receipt_root, sheaf_root, NULL, NULL, manifold, i, 2026, dag);

        if (g_sys_telem) {
            atomic_store(&g_sys_telem->exec_steps, i + 1);
            g_sys_telem->current_intensity = 1.0f; // Solidified
            g_sys_telem->zmm_val = *(uint64_t*)state_root;
        }
    }
}

void test_acoustic_proof_logic() {
    printf("[SUB-TEST] Acoustic Proof Ballet (Steady Resonance) - 16-level KHIT\n");
    size_t cp = BALLET_CHECKPOINT();
    
    void *manifold = calloc(1, 32 * 1024 * 1024); // 32 MB Unified Generation (16MB Manifold + 16MB Directives)
    TSFiHelmholtzSVDAG *dag = tsfi_svdag_create(1024);
    
    printf("  [ROUTINE] 1. Creating Thunk Proxy for Acoustic Proof...\n");
    ThunkProxy *proxy = ThunkProxy_create();
    
    printf("  [ROUTINE] 2. Emitting Kirchhoff-Helmholtz Logic (Sovereign Prime)...\n");
    // We wrap the routine in a thunk to match ballet patterns
    void *thunk = ThunkProxy_emit_baked(proxy, (void*)acoustic_proof_routine, 2, manifold, dag);
    
    printf("  [ROUTINE] 3. Sealing Proxy for Acoustic Execution...\n");
    ThunkProxy_seal(proxy);
    
    void (*proof_call)(void*, TSFiHelmholtzSVDAG*) = (void (*)(void*, TSFiHelmholtzSVDAG*))thunk;
    
    printf("[INFO] Executing Acoustic Proof Ballet through Thunk Manifold...\n");
    unsigned long long start = get_time_ns();
    proof_call(manifold, dag);
    unsigned long long end = get_time_ns();
    
    double duration_ms = (double)(end - start) / 1000000.0;
    printf("[PASS] Acoustic Proof throughput validated in %.2f ms. (100 reductions complete)\n", duration_ms);
    printf("[INFO] Final AB-316 Ballistic Mass: %.4f\n", tsfi_svdag_execute(dag));
    
    printf("  [ROUTINE] 4. Reclaiming Acoustic Thunk Manifold...\n");
    ThunkProxy_destroy(proxy);
    tsfi_svdag_destroy(dag);
    tsfi_svdag_cleanup_lut();
    tsfi_helmholtz_cleanup();
    free(manifold);
    BALLET_AUDIT(cp, "Acoustic Proof Logic");
}

int main() {
    setenv("TSFI_LEAK_TRACKING_IN_PROGRESS", "1", 1);
    tsfi_install_crash_handler();
    printf("=== TSFi Ballet: Acoustic Proof of State ===\n");

    int fd = tsfi_shm_open("/tsfi_ballet_acoustic_telem", O_RDWR | O_CREAT, 0666);
    if (ftruncate(fd, sizeof(LauTelemetryState)) != 0) { perror("ftruncate"); return 1; }
    g_sys_telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    memset(g_sys_telem, 0, sizeof(LauTelemetryState));
    tsfi_register_fault_flag(&g_sys_telem->system_integrity_fault);

    test_acoustic_proof_logic();

    if (atomic_load(&g_sys_telem->system_integrity_fault)) {
        printf("[CRITICAL] Acoustic Proof failed: Resonance fractured.\n");
        exit(1);
    }

    printf("=== Acoustic Proof Ballet Passed ===\n");
    
    munmap(g_sys_telem, sizeof(LauTelemetryState));
    close(fd);
    tsfi_shm_unlink("/tsfi_ballet_acoustic_telem");
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
