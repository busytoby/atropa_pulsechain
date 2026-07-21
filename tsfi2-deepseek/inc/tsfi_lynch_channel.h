#ifndef TSFI_LYNCH_CHANNEL_H
#define TSFI_LYNCH_CHANNEL_H

#include <stdint.h>
#include <stddef.h>

#define LYNCH_BUFFER_SIZE 720 // 8-Block Tape Header Size

typedef struct {
    uint8_t buffer_A[LYNCH_BUFFER_SIZE];
    uint8_t buffer_B[LYNCH_BUFFER_SIZE];
    uint8_t active_buffer_id; // 0 for A, 1 for B
    int is_dma_busy;
} tsfi_lynch_channel_t;

typedef struct {
    uint32_t slot_snapshots[32];
    uint8_t scsi_state_snapshot;
    int is_valid;
} tsfi_lynch_checkpoint_t;

/* Initialize William C. Lynch's Asynchronous Double-Buffering Disk Channel */
int tsfi_lynch_channel_init(tsfi_lynch_channel_t *chan);

/* Swap active double-buffer after DMA read cycle completes */
int tsfi_lynch_channel_swap_buffers(tsfi_lynch_channel_t *chan);

/* Capture atomic state checkpoint for fault-tolerant rollback */
int tsfi_lynch_checkpoint_capture(tsfi_lynch_checkpoint_t *ckpt, const uint32_t *slots, uint8_t scsi_reg);

/* Rollback state from atomic checkpoint upon transaction failure */
int tsfi_lynch_checkpoint_rollback(const tsfi_lynch_checkpoint_t *ckpt, uint32_t *slots_out, uint8_t *scsi_out);

#endif // TSFI_LYNCH_CHANNEL_H
