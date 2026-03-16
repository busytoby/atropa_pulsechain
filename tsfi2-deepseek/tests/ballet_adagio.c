#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdatomic.h>
#include <pthread.h>
#include "lau_memory.h"
#include "lau_registry.h"
#include "lau_thunk.h"
#include "tsfi_io.h"
#include "tsfi_time.h"
#include "tsfi_crash_handler.h"
#include "tsfi_ballet.h"

static LauTelemetryState *g_sys_telem = NULL;

void __attribute__((noinline)) adagio_native_routine(void *target, uint32_t mask, int timeout) {
    void *addrs[1] = { target };
    uint32_t masks[1] = { mask };
    
    if (g_sys_telem) {
        snprintf(g_sys_telem->last_directive_str, sizeof(g_sys_telem->last_directive_str), "Adagio Asynchronous Wait");
        g_sys_telem->current_intensity = 0.5f;
    }

    tsfi_monitor(NULL, 0, addrs, masks, 1, timeout);

    if (g_sys_telem) {
        g_sys_telem->current_intensity = 1.0f;
    }
}

void* async_resolution_thread(void *arg) {
    _Atomic uint32_t *target = (_Atomic uint32_t*)arg;
    struct timespec req = {0, 5000000}; // 5ms
    nanosleep(&req, NULL);
    atomic_store(target, 0xFFFFFFFF); // Resolve the wavefront
    return NULL;
}

void test_adagio_logic() {
    printf("[SUB-TEST] Adagio (Slow Movement) - Thunk Wait Routine\n");
    size_t cp = BALLET_CHECKPOINT();
    
    // Create a memory wavefront to monitor
    uint32_t *target = (uint32_t*)lau_malloc_wired(sizeof(uint32_t));
    atomic_store((_Atomic uint32_t*)target, 0);
    
    printf("  [ROUTINE] 1. Creating Thunk Proxy for Universal Wait...\n");
    ThunkProxy *proxy = ThunkProxy_create();
    
    printf("  [ROUTINE] 2. Emitting Adagio Logic (Steady State Monitor)...\n");
    void *thunk = ThunkProxy_emit_baked(proxy, (void*)adagio_native_routine, 0);
    
    printf("  [ROUTINE] 3. Sealing Proxy for Autonomous Execution...\n");
    ThunkProxy_seal(proxy);
    
    void (*adagio_call)(void*, uint32_t, int) = (void (*)(void*, uint32_t, int))thunk;
    
    printf("[INFO] Monitoring Steady State through Thunk Manifold...\n");
    
    pthread_t thread;
    pthread_create(&thread, NULL, async_resolution_thread, target);
    
    unsigned long long start = get_time_ns();
    // Monitor for 50ms (expected to be resolved by thread after ~5ms)
    adagio_call(target, 0xFFFFFFFF, 50);
    unsigned long long end = get_time_ns();
    
    pthread_join(thread, NULL);
    
    double duration_ms = (double)(end - start) / 1000000.0;
    printf("[PASS] Adagio asynchronous resolution confirmed after %.2f ms.\n", duration_ms);
    assert(duration_ms < 40.0); // Should resolve much faster than 50ms timeout
    assert(atomic_load((_Atomic uint32_t*)target) == 0xFFFFFFFF);
    
    printf("  [ROUTINE] 4. Reclaiming Adagio Thunk Manifold...\n");
    ThunkProxy_destroy(proxy);
    lau_free(target);
    
    extern void tsfi_io_cleanup(void);
    tsfi_io_cleanup();
    
    BALLET_AUDIT(cp, "Adagio Routine");
}

int main() {
    tsfi_install_crash_handler();
    printf("=== TSFi Ballet Step 8: Adagio (Slow Movement) ===\n");

    int fd = tsfi_shm_open("/tsfi_ballet_step8_telem", O_RDWR | O_CREAT, 0666);
    if (ftruncate(fd, sizeof(LauTelemetryState)) != 0) { perror("ftruncate"); return 1; }
    g_sys_telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    memset(g_sys_telem, 0, sizeof(LauTelemetryState));
    tsfi_register_fault_flag(&g_sys_telem->system_integrity_fault);

    test_adagio_logic();

    if (atomic_load(&g_sys_telem->system_integrity_fault)) {
        printf("[CRITICAL] Adagio failed: System integrity fractured.\n");
        exit(1);
    }

    printf("=== Adagio Step Passed ===\n");
    printf("PERFECTLY ZERO LEAKS\n");
    
    munmap(g_sys_telem, sizeof(LauTelemetryState));
    close(fd);
    tsfi_shm_unlink("/tsfi_ballet_step8_telem");
    return 0;
}
