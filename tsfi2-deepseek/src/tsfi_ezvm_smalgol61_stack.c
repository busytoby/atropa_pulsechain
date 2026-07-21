#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_ezvm_smalgol61_stack.h"
#include "tsfi_lowpower_fet.h"

int tsfi_ezvm_smalgol61_stack_init(
    uint32_t ezvm_proc_id,
    tsfi_ezvm_smalgol61_stack_t *ezvm
) {
    if (!ezvm) return -1;
    memset(ezvm, 0, sizeof(tsfi_ezvm_smalgol61_stack_t));

    ezvm->ezvm_proc_id = ezvm_proc_id;
    ezvm->evm_gas_units = 330; // 330 Gas / Auncient Ether Units per step slot

    // Initialize underlying SMALGOL-61 and Conway Dynamic Stack engines
    tsfi_smalgol61_proc_init(ezvm_proc_id, &ezvm->smalgol_proc);
    tsfi_conway_dynamic_stack_init(ezvm_proc_id, &ezvm->dynamic_stack);

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    ezvm->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(ezvm->tape_dat_bin, sizeof(ezvm->tape_dat_bin), "EZVM_SMALGOL61_%08X.DAT.BIN", ezvm_proc_id);
    return 0;
}

int tsfi_ezvm_smalgol61_stack_step(
    tsfi_ezvm_smalgol61_stack_t *ezvm,
    const char *smalgol_stmt,
    uint32_t call_return_pc,
    uint64_t call_local_word
) {
    if (!ezvm || !smalgol_stmt) return -1;

    // Handle dynamic stack push if statement is a procedure call
    if (strstr(smalgol_stmt, "CALL") || strstr(smalgol_stmt, "PUSH")) {
        tsfi_conway_dynamic_stack_push(&ezvm->dynamic_stack, call_return_pc, call_local_word);
    } else if (strstr(smalgol_stmt, "RETURN") || strstr(smalgol_stmt, "POP")) {
        uint32_t pop_pc = 0;
        uint64_t pop_word = 0;
        tsfi_conway_dynamic_stack_pop(&ezvm->dynamic_stack, &pop_pc, &pop_word);
    }

    // Step underlying SMALGOL-61 execution engine
    tsfi_smalgol61_proc_step(&ezvm->smalgol_proc, smalgol_stmt);

    printf("[e/ZVM EXEC SYSTEM] Proc: %u | Stack Depth: %u | Stmt: '%s' | Gas: %u | File: %s\n",
           ezvm->ezvm_proc_id, ezvm->dynamic_stack.depth, smalgol_stmt,
           ezvm->evm_gas_units, ezvm->tape_dat_bin);

    return 0; // Step execution successful
}
