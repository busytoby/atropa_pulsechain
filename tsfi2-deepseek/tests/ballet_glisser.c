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

typedef struct {
    float data[16];
} __attribute__((aligned(64))) GliderContext;

void glisser_benchmark(ThunkProxy *proxy, void *thunk, GliderContext *ctx1, GliderContext *ctx2) {
    (void)proxy;
    const int iterations = 1000000;
    void (*glider_call)(void) = (void (*)(void))thunk;
    
    if (g_sys_telem) {
        snprintf(g_sys_telem->last_directive_str, sizeof(g_sys_telem->last_directive_str), "Zero-Copy Context Gliding");
    }

    unsigned long long start = get_time_ns();
    for (int i = 0; i < iterations; i++) {
        // ROUTINE: The R15 Glide (Zero-Copy Switch)
        __asm__ volatile ("movq %0, %%r15" : : "r"(ctx1) : "r15");
        glider_call();
        __asm__ volatile ("movq %0, %%r15" : : "r"(ctx2) : "r15");
        glider_call();
        
        if (g_sys_telem && (i % 50000 == 0)) {
            atomic_store(&g_sys_telem->exec_steps, i);
            g_sys_telem->current_intensity = (float)i / (float)iterations;
        }
    }
    unsigned long long end = get_time_ns();
    
    if (g_sys_telem) {
        atomic_store(&g_sys_telem->exec_steps, iterations);
        g_sys_telem->current_intensity = 1.0f;
    }
    
    double total_switches = (double)iterations * 2.0;
    double per_glide_ns = (double)(end - start) / total_switches;
    printf("[BENCH] Glisser Benchmark: %.2f ns/glide (Zero-Copy).\n", per_glide_ns);
}

void test_glisser_logic() {
    printf("[SUB-TEST] Glisser (Gliding) - Thunk Context Routine\n");
    size_t cp = BALLET_CHECKPOINT();

    printf("  [ROUTINE] 1. Creating Context-Aware Thunk Proxy...\n");
    ThunkProxy *proxy = ThunkProxy_create();

    printf("  [ROUTINE] 2. Emitting R15-Relative Waveform Thunk...\n");
    // Thunk loads from [r15 + 0] into ZMM0
    void *thunk = ThunkProxy_emit_vgpr_load_relative(proxy, 0, 0); 
    
    printf("  [ROUTINE] 3. Sealing Proxy for High-Speed Gliding...\n");
    ThunkProxy_seal(proxy);

    GliderContext *ctx1 = (GliderContext*)lau_memalign(64, sizeof(GliderContext));
    GliderContext *ctx2 = (GliderContext*)lau_memalign(64, sizeof(GliderContext));
    memset(ctx1, 0, sizeof(GliderContext));
    memset(ctx2, 0, sizeof(GliderContext));

    printf("[INFO] Gliding across memory contexts...\n");
    glisser_benchmark(proxy, thunk, ctx1, ctx2);

    if (atomic_load(&g_sys_telem->system_integrity_fault)) {
        printf("[CRITICAL] Glisser failed: System integrity fractured.\n");
        exit(1);
    }

    printf("[INFO] Glisser Task is Finished. Friction is ZERO.\n");
    
    printf("  [ROUTINE] 4. Reclaiming Contexts & Destroying Proxy...\n");
    lau_free(ctx1);
    lau_free(ctx2);
    ThunkProxy_destroy(proxy);
    
    BALLET_AUDIT(cp, "Glisser Context Routine");
}

int main() {
    tsfi_install_crash_handler();
    printf("=== TSFi Ballet Step 4: Glisser (Gliding) ===\n");

    int fd = tsfi_shm_open("/tsfi_ballet_step4_telem", O_RDWR | O_CREAT, 0666);
    if (ftruncate(fd, sizeof(LauTelemetryState)) != 0) { perror("ftruncate"); return 1; }
    g_sys_telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    memset(g_sys_telem, 0, sizeof(LauTelemetryState));
    tsfi_register_fault_flag(&g_sys_telem->system_integrity_fault);

    test_glisser_logic();

    printf("=== Glisser Step Passed ===\n");
    printf("PERFECTLY ZERO LEAKS\n");
    
    munmap(g_sys_telem, sizeof(LauTelemetryState));
    close(fd);
    tsfi_shm_unlink("/tsfi_ballet_step4_telem");
    return 0;
}
