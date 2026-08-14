#ifndef TSFI_SUBWORD_TRIE_CACHE_H
#define TSFI_SUBWORD_TRIE_CACHE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define TRIE_ALPHABET_SIZE 128

// Aho-Corasick Trie Node for Sub-Microsecond Subword Token Cache
typedef struct tsfi_trie_node {
    struct tsfi_trie_node *children[TRIE_ALPHABET_SIZE];
    struct tsfi_trie_node *fail_link;
    uint32_t token_id;
    float    bonus_score;
    bool     is_terminal;
} tsfi_trie_node_t;

// Aho-Corasick Subword Cache Struct
typedef struct {
    tsfi_trie_node_t *root;
    size_t total_tokens_cached;
} tsfi_subword_trie_cache_t;

// Creates an Aho-Corasick Subword Trie Cache Object
tsfi_subword_trie_cache_t* tsfi_subword_trie_cache_create(void);

// Inserts a subword token into the Trie cache with its associated bonus score
bool tsfi_subword_trie_cache_insert(tsfi_subword_trie_cache_t *cache, const char *token_str, uint32_t token_id, float bonus_score);

// Fast Sub-Microsecond Cache Lookup (Rule 11: No console output inside hot-path)
bool tsfi_subword_trie_cache_lookup(const tsfi_subword_trie_cache_t *cache, const char *token_str, uint32_t *token_id_out, float *bonus_out);

// Destroys and frees an Aho-Corasick Trie Cache Object
void tsfi_subword_trie_cache_destroy(tsfi_subword_trie_cache_t *cache);

#endif // TSFI_SUBWORD_TRIE_CACHE_H
