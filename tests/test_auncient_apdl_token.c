#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Coaxial Token-Ring Consensus State Map
typedef struct {
    uint32_t token_owner_id;        // Node ID currently holding the token (0, 1, 2)
    gate_state_t vdm_write_gates[3]; // Write gates for nodes 0, 1, and 2
    char node_values[3][64];         // Node local memory values
} token_ring_t;

// -------------------------------------------------------------
// VDM Transmit Gated by Coaxial Token
// -------------------------------------------------------------
bool vdm_transmit_payload(token_ring_t *ring, uint32_t node_id, const char *payload) {
    // Cut off target gate by default
    ring->vdm_write_gates[node_id] = CUTOFF_STATE;

    // Precondition check: Node must own the token to transmit
    if (ring->token_owner_id != node_id) {
        return false; // Collision blocked: Node does not hold token!
    }

    // Pass: Establish conduction and write payload
    ring->vdm_write_gates[node_id] = CONDUC_STATE;
    strcpy(ring->node_values[node_id], payload);

    // Pass Token to the next node in the ring
    ring->token_owner_id = (node_id + 1) % 3;

    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT APDL COAXIAL TOKEN-RING VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    token_ring_t ring = {
        .token_owner_id = 0, // Node 0 starts with the token
        .vdm_write_gates = { CUTOFF_STATE, CUTOFF_STATE, CUTOFF_STATE },
        .node_values = { "INIT_0", "INIT_1", "INIT_2" }
    };

    // 1. Node 0 transmits with token -> Should succeed
    printf("[TEST] Node 0 transmits payload (Holds Token)...\n");
    fflush(stdout);
    bool ok = vdm_transmit_payload(&ring, 0, "DATA_FROM_NODE_0");
    assert(ok == true);
    assert(ring.vdm_write_gates[0] == CONDUC_STATE);
    assert(strcmp(ring.node_values[0], "DATA_FROM_NODE_0") == 0);
    assert(ring.token_owner_id == 1); // Token passed to Node 1
    printf("   ✓ Node 0 transmission succeeded. Token passed to Node 1.\n");
    fflush(stdout);

    // 2. Node 2 attempts to transmit without token -> Should block
    printf("[TEST] Node 2 attempts transmission (Does NOT Hold Token)...\n");
    fflush(stdout);
    ok = vdm_transmit_payload(&ring, 2, "DATA_FROM_NODE_2");
    assert(ok == false);
    assert(ring.vdm_write_gates[2] == CUTOFF_STATE);
    assert(strcmp(ring.node_values[2], "INIT_2") == 0); // No change
    assert(ring.token_owner_id == 1); // Token remains with Node 1
    printf("   ✓ Out-of-turn transmission blocked successfully.\n");
    fflush(stdout);

    // 3. Node 1 transmits with token -> Should succeed
    printf("[TEST] Node 1 transmits payload (Holds Token)...\n");
    fflush(stdout);
    ok = vdm_transmit_payload(&ring, 1, "DATA_FROM_NODE_1");
    assert(ok == true);
    assert(ring.vdm_write_gates[1] == CONDUC_STATE);
    assert(strcmp(ring.node_values[1], "DATA_FROM_NODE_1") == 0);
    assert(ring.token_owner_id == 2); // Token passed to Node 2
    printf("   ✓ Node 1 transmission succeeded. Token passed to Node 2.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("APDL COAXIAL TOKEN-RING TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
