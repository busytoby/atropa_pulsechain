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

// Simulated Pease-Shostak-Lamport Byzantine proposal packet with hash authentication
typedef struct {
    uint32_t sender_node;
    uint32_t target_node;
    uint32_t value;
    uint32_t signature; // Simple signed checksum hash: (sender_node + value) * 17
    double carrier_freq;
    double quality_factor;
} byzantine_packet_t;

typedef struct {
    uint32_t node_id;
    bool byzantine_flagged[3];
    uint32_t received_values[3];
} consensus_node_t;

// Helper to compute signature
static inline uint32_t sign_value(uint32_t sender, uint32_t val) {
    return (sender + val) * 17;
}

// -------------------------------------------------------------
// SIGACT Byzantine Fault Detection
// -------------------------------------------------------------
bool process_byzantine_proposal(consensus_node_t *local_node, const byzantine_packet_t *pkt) {
    // 0. Physical Layer Audit (Fourier Gate): Verify carrier frequency & quality factor
    bool fourier_ok = (pkt->carrier_freq == 440.0 && pkt->quality_factor >= 0.8);
    if (!fourier_ok) {
        local_node->byzantine_flagged[pkt->sender_node] = true;
        return false; // Physical anomaly -> Flagged as Byzantine
    }

    // 1. Verify cryptographic signature/checksum
    uint32_t expected_sig = sign_value(pkt->sender_node, pkt->value);
    if (pkt->signature != expected_sig) {
        local_node->byzantine_flagged[pkt->sender_node] = true;
        return false; // Signature mismatch -> Flagged as Byzantine
    }

    // 2. Traitor Detection: Check for conflicting values sent by the same node
    if (local_node->received_values[pkt->sender_node] != 0 &&
        local_node->received_values[pkt->sender_node] != pkt->value) {
        local_node->byzantine_flagged[pkt->sender_node] = true;
        return false; // Traitor detected -> Conflicting signed values
    }

    local_node->received_values[pkt->sender_node] = pkt->value;
    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SIGACT BYZANTINE FAULT TOLERANCE SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    consensus_node_t node1 = {
        .node_id = 1,
        .byzantine_flagged = { false, false, false },
        .received_values = { 0, 0, 0 }
    };

    // 1. Compliant Case: Consistent proposal from Node 0 -> Should pass
    byzantine_packet_t pkt_ok = {
        .sender_node = 0,
        .target_node = 1,
        .value = 100,
        .signature = sign_value(0, 100),
        .carrier_freq = 440.0,
        .quality_factor = 0.85
    };

    printf("[TEST] Processing compliant proposal from Node 0...\n");
    fflush(stdout);
    bool ok = process_byzantine_proposal(&node1, &pkt_ok);
    assert(ok == true);
    assert(node1.byzantine_flagged[0] == false);
    printf("   ✓ Proposal accepted successfully.\n");
    fflush(stdout);

    // 2. Traitor Case: Node 0 attempts to send a conflicting proposal to Node 1 -> Should flag
    byzantine_packet_t pkt_conflict = {
        .sender_node = 0,
        .target_node = 1,
        .value = 200, // Conflict: 200 vs previously sent 100
        .signature = sign_value(0, 200),
        .carrier_freq = 440.0,
        .quality_factor = 0.85
    };

    printf("[TEST] Processing conflicting proposal from Node 0 (Traitor check)...\n");
    fflush(stdout);
    ok = process_byzantine_proposal(&node1, &pkt_conflict);
    assert(ok == false);
    assert(node1.byzantine_flagged[0] == true);
    printf("   ✓ Byzantine traitor detected and flagged successfully.\n");
    fflush(stdout);

    // 3. Fourier Gate Case: Node 2 attempts transmission with off-frequency carrier -> Should block and flag
    consensus_node_t node2 = {
        .node_id = 2,
        .byzantine_flagged = { false, false, false },
        .received_values = { 0, 0, 0 }
    };
    byzantine_packet_t pkt_de_tuned = {
        .sender_node = 0,
        .target_node = 2,
        .value = 300,
        .signature = sign_value(0, 300),
        .carrier_freq = 415.0, // De-tuned carrier
        .quality_factor = 0.50  // Low quality factor
    };

    printf("[TEST] Processing de-tuned proposal from Node 0 (Fourier Gate check)...\n");
    fflush(stdout);
    ok = process_byzantine_proposal(&node2, &pkt_de_tuned);
    assert(ok == false);
    assert(node2.byzantine_flagged[0] == true);
    printf("   ✓ Physical signal anomaly trapped. Node 0 flagged as Byzantine.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("SIGACT BYZANTINE TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
