#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_conway_ledger_process.h"
#include "tsfi_lowpower_fet.h"

int tsfi_conway_ledger_process_init(
    uint32_t process_id,
    const char *submitter,
    tsfi_conway_ledger_process_t *proc
) {
    if (!proc) return -1;
    memset(proc, 0, sizeof(tsfi_conway_ledger_process_t));

    proc->process_id = process_id;
    snprintf(proc->submitter, sizeof(proc->submitter), "%s", submitter ? submitter : "0x0000000000000000000000000000000000000000");
    proc->program_counter = 0;
    proc->state = CONWAY_PROC_STATE_INIT;
    proc->evm_gas_units = 350; // 350 Gas / Auncient Ether Units per process step

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    proc->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(proc->tape_dat_bin, sizeof(proc->tape_dat_bin), "CONWAY_LEDGER_PROC_%08X.DAT.BIN", process_id);
    return 0;
}

int tsfi_conway_ledger_process_step(
    tsfi_conway_ledger_process_t *proc,
    uint64_t tx_id,
    uint64_t tx_op_code
) {
    if (!proc) return -1;

    proc->current_tx_id = tx_id;
    proc->program_counter++;

    // State Machine step execution directly inside binary ledger
    switch (proc->state) {
        case CONWAY_PROC_STATE_INIT:
            proc->state = CONWAY_PROC_STATE_FETCH_TX;
            break;
        case CONWAY_PROC_STATE_FETCH_TX:
            proc->state = CONWAY_PROC_STATE_EXECUTE_OP;
            break;
        case CONWAY_PROC_STATE_EXECUTE_OP:
            proc->state = CONWAY_PROC_STATE_COMMIT_LEDGER;
            break;
        case CONWAY_PROC_STATE_COMMIT_LEDGER:
            proc->state = CONWAY_PROC_STATE_YIELDED;
            break;
        case CONWAY_PROC_STATE_YIELDED:
        default:
            proc->state = CONWAY_PROC_STATE_FETCH_TX;
            break;
    }

    printf("[CONWAY LEDGER PROCESS] Proc ID: %u | Submitter: %s | PC: 0x%04X | Tx ID: %llu | OpCode: 0x%016llX | State: %d | Gas: %u | File: %s\n",
           proc->process_id, proc->submitter, proc->program_counter,
           (unsigned long long)tx_id, (unsigned long long)tx_op_code,
           proc->state, proc->evm_gas_units, proc->tape_dat_bin);

    return 0; // Process step complete
}
