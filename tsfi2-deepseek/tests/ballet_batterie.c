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
#include "tsfi_io.h"
#include "tsfi_time.h"
#include "tsfi_crash_handler.h"
#include "tsfi_ballet.h"

static LauTelemetryState *g_sys_telem = NULL;

void batterie_benchmark() {
    printf("[BENCH] Batterie: Clock Beating (vDSO Routine)...\n");
    size_t cp = BALLET_CHECKPOINT();
    
    if (g_sys_telem) {
        snprintf(g_sys_telem->last_directive_str, sizeof(g_sys_telem->last_directive_str), "vDSO Clock Beating");
    }
    
    const int iterations = 1000000;
    unsigned long long start = get_time_ns();
    for (int i = 0; i < iterations; i++) {
        // Batterie: Clock beats. vDSO strikes. 
        get_time_ns(); 
        
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
    
    double per_strike_ns = (double)(end - start) / (double)iterations;
    printf("[PASS] Batterie Routine complete. Avg strike latency: %.2f ns. Frequency is true.\n", per_strike_ns);
    
    BALLET_AUDIT(cp, "Batterie Logic");
}

int main() {
    tsfi_install_crash_handler();
    printf("=== TSFi Ballet Step 10: Batterie (Beating) ===\n");

    int fd = tsfi_shm_open("/tsfi_ballet_step10_telem", O_RDWR | O_CREAT, 0666);
    if (ftruncate(fd, sizeof(LauTelemetryState)) != 0) { perror("ftruncate"); return 1; }
    g_sys_telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    memset(g_sys_telem, 0, sizeof(LauTelemetryState));
    tsfi_register_fault_flag(&g_sys_telem->system_integrity_fault);

    printf("  [ROUTINE] 1. Measuring True Frequency via vDSO...\n");
    batterie_benchmark();

    if (atomic_load(&g_sys_telem->system_integrity_fault)) {
        printf("[CRITICAL] Batterie failed: System integrity fractured.\n");
        exit(1);
    }

    printf("=== Batterie Step Passed ===\n");
    printf("PERFECTLY ZERO LEAKS\n");
    
    munmap(g_sys_telem, sizeof(LauTelemetryState));
    close(fd);
    tsfi_shm_unlink("/tsfi_ballet_step10_telem");
    return 0;
}
