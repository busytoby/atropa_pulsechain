#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_autodin_cumulative_gateway.h"

int autodin_cumulative_gateway_initialize(
    uint32_t gateway_identifier,
    autodin_cumulative_gateway_t *gateway
) {
    if (!gateway) return -1;

    memset(gateway, 0, sizeof(autodin_cumulative_gateway_t));
    gateway->gateway_identifier = gateway_identifier;

    // Security Governance Provenance Signature
    snprintf(gateway->security_governance_provenance_signature,
             sizeof(gateway->security_governance_provenance_signature),
             "SIG_2026_USLM_AFFIRMED");

    return 0;
}

int autodin_cumulative_gateway_verify_tape_label(
    autodin_cumulative_gateway_t *gateway,
    const uint8_t *tape_buffer_720bytes
) {
    if (!gateway || !tape_buffer_720bytes) return -1;

    // Validate 8-block 720-byte tape label header format
    if (memcmp(tape_buffer_720bytes, "VOL1", 4) != 0) {
        printf("[AUTOMATED DIGITAL NETWORK GATEWAY ERROR] Invalid Volume Header Magic!\n");
        gateway->total_rejected_batches++;
        return -1;
    }

    if (memcmp(tape_buffer_720bytes + 80, "HDR1", 4) != 0) {
        printf("[AUTOMATED DIGITAL NETWORK GATEWAY ERROR] Invalid Data Header Magic!\n");
        gateway->total_rejected_batches++;
        return -1;
    }

    // Check for provenance signature in header block 3
    if (strstr((const char *)(tape_buffer_720bytes + 160), "SIG_2026_USLM_AFFIRMED") == NULL &&
        strstr((const char *)(tape_buffer_720bytes + 240), "SIG_2026_USLM_AFFIRMED") == NULL) {
        printf("[AUTOMATED DIGITAL NETWORK GATEWAY ERROR] Missing Governance Provenance Signature!\n");
        gateway->total_rejected_batches++;
        return -1;
    }

    gateway->total_verified_batches++;
    printf("[AUTOMATED DIGITAL NETWORK GATEWAY SUCCESS] Tape Label Verification Passed for Gateway #%u\n", gateway->gateway_identifier);
    return 0;
}

int autodin_cumulative_gateway_process_batch(
    autodin_cumulative_gateway_t *gateway,
    autodin_cumulative_permutation_engine_t *engine
) {
    if (!gateway || !engine) return -1;

    uint64_t root_cumulative_hash = 0;
    if (autodin_cumulative_permutation_evaluate(engine, &root_cumulative_hash) != 0) {
        gateway->total_rejected_batches++;
        return -1;
    }

    uint8_t tape_buffer_720bytes[720];
    if (autodin_cumulative_permutation_inscribe_tape_label(engine, tape_buffer_720bytes) != 0) {
        gateway->total_rejected_batches++;
        return -1;
    }

    return autodin_cumulative_gateway_verify_tape_label(gateway, tape_buffer_720bytes);
}
