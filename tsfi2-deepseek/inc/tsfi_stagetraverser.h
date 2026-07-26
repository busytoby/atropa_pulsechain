#ifndef TSFI_STAGETRAVERSER_H
#define TSFI_STAGETRAVERSER_H

#include "tsfi_primroot.h"
#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_TRAVERSED_NODES 16

typedef struct {
    bool include_inactive;
    char type_filter[32];
} TSFiTraversalPredicate;

typedef struct {
    TSFiPrimNode *nodes[TSFI_MAX_TRAVERSED_NODES];
    int count;
} TSFiTraversalRange;

// Initialize a TraversalPredicate filter criteria
void tsfi_stagetraverser_init_predicate(TSFiTraversalPredicate *pred, bool include_inactive, const char *type_filter);

// Traverses the prim tree recursively, collecting all nodes that satisfy the predicate criteria
void tsfi_stagetraverser_traverse(TSFiPrimNode *root, const TSFiTraversalPredicate *pred, TSFiTraversalRange *range);

#endif // TSFI_STAGETRAVERSER_H
