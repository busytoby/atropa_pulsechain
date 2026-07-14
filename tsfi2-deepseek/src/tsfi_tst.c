#include "tsfi_tst.h"
#include <stdlib.h>
#include <string.h>

tsfi_tst_node* tsfi_tst_create_node(char ch) {
    tsfi_tst_node *node = (tsfi_tst_node*)malloc(sizeof(tsfi_tst_node));
    if (node) {
        node->ch = ch;
        node->value = NULL;
        node->left = NULL;
        node->eq = NULL;
        node->right = NULL;
    }
    return node;
}

tsfi_tst_node* tsfi_tst_insert(tsfi_tst_node *node, const char *key, const char *value) {
    if (!key || *key == '\0') return node;

    if (!node) {
        node = tsfi_tst_create_node(*key);
    }

    if (*key < node->ch) {
        node->left = tsfi_tst_insert(node->left, key, value);
    } else if (*key > node->ch) {
        node->right = tsfi_tst_insert(node->right, key, value);
    } else {
        if (*(key + 1) == '\0') {
            if (node->value) {
                free(node->value);
            }
            node->value = strdup(value);
        } else {
            node->eq = tsfi_tst_insert(node->eq, key + 1, value);
        }
    }
    return node;
}

const char* tsfi_tst_search(tsfi_tst_node *node, const char *key) {
    if (!node || !key || *key == '\0') return NULL;

    if (*key < node->ch) {
        return tsfi_tst_search(node->left, key);
    } else if (*key > node->ch) {
        return tsfi_tst_search(node->right, key);
    } else {
        if (*(key + 1) == '\0') {
            return node->value;
        }
        return tsfi_tst_search(node->eq, key + 1);
    }
}

void tsfi_tst_destroy(tsfi_tst_node *node) {
    if (!node) return;

    tsfi_tst_destroy(node->left);
    tsfi_tst_destroy(node->eq);
    tsfi_tst_destroy(node->right);

    if (node->value) {
        free(node->value);
    }
    free(node);
}
