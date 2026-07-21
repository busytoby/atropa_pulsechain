#ifndef TSFI_MANDERFIELD_NPL_TRANSPILER_H
#define TSFI_MANDERFIELD_NPL_TRANSPILER_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t transpiler_id;
    uint32_t ast_nodes_compacted;
    uint32_t bytes_saved;
    uint32_t evm_gas_units;             // 280 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_manderfield_npl_transpiler_t;

/* Initialize Manderfield-NPL Multi-Paradigm Transpiler & Compactor */
int tsfi_manderfield_npl_transpiler_init(
    uint32_t transpiler_id,
    tsfi_manderfield_npl_transpiler_t *transpiler
);

/* Transpile NPL / PL/I Code to Yul Contract Assembly */
int tsfi_manderfield_transpile_npl(
    tsfi_manderfield_npl_transpiler_t *transpiler,
    const char *npl_source,
    char *yul_out,
    size_t max_len
);

/* NAA Restricted Small-Memory AST Compactor */
int tsfi_manderfield_compact_ast(
    tsfi_manderfield_npl_transpiler_t *transpiler,
    size_t raw_ast_bytes,
    size_t *compacted_bytes
);

#endif // TSFI_MANDERFIELD_NPL_TRANSPILER_H
