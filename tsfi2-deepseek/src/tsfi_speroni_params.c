#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_speroni_params.h"
#include "tsfi_lowpower_fet.h"

int tsfi_speroni_param_list_init(tsfi_speroni_param_list_t *list) {
    if (!list) return -1;
    memset(list, 0, sizeof(tsfi_speroni_param_list_t));
    list->evm_gas_units = 270; // 270 Gas / Auncient Ether Units

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    list->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(list->tape_dat_bin, sizeof(list->tape_dat_bin), "SPERONI_PARAMS_00000001.DAT.BIN");
    return 0;
}

int tsfi_speroni_param_list_bind(
    tsfi_speroni_param_list_t *list,
    const uint64_t *values,
    size_t count
) {
    if (!list || !values || count > SPERONI_MAX_PARAMS) return -1;

    list->param_count = (uint32_t)count;
    for (size_t i = 0; i < count; i++) {
        list->param_values[i] = values[i];
    }

    printf("[SPERONI PARAM LIST] Bound %u Params | Gas: %u | File: %s\n",
           list->param_count, list->evm_gas_units, list->tape_dat_bin);

    return 0; // Parameter binding successful
}

uint32_t tsfi_speroni_arithmetize_header_offset(uint32_t block_index, uint32_t field_idx) {
    // Speroni 1961 compile-time arithmetizing offset formula:
    // Block offset (80 bytes per block) + Field index (4 bytes per field word)
    return (block_index * 80) + (field_idx * 4);
}
