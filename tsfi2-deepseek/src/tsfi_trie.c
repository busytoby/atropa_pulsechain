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

static inline int get_str_height(tsfi_string_bst_node *n) {
    return n ? n->height : 0;
}

static inline int get_str_balance(tsfi_string_bst_node *n) {
    return n ? (get_str_height(n->left) - get_str_height(n->right)) : 0;
}

static tsfi_string_bst_node* rotate_str_right(tsfi_string_bst_node *y) {
    tsfi_string_bst_node *x = y->left;
    tsfi_string_bst_node *T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = 1 + (get_str_height(y->left) > get_str_height(y->right) ? get_str_height(y->left) : get_str_height(y->right));
    x->height = 1 + (get_str_height(x->left) > get_str_height(x->right) ? get_str_height(x->left) : get_str_height(x->right));

    return x;
}

static tsfi_string_bst_node* rotate_str_left(tsfi_string_bst_node *x) {
    tsfi_string_bst_node *y = x->right;
    tsfi_string_bst_node *T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = 1 + (get_str_height(x->left) > get_str_height(x->right) ? get_str_height(x->left) : get_str_height(x->right));
    y->height = 1 + (get_str_height(y->left) > get_str_height(y->right) ? get_str_height(y->left) : get_str_height(y->right));

    return y;
}

tsfi_string_bst_node* tsfi_string_bst_insert(tsfi_string_bst_node *node, const char *key, const char *value) {
    if (!node) {
        tsfi_string_bst_node *new_node = (tsfi_string_bst_node*)malloc(sizeof(tsfi_string_bst_node));
        if (new_node) {
            new_node->key = strdup(key);
            new_node->value = strdup(value);
            new_node->height = 1;
            new_node->left = NULL;
            new_node->right = NULL;
        }
        return new_node;
    }

    int cmp = strcmp(key, node->key);
    if (cmp < 0) {
        node->left = tsfi_string_bst_insert(node->left, key, value);
    } else if (cmp > 0) {
        node->right = tsfi_string_bst_insert(node->right, key, value);
    } else {
        free(node->value);
        node->value = strdup(value);
        return node;
    }

    node->height = 1 + (get_str_height(node->left) > get_str_height(node->right) ? get_str_height(node->left) : get_str_height(node->right));

    int balance = get_str_balance(node);

    if (balance > 1 && strcmp(key, node->left->key) < 0) {
        return rotate_str_right(node);
    }
    if (balance < -1 && strcmp(key, node->right->key) > 0) {
        return rotate_str_left(node);
    }
    if (balance > 1 && strcmp(key, node->left->key) > 0) {
        node->left = rotate_str_left(node->left);
        return rotate_str_right(node);
    }
    if (balance < -1 && strcmp(key, node->right->key) < 0) {
        node->right = rotate_str_right(node->right);
        return rotate_str_left(node);
    }

    return node;
}

const char* tsfi_string_bst_find(tsfi_string_bst_node *root, const char *key) {
    if (!root) return NULL;
    int cmp = strcmp(key, root->key);
    if (cmp < 0) return tsfi_string_bst_find(root->left, key);
    if (cmp > 0) return tsfi_string_bst_find(root->right, key);
    return root->value;
}

void tsfi_string_bst_destroy(tsfi_string_bst_node *root) {
    if (!root) return;
    tsfi_string_bst_destroy(root->left);
    tsfi_string_bst_destroy(root->right);
    free(root->key);
    free(root->value);
    free(root);
}

static void trie_to_bst_dfs(tsfi_trie_node *trie_node, char *buffer, int depth, tsfi_string_bst_node **bst_root) {
    if (!trie_node) return;

    buffer[depth] = trie_node->ch;
    buffer[depth + 1] = '\0';

    if (trie_node->is_end) {
        *bst_root = tsfi_string_bst_insert(*bst_root, buffer, trie_node->phoneme);
    }

    trie_to_bst_dfs(trie_node->child, buffer, depth + 1, bst_root);
    trie_to_bst_dfs(trie_node->sibling, buffer, depth, bst_root);
}

tsfi_string_bst_node* tsfi_trie_to_string_bst(tsfi_trie_node *trie_root) {
    if (!trie_root) return NULL;
    tsfi_string_bst_node *bst_root = NULL;
    char buffer[256];
    trie_to_bst_dfs(trie_root->child, buffer, 0, &bst_root);
    return bst_root;
}
