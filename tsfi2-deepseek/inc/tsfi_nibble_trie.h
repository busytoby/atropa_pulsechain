#ifndef TSFI_NIBBLE_TRIE_H
#define TSFI_NIBBLE_TRIE_H

typedef struct tsfi_nibble_node {
    const char *value;
    struct tsfi_nibble_node *children[16];
} tsfi_nibble_node;

// Create a new Hex-Nibble node
tsfi_nibble_node* tsfi_nibble_create_node(void);

// Insert an address lookup mapping (expects standard hex string or dynamic prefix)
void tsfi_nibble_insert(tsfi_nibble_node *root, const char *hex_addr, const char *value);

// Resolve address target mapping from hex address
const char* tsfi_nibble_lookup(tsfi_nibble_node *root, const char *hex_addr);

// Destroy Hex-Nibble Trie
void tsfi_nibble_destroy(tsfi_nibble_node *root);

#endif // TSFI_NIBBLE_TRIE_H
