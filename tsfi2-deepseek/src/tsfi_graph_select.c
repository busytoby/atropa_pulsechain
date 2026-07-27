#include "tsfi_graph_select.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* --- 2-3 Tree Lifecycle Helpers --- */
TwoThreeNode* tsfi_create_23_node(bool is_leaf) {
    TwoThreeNode *node = (TwoThreeNode*)malloc(sizeof(TwoThreeNode));
    if (!node) return NULL;
    node->is_leaf = is_leaf;
    node->num_keys = 0;
    node->keys[0] = 0;
    node->keys[1] = 0;
    node->graphs[0] = NULL;
    node->graphs[1] = NULL;
    node->graphs[2] = NULL;
    node->children[0] = NULL;
    node->children[1] = NULL;
    node->children[2] = NULL;
    return node;
}

void tsfi_free_23_node(TwoThreeNode *node) {
    if (!node) return;
    if (!node->is_leaf) {
        for (size_t i = 0; i < node->num_keys + 1; i++) {
            tsfi_free_23_node(node->children[i]);
        }
    }
    for (size_t i = 0; i < 3; i++) {
        if (node->graphs[i]) {
            free(node->graphs[i]);
        }
    }
    free(node);
}

/* --- Verlet Integration FET Discharge Simulator --- */
static void simulate_fet_discharge(UndirectedGraph *g, size_t node_idx) {
    if (!g || node_idx >= g->size) return;
    
    /* Verlet integration step simulating soft-body mass-spring decay of FET charge state */
    float x = g->nodes[node_idx].feature;
    float x_prev = x * 1.05f;
    float k_spring = 0.5f;
    float force = -k_spring * x;
    float dt = 0.05f;
    float x_next = 2.0f * x - x_prev + force * dt * dt;
    
    g->nodes[node_idx].feature = (x_next < 0.0f) ? 0.0f : x_next;
}

/* --- GNN Pivot Predictor --- */
static float predict_pivot_score(const UndirectedGraph *g, size_t idx, const SimpleGNNWeights *gnn) {
    float self_val = g->nodes[idx].feature;
    float sum_neighbors = 0.0f;
    size_t deg = g->nodes[idx].degree;
    
    for (size_t i = 0; i < deg; i++) {
        size_t neighbor_idx = g->nodes[idx].neighbors[i];
        if (neighbor_idx < g->size) {
            sum_neighbors += g->nodes[neighbor_idx].feature;
        }
    }
    
    float neighbor_mean = deg > 0 ? sum_neighbors / deg : 0.0f;
    
    /* GNN Feedforward combined layers */
    float hidden = self_val * gnn->weights[0] + neighbor_mean * gnn->weights[1] + gnn->weights[2];
    if (hidden < 0.0f) hidden = 0.0f; /* ReLU activation */
    
    return hidden * gnn->weights[3] + gnn->weights[4];
}

/* --- Sorting Helpers for Min-Heap --- */
static void swap_nodes(GraphNode *a, GraphNode *b) {
    GraphNode tmp = *a;
    *a = *b;
    *b = tmp;
}

/* --- GNN-Weighted Min-Heap Tournament Selection --- */
static void min_heapify_nodes(GraphNode *nodes, size_t n, size_t i, const UndirectedGraph *g, const SimpleGNNWeights *gnn) {
    size_t smallest = i;
    size_t left = 2 * i + 1;
    size_t right = 2 * i + 2;

    /* Compute GNN-adjusted effective values for comparison */
    float val_s = (float)nodes[smallest].val;
    size_t id_s = (size_t)nodes[smallest].id;
    if (id_s < g->size) val_s -= predict_pivot_score(g, id_s, gnn);

    if (left < n) {
        float val_l = (float)nodes[left].val;
        size_t id_l = (size_t)nodes[left].id;
        if (id_l < g->size) val_l -= predict_pivot_score(g, id_l, gnn);
        if (val_l < val_s) {
            smallest = left;
            val_s = val_l;
        }
    }

    if (right < n) {
        float val_r = (float)nodes[right].val;
        size_t id_r = (size_t)nodes[right].id;
        if (id_r < g->size) val_r -= predict_pivot_score(g, id_r, gnn);
        if (val_r < val_s) {
            smallest = right;
        }
    }

    if (smallest != i) {
        swap_nodes(&nodes[i], &nodes[smallest]);
        
        /* Consider simulated hardware FET discharge cycle during tree updates */
        size_t displaced_idx = (size_t)nodes[smallest].id;
        simulate_fet_discharge((UndirectedGraph*)g, displaced_idx);

        min_heapify_nodes(nodes, n, smallest, g, gnn);
    }
}

GraphNode tsfi_adaptive_graph_select(UndirectedGraph *g, size_t k, const SimpleGNNWeights *weights) {
    if (!g || g->size == 0 || k >= g->size) {
        GraphNode dummy = {0, 0, 0.0f, 0, {0}};
        return dummy;
    }

    /* Allocate working workspace array */
    GraphNode *temp_nodes = (GraphNode*)malloc(g->size * sizeof(GraphNode));
    memcpy(temp_nodes, g->nodes, g->size * sizeof(GraphNode));

    /* Build min-heap: O(N) */
    for (int i = (int)(g->size / 2) - 1; i >= 0; i--) {
        min_heapify_nodes(temp_nodes, g->size, (size_t)i, g, weights);
    }

    /* Extract minimum element K times to find the target node: O(K log N) */
    for (size_t i = 0; i < k; i++) {
        swap_nodes(&temp_nodes[0], &temp_nodes[g->size - 1 - i]);
        
        /* Trigger FET discharge cycle on the winning selection candidate */
        size_t winner_idx = (size_t)temp_nodes[0].id;
        simulate_fet_discharge(g, winner_idx);

        min_heapify_nodes(temp_nodes, g->size - 1 - i, 0, g, weights);
    }

    GraphNode result = temp_nodes[0];
    free(temp_nodes);
    return result;
}
