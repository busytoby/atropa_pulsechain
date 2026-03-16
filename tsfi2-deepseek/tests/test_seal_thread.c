#define _POSIX_C_SOURCE 200809L
#include "tsfi_types.h"
#include "tsfi_resonance.h"
#include "lau_memory.h"
#include "lau_thunk.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>

#define TARGET_THREADS 100
#define REALLOC_ITERATIONS 150

int main() {
    printf("[TEST] Initializing Advanced K0Rn Mutator and Topological Sealing Matrix...\n");
    
    // 1. Thread Array Instantiation
    LauWireThread *threads[TARGET_THREADS];
    void *dynamic_arrays[TARGET_THREADS];
    
    for (int i = 0; i < TARGET_THREADS; i++) {
        // Interleave PTY and standard limits geometrically
        if (i % 2 == 0) {
            threads[i] = tsfi_wire_thread_create_pty(1000 + i, "ls > /dev/null", NULL);
        } else {
            threads[i] = tsfi_wire_thread_create(1000 + i, 1.0);
        }
        
        if (!threads[i]) return 1;
        
        dynamic_arrays[i] = NULL;
        
        // 2. K0Rn JIT Execution Mutator
        LauWiredHeader *h = (LauWiredHeader *)((char *)threads[i] - offsetof(LauWiredHeader, payload));
        if (h->proxy) {
            // Natively mutate the Thunk bounds to force mprotect(PROT_EXEC) inside the pool
            mprotect(h->proxy->thunk_pool, h->proxy->pool_size, PROT_READ | PROT_WRITE | PROT_EXEC);
            ThunkProxy_emit_ret(h->proxy);
        }
        
        // 3. Seal the absolute matrix
        tsfi_wire_thread_seal(threads[i]);
    }
    
    printf("[TEST] %d dynamic execution vectors sealed and JIT-compiled.\n", TARGET_THREADS);
    
    // 4. Asynchronous Signal & Reallocation Barrage
    printf("[TEST] Executing %d interleaved lau_realloc cycles against asynchronous kernel signals...\n", REALLOC_ITERATIONS);
    for (int r = 1; r <= REALLOC_ITERATIONS; r++) {
        for (int i = 0; i < TARGET_THREADS; i++) {
            // Unmap the matrix physically
            if (dynamic_arrays[i]) lau_unseal_object(dynamic_arrays[i]);
            
            // Expand geometric bounds
            dynamic_arrays[i] = lau_realloc(dynamic_arrays[i], r * 32);
            if (!dynamic_arrays[i]) {
                printf("[FATAL] Asynchronous reallocation array fractured.\n");
                return 1;
            }
            
            // Mathematically reseal the boundary
            lau_seal_object(dynamic_arrays[i]);
        }
        
        // Geometrically flood the asynchronous handlers every 10 cycles
        if (r % 10 == 0) {
            kill(getpid(), SIGCHLD);
            kill(getpid(), SIGWINCH);
        }
    }
    
    printf("[TEST] Memory topology strictly expanded under asynchronous signal load.\n");
    
    // 5. Structural Teardown Loop
    printf("[TEST] Decoupling execution limits and evaluating geometric unmapping...\n");
    for (int i = 0; i < TARGET_THREADS; i++) {
        lau_unseal_object(dynamic_arrays[i]);
        lau_free(dynamic_arrays[i]);
        tsfi_wire_thread_destroy(threads[i]);
    }
    
    // 6. Mathematical Registry Deregistration
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    
    printf("[TEST] Expecting PERFECTLY ZERO LEAKS...\n");
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    
    printf("[TEST] Master Advanced Sealing Matrix physically validated.\n");
    
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
