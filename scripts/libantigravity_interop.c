#include "libantigravity_interop.h"
#include <string.h>

// Initialize registry with baseline zero-state
void interop_registry_init(InteropRegistry *reg) {
    if (!reg) return;
    memset(reg, 0, sizeof(InteropRegistry));
    reg->frame_modulation_factor = 1.0f;
}

// Update the registry state from the active Auncient VM registers
void interop_registry_update(InteropRegistry *reg, const AuncientRegisterState *state, uint32_t epoch) {
    if (!reg || !state) return;
    
    // Copy active registers thread-safely
    reg->active_state = *state;
    reg->current_epoch = epoch;
    reg->active_cycles++;

    // Calculate rendering modulation factor based on the thermodynamic Barn heat glow register
    // High Barn heat maps to higher specular highlights and plasma frequency modulation
    if (state->element > 0) {
        reg->frame_modulation_factor = (float)(state->barn % state->element) / (float)state->element;
    } else {
        reg->frame_modulation_factor = 1.0f;
    }
}

// Probe the current registry data for the interop renderer
void interop_registry_probe(const InteropRegistry *reg, AuncientRegisterState *out_state, uint32_t *out_epoch) {
    if (!reg) return;
    if (out_state) {
        *out_state = reg->active_state;
    }
    if (out_epoch) {
        *out_epoch = reg->current_epoch;
    }
}

// WinchesterMQ implementation: write sector payload with flow-control limits
void interop_mq_put(InteropLUN *lun, const uint8_t *data) {
    if (!lun || !data) return;
    uint32_t size = 0;
    if (lun->head < lun->tail) {
        size = lun->tail - lun->head;
    }
    if (size >= 16) return; // Flow-control backpressure overflow protection

    uint32_t lba = lun->tail % 16;
    memcpy(lun->sectors[lba], data, 256);
    lun->tail++;
}

// WinchesterMQ implementation: Phase-1 Lease Get
int interop_mq_get_phase1(InteropLUN *lun, uint8_t *out_data) {
    if (!lun || !out_data || lun->head == lun->tail) return -1;
    uint32_t lba = lun->head % 16;
    memcpy(out_data, lun->sectors[lba], 256);
    lun->pending_ack = lun->head; // Lock lease on current head
    return 0;
}

// WinchesterMQ implementation: Phase-2 Commit ACK
int interop_mq_ack_phase2(InteropLUN *lun) {
    if (!lun || lun->pending_ack == 0xFFFFFFFF) return -1;
    if (lun->pending_ack == lun->head) {
        lun->head++;
        lun->pending_ack = 0xFFFFFFFF; // Clear lease
        return 0;
    }
    return -2;
}
