#ifndef TSFI_SPERONI_PARAMS_H
#define TSFI_SPERONI_PARAMS_H

#include <stdint.h>
#include <stddef.h>

#define SPERONI_MAX_PARAMS 8

typedef struct {
    uint32_t param_count;                       // Number of parameters in list
    uint64_t param_values[SPERONI_MAX_PARAMS];  // Arithmetized parameter values
    uint32_t evm_gas_units;                     // 270 Gas / Auncient Ether Units
    double fet_power_watts;                     // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];                     // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_speroni_param_list_t;

/* Initialize a Speroni Generalized Parameter List */
int tsfi_speroni_param_list_init(tsfi_speroni_param_list_t *list);

/* Bind parameters using Speroni arithmetizing declaration offsets */
int tsfi_speroni_param_list_bind(
    tsfi_speroni_param_list_t *list,
    const uint64_t *values,
    size_t count
);

/* Compute arithmetized field byte offset inside 720-byte Yul tape header */
uint32_t tsfi_speroni_arithmetize_header_offset(uint32_t block_index, uint32_t field_idx);

#endif // TSFI_SPERONI_PARAMS_H
