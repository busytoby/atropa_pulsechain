#ifndef TSFI_CONWAY_PIPE_H
#define TSFI_CONWAY_PIPE_H

#include <stdint.h>
#include <stddef.h>

#define CONWAY_PIPE_CAPACITY 256

typedef struct {
    uint64_t ring_buffer[CONWAY_PIPE_CAPACITY]; // 256-entry lock-free pipe ring buffer
    uint32_t head;                               // Pipe write head index
    uint32_t tail;                               // Pipe read tail index
    uint32_t count;                              // Current item count in pipe
    uint8_t  is_full;                            // 1 if pipe is full, 0 otherwise
    uint8_t  is_empty;                           // 1 if pipe is empty, 0 otherwise
    uint32_t evm_gas_units;                      // 280 Gas / Auncient Ether Units
    double fet_power_watts;                      // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];                      // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_conway_pipe_t;

/* Initialize a Conway coroutine stream pipe */
int tsfi_conway_pipe_init(tsfi_conway_pipe_t *pipe);

/* Push data word into coroutine pipe; yields if full */
int tsfi_conway_pipe_push(tsfi_conway_pipe_t *pipe, uint64_t data_word);

/* Pull data word from coroutine pipe; yields if empty */
int tsfi_conway_pipe_pull(tsfi_conway_pipe_t *pipe, uint64_t *out_data_word);

#endif // TSFI_CONWAY_PIPE_H
