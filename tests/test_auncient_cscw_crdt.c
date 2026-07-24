#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// Simulated CSCW LWW-Register CRDT
typedef struct {
    uint32_t value;
    uint64_t timestamp;
    uint32_t node_id; // Tie-breaker node identifier
} crdt_register_t;

// -------------------------------------------------------------
// CSCW CRDT Last-Write-Wins Merge Rules
// -------------------------------------------------------------
bool merge_crdt_register(crdt_register_t *local, const crdt_register_t *incoming) {
    // Rule 1: Accept incoming if its timestamp is strictly higher
    if (incoming->timestamp > local->timestamp) {
        local->value = incoming->value;
        local->timestamp = incoming->timestamp;
        local->node_id = incoming->node_id;
        return true;
    }

    // Rule 2: If timestamps are identical, resolve tie-breaker using higher node ID
    if (incoming->timestamp == local->timestamp && incoming->node_id > local->node_id) {
        local->value = incoming->value;
        local->node_id = incoming->node_id;
        return true;
    }

    // Otherwise, discard the incoming change (older or lower priority)
    return false;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT CSCW CRDT CONVERGENCE SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    crdt_register_t local_reg = {
        .value = 10,
        .timestamp = 1000,
        .node_id = 1
    };

    // 1. Compliant Case: Incoming has newer timestamp (1001) -> Should update
    crdt_register_t incoming_newer = {
        .value = 20,
        .timestamp = 1001,
        .node_id = 2
    };

    printf("[TEST] Merging newer incoming CRDT write...\n");
    fflush(stdout);
    bool updated = merge_crdt_register(&local_reg, &incoming_newer);
    assert(updated == true);
    assert(local_reg.value == 20);
    assert(local_reg.timestamp == 1001);
    printf("   ✓ Local register successfully updated to newer value.\n");
    fflush(stdout);

    // 2. Erroneous/Stale Case: Incoming has older timestamp (999) -> Should reject
    crdt_register_t incoming_stale = {
        .value = 30,
        .timestamp = 999,
        .node_id = 3
    };

    printf("[TEST] Merging stale incoming CRDT write...\n");
    fflush(stdout);
    updated = merge_crdt_register(&local_reg, &incoming_stale);
    assert(updated == false);
    assert(local_reg.value == 20); // Remains unchanged
    printf("   ✓ Stale update successfully discarded. Local state preserved.\n");
    fflush(stdout);

    // 3. Tie-breaker Case: Identical timestamp, higher node ID -> Should update
    crdt_register_t incoming_tie = {
        .value = 40,
        .timestamp = 1001, // Identical to local
        .node_id = 5       // Higher than local node_id (2)
    };

    printf("[TEST] Merging identical timestamp with higher node ID (tie-breaker)...\n");
    fflush(stdout);
    updated = merge_crdt_register(&local_reg, &incoming_tie);
    assert(updated == true);
    assert(local_reg.value == 40);
    assert(local_reg.node_id == 5);
    printf("   ✓ Local register updated via node tie-breaker rules.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("CSCW CRDT TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
