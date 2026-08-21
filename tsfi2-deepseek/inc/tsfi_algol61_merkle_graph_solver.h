#ifndef TSFI_ALGOL61_MERKLE_GRAPH_SOLVER_H
#define TSFI_ALGOL61_MERKLE_GRAPH_SOLVER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define TSFI_YI_HEXAGRAM_COUNT 64
#define TSFI_MAX_PROVER_NAME_LEN 128
#define TSFI_MAX_PROVERS_CAPACITY 2048

typedef enum {
    TSFI_SOLVER_DOMAIN_HARMONIC_EDO22 = 0,
    TSFI_SOLVER_DOMAIN_CRYPTO_ENTROPY  = 1,
    TSFI_SOLVER_DOMAIN_INSTRUCTION_HAL = 2,
    TSFI_SOLVER_DOMAIN_SOVEREIGN_ID    = 3,
    TSFI_SOLVER_DOMAIN_PHYSICS_FET     = 4
} TsfiProverDomain;

typedef struct {
    char name[TSFI_MAX_PROVER_NAME_LEN];
    TsfiProverDomain domain;
    uint8_t yi_hexagram_index; // 0..63 canonical YI coordinate
    uint32_t ast_hash;         // FNV-1a discrete hash of the Algol61 AST
    uint32_t precondition_cnt;
    uint32_t postcondition_cnt;
} TsfiAlgol61Node;

typedef struct {
    uint32_t node_count;
    TsfiAlgol61Node nodes[TSFI_MAX_PROVERS_CAPACITY];
    uint32_t hexagram_buckets[TSFI_YI_HEXAGRAM_COUNT];
    uint8_t merkle_root[32];
} TsfiAlgol61KnowledgeGraph;

bool tsfi_algol61_graph_init(TsfiAlgol61KnowledgeGraph *graph);
bool tsfi_algol61_graph_ingest_prover(TsfiAlgol61KnowledgeGraph *graph, const char *filepath);
bool tsfi_algol61_graph_compute_merkle_root(TsfiAlgol61KnowledgeGraph *graph);
uint32_t tsfi_algol61_graph_query_hexagram(const TsfiAlgol61KnowledgeGraph *graph, uint8_t hexagram_idx, TsfiAlgol61Node *out_nodes, uint32_t max_out);

#endif // TSFI_ALGOL61_MERKLE_GRAPH_SOLVER_H
