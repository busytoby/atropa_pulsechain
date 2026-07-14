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

#endif // TSFI_QING_GRAPH_H
