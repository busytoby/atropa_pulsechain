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

// Alphabetically ordered Binary Search Tree (AVL balanced) for strings
typedef struct tsfi_string_bst_node {
    char *key;
    char *value;
    int height;
    struct tsfi_string_bst_node *left;
    struct tsfi_string_bst_node *right;
} tsfi_string_bst_node;

// Insert a key-value pair into the alphabetically-ordered string BST (AVL-balanced)
tsfi_string_bst_node* tsfi_string_bst_insert(tsfi_string_bst_node *node, const char *key, const char *value);

// Search for a value in the string BST
const char* tsfi_string_bst_find(tsfi_string_bst_node *root, const char *key);

// Free all resources in the string BST
void tsfi_string_bst_destroy(tsfi_string_bst_node *root);

// Convert a Trie into an alphabetically sorted AVL BST
tsfi_string_bst_node* tsfi_trie_to_string_bst(tsfi_trie_node *trie_root);

#endif // TSFI_TRIE_H
