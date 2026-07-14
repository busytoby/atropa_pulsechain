#ifndef TSFI_QING_BST_H
#define TSFI_QING_BST_H

#include <stdint.h>
#include <stddef.h>
#include "lau_yul_thunk_internal.h"

typedef struct tsfi_qing_bst_node {
    uint64_t virtual_address;
    CachedContract *contract;
    int height;
    struct tsfi_qing_bst_node *left;
    struct tsfi_qing_bst_node *right;
} tsfi_qing_bst_node;

// Insert a contract node into the BST
tsfi_qing_bst_node* tsfi_qing_bst_insert(tsfi_qing_bst_node *root, uint64_t virtual_address, CachedContract *contract);

// Find a contract in the BST by its virtual address
CachedContract* tsfi_qing_bst_find(tsfi_qing_bst_node *root, uint64_t virtual_address);

// Populate the BST from the active cached contracts registry
tsfi_qing_bst_node* tsfi_qing_bst_populate(void);

// Free all nodes in the BST
void tsfi_qing_bst_destroy(tsfi_qing_bst_node *root);

extern tsfi_qing_bst_node *g_runtime_qing_bst;

// Thread-safe concurrent read BST lookup
CachedContract* tsfi_qing_bst_find_safe(uint64_t virtual_address);

// Thread-safe concurrent write BST invalidation
void tsfi_qing_bst_invalidate_safe(void);

// Thread-safe concurrent write BST initialization
void tsfi_qing_bst_populate_safe(void);

#endif // TSFI_QING_BST_H
