#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_trie.h"

int main(void) {
    printf("=== TSFi Speech Synthesizer Trie Verification ===\n");

    // 1. Initialize Trie Root
    tsfi_trie_node *root = tsfi_trie_create_node('\0');
    assert(root != NULL);

    // 2. Insert Grapheme-to-Phoneme mappings
    tsfi_trie_insert(root, "eight", "EY T");
    tsfi_trie_insert(root, "eighty", "EY T IY");
    tsfi_trie_insert(root, "sh", "SH");
    tsfi_trie_insert(root, "she", "SH IY");
    tsfi_trie_insert(root, "shell", "SH EH L");

    // 3. Test exact lookups
    printf("[TRIE] Running exact pronunciation lookups...\n");
    const char *p1 = tsfi_trie_lookup(root, "eight");
    assert(p1 != NULL);
    assert(strcmp(p1, "EY T") == 0);
    printf("  [PASS] 'eight' mapped correctly to: %s\n", p1);

    const char *p2 = tsfi_trie_lookup(root, "eighty");
    assert(p2 != NULL);
    assert(strcmp(p2, "EY T IY") == 0);
    printf("  [PASS] 'eighty' mapped correctly to: %s\n", p2);

    const char *p_none = tsfi_trie_lookup(root, "nonexistent");
    assert(p_none == NULL);
    printf("  [PASS] Non-existent word correctly returned NULL.\n");

    // 4. Test longest prefix tokenization lookups
    printf("[TRIE] Running longest prefix rules translations...\n");
    int match_len = 0;
    const char *pref1 = tsfi_trie_longest_prefix(root, "shelter", &match_len);
    assert(pref1 != NULL);
    assert(match_len == 3); // matches "she"
    assert(strcmp(pref1, "SH IY") == 0);
    printf("  [PASS] 'shelter' matched prefix 'she' (len %d): %s\n", match_len, pref1);

    const char *pref2 = tsfi_trie_longest_prefix(root, "shun", &match_len);
    assert(pref2 != NULL);
    assert(match_len == 2); // matches "sh"
    assert(strcmp(pref2, "SH") == 0);
    printf("  [PASS] 'shun' matched prefix 'sh' (len %d): %s\n", match_len, pref2);

    // 5. Cleanup
    tsfi_trie_destroy(root);
    printf("=== ALL TRIE DICTIONARY TESTS PASSED ===\n");
    return 0;
}
