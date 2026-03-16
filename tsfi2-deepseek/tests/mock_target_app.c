#include "lau_memory.h"
#include "lau_registry.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "tsfi_raw.h"

// Mock Logic Function
void run_mock_logic(const char *name) {
    lau_telemetry_record_exec(name);
    tsfi_raw_usleep(100000); // 100ms delay to allow Cockpit to catch the frame
}

int main(int argc, char **argv) {
    printf("[TARGET] Starting Mock App (PID: %d)...\n", getpid());
    
    if (argc > 1 && strcmp(argv[1], "crash") == 0) {
        run_mock_logic("PRE_CRASH");
        printf("[TARGET] Simulating CRASH...\n");
        int *p = NULL;
        *p = 42; // Segfault
    }
    
    if (argc > 1 && strcmp(argv[1], "spam") == 0) {
        printf("[TARGET] Simulating SPAM (Ring Buffer Wrap)...\n");
        for(int i=0; i<2000; i++) {
            // Allocate and free rapidly to generate events
            void *p = lau_malloc(16);
            lau_free(p);
            // Every 100 iterations, record exec to mark progress
            if (i % 100 == 0) {
                char buf[32];
                snprintf(buf, sizeof(buf), "SPAM_%d", i);
                lau_telemetry_record_exec(buf);
            }
        }
        run_mock_logic("SPAM_DONE");
        return 0;
    }
    
    // 1. Init
    run_mock_logic("BOOT_SEQUENCE");
    
    // 2. Allocation Phase
    printf("[TARGET] Allocating memory...\n");
    void *p1 = lau_malloc(1024 * 1024); // 1MB
    run_mock_logic("ALLOC_PHASE_1");
    
    void *p2 = lau_malloc_wired(512 * 1024); // 512KB Wired
    run_mock_logic("ALLOC_PHASE_2");
    
    // 3. Compute Phase
    printf("[TARGET] Simulating Compute...\n");
    for(int i=0; i<5; i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "COMPUTE_STEP_%d", i);
        run_mock_logic(buf);
    }
    
    // 4. Cleanup
    printf("[TARGET] Cleaning up...\n");
    lau_free(p1);
    run_mock_logic("FREE_PHASE_1");
    
    lau_free(p2);
    run_mock_logic("FREE_PHASE_2");
    
    run_mock_logic("SHUTDOWN_SEQUENCE");
    printf("[TARGET] Done.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}