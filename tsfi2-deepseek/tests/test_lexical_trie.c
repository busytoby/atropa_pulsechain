#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_lexical_trie.h"

int main(void) {
    printf("[Lexical Trie] Starting dictionary lookup tests...\n");
    fflush(stdout);

    TSFiLexicalTrie *dict = tsfi_lexical_trie_create();
    assert(dict != NULL);

    tsfi_lexical_trie_insert(dict, "tone");
    tsfi_lexical_trie_insert(dict, "synthesizer");

    int found = tsfi_lexical_trie_contains(dict, "tone");
    printf("  [Lexical Contains] 'tone' found: %d (Expected 1)\n", found);
    fflush(stdout);
    assert(found == 1);

    found = tsfi_lexical_trie_contains(dict, "invalid_word");
    printf("  [Lexical Contains] 'invalid_word' found: %d (Expected 0)\n", found);
    fflush(stdout);
    assert(found == 0);

    // Cleanup
    tsfi_lexical_trie_destroy(dict);

    printf("[PASS] Dictionary lookup verified successfully!\n");
    fflush(stdout);
    return 0;
}
