#ifndef TSFI_SPERONI_DEFINE_FORMAT_H
#define TSFI_SPERONI_DEFINE_FORMAT_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    char macro_name[32];               // DEFINE Macro Name
    char macro_body[128];              // DEFINE Macro Expansion Body
} tsfi_speroni_define_entry_t;

typedef struct {
    char format_name[32];              // FORMAT Identifier
    char format_spec[128];             // FORMAT Layout Specification
} tsfi_speroni_format_entry_t;

typedef struct {
    uint32_t proc_id;                   // Processor ID
    tsfi_speroni_define_entry_t macros[8]; // DEFINE Macro Table
    size_t macro_count;
    tsfi_speroni_format_entry_t formats[8]; // FORMAT Table
    size_t format_count;
    uint32_t evm_gas_units;             // 210 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_speroni_define_format_proc_t;

/* Initialize Speroni DEFINE Macro & FORMAT Table Processor */
int tsfi_speroni_define_format_proc_init(
    uint32_t proc_id,
    tsfi_speroni_define_format_proc_t *proc
);

/* Add Speroni DEFINE macro entry */
int tsfi_speroni_add_define_macro(
    tsfi_speroni_define_format_proc_t *proc,
    const char *macro_name,
    const char *macro_body
);

/* Add Speroni FORMAT declaration entry */
int tsfi_speroni_add_format_spec(
    tsfi_speroni_define_format_proc_t *proc,
    const char *format_name,
    const char *format_spec
);

/* Expand macro and format output string into tape buffer */
int tsfi_speroni_expand_define_format(
    const tsfi_speroni_define_format_proc_t *proc,
    const char *target_macro,
    const char *target_format,
    char *out_buf, size_t out_max
);

#endif // TSFI_SPERONI_DEFINE_FORMAT_H
