#ifndef TSFI_COMMAND_QUEUE_H
#define TSFI_COMMAND_QUEUE_H

#include "tsfi_types.h"
#include "tsfi_comm.h"

// --- TSFi Helmholtz Command Queue ---
// Manages the execution of Fortran-style "Cards" blocked into 512-byte units.
// Every queue is anchored to a Subjective WaveSystem (The Descriptor).

typedef struct {
    TsfiSubjectiveWaveSystem *descriptor;
    uint8_t *buffer;      // 512-byte blocked card data
    size_t   block_count;
    size_t   current_block;
    size_t   current_offset;
} TsfiCommandQueue;

// Initialization: Binds a queue to a specific WaveSystem descriptor
void tsfi_command_queue_init(TsfiCommandQueue *q, TsfiSubjectiveWaveSystem *descriptor, uint8_t *blocked_data, size_t count);

// Execution: Processes the next "Card" in the queue using the bound thunks
// Returns false when the queue is exhausted or a fracture is detected.
bool tsfi_command_queue_step(TsfiCommandQueue *q);

// Cleanup
void tsfi_command_queue_teardown(TsfiCommandQueue *q);

#endif
