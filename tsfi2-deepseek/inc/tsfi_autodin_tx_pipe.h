#ifndef TSFI_AUTODIN_TX_PIPE_H
#define TSFI_AUTODIN_TX_PIPE_H

#include <stdint.h>
#include <stddef.h>

#define AUTODIN_TX_PIPE_CAPACITY 256

typedef struct {
    uint64_t tx_id;              // AUTODIN Transaction ID
    char submitter[64];          // Original Submitter Address (tx_origin)
    uint64_t payload_word;       // Transaction payload word
} tsfi_autodin_tx_entry_t;

typedef struct {
    tsfi_autodin_tx_entry_t ring_buffer[AUTODIN_TX_PIPE_CAPACITY];
    uint32_t head;               // Pipe write head index
    uint32_t tail;               // Pipe read tail index
    uint32_t count;              // Current transaction count in pipe
    uint32_t total_tx_processed; // Lifetime transactions processed by pipe
    uint32_t evm_gas_units;      // 280 Gas / Auncient Ether Units per transaction
    double fet_power_watts;      // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];      // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_autodin_tx_pipe_t;

/* Initialize an AUTODIN transaction coroutine stream pipe */
int tsfi_autodin_tx_pipe_init(tsfi_autodin_tx_pipe_t *pipe);

/* Push an AUTODIN transaction into the coroutine pipe; yields if full */
int tsfi_autodin_tx_pipe_push(
    tsfi_autodin_tx_pipe_t *pipe,
    uint64_t tx_id,
    const char *submitter,
    uint64_t payload
);

/* Pull an AUTODIN transaction from the coroutine pipe; yields if empty */
int tsfi_autodin_tx_pipe_pull(
    tsfi_autodin_tx_pipe_t *pipe,
    tsfi_autodin_tx_entry_t *out_entry
);

#endif // TSFI_AUTODIN_TX_PIPE_H
