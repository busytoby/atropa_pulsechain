#ifndef TSFI_CONWAY_COROUTINE_H
#define TSFI_CONWAY_COROUTINE_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    CONWAY_YIELD_INIT = 0,
    CONWAY_YIELD_TRAVERSAL,
    CONWAY_YIELD_LOCK_ACQUIRED,
    CONWAY_YIELD_EVALUATED,
    CONWAY_YIELD_COMPLETE
} tsfi_conway_yield_state_t;

typedef struct {
    uint32_t node_index;                 // DAT / 2-3 Tree Node Index
    tsfi_conway_yield_state_t yield_state;// Current embedded coroutine yield state
    uint32_t instruction_pc;             // Coroutine instruction pointer / PC
    uint32_t evm_gas_units;              // 320 Gas / Auncient Ether Units
    double fet_power_watts;              // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];              // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_conway_coroutine_node_t;

/* Initialize and step an embedded tree coroutine context */
int tsfi_conway_step_tree_coroutine(
    uint32_t node_index,
    tsfi_conway_coroutine_node_t *node_ctx
);

#endif // TSFI_CONWAY_COROUTINE_H
