#ifndef TSFI_STAGEPOP_H
#define TSFI_STAGEPOP_H

#include "tsfi_primroot.h"
#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_TRAVERSED_PRIMS 16

typedef struct {
    TSFiPrimNode *matched_prims[TSFI_MAX_TRAVERSED_PRIMS];
    int match_count;
} TSFiStageTraversalResult;

// Performs a recursive depth-first traversal of the prim tree to find all nodes matching a specific type
void tsfi_stagepop_traverse_type(TSFiPrimNode *root, const char *type_name, TSFiStageTraversalResult *result);

#endif // TSFI_STAGEPOP_H
