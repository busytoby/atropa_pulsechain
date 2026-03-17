#include <stdint.h>
#include <stddef.h>
#include <sys/mman.h>
#include "tsfi_io.h"

/**
 * TSFi Ballet Step 2: Étendre (Stretching)
 * Address stretches. Memory maps. Asset is visible.
 * This thunk wraps the high-throughput memory mapping logic.
 */
void *tsfi_ballet_etendre_thunk(const char *path, size_t size, int flags) {
    if (!path || size == 0) return NULL;
    
    // Stretch the address space by mapping the file
    // We utilize the TSFi I/O layer for high-throughput mapping
    void *ptr = tsfi_io_map_file(path, size, flags);
    
    if (ptr != MAP_FAILED) {
        // Asset is now visible within the process manifold
        return ptr;
    }
    
    return NULL;
}
