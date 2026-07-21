#ifndef TSFI_KNUTH_YUL_TRANSPILER_H
#define TSFI_KNUTH_YUL_TRANSPILER_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    char original_yul[512];
    char optimized_yul[512];
    uint32_t gas_saved;
} tsfi_knuth_yul_transpile_result_t;

/* Transpile dual Yul sstore calls into a single Knuth Base 2i packed sstore call */
int tsfi_knuth_transpile_yul_sstore(
    const char *slot_a,
    const char *var_x,
    const char *slot_b,
    const char *var_y,
    tsfi_knuth_yul_transpile_result_t *out_res
);

#endif // TSFI_KNUTH_YUL_TRANSPILER_H
