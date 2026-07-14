#include "tsfi_qing_bst.h"
#include <stdlib.h>
#include <string.h>

tsfi_qing_bst_node* tsfi_qing_bst_insert(tsfi_qing_bst_node *root, uint64_t virtual_address, CachedContract *contract) {
    if (!root) {
        tsfi_qing_bst_node *node = (tsfi_qing_bst_node*)malloc(sizeof(tsfi_qing_bst_node));
        if (node) {
            node->virtual_address = virtual_address;
            node->contract = contract;
            node->left = NULL;
            node->right = NULL;
        }
        return node;
    }
    
    if (virtual_address < root->virtual_address) {
        root->left = tsfi_qing_bst_insert(root->left, virtual_address, contract);
    } else if (virtual_address > root->virtual_address) {
        root->right = tsfi_qing_bst_insert(root->right, virtual_address, contract);
    }
    return root;
}

CachedContract* tsfi_qing_bst_find(tsfi_qing_bst_node *root, uint64_t virtual_address) {
    if (!root) {
        return NULL;
    }
    if (virtual_address == root->virtual_address) {
        return root->contract;
    }
    if (virtual_address < root->virtual_address) {
        return tsfi_qing_bst_find(root->left, virtual_address);
    }
    return tsfi_qing_bst_find(root->right, virtual_address);
}

tsfi_qing_bst_node* tsfi_qing_bst_populate(void) {
    tsfi_qing_bst_node *root = NULL;
    for (int i = 0; i < g_cached_contracts_count; i++) {
        // Only index contracts that have a valid virtual address representing a tokenized Qing
        if (g_cached_contracts[i].virtual_address > 0) {
            root = tsfi_qing_bst_insert(root, g_cached_contracts[i].virtual_address, &g_cached_contracts[i]);
        }
    }
    return root;
}

void tsfi_qing_bst_destroy(tsfi_qing_bst_node *root) {
    if (!root) {
        return;
    }
    tsfi_qing_bst_destroy(root->left);
    tsfi_qing_bst_destroy(root->right);
    free(root);
}
