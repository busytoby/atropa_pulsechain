#ifndef TSFI_ALGOL68_MODE_TRANSPILER_H
#define TSFI_ALGOL68_MODE_TRANSPILER_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    ALGOL68_MODE_STRUCT = 0,
    ALGOL68_MODE_REF,
    ALGOL68_MODE_PROC,
    ALGOL68_MODE_UNION,
    ALGOL68_MODE_FLEX_ARRAY
} tsfi_algol68_mode_kind_t;

typedef struct {
    tsfi_algol68_mode_kind_t mode_kind;
    uint32_t memory_words_cnt;   // Allocated 32-byte Yul memory words
    uint32_t union_variant_tag;  // Tag for union modes
    uint32_t evm_gas_units;      // 550 Gas / Auncient Ether Units
    double fet_power_watts;      // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];      // Rule 13 Filename (.DAT.BIN)
} tsfi_algol68_mode_summary_t;

/* Transpile ALGOL 68 mode definition into Yul virtual hardware memory layout */
int tsfi_algol68_transpile_mode(
    tsfi_algol68_mode_kind_t mode_kind,
    uint32_t words_cnt,
    uint32_t variant_tag,
    tsfi_algol68_mode_summary_t *out_summary
);

#endif // TSFI_ALGOL68_MODE_TRANSPILER_H
