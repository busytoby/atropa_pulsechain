#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_speroni_define_format.h"
#include "tsfi_lowpower_fet.h"

int tsfi_speroni_define_format_proc_init(
    uint32_t proc_id,
    tsfi_speroni_define_format_proc_t *proc
) {
    if (!proc) return -1;
    memset(proc, 0, sizeof(tsfi_speroni_define_format_proc_t));

    proc->proc_id = proc_id;
    proc->evm_gas_units = 210; // 210 Gas / Auncient Ether Units per DEFINE/FORMAT slot

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    proc->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(proc->tape_dat_bin, sizeof(proc->tape_dat_bin), "SPERONI_DEFINE_FORMAT_%08X.DAT.BIN", proc_id);
    return 0;
}

int tsfi_speroni_add_define_macro(
    tsfi_speroni_define_format_proc_t *proc,
    const char *macro_name,
    const char *macro_body
) {
    if (!proc || !macro_name || !macro_body || proc->macro_count >= 8) return -1;

    tsfi_speroni_define_entry_t *entry = &proc->macros[proc->macro_count++];
    strncpy(entry->macro_name, macro_name, sizeof(entry->macro_name) - 1);
    strncpy(entry->macro_body, macro_body, sizeof(entry->macro_body) - 1);

    return 0;
}

int tsfi_speroni_add_format_spec(
    tsfi_speroni_define_format_proc_t *proc,
    const char *format_name,
    const char *format_spec
) {
    if (!proc || !format_name || !format_spec || proc->format_count >= 8) return -1;

    tsfi_speroni_format_entry_t *entry = &proc->formats[proc->format_count++];
    strncpy(entry->format_name, format_name, sizeof(entry->format_name) - 1);
    strncpy(entry->format_spec, format_spec, sizeof(entry->format_spec) - 1);

    return 0;
}

int tsfi_speroni_expand_define_format(
    const tsfi_speroni_define_format_proc_t *proc,
    const char *target_macro,
    const char *target_format,
    char *out_buf, size_t out_max
) {
    if (!proc || !target_macro || !target_format || !out_buf || out_max == 0) return -1;

    const char *body = "UNKNOWN_MACRO";
    const char *spec = "UNKNOWN_FORMAT";

    for (size_t i = 0; i < proc->macro_count; i++) {
        if (strcmp(proc->macros[i].macro_name, target_macro) == 0) {
            body = proc->macros[i].macro_body;
            break;
        }
    }

    for (size_t i = 0; i < proc->format_count; i++) {
        if (strcmp(proc->formats[i].format_name, target_format) == 0) {
            spec = proc->formats[i].format_spec;
            break;
        }
    }

    snprintf(out_buf, out_max, "DEFINE(%s -> %s) | FORMAT(%s -> %s)", target_macro, body, target_format, spec);

    printf("[SPERONI DEFINE/FORMAT] Proc: %u | Expanded: %s | Gas: %u | File: %s\n",
           proc->proc_id, out_buf, proc->evm_gas_units, proc->tape_dat_bin);

    return 0; // Expansion successful
}
