#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_lynch_channel.h"

int tsfi_lynch_channel_init(tsfi_lynch_channel_t *chan) {
    if (!chan) return -1;
    memset(chan, 0, sizeof(tsfi_lynch_channel_t));
    chan->active_buffer_id = 0;
    chan->is_dma_busy = 0;
    printf("[LYNCH CHANNEL] Initialized Asynchronous Double-Buffering Disk Channel (720-Byte Ping-Pong)\n");
    return 0;
}

int tsfi_lynch_channel_swap_buffers(tsfi_lynch_channel_t *chan) {
    if (!chan) return -1;
    chan->active_buffer_id = (chan->active_buffer_id == 0) ? 1 : 0;
    printf("[LYNCH CHANNEL] Swapped Active Disk Buffer -> Active Buffer ID: %u\n", chan->active_buffer_id);
    return 0;
}

int tsfi_lynch_checkpoint_capture(tsfi_lynch_checkpoint_t *ckpt, const uint32_t *slots, uint8_t scsi_reg) {
    if (!ckpt || !slots) return -1;
    memcpy(ckpt->slot_snapshots, slots, sizeof(ckpt->slot_snapshots));
    ckpt->scsi_state_snapshot = scsi_reg;
    ckpt->is_valid = 1;
    printf("[LYNCH CHECKPOINT] Captured Atomic State Checkpoint (SCSI Reg: 0x%02X)\n", scsi_reg);
    return 0;
}

int tsfi_lynch_checkpoint_rollback(const tsfi_lynch_checkpoint_t *ckpt, uint32_t *slots_out, uint8_t *scsi_out) {
    if (!ckpt || !ckpt->is_valid || !slots_out || !scsi_out) return -1;
    memcpy(slots_out, ckpt->slot_snapshots, sizeof(ckpt->slot_snapshots));
    *scsi_out = ckpt->scsi_state_snapshot;
    printf("[LYNCH CHECKPOINT] Rollback Executed -> Restored Pre-Transaction Checkpoint State\n");
    return 0;
}
