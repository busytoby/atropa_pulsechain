#ifndef TSFI_GRAPH_CARDS_H
#define TSFI_GRAPH_CARDS_H

#include "tsfi_types.h"
#include "tsfi_taste_cache.h"

// --- ICPC 2020 Asia Taipei: Problem G (Graph Cards) ---
// Performs Isomorphism testing on unicyclic puppet skeletons.

#define MAX_GRAPH_NODES 1024

typedef struct {
    uint32_t u, v;
} TsfiEdge;

DEFINE_MAPPED_STRUCT(TsfiGraphCard,
    uint32_t node_count;
    uint32_t edge_count;
    TsfiEdge edges[MAX_GRAPH_NODES];
    TsfiTasteAtom canonical_atom; // Terminal Subjective Identity
)

#ifdef __cplusplus
extern "C" {
#endif

// Thunk: Solves the unicyclic isomorphism and populates canonical_atom
void tsfi_thunk_solve_graph_isomorphism(TsfiGraphCard *card);

#ifdef __cplusplus
}
#endif

#endif
