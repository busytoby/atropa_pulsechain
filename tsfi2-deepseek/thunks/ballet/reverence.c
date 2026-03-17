#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include "tsfi_io.h"

/**
 * TSFi Ballet Step 15: Révérence (Final Bow)
 * Memory unlinks. System bows. Task is finished.
 * This thunk ensures a clean teardown of the performance manifold.
 */
void tsfi_ballet_reverence_thunk(const char *shm_name) {
    if (!shm_name) return;
    
    // Memory unlinks from the OS namespace
    tsfi_shm_unlink(shm_name);
    
    // System bows: The performance task is now finished
}
