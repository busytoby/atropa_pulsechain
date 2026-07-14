#include "tsfi_trie.h"
#include <stdlib.h>
#include <string.h>

tsfi_trie_node* tsfi_trie_create_node(char ch) {
    tsfi_trie_node *node = (tsfi_trie_node*)malloc(sizeof(tsfi_trie_node));
    if (node) {
        node->ch = ch;
        node->is_end = false;
        node->phoneme = NULL;
        node->accumulator = 0;
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
        next->accumulator = (curr->accumulator + (uint8_t)ch) % MOTZKIN_PRIME;
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

uint64_t tsfi_trie_get_accumulator(tsfi_trie_node *root, const char *word) {
    if (!root || !word) return 0;
    
    tsfi_trie_node *curr = root;
    size_t len = strlen(word);
    
    for (size_t i = 0; i < len; i++) {
        curr = find_child(curr, word[i]);
        if (!curr) {
            return 0;
        }
    }
    
    return curr->accumulator;
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

typedef struct {
    char *key;
    char *val;
} tsfi_trie_entry;

static void trie_count_entries(tsfi_trie_node *node, size_t *count) {
    if (!node) return;
    if (node->is_end) (*count)++;
    trie_count_entries(node->child, count);
    trie_count_entries(node->sibling, count);
}

static void trie_collect_entries(tsfi_trie_node *node, char *buffer, int depth, tsfi_trie_entry *entries, size_t *index) {
    if (!node) return;
    buffer[depth] = node->ch;
    buffer[depth + 1] = '\0';
    if (node->is_end) {
        entries[*index].key = strdup(buffer);
        entries[*index].val = strdup(node->phoneme);
        (*index)++;
    }
    trie_collect_entries(node->child, buffer, depth + 1, entries, index);
    trie_collect_entries(node->sibling, buffer, depth, entries, index);
}

static void swap_entries(tsfi_trie_entry *a, tsfi_trie_entry *b) {
    tsfi_trie_entry temp = *a;
    *a = *b;
    *b = temp;
}

static size_t partition_entries(tsfi_trie_entry *arr, size_t low, size_t high) {
    size_t mid = low + (high - low) / 2;
    if (strcmp(arr[low].key, arr[mid].key) > 0) swap_entries(&arr[low], &arr[mid]);
    if (strcmp(arr[low].key, arr[high].key) > 0) swap_entries(&arr[low], &arr[high]);
    if (strcmp(arr[mid].key, arr[high].key) > 0) swap_entries(&arr[mid], &arr[high]);
    
    tsfi_trie_entry pivot = arr[mid];
    swap_entries(&arr[mid], &arr[high]);
    
    size_t i = low;
    for (size_t j = low; j < high; j++) {
        if (strcmp(arr[j].key, pivot.key) < 0) {
            swap_entries(&arr[i], &arr[j]);
            i++;
        }
    }
    swap_entries(&arr[i], &arr[high]);
    return i;
}

static void sort_entries(tsfi_trie_entry *arr, size_t low, size_t high) {
    if (low < high) {
        size_t p = partition_entries(arr, low, high);
        if (p > 0) sort_entries(arr, low, p - 1);
        sort_entries(arr, p + 1, high);
    }
}

static tsfi_string_bst_node* build_balanced_bst_from_sorted(tsfi_trie_entry *arr, int low, int high) {
    if (low > high) return NULL;
    
    int mid = low + (high - low) / 2;
    
    tsfi_string_bst_node *node = (tsfi_string_bst_node*)malloc(sizeof(tsfi_string_bst_node));
    if (node) {
        node->key = strdup(arr[mid].key);
        node->value = strdup(arr[mid].val);
        node->left = build_balanced_bst_from_sorted(arr, low, mid - 1);
        node->right = build_balanced_bst_from_sorted(arr, mid + 1, high);
        
        int hl = node->left ? node->left->height : 0;
        int hr = node->right ? node->right->height : 0;
        node->height = 1 + (hl > hr ? hl : hr);
    }
    return node;
}

tsfi_string_bst_node* tsfi_trie_to_string_bst_balanced(tsfi_trie_node *trie_root) {
    if (!trie_root) return NULL;
    
    size_t count = 0;
    trie_count_entries(trie_root->child, &count);
    if (count == 0) return NULL;
    
    tsfi_trie_entry *entries = (tsfi_trie_entry*)malloc(count * sizeof(tsfi_trie_entry));
    size_t index = 0;
    char buffer[256];
    trie_collect_entries(trie_root->child, buffer, 0, entries, &index);
    
    if (count > 1) {
        sort_entries(entries, 0, count - 1);
    }
    
    tsfi_string_bst_node *bst_root = build_balanced_bst_from_sorted(entries, 0, (int)count - 1);
    
    for (size_t i = 0; i < count; i++) {
        free(entries[i].key);
        free(entries[i].val);
    }
    free(entries);
    
    return bst_root;
}
