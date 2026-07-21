#include "tsfi_parc_ingerman_alg141.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int tsfi_ingerman_alg141_initialize(tsfi_ingerman_alg141_t *graph, uint32_t node_count) {
    if (!graph || node_count == 0 || node_count > INGERMAN_ALG141_MAX_NODES) return -1;
    memset(graph, 0, sizeof(tsfi_ingerman_alg141_t));

    graph->node_count = node_count;
    snprintf(graph->dynamic_contract_address, sizeof(graph->dynamic_contract_address),
             "dynamic_0x7343d8afa9d6e3376873ea24ccba7c7230aab14b");

    return 0;
}

int tsfi_ingerman_alg141_add_edge(tsfi_ingerman_alg141_t *graph, uint32_t src_node, uint32_t dest_node) {
    if (!graph || src_node >= graph->node_count || dest_node >= graph->node_count) return -1;

    graph->adjacency_matrix[src_node][dest_node] = true;
    return 0;
}

int tsfi_ingerman_alg141_compute_path_matrix(tsfi_ingerman_alg141_t *graph) {
    if (!graph) return -1;

    uint32_t N = graph->node_count;

    // Step 1: Copy Adjacency Matrix into Path Matrix
    for (uint32_t i = 0; i < N; i++) {
        for (uint32_t j = 0; j < N; j++) {
            graph->path_matrix[i][j] = graph->adjacency_matrix[i][j];
        }
    }

    // Step 2: Ingerman Algorithm 141 Transitive Closure Computation (CACM 1962)
    // P[i,j] = P[i,j] OR (P[i,k] AND P[k,j])
    for (uint32_t k = 0; k < N; k++) {
        for (uint32_t i = 0; i < N; i++) {
            if (graph->path_matrix[i][k]) {
                for (uint32_t j = 0; j < N; j++) {
                    if (graph->path_matrix[k][j]) {
                        graph->path_matrix[i][j] = true;
                    }
                }
            }
        }
    }

    return 0;
}

bool tsfi_ingerman_alg141_is_reachable(const tsfi_ingerman_alg141_t *graph, uint32_t src_node, uint32_t dest_node) {
    if (!graph || src_node >= graph->node_count || dest_node >= graph->node_count) return false;
    return graph->path_matrix[src_node][dest_node];
}

int tsfi_ingerman_alg141_render_matrix(const tsfi_ingerman_alg141_t *graph, char *out_buf, size_t max_buf_len) {
    if (!graph || !out_buf || max_buf_len < 256) return -1;

    size_t written = 0;
    written += snprintf(out_buf + written, max_buf_len - written,
                         "Peter Z. Ingerman Algorithm 141 Path Matrix (Nodes: %u)\n"
                         "Contract: %s\n", graph->node_count, graph->dynamic_contract_address);

    for (uint32_t i = 0; i < graph->node_count; i++) {
        written += snprintf(out_buf + written, max_buf_len - written, "Node %02u | ", i);
        for (uint32_t j = 0; j < graph->node_count; j++) {
            written += snprintf(out_buf + written, max_buf_len - written, "%c ", graph->path_matrix[i][j] ? '1' : '0');
        }
        written += snprintf(out_buf + written, max_buf_len - written, "\n");
    }

    return 0;
}
