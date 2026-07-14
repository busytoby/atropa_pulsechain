#ifndef TSFI_TRIE_H
#define TSFI_TRIE_H

#include <stdbool.h>

typedef struct tsfi_trie_node {
    char ch;
    bool is_end;
    char *phoneme;
    struct tsfi_trie_node *sibling; // Next child of the parent
    struct tsfi_trie_node *child;   // First child of this node
} tsfi_trie_node;

// Create a new Trie node
tsfi_trie_node* tsfi_trie_create_node(char ch);

// Insert a word-to-phoneme mapping into the Trie
void tsfi_trie_insert(tsfi_trie_node *root, const char *word, const char *phoneme);

// Lookup exact word pronunciation in the Trie
const char* tsfi_trie_lookup(tsfi_trie_node *root, const char *word);

// Find the longest matching prefix for text-to-phoneme tokenization
const char* tsfi_trie_longest_prefix(tsfi_trie_node *root, const char *text, int *prefix_len);

// Free all resources allocated in the Trie
void tsfi_trie_destroy(tsfi_trie_node *root);

#endif // TSFI_TRIE_H
