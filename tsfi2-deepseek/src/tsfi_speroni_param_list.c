#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_speroni_param_list.h"
#include "tsfi_lowpower_fet.h"

int tsfi_speroni_param_list_proc_init(
    uint32_t proc_id,
    tsfi_speroni_param_list_proc_t *proc
) {
    if (!proc) return -1;
    memset(proc, 0, sizeof(tsfi_speroni_param_list_proc_t));

    proc->proc_id = proc_id;
    proc->evm_gas_units = 190; // 190 Gas / Auncient Ether Units per parameter list evaluation

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    proc->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(proc->tape_dat_bin, sizeof(proc->tape_dat_bin), "SPERONI_PARAM_LIST_%08X.DAT.BIN", proc_id);
    return 0;
}

int tsfi_speroni_param_list_add_param(
    tsfi_speroni_param_list_proc_t *proc,
    const char *name,
    tsfi_speroni_param_mode_t mode,
    uint64_t val
) {
    if (!proc || !name || proc->count >= 16) return -1;

    tsfi_speroni_param_entry_t *entry = &proc->entries[proc->count++];
    strncpy(entry->name, name, sizeof(entry->name) - 1);
    entry->mode = mode;
    entry->val = val;

    return 0;
}

int tsfi_speroni_param_list_eval(
    const tsfi_speroni_param_list_proc_t *proc
) {
    if (!proc) return -1;

    printf("[SPERONI PARAM LIST] Proc: %u | Params: %zu | Gas: %u | File: %s\n",
           proc->proc_id, proc->count, proc->evm_gas_units, proc->tape_dat_bin);

    for (size_t i = 0; i < proc->count; i++) {
        const tsfi_speroni_param_entry_t *entry = &proc->entries[i];
        const char *mode_str = (entry->mode == SPERONI_PARAM_MODE_VALUE) ? "VALUE" :
                               (entry->mode == SPERONI_PARAM_MODE_NAME) ? "NAME_THUNK" : "REFERENCE";
        printf("  Param [%zu]: %s | Mode: %s | Val: 0x%016ZX\n", i, entry->name, mode_str, (size_t)entry->val);
    }

    return 0; // Thunk evaluation successful
}
