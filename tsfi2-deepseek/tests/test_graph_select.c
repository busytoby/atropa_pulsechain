#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "tsfi_graph_select.h"

int main(void) {
    srand((unsigned int)time(NULL));
    printf("=== Auncient GNN-Assisted Adaptive Graph Selection Test ===\n");

    /* 1. Setup 2-3 Tree structure containing Graph data */
    TwoThreeNode *root = tsfi_create_23_node(true);
    assert(root);

    UndirectedGraph *g = (UndirectedGraph*)malloc(sizeof(UndirectedGraph));
    assert(g);
    g->size = 10;

    /* Populate Graph nodes with values and features */
    uint64_t expected_sorted[10];
    for (size_t i = 0; i < 10; i++) {
        g->nodes[i].id = i;
        g->nodes[i].val = (uint64_t)(rand() % 1000 + 50);
        g->nodes[i].feature = (float)(rand() % 100) / 10.0f;
        g->nodes[i].degree = 2;
        /* Cycle neighbors graph topology */
        g->nodes[i].neighbors[0] = (i + 1) % 10;
        g->nodes[i].neighbors[1] = (i + 9) % 10;
        expected_sorted[i] = g->nodes[i].val;
    }

    /* Sort expected values for verification reference */
    for (size_t i = 0; i < 10; i++) {
        for (size_t j = i + 1; j < 10; j++) {
            if (expected_sorted[i] > expected_sorted[j]) {
                uint64_t tmp = expected_sorted[i];
                expected_sorted[i] = expected_sorted[j];
                expected_sorted[j] = tmp;
            }
        }
    }

    /* Attach graph to the 2-3 tree leaf node */
    root->graphs[0] = g;
    root->num_keys = 1;
    root->keys[0] = 500; /* Arbitrary partition key */

    /* 2. Instantiate Simple GNN weights for pivot prediction */
    SimpleGNNWeights gnn = {
        .weights = {0.4f, 0.6f, 0.1f, 0.8f, 0.2f}
    };

    float init_fet_sum = 0.0f;
    for (size_t i = 0; i < 10; i++) {
        init_fet_sum += root->graphs[0]->nodes[i].feature;
    }
    
    GraphNode min_node = tsfi_adaptive_graph_select(root->graphs[0], 0, &gnn);
    GraphNode med_node = tsfi_adaptive_graph_select(root->graphs[0], 4, &gnn);
    GraphNode max_node = tsfi_adaptive_graph_select(root->graphs[0], 9, &gnn);
 
    printf("  [VERIFY] Rank 0 (Min):   Selected val = %lu, Expected = %lu\n", min_node.val, expected_sorted[0]);
    printf("  [VERIFY] Rank 4 (Med):   Selected val = %lu, Expected = %lu\n", med_node.val, expected_sorted[4]);
    printf("  [VERIFY] Rank 9 (Max):   Selected val = %lu, Expected = %lu\n", max_node.val, expected_sorted[9]);
 
    assert(min_node.val == expected_sorted[0]);
    assert(med_node.val == expected_sorted[4]);
    assert(max_node.val == expected_sorted[9]);

    float post_fet_sum = 0.0f;
    for (size_t i = 0; i < 10; i++) {
        post_fet_sum += root->graphs[0]->nodes[i].feature;
    }
    printf("  [VERIFY] Verlet FET sum decay: initial=%.2f, post=%.2f\n", init_fet_sum, post_fet_sum);
    assert(post_fet_sum < init_fet_sum);

    /* Cleanup */
    tsfi_free_23_node(root);
    printf("All GNN-assisted graph selection verification checks PASSED!\n");
    return 0;
}
