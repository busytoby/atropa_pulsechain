#include "tsfi_command_queue.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BLOCK_SIZE 512

void tsfi_command_queue_init(TsfiCommandQueue *q, TsfiSubjectiveWaveSystem *descriptor, uint8_t *blocked_data, size_t count) {
    if (!q || !descriptor || !blocked_data) return;
    q->descriptor = descriptor;
    q->buffer = blocked_data;
    q->block_count = count;
    q->current_block = 0;
    q->current_offset = 0;
    printf("[QUEUE] Helmholtz Command Queue initialized with %zu blocks, bound to WaveSystem %016llX\n", 
           count, (unsigned long long)descriptor->subjective_hash);
}

bool tsfi_command_queue_step(TsfiCommandQueue *q) {
    if (!q || q->current_block >= q->block_count) return false;

    uint8_t *block = q->buffer + (q->current_block * BLOCK_SIZE);
    
    // Check if we've reached the end of the current block
    if (q->current_offset >= BLOCK_SIZE || block[q->current_offset] == 0) {
        q->current_block++;
        q->current_offset = 0;
        return tsfi_command_queue_step(q); // Recursive call to next block
    }

    // Read the record length (Card length)
    uint8_t record_len = block[q->current_offset++];
    uint8_t *card_data = &block[q->current_offset];
    
    // Move offset forward for next step
    q->current_offset += record_len;

    // --- Card Execution (Thunk Dispatch) ---
    // In a real ICPC realization, we would use ThunkProxy to jump into the instruction logic.
    // For now, we simulate the execution against the descriptor.
    printf("[QUEUE] Executing Card (%u bytes): \"%.*s\"\n", record_len, record_len, card_data);
    
    // Update descriptor sequence to reflect activity
    (*q->descriptor->counter)++;

    return true;
}

void tsfi_command_queue_teardown(TsfiCommandQueue *q) {
    if (q) {
        memset(q, 0, sizeof(TsfiCommandQueue));
    }
}
