#include "tsfi_qing_bst.h"
#include <stdlib.h>
#include <string.h>

static inline int max_val(int a, int b) {
    return (a > b) ? a : b;
}

static inline int get_height(tsfi_qing_bst_node *node) {
    return node ? node->height : 0;
}

static inline int get_balance(tsfi_qing_bst_node *node) {
    return node ? (get_height(node->left) - get_height(node->right)) : 0;
}

static tsfi_qing_bst_node* rotate_right(tsfi_qing_bst_node *y) {
    tsfi_qing_bst_node *x = y->left;
    tsfi_qing_bst_node *T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = max_val(get_height(y->left), get_height(y->right)) + 1;
    x->height = max_val(get_height(x->left), get_height(x->right)) + 1;

    return x;
}

static tsfi_qing_bst_node* rotate_left(tsfi_qing_bst_node *x) {
    tsfi_qing_bst_node *y = x->right;
    tsfi_qing_bst_node *T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = max_val(get_height(x->left), get_height(x->right)) + 1;
    y->height = max_val(get_height(y->left), get_height(y->right)) + 1;

    return y;
}

tsfi_qing_bst_node* tsfi_qing_bst_insert(tsfi_qing_bst_node *root, uint64_t virtual_address, CachedContract *contract) {
    if (!root) {
        tsfi_qing_bst_node *node = (tsfi_qing_bst_node*)malloc(sizeof(tsfi_qing_bst_node));
        if (node) {
            node->virtual_address = virtual_address;
            node->contract = contract;
            node->height = 1;
            node->left = NULL;
            node->right = NULL;
        }
        return node;
    }

    if (virtual_address < root->virtual_address) {
        root->left = tsfi_qing_bst_insert(root->left, virtual_address, contract);
    } else if (virtual_address > root->virtual_address) {
        root->right = tsfi_qing_bst_insert(root->right, virtual_address, contract);
    } else {
        return root; // Equal keys are not allowed in BST
    }

    root->height = max_val(get_height(root->left), get_height(root->right)) + 1;

    int balance = get_balance(root);

    // Left Left Case
    if (balance > 1 && virtual_address < root->left->virtual_address) {
        return rotate_right(root);
    }

    // Right Right Case
    if (balance < -1 && virtual_address > root->right->virtual_address) {
        return rotate_left(root);
    }

    // Left Right Case
    if (balance > 1 && virtual_address > root->left->virtual_address) {
        root->left = rotate_left(root->left);
        return rotate_right(root);
    }

    // Right Left Case
    if (balance < -1 && virtual_address < root->right->virtual_address) {
        root->right = rotate_right(root->right);
        return rotate_left(root);
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
