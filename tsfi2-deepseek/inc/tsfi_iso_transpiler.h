#ifndef TSFI_ISO_TRANSPILER_H
#define TSFI_ISO_TRANSPILER_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t yul_selector;
    uint8_t calldata[32];
    int calldata_len;
    int is_equivalent;
} tsfi_iso_transpile_result_t;

/* Transpile a Runcible CLI command into compiled Yul contract calldata */
int tsfi_iso_transpile_runcible_to_yul(const char *cmd_line, tsfi_iso_transpile_result_t *out_result);

/* Verify mathematical equivalence of a state transition across Runcible CLI and Yul VM */
int tsfi_iso_validate_equivalence(const char *cmd_line);

#endif // TSFI_ISO_TRANSPILER_H
