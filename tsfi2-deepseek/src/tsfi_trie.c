#include "tsfi_trie.h"
#include <stdlib.h>
#include <string.h>

tsfi_trie_node* tsfi_trie_create_node(char ch) {
    tsfi_trie_node *node = (tsfi_trie_node*)malloc(sizeof(tsfi_trie_node));
    if (node) {
        node->ch = ch;
        node->is_end = false;
        node->phoneme = NULL;
        node->sibling = NULL;
        node->child = NULL;
    }
    return node;
}

static tsfi_trie_node* find_child(tsfi_trie_node *parent, char ch) {
    if (!parent) return NULL;
    tsfi_trie_node *curr = parent->child;
    while (curr) {
        if (curr->ch == ch) {
            return curr;
        }
        curr = curr->sibling;
    }
    return NULL;
}

static tsfi_trie_node* add_child(tsfi_trie_node *parent, char ch) {
    tsfi_trie_node *new_node = tsfi_trie_create_node(ch);
    if (new_node) {
        new_node->sibling = parent->child;
        parent->child = new_node;
    }
    return new_node;
}

void tsfi_trie_insert(tsfi_trie_node *root, const char *word, const char *phoneme) {
    if (!root || !word) return;
    
    tsfi_trie_node *curr = root;
    size_t len = strlen(word);
    
    for (size_t i = 0; i < len; i++) {
        char ch = word[i];
        tsfi_trie_node *next = find_child(curr, ch);
        if (!next) {
            next = add_child(curr, ch);
        }
        curr = next;
    }
    
    curr->is_end = true;
    if (curr->phoneme) {
        free(curr->phoneme);
    }
    curr->phoneme = strdup(phoneme);
}

const char* tsfi_trie_lookup(tsfi_trie_node *root, const char *word) {
    if (!root || !word) return NULL;
    
    tsfi_trie_node *curr = root;
    size_t len = strlen(word);
    
    for (size_t i = 0; i < len; i++) {
        char ch = word[i];
        curr = find_child(curr, ch);
        if (!curr) {
            return NULL;
        }
    }
    
    return curr->is_end ? curr->phoneme : NULL;
}

const char* tsfi_trie_longest_prefix(tsfi_trie_node *root, const char *text, int *prefix_len) {
    if (!root || !text || !prefix_len) return NULL;
    
    tsfi_trie_node *curr = root;
    size_t len = strlen(text);
    const char *last_match_phoneme = NULL;
    int last_match_len = 0;
    
    for (size_t i = 0; i < len; i++) {
        char ch = text[i];
        curr = find_child(curr, ch);
        if (!curr) {
            break;
        }
        if (curr->is_end) {
            last_match_phoneme = curr->phoneme;
            last_match_len = i + 1;
        }
    }
    
    *prefix_len = last_match_len;
    return last_match_phoneme;
}

void tsfi_trie_destroy(tsfi_trie_node *root) {
    if (!root) return;
    
    // Destroy siblings and children recursively
    tsfi_trie_destroy(root->child);
    tsfi_trie_destroy(root->sibling);
    
    if (root->phoneme) {
        free(root->phoneme);
    }
    free(root);
}
