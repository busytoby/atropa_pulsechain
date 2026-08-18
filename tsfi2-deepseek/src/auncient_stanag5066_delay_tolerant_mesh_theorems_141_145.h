#ifndef AUNCIENT_STANAG5066_DELAY_TOLERANT_MESH_THEOREMS_141_145_H
#define AUNCIENT_STANAG5066_DELAY_TOLERANT_MESH_THEOREMS_141_145_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define STANAG_TPA_BASE 0x0100
#define MAX_SREJ_RETRIES 3

typedef struct {
    uint32_t link_id;
    uint32_t orbital_delay_seconds;
    uint32_t link_baud_rate;
    uint32_t retransmission_count;
    float    accumulated_clock_drift_ms;
    bool     asymptotic_liveness_verified;
    bool     compact_retransmission_bounded;
    bool     relativistic_clock_drift_bounded;
    bool     intermittent_convergence_verified;
    bool     stanag_delay_parity_verified;
    uint32_t rule18_parity_checksum;
} StanagDelayTolerantMeshState;

void auncient_stanag_dtn_init(StanagDelayTolerantMeshState *state, uint32_t link_id, uint32_t delay_sec, uint32_t baud);
bool auncient_stanag_dtn_retransmit_frame(StanagDelayTolerantMeshState *state);
bool auncient_stanag_dtn_verify_theorems_141_145(StanagDelayTolerantMeshState *state);
uint32_t auncient_stanag_dtn_compute_rule18(const StanagDelayTolerantMeshState *state);

#endif /* AUNCIENT_STANAG5066_DELAY_TOLERANT_MESH_THEOREMS_141_145_H */
