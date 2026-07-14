#ifndef TSFI_QING_GRAPH_H
#define TSFI_QING_GRAPH_H

#include "tsfi_qing_bst.h"

typedef enum {
    TSFI_NET_ZMM,
    TSFI_NET_ANVIL_EVM,
    TSFI_NET_PULSECHAIN,
    TSFI_NET_COUNT
} tsfi_network_id;

typedef struct tsfi_qing_graph_node {
    tsfi_network_id net_id;
    const char *net_name;
    tsfi_qing_bst_node *bst_root;
    struct tsfi_qing_graph_node *adj[TSFI_NET_COUNT];
    int adj_count;
} tsfi_qing_graph_node;

// Initialize the tripartite graph of BSTs
void tsfi_qing_graph_init(tsfi_qing_graph_node nodes[TSFI_NET_COUNT]);

// Connect two nodes in the undirected graph
void tsfi_qing_graph_add_edge(tsfi_qing_graph_node *n1, tsfi_qing_graph_node *n2);

// Search for a contract address across the entire graph by routing
CachedContract* tsfi_qing_graph_route_find(tsfi_qing_graph_node nodes[TSFI_NET_COUNT], tsfi_network_id start_net, uint64_t virtual_address);

// Free all resources in the graph
void tsfi_qing_graph_destroy(tsfi_qing_graph_node nodes[TSFI_NET_COUNT]);

typedef enum {
    NODE_TYPE_QING_GRAPH,
    NODE_TYPE_RAW_DATA,
    NODE_TYPE_FIRMWARE
} tsfi_node_type;

typedef struct tsfi_23_node {
    tsfi_node_type type;
    union {
        tsfi_qing_graph_node *qing_graph;
        struct {
            uint64_t key;
            uint64_t val;
        } raw;
        struct {
            char name[64];
            uint32_t checksum;
        } fw;
    } payload;
    struct tsfi_23_node *children[3];
    int child_count;
} tsfi_23_node;

// Search for a contract address recursively starting from a 2-3 tree node
CachedContract* tsfi_23_node_search(tsfi_23_node *node, uint64_t virtual_address);

#endif // TSFI_QING_GRAPH_H
