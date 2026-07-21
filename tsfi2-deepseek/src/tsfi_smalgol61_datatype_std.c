#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_smalgol61_datatype_std.h"
#include "tsfi_lowpower_fet.h"

int tsfi_smalgol61_datatype_registry_init(
    uint32_t registry_id,
    tsfi_smalgol61_datatype_registry_t *registry
) {
    if (!registry) return -1;
    memset(registry, 0, sizeof(tsfi_smalgol61_datatype_registry_t));

    registry->registry_id = registry_id;
    registry->evm_gas_units = 250; // 250 Gas / Auncient Ether Units per stack word normalization

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    registry->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(registry->tape_dat_bin, sizeof(registry->tape_dat_bin), "SMALGOL61_DATATYPE_%08X.DAT.BIN", registry_id);
    return 0;
}

int tsfi_smalgol61_push_int(tsfi_smalgol61_datatype_registry_t *registry, int32_t val) {
    if (!registry || registry->stack_pointer >= 32) return -1;
    smalgol_stack_word_t *w = &registry->stack[registry->stack_pointer++];
    w->type_tag = SMALGOL_TYPE_INTEGER;
    w->status_flags = 0x0001;
    w->payload.i_val = val;
    return 0;
}

int tsfi_smalgol61_push_real(tsfi_smalgol61_datatype_registry_t *registry, float val) {
    if (!registry || registry->stack_pointer >= 32) return -1;
    smalgol_stack_word_t *w = &registry->stack[registry->stack_pointer++];
    w->type_tag = SMALGOL_TYPE_REAL;
    w->status_flags = 0x0001;
    w->payload.r_val = val;
    return 0;
}

int tsfi_smalgol61_push_bool(tsfi_smalgol61_datatype_registry_t *registry, uint32_t val) {
    if (!registry || registry->stack_pointer >= 32) return -1;
    smalgol_stack_word_t *w = &registry->stack[registry->stack_pointer++];
    w->type_tag = SMALGOL_TYPE_BOOLEAN;
    w->status_flags = 0x0001;
    w->payload.b_val = (val != 0) ? 1 : 0;
    return 0;
}

int tsfi_smalgol61_push_label(tsfi_smalgol61_datatype_registry_t *registry, uint32_t offset) {
    if (!registry || registry->stack_pointer >= 32) return -1;
    smalgol_stack_word_t *w = &registry->stack[registry->stack_pointer++];
    w->type_tag = SMALGOL_TYPE_LABEL;
    w->status_flags = 0x0001;
    w->payload.l_offset = offset;
    return 0;
}

int tsfi_smalgol61_pop_word(
    tsfi_smalgol61_datatype_registry_t *registry,
    smalgol_stack_word_t *out_word
) {
    if (!registry || !out_word || registry->stack_pointer == 0) return -1;
    *out_word = registry->stack[--registry->stack_pointer];

    printf("[SMALGOL STACK TYPE] Reg ID: %u | SP: %zu | Type: 0x%04X | Gas: %u | File: %s\n",
           registry->registry_id, registry->stack_pointer, out_word->type_tag, registry->evm_gas_units, registry->tape_dat_bin);

    return 0;
}
