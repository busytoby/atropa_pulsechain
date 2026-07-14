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

void tsfi_23_node_destroy(tsfi_23_node *root) {
    if (!root) return;
    for (int i = 0; i < root->child_count; i++) {
        tsfi_23_node_destroy(root->children[i]);
    }
    free(root);
}

tsfi_23_node* tsfi_23_node_add_child(tsfi_23_node *parent, tsfi_23_node *child, tsfi_23_node **new_sibling_out) {
    if (!parent || !child) return parent;

    // Place child temporarily
    parent->children[parent->child_count++] = child;

    // Check if we overflowed (4 children in a 2-3 tree node is invalid, triggers split)
    if (parent->child_count == 4) {
        tsfi_23_node *sibling = (tsfi_23_node*)malloc(sizeof(tsfi_23_node));
        if (sibling) {
            sibling->type = parent->type;
            sibling->key_count = 0;
            sibling->child_count = 2;
            sibling->children[0] = parent->children[2];
            sibling->children[1] = parent->children[3];
            sibling->children[2] = NULL;
            sibling->children[3] = NULL;

            parent->children[2] = NULL;
            parent->children[3] = NULL;
            parent->child_count = 2;

            if (new_sibling_out) {
                *new_sibling_out = sibling;
            }
        }
    } else {
        if (new_sibling_out) {
            *new_sibling_out = NULL;
        }
    }

    return parent;
}

static tsfi_23_node* insert_internal(tsfi_23_node *node, uint64_t key, tsfi_node_type type, void *payload, uint64_t *pushed_key, tsfi_23_node **pushed_child) {
    if (node->is_leaf) {
        int i = node->key_count - 1;
        while (i >= 0 && node->keys[i] > key) {
            node->keys[i + 1] = node->keys[i];
            node->payloads[i + 1] = node->payloads[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->payloads[i + 1] = payload;
        node->key_count++;

        if (node->key_count == 3) {
            tsfi_23_node *sibling = (tsfi_23_node*)malloc(sizeof(tsfi_23_node));
            if (sibling) {
                sibling->type = type;
                sibling->is_leaf = 1;
                sibling->key_count = 1;
                sibling->child_count = 0;
                sibling->keys[0] = node->keys[2];
                sibling->payloads[0] = node->payloads[2];

                node->key_count = 1;
                *pushed_key = node->keys[1];
                *pushed_child = sibling;
            }
            return node;
        }
        *pushed_child = NULL;
        return node;
    }

    int child_idx = 0;
    if (key > node->keys[0]) {
        child_idx = 1;
        if (node->key_count == 2 && key > node->keys[1]) {
            child_idx = 2;
        }
    }

    uint64_t mid_key;
    tsfi_23_node *new_sibling = NULL;
    insert_internal(node->children[child_idx], key, type, payload, &mid_key, &new_sibling);

    if (new_sibling) {
        int i = node->key_count - 1;
        while (i >= 0 && node->keys[i] > mid_key) {
            node->keys[i + 1] = node->keys[i];
            node->children[i + 2] = node->children[i + 1];
            i--;
        }
        node->keys[i + 1] = mid_key;
        node->children[i + 2] = new_sibling;
        node->key_count++;
        node->child_count++;

        if (node->key_count == 3) {
            tsfi_23_node *sibling = (tsfi_23_node*)malloc(sizeof(tsfi_23_node));
            if (sibling) {
                sibling->type = node->type;
                sibling->is_leaf = 0;
                sibling->key_count = 1;
                sibling->child_count = 2;
                sibling->keys[0] = node->keys[2];
                sibling->children[0] = node->children[2];
                sibling->children[1] = node->children[3];

                node->key_count = 1;
                node->child_count = 2;
                *pushed_key = node->keys[1];
                *pushed_child = sibling;
            }
            return node;
        }
    }
    *pushed_child = NULL;
    return node;
}

tsfi_23_node* tsfi_23_insert(tsfi_23_node *root, uint64_t key, tsfi_node_type type, void *payload) {
    if (!root) {
        tsfi_23_node *node = (tsfi_23_node*)malloc(sizeof(tsfi_23_node));
        if (node) {
            node->type = type;
            node->is_leaf = 1;
            node->key_count = 1;
            node->child_count = 0;
            node->keys[0] = key;
            node->payloads[0] = payload;
        }
        return node;
    }

    uint64_t pushed_key;
    tsfi_23_node *pushed_child = NULL;
    root = insert_internal(root, key, type, payload, &pushed_key, &pushed_child);

    if (pushed_child) {
        tsfi_23_node *new_root = (tsfi_23_node*)malloc(sizeof(tsfi_23_node));
        if (new_root) {
            new_root->type = root->type;
            new_root->is_leaf = 0;
            new_root->key_count = 1;
            new_root->child_count = 2;
            new_root->keys[0] = pushed_key;
            new_root->children[0] = root;
            new_root->children[1] = pushed_child;
        }
        return new_root;
    }
    return root;
}
