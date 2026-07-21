#ifndef TSFI_PARC_INGERMAN_ALG141_H
#define TSFI_PARC_INGERMAN_ALG141_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define INGERMAN_ALG141_MAX_NODES 64

/* Peter Z. Ingerman Algorithm 141 Graph Structure */
typedef struct {
    uint32_t node_count;
    bool adjacency_matrix[INGERMAN_ALG141_MAX_NODES][INGERMAN_ALG141_MAX_NODES];
    bool path_matrix[INGERMAN_ALG141_MAX_NODES][INGERMAN_ALG141_MAX_NODES];
    char dynamic_contract_address[64];
} tsfi_ingerman_alg141_t;

/* Initialize Algorithm 141 State Machine */
int tsfi_ingerman_alg141_initialize(tsfi_ingerman_alg141_t *graph, uint32_t node_count);

/* Add Directed Edge in Adjacency Matrix */
int tsfi_ingerman_alg141_add_edge(tsfi_ingerman_alg141_t *graph, uint32_t src_node, uint32_t dest_node);

/* Compute Transitive Closure Path Matrix (Ingerman CACM Algorithm 141, 1962) */
int tsfi_ingerman_alg141_compute_path_matrix(tsfi_ingerman_alg141_t *graph);

/* Check Path Reachability between Nodes */
bool tsfi_ingerman_alg141_is_reachable(const tsfi_ingerman_alg141_t *graph, uint32_t src_node, uint32_t dest_node);

/* Render Path Matrix to String Buffer */
int tsfi_ingerman_alg141_render_matrix(const tsfi_ingerman_alg141_t *graph, char *out_buf, size_t max_buf_len);

#endif // TSFI_PARC_INGERMAN_ALG141_H
