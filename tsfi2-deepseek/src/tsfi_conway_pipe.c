#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_conway_pipe.h"
#include "tsfi_lowpower_fet.h"

int tsfi_conway_pipe_init(tsfi_conway_pipe_t *pipe) {
    if (!pipe) return -1;
    memset(pipe, 0, sizeof(tsfi_conway_pipe_t));
    pipe->is_empty = 1;
    pipe->evm_gas_units = 280; // 280 Gas / Auncient Ether Units

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    pipe->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(pipe->tape_dat_bin, sizeof(pipe->tape_dat_bin), "CONWAY_PIPE_00000001.DAT.BIN");
    return 0;
}

int tsfi_conway_pipe_push(tsfi_conway_pipe_t *pipe, uint64_t data_word) {
    if (!pipe) return -1;

    // Check if pipe is full (Yield Producer Coroutine)
    if (pipe->count >= CONWAY_PIPE_CAPACITY) {
        pipe->is_full = 1;
        printf("[CONWAY PIPE] Pipe FULL (%u/%d)! Producer Yielding...\n", pipe->count, CONWAY_PIPE_CAPACITY);
        return 1; // Yield state: Pipe Full
    }

    pipe->ring_buffer[pipe->head] = data_word;
    pipe->head = (pipe->head + 1) % CONWAY_PIPE_CAPACITY;
    pipe->count++;
    pipe->is_empty = 0;
    if (pipe->count >= CONWAY_PIPE_CAPACITY) pipe->is_full = 1;

    printf("[CONWAY PIPE PUSH] Word: 0x%016llX | Head: %u | Count: %u | Ether Gas: %u | File: %s\n",
           (unsigned long long)data_word, pipe->head, pipe->count, pipe->evm_gas_units, pipe->tape_dat_bin);

    return 0; // Push successful
}

int tsfi_conway_pipe_pull(tsfi_conway_pipe_t *pipe, uint64_t *out_data_word) {
    if (!pipe || !out_data_word) return -1;

    // Check if pipe is empty (Yield Consumer Coroutine)
    if (pipe->count == 0) {
        pipe->is_empty = 1;
        printf("[CONWAY PIPE] Pipe EMPTY! Consumer Yielding...\n");
        return 2; // Yield state: Pipe Empty
    }

    *out_data_word = pipe->ring_buffer[pipe->tail];
    pipe->tail = (pipe->tail + 1) % CONWAY_PIPE_CAPACITY;
    pipe->count--;
    pipe->is_full = 0;
    if (pipe->count == 0) pipe->is_empty = 1;

    printf("[CONWAY PIPE PULL] Word: 0x%016llX | Tail: %u | Remaining: %u | Ether Gas: %u | File: %s\n",
           (unsigned long long)*out_data_word, pipe->tail, pipe->count, pipe->evm_gas_units, pipe->tape_dat_bin);

    return 0; // Pull successful
}
