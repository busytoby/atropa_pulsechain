#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_autodin_tx_pipe.h"
#include "tsfi_lowpower_fet.h"

int tsfi_autodin_tx_pipe_init(tsfi_autodin_tx_pipe_t *pipe) {
    if (!pipe) return -1;
    memset(pipe, 0, sizeof(tsfi_autodin_tx_pipe_t));
    pipe->evm_gas_units = 280; // 280 Gas / Auncient Ether Units per transaction

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    pipe->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(pipe->tape_dat_bin, sizeof(pipe->tape_dat_bin), "AUTODIN_TX_PIPE_00000001.DAT.BIN");
    return 0;
}

int tsfi_autodin_tx_pipe_push(
    tsfi_autodin_tx_pipe_t *pipe,
    uint64_t tx_id,
    const char *submitter,
    uint64_t payload
) {
    if (!pipe) return -1;

    // Check if pipe is full (Yield Producer Coroutine)
    if (pipe->count >= AUTODIN_TX_PIPE_CAPACITY) {
        printf("[AUTODIN TX PIPE] Pipe FULL (%u/%d)! Yielding producer for Tx: %llu\n",
               pipe->count, AUTODIN_TX_PIPE_CAPACITY, (unsigned long long)tx_id);
        return 1; // Yield state: Pipe Full
    }

    tsfi_autodin_tx_entry_t *entry = &pipe->ring_buffer[pipe->head];
    entry->tx_id = tx_id;
    snprintf(entry->submitter, sizeof(entry->submitter), "%s", submitter ? submitter : "0x0000000000000000000000000000000000000000");
    entry->payload_word = payload;

    pipe->head = (pipe->head + 1) % AUTODIN_TX_PIPE_CAPACITY;
    pipe->count++;
    pipe->total_tx_processed++;

    printf("[AUTODIN TX PUSH] Tx ID: %llu | Submitter: %s | Payload: 0x%016llX | Gas: %u | File: %s\n",
           (unsigned long long)tx_id, entry->submitter, (unsigned long long)payload,
           pipe->evm_gas_units, pipe->tape_dat_bin);

    return 0; // Push successful
}

int tsfi_autodin_tx_pipe_pull(
    tsfi_autodin_tx_pipe_t *pipe,
    tsfi_autodin_tx_entry_t *out_entry
) {
    if (!pipe || !out_entry) return -1;

    // Check if pipe is empty (Yield Consumer Coroutine)
    if (pipe->count == 0) {
        printf("[AUTODIN TX PIPE] Pipe EMPTY! Yielding consumer...\n");
        return 2; // Yield state: Pipe Empty
    }

    *out_entry = pipe->ring_buffer[pipe->tail];
    pipe->tail = (pipe->tail + 1) % AUTODIN_TX_PIPE_CAPACITY;
    pipe->count--;

    printf("[AUTODIN TX PULL] Tx ID: %llu | Submitter: %s | Remaining Tx: %u | Gas: %u | File: %s\n",
           (unsigned long long)out_entry->tx_id, out_entry->submitter, pipe->count,
           pipe->evm_gas_units, pipe->tape_dat_bin);

    return 0; // Pull successful
}
