#ifndef TSFI_AUTODIN_CONWAY_TX_H
#define TSFI_AUTODIN_CONWAY_TX_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint64_t tx_sequence_id;      // Base transaction sequence ID
    uint32_t batch_limit_n;       // Maximum fixed/bounded transactions to process before yielding
    uint32_t processed_count;     // Current transactions processed in this coroutine step
    uint8_t  is_batch_complete;   // 1 if bounded batch N complete, 0 if active
    uint32_t evm_gas_units;       // 450 Gas / Auncient Ether Units
    double fet_power_watts;       // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];       // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_autodin_conway_tx_summary_t;

/* Execute a bounded batch of ordered AUTODIN transactions using a Conway coroutine context */
int tsfi_autodin_step_conway_tx(
    uint64_t start_tx_id,
    uint32_t batch_limit_n,
    tsfi_autodin_conway_tx_summary_t *out_summary
);

#endif // TSFI_AUTODIN_CONWAY_TX_H
