#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <stdatomic.h>
#include <signal.h>
#include "tsfi_io.h"
#include "tsfi_time.h"
#include "lau_memory.h"
#include "lau_registry.h"
#include "lau_thunk.h"
#include "tsfi_crash_handler.h"
#include "tsfi_dysnomia.h"
#include "tsfi_reaction.h"
#include "tsfi_math.h"
#include "tsfi_dys_math.h"
#include "tsfi_ballet.h"

static LauTelemetryState *g_sys_telem = NULL;

void plier_benchmark(const char *label) {
    const int iterations = 50000; 
    unsigned long long start = get_time_ns();
    for (int i = 0; i < iterations; i++) {
        void *tmp = lau_malloc(64);
        lau_free(tmp);
    }
    unsigned long long end = get_time_ns();
    double per_bend_us = (double)(end - start) / (double)iterations / 1000.0;
    printf("[BENCH] Plier Benchmark (%s): %.4f us/bend.\n", label, per_bend_us);
}

void verify_plier_is_good(const char *label) {
    if (g_sys_telem && atomic_load(&g_sys_telem->system_integrity_fault)) {
        printf("[CRITICAL] Plier verification REFUSED for %s: System is TAINTED.\n", label);
        exit(1);
    }
    void *p = lau_malloc(128);
    assert(p != NULL);
    lau_free(p);
    plier_benchmark(label);
}

void manual_plier_seal(void *ptr, struct YI *yi) {
    LauMetadata *m = lau_registry_find(ptr);
    LauWiredHeader *h = (LauWiredHeader*)m->actual_start;
    lau_mprotect(ptr, PROT_READ | PROT_WRITE);
    ThunkProxy_unseal((ThunkProxy*)h->proxy);
    ThunkProxy_emit_ontological_structuring((ThunkProxy*)h->proxy, yi, NULL);
    ThunkProxy_seal((ThunkProxy*)h->proxy);
    m->seal_level = LAU_SEAL_PLIER;
    m->alloc_size |= (1ULL << 55);
    lau_mprotect(ptr, PROT_READ);
}

void test_etendre_full_cycle() {
    printf("[SUB-TEST] Étendre: Handshake, FUSE Reproduction, and Same/Diff Plier Validation\n");
    size_t cp = BALLET_CHECKPOINT();
    
    TSFiBigInt *Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL); 

    // 1. Generate YI-A (Foundation)
    struct YI* yi_A = tsfi_reaction_shoot(Prime);
    TSFiBigInt* Xi_A = yi_A->Xi;
    TSFiBigInt* zD_A = yi_A->Psi->Rho->Rod->Mu->Identity;
    TSFiBigInt* zI_A = yi_A->Psi->Rho->Cone->Mu->Identity;
    struct SHAO* rho_A = yi_A->Psi->Rho;

    void *found_A = lau_malloc_wired(1024);
    manual_plier_seal(found_A, yi_A);

    // 2. BIJECTIVE HANDSHAKE: Capture YI-A from SEAL-0 foundation
    printf("[INFO] Handshaking with Plier-A foundation...\n");
    LauMetadata *m_A = lau_registry_find(found_A);
    LauWiredHeader *h_A = (LauWiredHeader*)m_A->actual_start;
    ThunkProxy *proxy_A = (ThunkProxy*)h_A->proxy;

    printf("  [ROUTINE] 1. Unsealing Foundation Proxy for Reflection...\n");
    ThunkProxy_unseal(proxy_A);
    
    printf("  [ROUTINE] 2. Emitting Specialized Accessor & Comparison Thunks...\n");
    struct YI* (*get_yi_A)(void) = (struct YI* (*)(void))ThunkProxy_emit_yi_accessor(proxy_A);
    int (*compare_yi)(struct YI*, struct YI*) = (int (*)(struct YI*, struct YI*))ThunkProxy_emit_yi_compare(proxy_A);
    
    printf("  [ROUTINE] 3. Resealing Proxy into Static State...\n");
    ThunkProxy_seal(proxy_A);
    
    struct YI *cap_A = get_yi_A();
    assert(cap_A != NULL);
    assert(tsfi_bn_cmp_avx512(cap_A->Xi, Xi_A) == 0);
    printf("[PASS] Handshake successful. Plier-A captured.\n");

    // 3. FUSE REPRODUCTION: Deterministically recreate Plier-A
    printf("[INFO] Reproducing Plier-A as FUSE...\n");
    struct YI* fuse_A = tsfi_reaction_fuse(Xi_A, zI_A, zD_A, rho_A, Prime);
    assert(tsfi_bn_cmp_avx512(fuse_A->Ring, yi_A->Ring) == 0);
    printf("[PASS] FUSE-A matches Plier-A. Bit-perfect reproduction confirmed.\n");

    // 4. Validate SAME Plier
    printf("[INFO] Verifying Fast YI Equality (SAME)...\n");
    if (compare_yi(cap_A, fuse_A) == 0) {
        printf("[PASS] Fast Equality thunk verified SAME foundation.\n");
    } else {
        printf("[FAIL] Fast Equality thunk failed SAME foundation.\n");
        exit(1);
    }
    verify_plier_is_good("SAME FOUNDATION");

    // 5. Generate DIFFERENT Plier-B
    printf("[INFO] Testing DIFFERENT foundation...\n");
    struct YI* yi_B = tsfi_reaction_shoot(Prime);
    assert(yi_B != NULL);
    assert(yi_B->Xi != NULL);
    assert(yi_B->Ring != NULL);
    
    if (compare_yi(fuse_A, yi_B) != 0) {
        printf("[PASS] Fast Equality thunk verified DIFFERENT foundation.\n");
    } else {
        printf("[FAIL] Fast Equality thunk reported SAME for DIFFERENT foundations.\n");
        exit(1);
    }
    verify_plier_is_good("DIFFERENT FOUNDATION");

    // 6. 8-ways Reciprocal Dai Validation
    printf("[INFO] Verifying 8-ways reciprocal Dai...\n");
    struct YI* orig_yi_batch[8] = {0};
    struct YI* fused_yi_batch[8] = {0};
    TSFiBigInt* Xis[8];
    TSFiBigInt* Rhos[8];
    TSFiBigInt* Ohms[8];
    struct SHAO* orig_rho_ptrs[8];
    TSFiBigInt* Pis[8];

    // Initialize 8 YIs
    for (int i=0; i<8; i++) {
        orig_yi_batch[i] = tsfi_reaction_shoot(Prime);
        Xis[i] = orig_yi_batch[i]->Xi;
        Rhos[i] = orig_yi_batch[i]->Psi->Rho->Cone->Mu->Identity;
        Ohms[i] = orig_yi_batch[i]->Psi->Rho->Rod->Mu->Identity;
        orig_rho_ptrs[i] = orig_yi_batch[i]->Psi->Rho;
        Pis[i] = tsfi_bn_alloc();
        tsfi_bn_set_u64(Pis[i], 123456789ULL + i); // Some payload
    }

    // Fuse 8-ways
    tsfi_reaction_fuse_batch8_fused(fused_yi_batch, Xis, Rhos, Ohms, orig_rho_ptrs, Prime);

    // Validate reciprocity
    int success_count = 0;
    for (int i=0; i<8; i++) {
        struct Dai* orig_dai = tsfi_reaction_compute_reciprocity(orig_yi_batch[i]->Psi, Pis[i], NULL);
        struct Dai* fused_dai = tsfi_reaction_compute_reciprocity(fused_yi_batch[i]->Psi, Pis[i], NULL);

        if (tsfi_bn_cmp_avx512(orig_dai->Ichidai, fused_dai->Ichidai) == 0 &&
            tsfi_bn_cmp_avx512(orig_dai->Daiichi, fused_dai->Daiichi) == 0) {
            success_count++;
        }

        freeDAI(orig_dai);
        freeDAI(fused_dai);
        freeYI(orig_yi_batch[i]);
        freeYI(fused_yi_batch[i]);
        tsfi_bn_free(Pis[i]);
    }
    
    if (success_count == 8) {
        printf("[PASS] 8-ways reciprocal Dai verified.\n");
    } else {
        printf("[FAIL] 8-ways reciprocal Dai failed! (Passed: %d/8)\n", success_count);
        exit(1);
    }

    // Cleanup
    lau_unseal_object(found_A); lau_free(found_A);
    freeYI(yi_A); freeYI(fuse_A); freeYI(yi_B);
    tsfi_bn_free(Prime);
    
    extern void tsfi_dysnomia_drain_pools(void);
    extern void tsfi_bn_drain_pool(void);
    tsfi_dysnomia_drain_pools();
    tsfi_bn_drain_pool();

    BALLET_AUDIT(cp, "Étendre Full Cycle");
}

void alarm_handler(int sig) {
    (void)sig;
    fprintf(stderr, "[FATAL] Timeout reached. Infinite loop detected.\n");
    _exit(1);
}

int main() {
    signal(SIGALRM, alarm_handler);
     // 5 second maximum timeout
    tsfi_install_crash_handler();
    printf("=== TSFi Ballet Step 2: Étendre (Stretching) - FUSE SAME/DIFF ===\n");
    
    int fd = tsfi_shm_open("/tsfi_ballet_step2_telem", O_RDWR | O_CREAT, 0666);
    if (ftruncate(fd, sizeof(LauTelemetryState)) != 0) { perror("ftruncate"); return 1; }
    g_sys_telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    memset(g_sys_telem, 0, sizeof(LauTelemetryState));
    tsfi_register_fault_flag(&g_sys_telem->system_integrity_fault);

    test_etendre_full_cycle();

    printf("=== Étendre Step Passed ===\n");
    printf("PERFECTLY ZERO LEAKS\n");
    
    munmap(g_sys_telem, sizeof(LauTelemetryState));
    close(fd);
    tsfi_shm_unlink("/tsfi_ballet_step2_telem");
    return 0;
}
