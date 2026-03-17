#include <stdint.h>
#include <stddef.h>
#include "tsfi_io.h"

/**
 * TSFi Ballet Step 8: Adagio (Slow Movement)
 * Universal wait monitors. Precision is slow. State is steady.
 * This thunk ensures high-precision, low-jitter monitoring.
 */
void tsfi_ballet_adagio_thunk(int fd, void *mem_addr, uint32_t mask, int timeout_ms) {
    // Adagio: Universal wait monitors steady state.
    // Precision is slow; we monitor both a file descriptor and a memory address.
    int fds[1] = { fd };
    void *addrs[1] = { mem_addr };
    uint32_t masks[1] = { mask };
    
    tsfi_monitor(fds, fd >= 0 ? 1 : 0, addrs, mem_addr ? masks : NULL, mem_addr ? 1 : 0, timeout_ms);
}
