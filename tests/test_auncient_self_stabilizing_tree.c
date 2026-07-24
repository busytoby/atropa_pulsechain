#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#define NUM_NODES 4
#define INF 9999

typedef struct {
    int parent_id;
    int distance;
} bfs_node_t;

// Adjacency matrix representing the network topology (coaxial links)
// Node 0 - Node 1 - Node 2 - Node 3
static const bool adj_matrix[NUM_NODES][NUM_NODES] = {
    { false, true,  false, false }, // Node 0 (Root)
    { true,  false, true,  false }, // Node 1
    { false, true,  false, true  }, // Node 2
    { false, false, true,  false }  // Node 3
};

// -------------------------------------------------------------
// Self-Stabilizing BFS Spanning Tree (Dolev-Israeli-Welch)
// -------------------------------------------------------------
bool self_stabilize_round(bfs_node_t *nodes) {
    bool changed = false;

    // Node 0 is the static root
    if (nodes[0].distance != 0 || nodes[0].parent_id != 0) {
        nodes[0].distance = 0;
        nodes[0].parent_id = 0;
        changed = true;
    }

    // Every other node updates based on active neighbor distances
    for (int i = 1; i < NUM_NODES; i++) {
        int min_dist = INF;
        int best_parent = -1;

        for (int j = 0; j < NUM_NODES; j++) {
            if (adj_matrix[i][j]) {
                if (nodes[j].distance < min_dist) {
                    min_dist = nodes[j].distance;
                    best_parent = j;
                }
            }
        }

        int target_dist = (best_parent != -1) ? (min_dist + 1) : INF;
        if (nodes[i].distance != target_dist || nodes[i].parent_id != best_parent) {
            nodes[i].distance = target_dist;
            nodes[i].parent_id = best_parent;
            changed = true;
        }
    }

    return changed;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SELF-STABILIZING SPANNING TREE SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initialize nodes in a corrupted state (routing loop / random metrics)
    bfs_node_t nodes[NUM_NODES] = {
        { .parent_id = 2, .distance = 5 },   // Root (corrupted)
        { .parent_id = 2, .distance = 10 },  // Node 1 (looping)
        { .parent_id = 1, .distance = 10 },  // Node 2 (looping)
        { .parent_id = -1, .distance = INF } // Node 3
    };

    printf("[TEST] Running self-stabilization loop from corrupted state...\n");
    fflush(stdout);

    // Run convergence iterations
    int rounds = 0;
    while (self_stabilize_round(nodes) && rounds < 10) {
        rounds++;
        printf("   Round %d: Node1(P:%d, D:%d), Node2(P:%d, D:%d), Node3(P:%d, D:%d)\n",
               rounds, nodes[1].parent_id, nodes[1].distance,
               nodes[2].parent_id, nodes[2].distance,
               nodes[3].parent_id, nodes[3].distance);
        fflush(stdout);
    }

    // Verify BFS spanning tree properties
    // Root should stabilize to 0
    assert(nodes[0].distance == 0);
    assert(nodes[0].parent_id == 0);

    // Node 1 is 1 hop from Root
    assert(nodes[1].distance == 1);
    assert(nodes[1].parent_id == 0);

    // Node 2 is 2 hops from Root (via Node 1)
    assert(nodes[2].distance == 2);
    assert(nodes[2].parent_id == 1);

    // Node 3 is 3 hops from Root (via Node 2)
    assert(nodes[3].distance == 3);
    assert(nodes[3].parent_id == 2);

    printf("   ✓ Network converged to correct BFS tree in %d rounds.\n", rounds);
    fflush(stdout);

    printf("=============================================================\n");
    printf("SELF-STABILIZING TREE TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
