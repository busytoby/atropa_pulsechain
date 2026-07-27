#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_lexical_trie.h"

TSFiLexicalTrie* tsfi_lexical_trie_create(void) {
    TSFiLexicalTrie *dict = calloc(1, sizeof(TSFiLexicalTrie));
    if (!dict) return NULL;
    dict->root = tsfi_trie_create_node(0);
    return dict;
}

void tsfi_lexical_trie_insert(TSFiLexicalTrie *dict, const char *word) {
    if (!dict || !dict->root || !word) return;
    tsfi_trie_insert(dict->root, word, "VALID");
}

int tsfi_lexical_trie_contains(TSFiLexicalTrie *dict, const char *word) {
    if (!dict || !dict->root || !word) return 0;
    const char *res = tsfi_trie_lookup(dict->root, word);
    return (res && strcmp(res, "VALID") == 0) ? 1 : 0;
}

void tsfi_lexical_trie_destroy(TSFiLexicalTrie *dict) {
    if (!dict) return;
    if (dict->root) {
        tsfi_trie_destroy(dict->root);
    }
    free(dict);
}
