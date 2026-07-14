#include "tsfi_radix_g2p.h"
#include <stdlib.h>
#include <string.h>

tsfi_radix_node* tsfi_radix_create_node(const char *edge_label) {
    tsfi_radix_node *node = (tsfi_radix_node*)malloc(sizeof(tsfi_radix_node));
    if (node) {
        node->edge_label = edge_label ? strdup(edge_label) : NULL;
        node->phoneme = NULL;
        node->rule_count = 0;
        for (int i = 0; i < MAX_CONTEXT_RULES; i++) {
            node->rules[i].lookahead_pattern = NULL;
            node->rules[i].context_phoneme = NULL;
        }
        node->child = NULL;
        node->sibling = NULL;
    }
    return node;
}

void tsfi_radix_insert_rule(tsfi_radix_node *root, const char *prefix, const char *default_phoneme, const char *lookahead, const char *context_phoneme) {
    if (!root || !prefix) return;

    tsfi_radix_node *curr = root;
    tsfi_radix_node *child = curr->child;
    tsfi_radix_node *prev = NULL;

    // Search sibling list for matching edge label prefix
    while (child) {
        if (child->edge_label && strcmp(child->edge_label, prefix) == 0) {
            curr = child;
            goto finalize_node;
        }
        prev = child;
        child = child->sibling;
    }

    // Create new node if no match
    tsfi_radix_node *new_node = tsfi_radix_create_node(prefix);
    if (prev) {
        prev->sibling = new_node;
    } else {
        curr->child = new_node;
    }
    curr = new_node;

finalize_node:
    if (default_phoneme) {
        if (curr->phoneme) free(curr->phoneme);
        curr->phoneme = strdup(default_phoneme);
    }

    if (lookahead && context_phoneme && curr->rule_count < MAX_CONTEXT_RULES) {
        curr->rules[curr->rule_count].lookahead_pattern = strdup(lookahead);
        curr->rules[curr->rule_count].context_phoneme = strdup(context_phoneme);
        curr->rule_count++;
    }
}

const char* tsfi_radix_translate(tsfi_radix_node *root, const char *text, int *matched_len) {
    if (!root || !text || *text == '\0') return NULL;

    tsfi_radix_node *child = root->child;
    while (child) {
        if (child->edge_label) {
            size_t label_len = strlen(child->edge_label);
            if (strncmp(text, child->edge_label, label_len) == 0) {
                *matched_len = (int)label_len;

                // Check all registered context lookahead rules
                for (int i = 0; i < child->rule_count; i++) {
                    if (child->rules[i].lookahead_pattern && child->rules[i].context_phoneme) {
                        const char *lookahead_ptr = text + label_len;
                        size_t look_len = strlen(child->rules[i].lookahead_pattern);
                        if (strncmp(lookahead_ptr, child->rules[i].lookahead_pattern, look_len) == 0) {
                            *matched_len += (int)look_len;
                            return child->rules[i].context_phoneme;
                        }
                    }
                }
                return child->phoneme;
            }
        }
        child = child->sibling;
    }
    return NULL;
}

void tsfi_radix_destroy(tsfi_radix_node *node) {
    if (!node) return;

    tsfi_radix_destroy(node->child);
    tsfi_radix_destroy(node->sibling);

    if (node->edge_label) free(node->edge_label);
    if (node->phoneme) free(node->phoneme);
    
    for (int i = 0; i < node->rule_count; i++) {
        if (node->rules[i].lookahead_pattern) free(node->rules[i].lookahead_pattern);
        if (node->rules[i].context_phoneme) free(node->rules[i].context_phoneme);
    }
    free(node);
}
