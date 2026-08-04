#ifndef TSFI2_COMPILER_BIN_H
#define TSFI2_COMPILER_BIN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Packages compiled guest VM bytecode into a TSV-aligned .dat.bin executable asset.
 * Parameters:
 *   filepath: Destination file path ending with ".dat.bin".
 *   entry_point: Program entry point memory address.
 *   instruction_count: Compiled instructions count.
 *   bytecode: Raw compiled guest code buffer.
 *   bytecode_len: Length of the bytecode buffer.
 * Returns:
 *   true on successful packaging, false on file/validation error.
 */
bool tsfi2_compile_to_dat_bin(
    const char *filepath,
    uint32_t entry_point,
    uint32_t instruction_count,
    const uint8_t *bytecode,
    size_t bytecode_len
);

bool tsfi2_compile_to_dat_bin_ext(
    const char *filepath,
    uint32_t entry_point,
    uint32_t instruction_count,
    const char *custom_keys,
    const char *custom_values,
    const uint8_t *bytecode,
    size_t bytecode_len
);

#endif /* TSFI2_COMPILER_BIN_H */
