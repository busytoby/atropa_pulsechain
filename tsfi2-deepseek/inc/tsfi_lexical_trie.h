#ifndef TSFI_LEXICAL_TRIE_H
#define TSFI_LEXICAL_TRIE_H

#include "tsfi_trie.h"

// Lexical Trie Dictionary Structure
typedef struct {
    tsfi_trie_node *root;
} TSFiLexicalTrie;

// Create lexical dictionary trie
TSFiLexicalTrie* tsfi_lexical_trie_create(void);

// Insert valid word to lexical dictionary
void tsfi_lexical_trie_insert(TSFiLexicalTrie *dict, const char *word);

// Check if word is valid in dictionary
int tsfi_lexical_trie_contains(TSFiLexicalTrie *dict, const char *word);

// Destroy lexical dictionary
void tsfi_lexical_trie_destroy(TSFiLexicalTrie *dict);

#endif // TSFI_LEXICAL_TRIE_H
