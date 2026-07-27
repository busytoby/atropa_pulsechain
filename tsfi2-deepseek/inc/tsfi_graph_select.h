#ifndef TSFI_GRAPH_SELECT_H
#define TSFI_GRAPH_SELECT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MAX_GRAPH_NEIGHBORS 8
#define MAX_GRAPH_NODES 32

typedef struct {
    uint64_t id;
    uint64_t val;
    float feature;
    size_t degree;
    size_t neighbors[MAX_GRAPH_NEIGHBORS];
} GraphNode;

typedef struct {
    GraphNode nodes[MAX_GRAPH_NODES];
    size_t size;
} UndirectedGraph;

typedef struct TwoThreeNode {
    bool is_leaf;
    size_t num_keys;
    uint64_t keys[2];
    UndirectedGraph *graphs[3]; /* Graphs associated with keys/children */
    struct TwoThreeNode *children[3];
} TwoThreeNode;

/**
 * @brief Simple neural projection layer weights (5 parameters).
 */
typedef struct {
    float weights[5];
} SimpleGNNWeights;

/**
 * @brief Selects the k-th smallest node (by val) in an UndirectedGraph using GNN-assisted pivot selection.
 *        If partition splits degrade, it automatically switches to deterministic Median-of-Medians.
 * @param g The graph.
 * @param k The target rank (0-indexed).
 * @param weights The GNN weights for pivot prediction.
 * @return The k-th smallest GraphNode.
 */
GraphNode tsfi_adaptive_graph_select(UndirectedGraph *g, size_t k, const SimpleGNNWeights *weights);

/**
 * @brief Initialize a new 2-3 tree node.
 */
TwoThreeNode* tsfi_create_23_node(bool is_leaf);

/**
 * @brief Free a 2-3 tree node recursively.
 */
void tsfi_free_23_node(TwoThreeNode *node);

#endif /* TSFI_GRAPH_SELECT_H */
