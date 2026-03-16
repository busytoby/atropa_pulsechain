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
#include "tsfi_svdag.h"
#include "tsfi_ballet.h"

static LauTelemetryState *g_sys_telem = NULL;

void tourner_routine() {
    printf("[ROUTINE] Tourner: SVDAG Decision (Graph Traversal Routine)...\n");
    size_t cp = BALLET_CHECKPOINT();
    
    if (g_sys_telem) {
        snprintf(g_sys_telem->last_directive_str, sizeof(g_sys_telem->last_directive_str), "SVDAG Decision Turning");
    }
    
    const int iterations = 10000;
    const int nodes = 256;
    float cumulative_mass = 0.0f;
    
    unsigned long long start = get_time_ns();
    for (int i = 0; i < iterations; i++) {
        // ROUTINE: Graph Lifecycle
        TSFiHelmholtzSVDAG *svdag = tsfi_svdag_create(nodes);
        
        // Tourner: SVDAG turns. Decision resolves. Secret is set.
        // We simulate the decision resolution by executing the DAG
        float mass = tsfi_svdag_execute(svdag);
        cumulative_mass += (mass * 0.0001f); // Stabilize mass tracking
        
        tsfi_svdag_destroy(svdag);
        
        // Live Telemetry Streaming
        if (g_sys_telem && (i % 500 == 0)) {
            atomic_store(&g_sys_telem->exec_steps, i);
            g_sys_telem->current_intensity = (float)i / (float)iterations;
            
            uint64_t mass_encoded;
            memcpy(&mass_encoded, &cumulative_mass, sizeof(float));
            g_sys_telem->zmm_val = mass_encoded; // Mirror the bounding convergence
        }
    }
    unsigned long long end = get_time_ns();
    
    if (g_sys_telem) {
        atomic_store(&g_sys_telem->exec_steps, iterations);
        g_sys_telem->current_intensity = 1.0f;
    }

    double per_tour_us = (double)(end - start) / (double)iterations / 1000.0;
    printf("[PASS] Tourner Routine complete. %.2f us/tour (%d nodes). Mass resolved: %.4f\n", per_tour_us, nodes, cumulative_mass);
    
    extern void tsfi_svdag_cleanup_lut(void);
    tsfi_svdag_cleanup_lut();

    BALLET_AUDIT(cp, "Tourner Routine");
}

void test_tourner_logic() {
    printf("[SUB-TEST] Tourner (Turning) - SVDAG Decision\n");

    printf("[INFO] Turning SVDAG decision nodes...\n");
    tourner_routine();

    if (atomic_load(&g_sys_telem->system_integrity_fault)) {
        printf("[CRITICAL] Tourner failed: System integrity fractured.\n");
        exit(1);
    }

    printf("[INFO] Tourner Task is Finished. SVDAG turns. Decision resolves.\n");
}

int main() {
    tsfi_install_crash_handler();
    printf("=== TSFi Ballet Step 7: Tourner (Turning) ===\n");

    int fd = tsfi_shm_open("/tsfi_ballet_step7_telem", O_RDWR | O_CREAT, 0666);
    if (ftruncate(fd, sizeof(LauTelemetryState)) != 0) { perror("ftruncate"); return 1; }
    g_sys_telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    memset(g_sys_telem, 0, sizeof(LauTelemetryState));
    tsfi_register_fault_flag(&g_sys_telem->system_integrity_fault);

    test_tourner_logic();

    printf("=== Tourner Step Passed ===\n");
    printf("PERFECTLY ZERO LEAKS\n");
    
    munmap(g_sys_telem, sizeof(LauTelemetryState));
    close(fd);
    tsfi_shm_unlink("/tsfi_ballet_step7_telem");
    return 0;
}
