#ifndef TSFI_SPERONI_PARAM_LIST_H
#define TSFI_SPERONI_PARAM_LIST_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    SPERONI_PARAM_MODE_VALUE = 0,
    SPERONI_PARAM_MODE_NAME = 1,
    SPERONI_PARAM_MODE_REF = 2
} tsfi_speroni_param_mode_t;

typedef struct {
    char name[32];                      // Parameter Identifier
    tsfi_speroni_param_mode_t mode;     // Value, Name, or Ref
    uint64_t val;                       // Bound Value / Memory Ref
} tsfi_speroni_param_entry_t;

typedef struct {
    uint32_t proc_id;                   // Procedure ID
    tsfi_speroni_param_entry_t entries[16]; // Generalized Parameter List
    size_t count;                       // Parameter Count
    uint32_t evm_gas_units;             // 190 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_speroni_param_list_proc_t;

/* Initialize Speroni Generalized Parameter List Processor */
int tsfi_speroni_param_list_proc_init(
    uint32_t proc_id,
    tsfi_speroni_param_list_proc_t *proc
);

/* Add parameter entry to Speroni generalized parameter list */
int tsfi_speroni_param_list_add_param(
    tsfi_speroni_param_list_proc_t *proc,
    const char *name,
    tsfi_speroni_param_mode_t mode,
    uint64_t val
);

/* Evaluate Speroni generalized parameter thunks inside e/ZVM stack */
int tsfi_speroni_param_list_eval(
    const tsfi_speroni_param_list_proc_t *proc
);

#endif // TSFI_SPERONI_PARAM_LIST_H
