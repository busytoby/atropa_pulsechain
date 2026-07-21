#ifndef TSFI_CONWAY_LEDGER_PROCESS_H
#define TSFI_CONWAY_LEDGER_PROCESS_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    CONWAY_PROC_STATE_INIT = 0,
    CONWAY_PROC_STATE_FETCH_TX,
    CONWAY_PROC_STATE_EXECUTE_OP,
    CONWAY_PROC_STATE_COMMIT_LEDGER,
    CONWAY_PROC_STATE_YIELDED,
    CONWAY_PROC_STATE_HALTED
} tsfi_conway_proc_state_t;

typedef struct {
    uint32_t process_id;           // Unique Conway Process ID
    char submitter[64];            // Original Submitter Address (tx_origin)
    uint32_t program_counter;      // Coroutine PC inside binary ledger
    tsfi_conway_proc_state_t state;// Active coroutine yield state
    uint64_t current_tx_id;        // Transaction ID being processed
    uint32_t evm_gas_units;        // 350 Gas / Auncient Ether Units per process step
    double fet_power_watts;        // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];        // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_conway_ledger_process_t;

/* Instantiate a multi-tx running process inside a binary ledger slice */
int tsfi_conway_ledger_process_init(
    uint32_t process_id,
    const char *submitter,
    tsfi_conway_ledger_process_t *proc
);

/* Step execution of a binary ledger coroutine process over an ordered transaction */
int tsfi_conway_ledger_process_step(
    tsfi_conway_ledger_process_t *proc,
    uint64_t tx_id,
    uint64_t tx_op_code
);

#endif // TSFI_CONWAY_LEDGER_PROCESS_H
