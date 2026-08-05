#ifndef TSFI2_COMPILER_H
#define TSFI2_COMPILER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Compiles a simple C program containing a main function and a return statement.
 * Example input: "int main() { return 42; }"
 * Parameters:
 *   source_code: The C source code string.
 *   out_bytecode: Output buffer to write the compiled bytecode.
 *   max_len: Size of the output buffer.
 *   out_bytecode_len: Output pointer for the compiled bytecode length.
 * Returns:
 *   true on successful compilation, false on syntax/parsing error.
 */
bool tsfi2_compile(
    const char *source_code,
    uint8_t *out_bytecode,
    size_t max_len,
    size_t *out_bytecode_len
);

#endif /* TSFI2_COMPILER_H */
