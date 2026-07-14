#ifndef TSFI_RADIX_G2P_H
#define TSFI_RADIX_G2P_H

#define MAX_CONTEXT_RULES 4

typedef struct {
    char *lookahead_pattern;
    char *context_phoneme;
} tsfi_context_rule;

typedef struct tsfi_radix_node {
    char *edge_label;
    char *phoneme;
    
    tsfi_context_rule rules[MAX_CONTEXT_RULES];
    int rule_count;
    
    struct tsfi_radix_node *child;
    struct tsfi_radix_node *sibling;
} tsfi_radix_node;

// Create a new Radix node
tsfi_radix_node* tsfi_radix_create_node(const char *edge_label);

// Insert a word prefix mapping with fallback and context rules
void tsfi_radix_insert_rule(tsfi_radix_node *root, const char *prefix, const char *default_phoneme, const char *lookahead, const char *context_phoneme);

// Translate input using context-sensitive prefix matching
const char* tsfi_radix_translate(tsfi_radix_node *root, const char *text, int *matched_len);

// Destroy Radix Trie resources
void tsfi_radix_destroy(tsfi_radix_node *root);

#endif // TSFI_RADIX_G2P_H
