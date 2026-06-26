#ifndef LIBANTIGRAVITY_INTEROP_H
#define LIBANTIGRAVITY_INTEROP_H

#include <stdint.h>

// Auncient VM WinchesterMQ State probe
typedef struct {
    uint64_t base;
    uint64_t secret;
    uint64_t signal;
    uint64_t contour;
    uint64_t pole;
    uint64_t coordinate;
    uint64_t foundation;
    uint64_t element;
    uint64_t dynamo;
    uint64_t monopole;
    uint64_t barn;
} AuncientRegisterState;

// WinchesterMQ Queue/LUN state structure
typedef struct {
    uint32_t head;
    uint32_t tail;
    uint32_t pending_ack;
    uint8_t sectors[16][256]; // 16 sector capacity for flow control backpressure
} InteropLUN;

// Thread-safe interop registry sharing VM and WMQ queue states with the renderer
typedef struct {
    AuncientRegisterState active_state;
    uint32_t current_epoch;
    uint32_t active_cycles;
    float frame_modulation_factor;
    float verlet_x[16];
    float verlet_y[16];
    float verlet_z[16];
    uint32_t active_verlet_count;
    InteropLUN network_lun; // Maps to LUN 4 (Network queue) for live chat telemetry
} InteropRegistry;

// Interop API exports
void interop_registry_init(InteropRegistry *reg);
void interop_registry_update(InteropRegistry *reg, const AuncientRegisterState *state, uint32_t epoch);
void interop_registry_probe(const InteropRegistry *reg, AuncientRegisterState *out_state, uint32_t *out_epoch);

// WinchesterMQ queue interfaces
void interop_mq_put(InteropLUN *lun, const uint8_t *data);
int interop_mq_get_phase1(InteropLUN *lun, uint8_t *out_data);
int interop_mq_ack_phase2(InteropLUN *lun);

#endif // LIBANTIGRAVITY_INTEROP_H
