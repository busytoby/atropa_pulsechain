#ifndef TSFI_ALGEBRAIC_QUERY_H
#define TSFI_ALGEBRAIC_QUERY_H

#include "tsfi_dat.h"

// Types of algebraic logic nodes
typedef enum {
    NODE_VAR = 0, // Voxel variable (coordinates)
    NODE_MUL,     // Joint multiplication (AND)
    NODE_ADD      // Disjunctive addition (OR)
} TSFiQueryNodeType;

typedef struct TSFiQueryNode {
    TSFiQueryNodeType type;
    char path[128]; // Used if NODE_VAR
    struct TSFiQueryNode *left;
    struct TSFiQueryNode *right;
} TSFiQueryNode;

// Compile an algebraic logic query node
TSFiQueryNode* tsfi_query_create_var(const char *path);
TSFiQueryNode* tsfi_query_create_mul(TSFiQueryNode *left, TSFiQueryNode *right);
TSFiQueryNode* tsfi_query_create_add(TSFiQueryNode *left, TSFiQueryNode *right);

// Evaluate the probability measure of the query over the DAT rails
float tsfi_query_evaluate(TSFiQueryNode *node, tsfi_dat *dat);

// Cleanup query structures
void tsfi_query_destroy(TSFiQueryNode *node);

#endif // TSFI_ALGEBRAIC_QUERY_H
