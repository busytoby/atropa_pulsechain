#ifndef TSFI2_LOADER_H
#define TSFI2_LOADER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t rip;
    uint64_t rbp;
    uint64_t rsp;
    bool halted;
    int exit_code;
} Tsfi2CpuState;

/*
 * Loads a TSV-aligned .dat.bin executable asset, verifies its checksum,
 * and runs its bytecode payload on a simulated CPU state.
 * Parameters:
 *   filepath: Path to the compiled .dat.bin program.
 *   cpu: Pointer to CPU state structure to initialize and execute.
 * Returns:
 *   true if the program executed and exited cleanly, false on validation or VM crash.
 */
bool tsfi2_load_and_execute(const char *filepath, Tsfi2CpuState *cpu);

#endif /* TSFI2_LOADER_H */
