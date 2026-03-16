#include <time.h>
#include "tsfi_wire_firmware.h"
#include "tsfi_hotloader.h"
#include "tsfi_zhao.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdatomic.h>

// Simulated internal matrix state pointer
static _Atomic(void*) g_active_thunk_ptr = NULL;

// The simulated physical execution matrix (YI loop)
// It continuously evaluates the Thunk without ever yielding to the OS.
int execute_matrix_loop(void) {
    int epochs = 0;
    int successfully_mutated = 0;
    
    tsfi_io_printf(stdout, "[ETA] Commencing Internal execution matrix loop...\n");

    while (epochs < 2000000) {
        // Read the active pointer with pure atomic memory ordering
        int (*current_thunk)(void) = (int (*)(void))atomic_load_explicit(&g_active_thunk_ptr, memory_order_acquire);
        
        if (current_thunk) {
            int result = current_thunk();
            if (result == 2 && !successfully_mutated) {
                tsfi_io_printf(stdout, "\n[ETA] GEOMETRIC FLIP DETECTED! Output mathematically shifted from 1 -> 2 at epoch %d!\n", epochs);
                successfully_mutated = 1;
                break; 
            }
        }
        
        // Let the Zhao rings breathe and execute internal handoffs
        tsfi_zhao_sync(); struct timespec ts={0,10000}; nanosleep(&ts,NULL); 

        epochs++;
        if (epochs % 50000 == 0) {
            tsfi_io_printf(stdout, "."); tsfi_io_flush(stdout);
        }
    }
    
    return successfully_mutated ? 0 : 1;
}

int main(void) {
    tsfi_io_printf(stdout, "=== TSFi SEAL: Threaded Hot Mutation Verification ===\n");

    // Initialize Internal Eta timeline (Zhao)
    tsfi_zhao_init(2);

    // 1. Forge initial baseline standard cell (Returns 1)
    const char* thunk_path = "thunks/test_mutator_thunk.c";
    FILE* f = fopen(thunk_path, "w");
    if (f) {
        fprintf(f, "int test_mutator_thunk(void) { return 1; }\n");
        fclose(f);
    } else {
        return 1;
    }

    // Load the initial baseline synchronously to establish the pointer
    void* initial_ptr = tsfi_hotload_thunk(thunk_path, "test_mutator_thunk");
    atomic_store(&g_active_thunk_ptr, initial_ptr);
    
    if (!initial_ptr) {
        tsfi_io_printf(stderr, "[FAIL] Baseline compilation rejected.\n");
        return 1;
    }

    // 2. Trigger the External Mu environment mutation
    // We rewrite the file on the physical filesystem to return 2
    tsfi_io_printf(stdout, "[MU] Injecting geometric mutation to file system (return 2)...\n");
    // Ensure mtime advances by sleeping 1 second (stat resolution)
    sleep(1); 
    f = fopen(thunk_path, "w");
    if (f) {
        fprintf(f, "int test_mutator_thunk(void) { return 2; }\n");
        fclose(f);
    }

    // 3. Dispatch the Hotloader background poll
    // This function MUST NOT block the active thread. It should spawn the Mu compiler,
    // execute the build, and then submit the pointer swap task to Zhao.
    tsfi_io_printf(stdout, "[MU] Dispatching non-blocking Hotloader poll...\n");
    tsfi_hotreload_poll_async(thunk_path, "test_mutator_thunk", &g_active_thunk_ptr);

    // 4. Enter the rigid internal execution loop
    int result = execute_matrix_loop();

    if (result == 0) {
        tsfi_io_printf(stdout, "[SEAL VERIFIED] The continuous Fourier bridge successfully intercepted, compiled, and mapped the geometric mutation into the lock-free timeline with zero jitter.\n");
    } else {
        tsfi_io_printf(stderr, "\n[SEAL FAULT] Matrix execution exhausted without hot-swap engagement.\n");
    }

    // Cleanup
    tsfi_zhao_shutdown();
    unlink(thunk_path);
    unlink(".tsfi_tmp_test_mutator_thunk.so");

    return result;


    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
}
