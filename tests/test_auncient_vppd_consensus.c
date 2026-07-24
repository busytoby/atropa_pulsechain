#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define NODE_COUNT 4
#define QUORUM_THRESHOLD 3

typedef enum {
    SEC_UNCLASSIFIED = 1,
    SEC_SECRET = 2,
    SEC_TOP_SECRET = 3
} security_level_t;

typedef struct {
    char name[32];
    int64_t initial_val;
    security_level_t required_level;
} proposal_t;

typedef struct {
    uint32_t node_id;
    security_level_t clearance;
} validator_node_t;

// Ackerman communal proof container
typedef struct {
    uint32_t signature_mask;
    uint32_t nonce;
} ackerman_proof_t;

// Validator evaluates proposal and returns signature bit if approved
static uint32_t validator_sign_proposal(const validator_node_t *node, const proposal_t *prop) {
    if (node->clearance >= prop->required_level) {
        return (1U << node->node_id); // Return validator signature bit
    }
    return 0; // Rejected
}

// Coordinator collects signatures and verifies Ackerman quorum
static bool verify_communal_quorum(const ackerman_proof_t *proof) {
    int count = 0;
    for (int i = 0; i < NODE_COUNT; i++) {
        if ((proof->signature_mask >> i) & 1) {
            count++;
        }
    }
    return (count >= QUORUM_THRESHOLD);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT VPPD ACKERMAN COMMUNAL CONSENSUS SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initialize 4 simulated validator containers
    validator_node_t network[NODE_COUNT] = {
        { .node_id = 0, .clearance = SEC_SECRET },
        { .node_id = 1, .clearance = SEC_SECRET },
        { .node_id = 2, .clearance = SEC_SECRET },
        { .node_id = 3, .clearance = SEC_TOP_SECRET } // High clearance node
    };

    // 1. Proposal A: Secret-level schema addition (Quorum expected)
    proposal_t prop_a = {
        .name = "dynamic_reg_secret",
        .initial_val = 100,
        .required_level = SEC_SECRET
    };

    printf("[TEST] Broadcasting Proposal A (Requires SECRET clearance)...\n");
    fflush(stdout);

    ackerman_proof_t proof_a = { .signature_mask = 0, .nonce = 1001 };
    for (int i = 0; i < NODE_COUNT; i++) {
        proof_a.signature_mask |= validator_sign_proposal(&network[i], &prop_a);
    }

    printf("   ✓ Signature mask gathered: 0x%02X (Validators signed: ", proof_a.signature_mask);
    for (int i = 0; i < NODE_COUNT; i++) {
        if ((proof_a.signature_mask >> i) & 1) printf("%d ", i);
    }
    printf(")\n");
    fflush(stdout);

    bool ok_a = verify_communal_quorum(&proof_a);
    assert(ok_a == true);
    printf("   ✓ Ackerman communal quorum verified. Proposal A accepted.\n");
    fflush(stdout);

    // 2. Proposal B: Top Secret-level schema addition (Quorum failure expected)
    proposal_t prop_b = {
        .name = "dynamic_reg_topsecret",
        .initial_val = 500,
        .required_level = SEC_TOP_SECRET
    };

    printf("[TEST] Broadcasting Proposal B (Requires TOP_SECRET clearance)...\n");
    fflush(stdout);

    ackerman_proof_t proof_b = { .signature_mask = 0, .nonce = 1002 };
    for (int i = 0; i < NODE_COUNT; i++) {
        proof_b.signature_mask |= validator_sign_proposal(&network[i], &prop_b);
    }

    printf("   ✓ Signature mask gathered: 0x%02X (Validators signed: ", proof_b.signature_mask);
    for (int i = 0; i < NODE_COUNT; i++) {
        if ((proof_b.signature_mask >> i) & 1) printf("%d ", i);
    }
    printf(")\n");
    fflush(stdout);

    bool ok_b = verify_communal_quorum(&proof_b);
    assert(ok_b == false); // Insufficient signatures (only node 3 has TOP_SECRET)
    printf("   ✓ Ackerman communal quorum rejected. Proposal B blocked at boundary.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("ACKERMAN CONSENSUS SIMULATION COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
