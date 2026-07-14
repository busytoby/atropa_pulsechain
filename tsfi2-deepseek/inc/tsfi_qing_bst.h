#ifndef TSFI_QING_BST_H
#define TSFI_QING_BST_H

#include <stdint.h>
#include <stddef.h>
#include "lau_yul_thunk_internal.h"

typedef struct tsfi_qing_bst_node {
    uint64_t virtual_address;
    CachedContract *contract;
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

#endif // TSFI_QING_BST_H
