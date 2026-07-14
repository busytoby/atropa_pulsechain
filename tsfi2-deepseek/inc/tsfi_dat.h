#ifndef TSFI_DAT_H
#define TSFI_DAT_H

#include "tsfi_trie.h"

typedef struct {
    int *base;
    int *check;
    char **values;
    int capacity;
} tsfi_dat;

// Compile a standard Trie into a Double-Array Trie (DAT) structure
tsfi_dat* tsfi_dat_compile(tsfi_trie_node *trie_root);

// Search for a key in the compiled DAT
const char* tsfi_dat_search(tsfi_dat *dat, const char *key);

// Free all resources allocated in the DAT
void tsfi_dat_destroy(tsfi_dat *dat);

#endif // TSFI_DAT_H
