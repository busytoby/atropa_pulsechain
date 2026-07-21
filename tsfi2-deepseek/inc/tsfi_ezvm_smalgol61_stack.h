#ifndef TSFI_EZVM_SMALGOL61_STACK_H
#define TSFI_EZVM_SMALGOL61_STACK_H

#include <stdint.h>
#include <stddef.h>
#include "tsfi_smalgol61_engine.h"
#include "tsfi_conway_dynamic_stack.h"

typedef struct {
    uint32_t ezvm_proc_id;                      // e/ZVM Process ID
    tsfi_smalgol61_proc_t smalgol_proc;         // SMALGOL-61 Process State
    tsfi_conway_dynamic_stack_t dynamic_stack;  // Dynamic Build-As-You-Go Stack State
    uint32_t evm_gas_units;                     // 330 Gas / Auncient Ether Units
    double fet_power_watts;                     // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];                     // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_ezvm_smalgol61_stack_t;

/* Initialize an e/ZVM Dynamic Stack SMALGOL-61 Executable System */
int tsfi_ezvm_smalgol61_stack_init(
    uint32_t ezvm_proc_id,
    tsfi_ezvm_smalgol61_stack_t *ezvm
);

/* Execute a SMALGOL-61 step with dynamic stack frame management */
int tsfi_ezvm_smalgol61_stack_step(
    tsfi_ezvm_smalgol61_stack_t *ezvm,
    const char *smalgol_stmt,
    uint32_t call_return_pc,
    uint64_t call_local_word
);

#endif // TSFI_EZVM_SMALGOL61_STACK_H
