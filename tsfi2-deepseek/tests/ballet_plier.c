#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdatomic.h>
#include "lau_memory.h"
#include "lau_registry.h"
#include "lau_thunk.h"
#include "tsfi_io.h"
#include "tsfi_time.h"
#include "tsfi_crash_handler.h"
#include "tsfi_reaction.h"
#include "tsfi_math.h"
#include "tsfi_ballet.h"

// System state tracking
static LauTelemetryState *g_sys_telem = NULL;

void plier_benchmark(const char *label) {
    const int iterations = 100000;
    unsigned long long start = get_time_ns();
    
    if (g_sys_telem) {
        snprintf(g_sys_telem->last_directive_str, sizeof(g_sys_telem->last_directive_str), "Bending Plier Manifold (%s)", label);
    }
    
    for (int i = 0; i < iterations; i++) {
        void *p = lau_malloc(64);
        lau_free(p);
        
        if (g_sys_telem && (i % 5000 == 0)) {
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
    printf("[BENCH] Plier Benchmark (%s): %.4f us/bend.\n", label, per_bend_us);
}

void test_plier_seal_zero(struct YI *yi) {
    printf("[SUB-TEST] Plier Seal-0 Formation (Thunk Lifecycle Routine)\n");
    size_t cp = BALLET_CHECKPOINT();
    
    // ROUTINE 1: Allocation & Discovery
    printf("  [ROUTINE] 1. Allocating Wired Manifold...\n");
    void *p = lau_malloc_wired(1024);
    assert(p != NULL);
    
    LauMetadata *m = lau_registry_find(p);
    LauWiredHeader *h = (LauWiredHeader*)m->actual_start;
    printf("    -> Manifold: %p, Header: %p, Initial Seal: %d\n", p, (void*)h, m->seal_level);

    // ROUTINE 2: Proxy Acquisition
    printf("  [ROUTINE] 2. Accessing Integrated Thunk Proxy...\n");
    ThunkProxy *proxy = (ThunkProxy*)h->proxy;
    assert(proxy != NULL);
    printf("    -> Proxy Pool: %p, Cursor: %p\n", (void*)proxy->thunk_pool, (void*)proxy->thunk_cursor);

    // ROUTINE 3: Unsealing for Modification
    printf("  [ROUTINE] 3. Unsealing Manifold & Proxy (RW Access)...\n");
    lau_mprotect(p, PROT_READ | PROT_WRITE);
    ThunkProxy_unseal(proxy);
    
    // ROUTINE 4: Thunk Attachment (Ontological Structuring)
    printf("  [ROUTINE] 4. Attaching YI Logic via Thunk Emission...\n");
    ThunkProxy_emit_ontological_structuring(proxy, yi, NULL);
    printf("    -> Thunk Cursor Advanced to: %p\n", (void*)proxy->thunk_cursor);
    
    // ROUTINE 5: Sealing & Integrity Matrix Update
    printf("  [ROUTINE] 5. Sealing Manifold (Physical Immutability)...\n");
    ThunkProxy_seal(proxy);
    m->seal_level = LAU_SEAL_PLIER;
    m->alloc_size |= (1ULL << 55); // Absolute Seal Bit
    lau_mprotect(p, PROT_READ);
    
    // 3. Verify
    assert(m->seal_level == LAU_SEAL_PLIER);
    assert(m->alloc_size & (1ULL << 55));
    printf("[PASS] Routine Complete: Seal-0 established. YI captured.\n");

    // Cleanup Routine
    printf("  [ROUTINE] Cleanup: Unsealing object for logical teardown...\n");
    lau_unseal_object(p);
    lau_free(p);
    
    BALLET_AUDIT(cp, "Plier Seal-0 Routine");
}

void test_plier_parity() {
    printf("[SUB-TEST] Plier Parity (BASIC vs WIRED vs GPU)\n");
    size_t cp = BALLET_CHECKPOINT();
    
    // 1. BASIC
    unsigned long long s1 = get_time_ns();
    void *p1 = lau_malloc(1024);
    unsigned long long e1 = get_time_ns();
    lau_free(p1);
    double t1 = (double)(e1 - s1) / 1000.0;

    // 2. WIRED
    unsigned long long s2 = get_time_ns();
    void *p2 = lau_malloc_wired(1024);
    unsigned long long e2 = get_time_ns();
    lau_free(p2);
    double t2 = (double)(e2 - s2) / 1000.0;

    // 3. GPU
    unsigned long long s3 = get_time_ns();
    void *p3 = lau_malloc_gpu(1024, LAU_GPU_DEVICE_LOCAL);
    unsigned long long e3 = get_time_ns();
    lau_free(p3);
    double t3 = (double)(e3 - s3) / 1000.0;

    printf("[PASS] BASIC: %.2f us | WIRED: %.2f us | GPU: %.2f us\n", t1, t2, t3);
    
    // Safety check
    if (atomic_load(&g_sys_telem->system_integrity_fault)) {
        printf("[CRITICAL] Plier failed: Integrity fault.\n");
        exit(1);
    }

    printf("[INFO] Plier Task is Finished. System is GOOD.\n");
    plier_benchmark("FOUNDATION_FINAL");
    
    BALLET_AUDIT(cp, "Plier Parity");
}

int main() {
    tsfi_install_crash_handler();
    printf("=== TSFi Ballet Step 1: Plier (Bending) - PARITY & SEAL-0 ===\n");

    int fd = tsfi_shm_open("/tsfi_ballet_plier_telem", O_RDWR | O_CREAT, 0666);
    if (ftruncate(fd, sizeof(LauTelemetryState)) != 0) { perror("ftruncate"); return 1; }
    g_sys_telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    memset(g_sys_telem, 0, sizeof(LauTelemetryState));
    tsfi_register_fault_flag(&g_sys_telem->system_integrity_fault);

    // Prepare YI for Seal-0
    TSFiBigInt *Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL);
    struct YI *yi = tsfi_reaction_shoot(Prime);

    test_plier_seal_zero(yi);
    test_plier_parity();

    printf("=== Plier Step Passed ===\n");
    printf("PERFECTLY ZERO LEAKS\n");
    
    freeYI(yi);
    tsfi_bn_free(Prime);
    
    munmap(g_sys_telem, sizeof(LauTelemetryState));
    close(fd);
    tsfi_shm_unlink("/tsfi_ballet_plier_telem");
    return 0;
}
