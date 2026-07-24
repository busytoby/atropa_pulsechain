#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define CLUSTER_SIZE 3

// Kermit Extended Packet
typedef struct {
    char type;             // 'S' (Proposal), 'A' (Approval), 'Y' (ACK), 'N' (NAK)
    uint32_t sender_node;
    uint32_t state_hash;   // Proposed post-transaction state hash
} kermit_extended_packet_t;

// Simulated Kermit Receiver State Machine
typedef struct {
    uint32_t proposed_hashes[CLUSTER_SIZE];
    bool proposal_received[CLUSTER_SIZE];
    uint32_t vote_count;
} kermit_receiver_t;

// -------------------------------------------------------------
// Kermit State Consensus Receiver Logic
// -------------------------------------------------------------
char process_kermit_consensus_packet(kermit_receiver_t *receiver, const kermit_extended_packet_t *pkt) {
    if (pkt->type == 'S') {
        if (pkt->sender_node < CLUSTER_SIZE) {
            receiver->proposed_hashes[pkt->sender_node] = pkt->state_hash;
            receiver->proposal_received[pkt->sender_node] = true;
        }
    }

    // Check if proposals from all nodes have been received
    bool all_received = true;
    for (int i = 0; i < CLUSTER_SIZE; i++) {
        if (!receiver->proposal_received[i]) {
            all_received = false;
            break;
        }
    }

    if (all_received) {
        // Count matches of proposed state hashes
        for (int i = 0; i < CLUSTER_SIZE; i++) {
            uint32_t matches = 0;
            uint32_t target_hash = receiver->proposed_hashes[i];
            for (int j = 0; j < CLUSTER_SIZE; j++) {
                if (receiver->proposed_hashes[j] == target_hash) {
                    matches++;
                }
            }

            // Ackerman Consensus Rule: Majority approval (>= 2 out of 3)
            if (matches >= 2) {
                return 'Y'; // ACK: Consensus reached, commit state
            }
        }
        return 'N'; // NAK: Disagreement, trigger converse rollback
    }

    return 'W'; // WAIT: Still collecting proposals
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT KERMIT POST-TRANSACTION CONSENSUS SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    kermit_receiver_t receiver;
    memset(&receiver, 0, sizeof(receiver));

    // 1. Compliant Case: Nodes 0 and 1 propose state hash 0xABC, Node 2 proposes 0x999 -> Should ACK
    kermit_extended_packet_t p0 = { .type = 'S', .sender_node = 0, .state_hash = 0xABC };
    kermit_extended_packet_t p1 = { .type = 'S', .sender_node = 1, .state_hash = 0xABC };
    kermit_extended_packet_t p2 = { .type = 'S', .sender_node = 2, .state_hash = 0x999 };

    printf("[TEST] Dispatching state proposals with majority agreement (0xABC)...\n");
    fflush(stdout);

    char res = process_kermit_consensus_packet(&receiver, &p0);
    assert(res == 'W'); // Still waiting

    res = process_kermit_consensus_packet(&receiver, &p1);
    assert(res == 'W'); // Still waiting

    res = process_kermit_consensus_packet(&receiver, &p2);
    assert(res == 'Y'); // Consensus reached! (ACK)
    printf("   ✓ Consensus reached (ACK resolved). Target state committed.\n");
    fflush(stdout);

    // 2. Reject Case: Three divergent state proposals (no majority) -> Should NAK
    memset(&receiver, 0, sizeof(receiver));
    p0.state_hash = 0x111;
    p1.state_hash = 0x222;
    p2.state_hash = 0x333;

    printf("[TEST] Dispatching divergent state proposals...\n");
    fflush(stdout);

    res = process_kermit_consensus_packet(&receiver, &p0);
    assert(res == 'W');

    res = process_kermit_consensus_packet(&receiver, &p1);
    assert(res == 'W');

    res = process_kermit_consensus_packet(&receiver, &p2);
    assert(res == 'N'); // Consensus failed! (NAK)
    printf("   ✓ Consensus failed (NAK resolved). Rollback triggered.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("KERMIT POST-TRANSACTION TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
