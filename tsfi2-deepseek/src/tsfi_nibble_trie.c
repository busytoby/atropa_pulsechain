#include "tsfi_nibble_trie.h"
#include <stdlib.h>
#include <string.h>

static inline int hex_to_nibble(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    return -1;
}

static const char* skip_address_prefix(const char *addr) {
    if (strncmp(addr, "dynamic_0x", 10) == 0) return addr + 10;
    if (strncmp(addr, "dynamic_", 8) == 0) return addr + 8;
    if (strncmp(addr, "0x", 2) == 0) return addr + 2;
    return addr;
}

tsfi_nibble_node* tsfi_nibble_create_node(void) {
    tsfi_nibble_node *node = (tsfi_nibble_node*)malloc(sizeof(tsfi_nibble_node));
    if (node) {
        node->value = NULL;
        for (int i = 0; i < 16; i++) {
            node->children[i] = NULL;
        }
    }
    return node;
}

void tsfi_nibble_insert(tsfi_nibble_node *root, const char *hex_addr, const char *value) {
    if (!root || !hex_addr) return;

    const char *addr = skip_address_prefix(hex_addr);
    tsfi_nibble_node *curr = root;

    while (*addr != '\0') {
        int idx = hex_to_nibble(*addr);
        if (idx == -1) {
            addr++;
            continue; // Skip non-hex formatting characters if any
        }

        if (!curr->children[idx]) {
            curr->children[idx] = tsfi_nibble_create_node();
        }
        curr = curr->children[idx];
        addr++;
    }

    curr->value = value; // Assign payload target
}

const char* tsfi_nibble_lookup(tsfi_nibble_node *root, const char *hex_addr) {
    if (!root || !hex_addr) return NULL;

    const char *addr = skip_address_prefix(hex_addr);
    tsfi_nibble_node *curr = root;

    while (*addr != '\0') {
        int idx = hex_to_nibble(*addr);
        if (idx == -1) {
            addr++;
            continue;
        }

        curr = curr->children[idx];
        if (!curr) return NULL;
        addr++;
    }

    return curr->value;
}

void tsfi_nibble_destroy(tsfi_nibble_node *root) {
    if (!root) return;

    for (int i = 0; i < 16; i++) {
        if (root->children[i]) {
            tsfi_nibble_destroy(root->children[i]);
        }
    }
    free(root);
}
