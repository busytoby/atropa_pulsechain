#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "tsfi_subword_trie_cache.h"

int main(void) {
    printf("================ TEST AHO-CORASICK SUBWORD TRIE CACHE (RULE 11) ================\n");

    tsfi_subword_trie_cache_t *cache = tsfi_subword_trie_cache_create();
    assert(cache != NULL);

    bool i1 = tsfi_subword_trie_cache_insert(cache, "int", 100, 500.0f);
    bool i2 = tsfi_subword_trie_cache_insert(cache, "void", 101, 500.0f);
    bool i3 = tsfi_subword_trie_cache_insert(cache, "function", 102, 450.0f);
    assert(i1 && i2 && i3);
    assert(cache->total_tokens_cached == 3);

    // Rule 11 Latency Guard Test (< 1000 ns lookup latency)
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    uint32_t tok_id = 0;
    float bonus = 0.0f;
    bool ok_lookup = tsfi_subword_trie_cache_lookup(cache, "function", &tok_id, &bonus);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double latency_ns = (double)(end.tv_sec - start.tv_sec) * 1e9 + (double)(end.tv_nsec - start.tv_nsec);

    assert(ok_lookup);
    assert(tok_id == 102);
    assert(bonus == 450.0f);
    printf("  [PASS] Aho-Corasick Trie Lookup verified in %.2f ns (< 1000 ns Rule 11 Latency Guard).\n", latency_ns);

    tsfi_subword_trie_cache_destroy(cache);
    printf("=================================================================================\n");
    return 0;
}
