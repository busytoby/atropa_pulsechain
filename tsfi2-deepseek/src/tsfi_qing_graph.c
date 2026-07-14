#include "tsfi_qing_graph.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void tsfi_qing_graph_init(tsfi_qing_graph_node nodes[TSFI_NET_COUNT]) {
    nodes[TSFI_NET_ZMM].net_id = TSFI_NET_ZMM;
    nodes[TSFI_NET_ZMM].net_name = "ZMM";
    nodes[TSFI_NET_ZMM].bst_root = NULL;
    nodes[TSFI_NET_ZMM].adj_count = 0;

    nodes[TSFI_NET_ANVIL_EVM].net_id = TSFI_NET_ANVIL_EVM;
    nodes[TSFI_NET_ANVIL_EVM].net_name = "Anvil_EVM";
    nodes[TSFI_NET_ANVIL_EVM].bst_root = NULL;
    nodes[TSFI_NET_ANVIL_EVM].adj_count = 0;

    nodes[TSFI_NET_PULSECHAIN].net_id = TSFI_NET_PULSECHAIN;
    nodes[TSFI_NET_PULSECHAIN].net_name = "PulseChain";
    nodes[TSFI_NET_PULSECHAIN].bst_root = NULL;
    nodes[TSFI_NET_PULSECHAIN].adj_count = 0;
}

void tsfi_qing_graph_add_edge(tsfi_qing_graph_node *n1, tsfi_qing_graph_node *n2) {
    if (!n1 || !n2) return;
    
    // Check if already connected to prevent duplicate edges
    for (int i = 0; i < n1->adj_count; i++) {
        if (n1->adj[i] == n2) return;
    }
    
    n1->adj[n1->adj_count++] = n2;
    n2->adj[n2->adj_count++] = n1;
}

// Internal DFS routing helper
static CachedContract* dfs_route_find(tsfi_qing_graph_node *current, uint64_t virtual_address, bool visited[TSFI_NET_COUNT]) {
    if (!current) return NULL;
    
    visited[current->net_id] = true;
    
    // 1. Search locally in current network's BST
    CachedContract *local_res = tsfi_qing_bst_find(current->bst_root, virtual_address);
    if (local_res) {
        return local_res;
    }
    
    // 2. Route search to adjacent networks in the undirected graph
    for (int i = 0; i < current->adj_count; i++) {
        tsfi_qing_graph_node *neighbor = current->adj[i];
        if (!visited[neighbor->net_id]) {
            CachedContract *routed_res = dfs_route_find(neighbor, virtual_address, visited);
            if (routed_res) {
                return routed_res;
            }
        }
    }
    
    return NULL;
}

CachedContract* tsfi_qing_graph_route_find(tsfi_qing_graph_node nodes[TSFI_NET_COUNT], tsfi_network_id start_net, uint64_t virtual_address) {
    bool visited[TSFI_NET_COUNT] = { false };
    return dfs_route_find(&nodes[start_net], virtual_address, visited);
}

void tsfi_qing_graph_destroy(tsfi_qing_graph_node nodes[TSFI_NET_COUNT]) {
    for (int i = 0; i < TSFI_NET_COUNT; i++) {
        if (nodes[i].bst_root) {
            tsfi_qing_bst_destroy(nodes[i].bst_root);
            nodes[i].bst_root = NULL;
        }
    }
}

CachedContract* tsfi_23_node_search(tsfi_23_node *node, uint64_t virtual_address) {
    if (!node) return NULL;

    // 1. If it's a Qing graph node, search local BST (and route search if needed)
    if (node->type == NODE_TYPE_QING_GRAPH && node->payload.qing_graph) {
        CachedContract *res = tsfi_qing_bst_find(node->payload.qing_graph->bst_root, virtual_address);
        if (res) return res;
    }

    // 2. Recursively search children nodes in the 2-3 Tree
    for (int i = 0; i < node->child_count; i++) {
        CachedContract *child_res = tsfi_23_node_search(node->children[i], virtual_address);
        if (child_res) return child_res;
    }

    return NULL;
}
