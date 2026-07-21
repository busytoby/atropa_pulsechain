#ifndef TSFI_MANDERFIELD_COMPILER_H
#define TSFI_MANDERFIELD_COMPILER_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t compiler_id;
    uint32_t instructions_compiled;
    uint32_t gas_allocated;
    uint32_t evm_gas_units;             // 280 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_manderfield_compiler_t;

/* Initialize Manderfield Small-Computer Compiler & Governance Suite */
int tsfi_manderfield_compiler_init(
    uint32_t compiler_id,
    tsfi_manderfield_compiler_t *compiler
);

/* NAA Small-Computer SMALGOL Code Compiler */
int tsfi_manderfield_compile_smalgol(
    tsfi_manderfield_compiler_t *compiler,
    const char *smalgol_source,
    uint8_t *bytecode_out,
    size_t *bytecode_len
);

/* ALGOL Bulletin Standard Compliance & Governance Monitor */
int tsfi_manderfield_governance_check(
    tsfi_manderfield_compiler_t *compiler,
    const char *dialect_name
);

#endif // TSFI_MANDERFIELD_COMPILER_H
