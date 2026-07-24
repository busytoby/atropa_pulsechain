#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Simulated Ackerman Collaborative Consensus Policy
typedef struct {
    bool node_approvals[3]; // Approvals from Node 0, 1, and 2
    gate_state_t policy_gate;
} ackerman_policy_t;

// -------------------------------------------------------------
// Ackerman Policy Negotiation (Bridging the Social-Technical Gap)
// -------------------------------------------------------------
bool evaluate_ackerman_policy(ackerman_policy_t *policy) {
    policy->policy_gate = CUTOFF_STATE;

    uint32_t approval_count = 0;
    for (int i = 0; i < 3; i++) {
        if (policy->node_approvals[i]) {
            approval_count++;
        }
    }

    // Social-Technical Gap Gating:
    // Instead of requiring rigid 100% absolute consensus for every minor change,
    // we support a flexible majority-based social negotiation policy (at least 2 approvals)
    if (approval_count >= 2) {
        policy->policy_gate = CONDUC_STATE;
        return true; // Negotiation succeeded. Conduction path open.
    }

    return false; // Negotiation failed. Path remains cutoff.
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ACKERMAN COLLABORATIVE CONSENSUS VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Compliant Case: Node 0 and Node 1 approve -> Should pass (majority)
    ackerman_policy_t policy_ok = {
        .node_approvals = { true, true, false },
        .policy_gate = CUTOFF_STATE
    };

    printf("[TEST] Evaluating policy with majority approvals (2/3)...\n");
    fflush(stdout);
    bool ok = evaluate_ackerman_policy(&policy_ok);
    assert(ok == true);
    assert(policy_ok.policy_gate == CONDUC_STATE);
    printf("   ✓ Policy approved. Conduction path open.\n");
    fflush(stdout);

    // 2. Reject Case: Only Node 0 approves -> Should fail
    ackerman_policy_t policy_fail = {
        .node_approvals = { true, false, false },
        .policy_gate = CUTOFF_STATE
    };

    printf("[TEST] Evaluating policy without majority approvals (1/3)...\n");
    fflush(stdout);
    ok = evaluate_ackerman_policy(&policy_fail);
    assert(ok == false);
    assert(policy_fail.policy_gate == CUTOFF_STATE);
    printf("   ✓ Insufficient approvals trapped successfully. Path cutoff.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("ACKERMAN CONSENSUS TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
