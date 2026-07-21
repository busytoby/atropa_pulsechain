#ifndef TSFI_AUTODIN_CUMULATIVE_GATEWAY_H
#define TSFI_AUTODIN_CUMULATIVE_GATEWAY_H

#include <stdint.h>
#include <stddef.h>
#include "tsfi_autodin_cumulative_permutation.h"
#include "tsfi_autodin_cumulative_telemetry.h"

typedef struct {
    uint32_t gateway_identifier;
    uint32_t total_verified_batches;
    uint32_t total_rejected_batches;
    char security_governance_provenance_signature[64]; // SIG_2026_USLM_AFFIRMED
} autodin_cumulative_gateway_t;

/* Initialize Automated Digital Network Cumulative Transaction Verification Gateway */
int autodin_cumulative_gateway_initialize(
    uint32_t gateway_identifier,
    autodin_cumulative_gateway_t *gateway
);

/* Verify 720-byte Yul Data Definition Language tape label governance signature and state invariants */
int autodin_cumulative_gateway_verify_tape_label(
    autodin_cumulative_gateway_t *gateway,
    const uint8_t *tape_buffer_720bytes
);

/* Execute full governance check and dynamic contract address resolution on cumulative transaction batch */
int autodin_cumulative_gateway_process_batch(
    autodin_cumulative_gateway_t *gateway,
    autodin_cumulative_permutation_engine_t *engine
);

#endif // TSFI_AUTODIN_CUMULATIVE_GATEWAY_H
