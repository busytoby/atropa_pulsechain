#ifndef TSFI_TST_H
#define TSFI_TST_H

typedef struct tsfi_tst_node {
    char ch;
    char *value;
    struct tsfi_tst_node *left;
    struct tsfi_tst_node *eq;
    struct tsfi_tst_node *right;
} tsfi_tst_node;

// Create a new TST node
tsfi_tst_node* tsfi_tst_create_node(char ch);

// Insert key-value mapping into TST
tsfi_tst_node* tsfi_tst_insert(tsfi_tst_node *root, const char *key, const char *value);

// Search key in TST
const char* tsfi_tst_search(tsfi_tst_node *root, const char *key);

// Free all resources allocated in TST
void tsfi_tst_destroy(tsfi_tst_node *root);

#endif // TSFI_TST_H
