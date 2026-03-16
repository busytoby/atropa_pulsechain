#define _POSIX_C_SOURCE 200809L
#include "tsfi_types.h"
#include "lau_memory.h"
#include "lau_thunk.h"
#include "tsfi_zmm_rpc.h"
#include "tsfi_io.h"
#include "tsfi_k0rn_ops.h"
#include "tsfi_k0rn_ir.h"
#include "tsfi_wiring.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

extern _Atomic size_t g_alloc_count;
extern _Atomic size_t g_active_allocs;

void dump_bytes(const char *name, void *ptr, size_t len) {
    (void)name;
    (void)len;
    uint8_t *b = (uint8_t*)ptr;
    (void)b;
}

void test_basic_allocators() {
    void *p1 = lau_malloc(128);
    void *p2 = lau_malloc_wired(256);
    void *p3 = lau_malloc_wired(512);
    void *p4 = lau_malloc_gpu(512, 1);
    
    *(char*)p1 = 1; *(char*)p2 = 1; *(char*)p3 = 1; *(char*)p4 = 1;
    lau_free(p1); lau_free(p2); lau_free(p3); lau_free(p4);
}

void test_thunk_pool_matrix() {
    size_t pg = sysconf(_SC_PAGESIZE);
    void *thunk_pool = lau_memalign(pg, pg);
    if (thunk_pool) {
        lau_mprotect(thunk_pool, PROT_READ | PROT_WRITE | PROT_EXEC);
        lau_free(thunk_pool);
    }
}

void test_legacy_tracker() {
    void *t1 = lau_malloc_loc(128, "tracker_test", 101);
    void *t2 = lau_memalign_wired_loc(512, 256, "tracker_test", 102);
    void *t3 = lau_memalign_loc(4096, 512, "tracker_test", 103);
    *(char*)t1 = 1; *(char*)t2 = 1; *(char*)t3 = 1;
    lau_free(t1); lau_free(t2); lau_free(t3);
}

void test_k0rn_codegen() {
    K0RnModule* ast_m = k0rn_module_create();
    k0rn_emit(ast_m, K0RN_IR_LOAD_STREAM, 0, 0, 0, 0);
    k0rn_emit(ast_m, K0RN_IR_LOAD_UNIFORM, 1, 0, 0, 0);
    k0rn_emit(ast_m, K0RN_IR_LOAD_UNIFORM, 2, 0, 0, 1);
    k0rn_emit(ast_m, K0RN_IR_MUL, 3, 0, 1, 0);
    k0rn_emit(ast_m, K0RN_IR_SUB, 0, 3, 2, 0);
    k0rn_emit(ast_m, K0RN_IR_STORE_STREAM, 0, 0, 0, 0);
    void* ast_thunk = NULL;
    uint32_t* ast_spv = NULL;
    size_t ast_spv_size = 0;
    k0rn_compile_dual(ast_m, &ast_thunk, &ast_spv, &ast_spv_size);
    k0rn_module_destroy(ast_m);
    
    if (ast_thunk) {
        ThunkProxy *proxy = (ThunkProxy*)ast_thunk;
        ThunkProxy_unseal(proxy);
        ThunkProxy_destroy(proxy);
    }
    if (ast_spv) lau_free(ast_spv);
}

void test_motzkin_locator() {
    char output[1024];
    tsfi_zmm_rpc_dispatch(NULL, "{\"method\": \"math.motzkin\", \"id\": 110}", output, sizeof(output));
}

void test_k0rn_bootstrap() {
    K0RnStream *sA = tsfi_k0rn_compile_char(0x41);
    if (sA) {
        if (sA->ops) lau_free(sA->ops);
        lau_free(sA);
    }
    K0RnStream *sB = tsfi_k0rn_compile_char(0x42);
    if (sB) {
        if (sB->ops) lau_free(sB->ops);
        lau_free(sB);
    }
}

void test_wavefront_proxies() {
    WaveSystem *ws = tsfi_create_system();
    WavefrontContext *wf = tsfi_create_wavefront(ws);
    
    LauSystemHeader *wh = (LauSystemHeader *)((char *)wf - offsetof(LauSystemHeader, payload));
    if (wh->resonance_as_status) { lau_free(wh->resonance_as_status); wh->resonance_as_status = NULL; }
    lau_free(wf);
    
    lau_final_cleanup(ws, -1);
}

typedef struct {
    const char *name;
    void (*func)();
    size_t allocs;
    long leaks;
} DiagnosticTest;

int compare_tests(const void *a, const void *b) {
    DiagnosticTest *ta = (DiagnosticTest *)a;
    DiagnosticTest *tb = (DiagnosticTest *)b;
    if (ta->leaks != tb->leaks) {
        return (ta->leaks > tb->leaks) - (ta->leaks < tb->leaks);
    }
    return (ta->allocs > tb->allocs) - (ta->allocs < tb->allocs);
}

int main() {
    printf("[BENCH] Initializing Unified Geometric Leak Reporter...\n\n");
    
    DiagnosticTest tests[] = {
        {"Basic geometrieZ", test_basic_allocators, 0, 0},
        {"Thunk Pool Matrix", test_thunk_pool_matrix, 0, 0},
        {"Legacy Tracking Topology", test_legacy_tracker, 0, 0},
        {"Dual-Backend K0Rn Codegen", test_k0rn_codegen, 0, 0},
        {"Motzkin SIMD Locator", test_motzkin_locator, 0, 0},
        {"K0Rn Bootstrap AST", test_k0rn_bootstrap, 0, 0},
        {"Wavefront Execution Proxies", test_wavefront_proxies, 0, 0}
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int failure_count = 0;
    
    for (int i = 0; i < num_tests; i++) {
        size_t start_allocs = atomic_load(&g_alloc_count);
        size_t start_active = atomic_load(&g_active_allocs);
        
        tests[i].func();
        
        size_t end_allocs = atomic_load(&g_alloc_count);
        size_t end_active = atomic_load(&g_active_allocs);
        
        tests[i].allocs = end_allocs - start_allocs;
        tests[i].leaks = (long)end_active - (long)start_active;
        if (tests[i].leaks < 0) tests[i].leaks = 0; // Normalize cache unmapping
    }
    
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    
    qsort(tests, num_tests, sizeof(DiagnosticTest), compare_tests);
    
    printf("==========================================\n");
    printf("[BENCH] FINAL TOPOLOGICAL REVIEW (SORTED)\n");
    printf("==========================================\n");
    
    for (int i = 0; i < num_tests; i++) {
        if (tests[i].leaks == 0) {
            printf("[PASS] %s: %zu allocations, 0 leaks.\n", tests[i].name, tests[i].allocs);
        } else {
            printf("[FAIL] %s: %zu allocations, %ld CRITICAL LEAKS!\n", tests[i].name, tests[i].allocs, tests[i].leaks);
            failure_count++;
        }
    }
    
    printf("\n");
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    
    if (failure_count == 0) {
        printf("\n[BENCH] SUCCESS: The overarching geometry is mathematically leak-free.\n");
        
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
    } else {
        printf("\n[BENCH] CRITICAL FAILURE: %d sub-systems actively leaking matrix state.\n", failure_count);
        return 1;
    }
}