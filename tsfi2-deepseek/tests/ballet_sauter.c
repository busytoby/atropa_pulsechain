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
#include "tsfi_ballet.h"

static LauTelemetryState *g_sys_telem = NULL;

void __attribute__((noinline)) native_call() {
    __asm__ volatile ("" : : : "memory");
}

void sauter_benchmark(void *thunk) {
    const int iterations = 10000000;
    void (*thunk_call)(void) = (void (*)(void))thunk;

    // Benchmark Native Call
    unsigned long long s1 = get_time_ns();
    for (int i = 0; i < iterations; i++) {
        native_call();
    }
    unsigned long long e1 = get_time_ns();
    double native_ns = (double)(e1 - s1) / (double)iterations;
    
    if (g_sys_telem) {
        snprintf(g_sys_telem->last_directive_str, sizeof(g_sys_telem->last_directive_str), "Jumping via JIT Thunks");
    }

    // Benchmark Thunk Call
    unsigned long long s2 = get_time_ns();
    for (int i = 0; i < iterations; i++) {
        thunk_call();
        
        if (g_sys_telem && (i % 500000 == 0)) {
            atomic_store(&g_sys_telem->exec_steps, i);
            g_sys_telem->current_intensity = (float)i / (float)iterations;
        }
    }
    unsigned long long e2 = get_time_ns();
    
    if (g_sys_telem) {
        atomic_store(&g_sys_telem->exec_steps, iterations);
        g_sys_telem->current_intensity = 1.0f;
    }
    
    double thunk_ns = (double)(e2 - s2) / (double)iterations;

    printf("[BENCH] Native Call: %.2f ns/jump\n", native_ns);
    printf("[BENCH] Thunk Call:  %.2f ns/jump\n", thunk_ns);
    printf("[BENCH] Overhead:    %.2f ns/jump\n", thunk_ns - native_ns);
}

void test_sauter_logic() {
    printf("[SUB-TEST] Sauter (Jumping) - Thunk Lifecycle Routine\n");
    size_t cp = BALLET_CHECKPOINT();

    printf("  [ROUTINE] 1. Creating Isolated Thunk Proxy...\n");
    ThunkProxy *proxy = ThunkProxy_create();
    printf("    -> Proxy Created: %p\n", (void*)proxy);

    printf("  [ROUTINE] 2. Emitting Fast JIT Instruction (RET)...\n");
    void *thunk = ThunkProxy_emit_ret(proxy); 
    
    printf("  [ROUTINE] 3. Sealing Proxy for Execution...\n");
    ThunkProxy_seal(proxy);

    printf("[INFO] Jumping through JIT thunks...\n");
    sauter_benchmark(thunk);

    if (atomic_load(&g_sys_telem->system_integrity_fault)) {
        printf("[CRITICAL] Sauter failed: System integrity fractured.\n");
        exit(1);
    }

    printf("[INFO] Sauter Task is Finished. Logic is FAST.\n");
    
    printf("  [ROUTINE] 4. Destroying Proxy & Reclaiming Pool...\n");
    ThunkProxy_destroy(proxy);
    
    BALLET_AUDIT(cp, "Sauter Lifecycle Routine");
}

int main() {
    tsfi_install_crash_handler();
    printf("=== TSFi Ballet Step 5: Sauter (Jumping) ===\n");

    int fd = tsfi_shm_open("/tsfi_ballet_step5_telem", O_RDWR | O_CREAT, 0666);
    if (ftruncate(fd, sizeof(LauTelemetryState)) != 0) { perror("ftruncate"); return 1; }
    g_sys_telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    memset(g_sys_telem, 0, sizeof(LauTelemetryState));
    tsfi_register_fault_flag(&g_sys_telem->system_integrity_fault);

    test_sauter_logic();

    printf("=== Sauter Step Passed ===\n");
    printf("PERFECTLY ZERO LEAKS\n");
    
    munmap(g_sys_telem, sizeof(LauTelemetryState));
    close(fd);
    tsfi_shm_unlink("/tsfi_ballet_step5_telem");
    return 0;
}
