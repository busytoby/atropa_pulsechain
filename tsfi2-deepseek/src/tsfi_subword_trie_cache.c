#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi_subword_trie_cache.h"

static tsfi_trie_node_t* create_node(void) {
    tsfi_trie_node_t *node = (tsfi_trie_node_t *)calloc(1, sizeof(tsfi_trie_node_t));
    return node;
}

tsfi_subword_trie_cache_t* tsfi_subword_trie_cache_create(void) {
    tsfi_subword_trie_cache_t *cache = (tsfi_subword_trie_cache_t *)calloc(1, sizeof(tsfi_subword_trie_cache_t));
    if (!cache) return NULL;
    cache->root = create_node();
    cache->total_tokens_cached = 0;
    return cache;
}

bool tsfi_subword_trie_cache_insert(tsfi_subword_trie_cache_t *cache, const char *token_str, uint32_t token_id, float bonus_score) {
    if (!cache || !cache->root || !token_str) return false;

    tsfi_trie_node_t *curr = cache->root;
    size_t len = strlen(token_str);
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)token_str[i];
        if (c >= TRIE_ALPHABET_SIZE) c = c % TRIE_ALPHABET_SIZE;
        if (!curr->children[c]) {
            curr->children[c] = create_node();
        }
        curr = curr->children[c];
    }
    curr->token_id = token_id;
    curr->bonus_score = bonus_score;
    curr->is_terminal = true;
    cache->total_tokens_cached++;
    return true;
}

// Fast Sub-Microsecond Cache Lookup (Rule 11: No console prints inside hot-path)
bool tsfi_subword_trie_cache_lookup(const tsfi_subword_trie_cache_t *cache, const char *token_str, uint32_t *token_id_out, float *bonus_out) {
    if (!cache || !cache->root || !token_str) return false;

    // Rule 11: Safety margin buffer check (>= 256 bytes)
    char safe_buf[256];
    snprintf(safe_buf, sizeof(safe_buf), "%s", token_str);

    const tsfi_trie_node_t *curr = cache->root;
    size_t len = strlen(safe_buf);
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)safe_buf[i];
        if (c >= TRIE_ALPHABET_SIZE) c = c % TRIE_ALPHABET_SIZE;
        if (!curr->children[c]) return false;
        curr = curr->children[c];
    }

    if (curr && curr->is_terminal) {
        if (token_id_out) *token_id_out = curr->token_id;
        if (bonus_out) *bonus_out = curr->bonus_score;
        return true;
    }
    return false;
}

static void free_trie_node(tsfi_trie_node_t *node) {
    if (!node) return;
    for (int i = 0; i < TRIE_ALPHABET_SIZE; i++) {
        if (node->children[i]) free_trie_node(node->children[i]);
    }
    free(node);
}

void tsfi_subword_trie_cache_destroy(tsfi_subword_trie_cache_t *cache) {
    if (!cache) return;
    if (cache->root) free_trie_node(cache->root);
    free(cache);
}
