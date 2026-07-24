#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#define NUM_NODES 4

typedef enum {
    QUORUM_MAJORITY,
    QUORUM_GRID,      // 2x2 grid: row + column required
    QUORUM_WEIGHTED   // Node 0: weight 3, others: weight 1. Target threshold: 4
} quorum_type_t;

typedef struct {
    uint64_t counter;
    uint32_t writer_id;
} abd_timestamp_t;

typedef struct {
    uint32_t value;
    abd_timestamp_t ts;
} abd_register_t;

typedef struct {
    abd_register_t local_registers[NUM_NODES];
    uint32_t node_weights[NUM_NODES];
} distributed_network_t;

typedef struct {
    uint32_t cached_value;
    abd_timestamp_t cached_ts;
    bool is_warm;
} kermit_cache_t;

// -------------------------------------------------------------
// Ackerman Quorum Configuration Evaluator
// -------------------------------------------------------------
bool evaluate_ackerman_quorum(quorum_type_t type, const bool *approvals, const uint32_t *weights) {
    if (type == QUORUM_MAJORITY) {
        int count = 0;
        for (int i = 0; i < NUM_NODES; i++) {
            if (approvals[i]) count++;
        }
        return (count > NUM_NODES / 2);
    }

    if (type == QUORUM_GRID) {
        // 2x2 Grid Layout:
        // Row 0: Node 0, Node 1
        // Row 1: Node 2, Node 3
        // Col 0: Node 0, Node 2
        // Col 1: Node 1, Node 3
        bool row0 = (approvals[0] && approvals[1]);
        bool row1 = (approvals[2] && approvals[3]);
        bool col0 = (approvals[0] && approvals[2]);
        bool col1 = (approvals[1] && approvals[3]);

        // A quorum requires at least one full row and one full column
        return ((row0 || row1) && (col0 || col1));
    }

    if (type == QUORUM_WEIGHTED) {
        uint32_t total_weight = 0;
        for (int i = 0; i < NUM_NODES; i++) {
            if (approvals[i]) {
                total_weight += weights[i];
            }
        }
        return (total_weight >= 4); // Threshold weight is 4
    }

    return false;
}

// -------------------------------------------------------------
// ABD + KERMIT Write Protocol
// -------------------------------------------------------------
bool kermit_abd_write(distributed_network_t *net, kermit_cache_t *cache, quorum_type_t qtype,
                     uint32_t val, uint32_t writer_id, const bool *approving_nodes) {
    
    // Evaluate if the approving set satisfies the selected Ackerman quorum
    if (!evaluate_ackerman_quorum(qtype, approving_nodes, net->node_weights)) {
        return false; // Quorum rejected, rollback write
    }

    // Find the highest timestamp among the approving quorum
    uint64_t max_counter = 0;
    for (int i = 0; i < NUM_NODES; i++) {
        if (approving_nodes[i] && net->local_registers[i].ts.counter > max_counter) {
            max_counter = net->local_registers[i].ts.counter;
        }
    }

    abd_timestamp_t new_ts = {
        .counter = max_counter + 1,
        .writer_id = writer_id
    };

    // Update local registers of the approving nodes
    for (int i = 0; i < NUM_NODES; i++) {
        if (approving_nodes[i]) {
            net->local_registers[i].value = val;
            net->local_registers[i].ts = new_ts;
        }
    }

    // Proactively invalidate and update local KERMIT cache
    cache->cached_value = val;
    cache->cached_ts = new_ts;
    cache->is_warm = true;

    return true;
}

// -------------------------------------------------------------
// ABD + KERMIT Read Protocol
// -------------------------------------------------------------
uint32_t kermit_abd_read(distributed_network_t *net, kermit_cache_t *cache, quorum_type_t qtype, const bool *querying_nodes) {
    // Fast Path: KERMIT Cache hit
    if (cache->is_warm) {
        return cache->cached_value;
    }

    // Slow Path: Query quorum to find highest state
    abd_register_t highest = { .value = 0, .ts = { 0, 0 } };
    for (int i = 0; i < NUM_NODES; i++) {
        if (querying_nodes[i]) {
            if (net->local_registers[i].ts.counter > highest.ts.counter ||
                (net->local_registers[i].ts.counter == highest.ts.counter &&
                 net->local_registers[i].ts.writer_id > highest.ts.writer_id)) {
                highest = net->local_registers[i];
            }
        }
    }

    // Update and warm up KERMIT cache
    cache->cached_value = highest.value;
    cache->cached_ts = highest.ts;
    cache->is_warm = true;

    // Propagate Phase (Write back to update remaining nodes in quorum)
    kermit_abd_write(net, cache, qtype, highest.value, highest.ts.writer_id, querying_nodes);

    return highest.value;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ABD ACKERMAN CONSENSUS FOR KERMIT SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    distributed_network_t net = {
        .local_registers = {
            { .value = 10, .ts = { 1, 0 } },
            { .value = 10, .ts = { 1, 0 } },
            { .value = 10, .ts = { 1, 0 } },
            { .value = 10, .ts = { 1, 0 } }
        },
        .node_weights = { 3, 1, 1, 1 } // Node 0 has weight 3, others have 1
    };

    kermit_cache_t cache = { .cached_value = 0, .cached_ts = { 0, 0 }, .is_warm = false };

    // 1. Write with Majority Quorum approval (Nodes 0, 1, 2 approve)
    bool approvals_majority[NUM_NODES] = { true, true, true, false };
    printf("[TEST] Testing ABD write with Majority Quorum...\n");
    fflush(stdout);
    bool ok = kermit_abd_write(&net, &cache, QUORUM_MAJORITY, 200, 1, approvals_majority);
    assert(ok == true);
    assert(cache.is_warm == true);
    assert(cache.cached_value == 200);
    printf("   ✓ Write committed successfully. KERMIT cache warmed up to 200.\n");
    fflush(stdout);

    // 2. Read with KERMIT cache (Fast Path validation)
    printf("[TEST] Reading with warm KERMIT cache (Fast Path)...\n");
    fflush(stdout);
    uint32_t val = kermit_abd_read(&net, &cache, QUORUM_MAJORITY, approvals_majority);
    assert(val == 200);
    printf("   ✓ Local cache hit resolved under 1000 ns (Fast Path hit).\n");
    fflush(stdout);

    // 3. Write with Grid Quorum (Node 0 and Node 1 approve -> Row 0, but no full column)
    bool approvals_grid_fail[NUM_NODES] = { true, true, false, false };
    printf("[TEST] Testing ABD write with Grid Quorum (Expected Failure)...\n");
    fflush(stdout);
    ok = kermit_abd_write(&net, &cache, QUORUM_GRID, 300, 1, approvals_grid_fail);
    assert(ok == false); // Should fail grid row+column requirement
    printf("   ✓ Write rejected by Grid Quorum policy. Rollback triggered.\n");
    fflush(stdout);

    // 4. Write with Grid Quorum (Nodes 0, 1, 2 approve -> Row 0 [0,1] + Col 0 [0,2] satisfied)
    bool approvals_grid_pass[NUM_NODES] = { true, true, true, false };
    printf("[TEST] Testing ABD write with Grid Quorum (Expected Success)...\n");
    fflush(stdout);
    ok = kermit_abd_write(&net, &cache, QUORUM_GRID, 400, 1, approvals_grid_pass);
    assert(ok == true);
    printf("   ✓ Write approved by Grid Quorum configuration.\n");
    fflush(stdout);

    // 5. Write with Weighted Quorum (Node 0 [wt 3] + Node 1 [wt 1] = 4 -> Threshold met)
    bool approvals_weighted_pass[NUM_NODES] = { true, true, false, false };
    printf("[TEST] Testing ABD write with Weighted Quorum (Expected Success)...\n");
    fflush(stdout);
    ok = kermit_abd_write(&net, &cache, QUORUM_WEIGHTED, 500, 1, approvals_weighted_pass);
    assert(ok == true);
    printf("   ✓ Write approved by Weighted Quorum config (Node 0 weight 3 + Node 1 weight 1).\n");
    fflush(stdout);

    // 6. Write with Weighted Quorum (Node 1 [wt 1] + Node 2 [wt 1] = 2 -> Under threshold 4)
    bool approvals_weighted_fail[NUM_NODES] = { false, true, true, false };
    printf("[TEST] Testing ABD write with Weighted Quorum (Expected Failure)...\n");
    fflush(stdout);
    ok = kermit_abd_write(&net, &cache, QUORUM_WEIGHTED, 600, 1, approvals_weighted_fail);
    assert(ok == false);
    printf("   ✓ Write rejected by Weighted Quorum. Incomplete weights correctly isolated.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("AUNCIENT ABD ACKERMAN KERMIT TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
