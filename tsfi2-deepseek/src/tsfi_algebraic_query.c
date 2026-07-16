#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_algebraic_query.h"

TSFiQueryNode* tsfi_query_create_var(const char *path) {
    if (!path) return NULL;
    TSFiQueryNode *node = calloc(1, sizeof(TSFiQueryNode));
    if (!node) return NULL;
    node->type = NODE_VAR;
    strncpy(node->path, path, sizeof(node->path) - 1);
    return node;
}

TSFiQueryNode* tsfi_query_create_mul(TSFiQueryNode *left, TSFiQueryNode *right) {
    if (!left || !right) return NULL;
    TSFiQueryNode *node = calloc(1, sizeof(TSFiQueryNode));
    if (!node) return NULL;
    node->type = NODE_MUL;
    node->left = left;
    node->right = right;
    return node;
}

TSFiQueryNode* tsfi_query_create_add(TSFiQueryNode *left, TSFiQueryNode *right) {
    if (!left || !right) return NULL;
    TSFiQueryNode *node = calloc(1, sizeof(TSFiQueryNode));
    if (!node) return NULL;
    node->type = NODE_ADD;
    node->left = left;
    node->right = right;
    return node;
}

float tsfi_query_evaluate(TSFiQueryNode *node, tsfi_dat *dat) {
    if (!node || !dat) return 0.0f;

    if (node->type == NODE_VAR) {
        const char *val_str = tsfi_dat_search(dat, node->path);
        if (!val_str) return 0.0f;
        
        // Convert stored percentage weights (0-100) to normalized probability measures
        float val = atof(val_str);
        if (val < 0.0f) return 0.0f;
        if (val > 100.0f) return 1.0f;
        return val / 100.0f;
    }

    float l_val = tsfi_query_evaluate(node->left, dat);
    float r_val = tsfi_query_evaluate(node->right, dat);

    if (node->type == NODE_MUL) {
        // Joint intersection (AND): P(A * B)
        return l_val * r_val;
    } else if (node->type == NODE_ADD) {
        // Disjunctive union (OR): P(A + B - A * B)
        return l_val + r_val - (l_val * r_val);
    }

    return 0.0f;
}

void tsfi_query_destroy(TSFiQueryNode *node) {
    if (!node) return;
    tsfi_query_destroy(node->left);
    tsfi_query_destroy(node->right);
    free(node);
}
