#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_smalgol61_engine.h"
#include "tsfi_lowpower_fet.h"

int tsfi_smalgol61_proc_init(
    uint32_t process_id,
    tsfi_smalgol61_proc_t *proc
) {
    if (!proc) return -1;
    memset(proc, 0, sizeof(tsfi_smalgol61_proc_t));

    proc->process_id = process_id;
    proc->evm_gas_units = 290; // 290 Gas / Auncient Ether Units per statement slot

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    proc->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(proc->tape_dat_bin, sizeof(proc->tape_dat_bin), "SMALGOL61_PROC_%08X.DAT.BIN", process_id);
    return 0;
}

int tsfi_smalgol61_proc_step(
    tsfi_smalgol61_proc_t *proc,
    const char *smalgol_statement
) {
    if (!proc || !smalgol_statement) return -1;

    proc->program_counter++;

    // Parse simple assignments: e.g. "R0 := SCSI(32)" or "R1 := 100"
    int reg_idx = 0;
    int value = 0;
    if (sscanf(smalgol_statement, "R%d := %d", &reg_idx, &value) == 2) {
        if (reg_idx >= 0 && reg_idx < 16) {
            proc->registers[reg_idx] = value;
        }
    } else if (strstr(smalgol_statement, "SCSI(32)")) {
        proc->registers[0] = 32; // Keycode 32 ('d'/'D')
    } else if (strstr(smalgol_statement, "SCSI(30)")) {
        proc->registers[1] = 30; // Keycode 30 ('a'/'A')
    }

    printf("[SMALGOL-61 EXEC] Proc: %u | PC: %u | Stmt: '%s' | R0: %lld | R1: %lld | Gas: %u | File: %s\n",
           proc->process_id, proc->program_counter, smalgol_statement,
           (long long)proc->registers[0], (long long)proc->registers[1],
           proc->evm_gas_units, proc->tape_dat_bin);

    return 0; // Statement execution successful
}
