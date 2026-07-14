#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_nibble_trie.h"

int main(void) {
    printf("=== TSFi Hex-Nibble Address Prefix Trie Verification ===\n");

    // Initialize root
    tsfi_nibble_node *root = tsfi_nibble_create_node();
    assert(root != NULL);

    // Insert dynamic ZMM contract mappings using various prefix formats
    printf("[NIBBLE] Inserting contract address target mappings...\n");
    tsfi_nibble_insert(root, "0x8F1ACB68fF904244322E5ecCa636F8Db4C635Db6", "musicMaker_contract");
    tsfi_nibble_insert(root, "dynamic_0x270049e5af030c305fe92c3af096a5e3445b7fb7", "WinchesterMQ_contract");
    tsfi_nibble_insert(root, "dynamic_1234567890abcdef1234567890abcdef12345678", "user_contract_a");

    // Lookups
    printf("[NIBBLE] Querying contract target resolutions...\n");
    const char *r1 = tsfi_nibble_lookup(root, "0x8F1ACB68fF904244322E5ecCa636F8Db4C635Db6");
    assert(r1 != NULL && strcmp(r1, "musicMaker_contract") == 0);
    printf("  [PASS] '0x8F1ACB...' -> %s\n", r1);

    // Test with prefix changes - should resolve identical values since dynamic_ prefixes are skipped
    const char *r2 = tsfi_nibble_lookup(root, "dynamic_0x8F1ACB68fF904244322E5ecCa636F8Db4C635Db6");
    assert(r2 != NULL && strcmp(r2, "musicMaker_contract") == 0);
    printf("  [PASS] 'dynamic_0x8F1ACB...' -> %s\n", r2);

    const char *r3 = tsfi_nibble_lookup(root, "0x270049e5af030c305fe92c3af096a5e3445b7fb7");
    assert(r3 != NULL && strcmp(r3, "WinchesterMQ_contract") == 0);
    printf("  [PASS] '0x270049e5...' -> %s\n", r3);

    const char *r_none = tsfi_nibble_lookup(root, "0xaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    assert(r_none == NULL);
    printf("  [PASS] Non-existent contract address resolved to NULL.\n");

    // Clean up
    tsfi_nibble_destroy(root);
    printf("=== ALL HEX-NIBBLE ADDR TRIE TESTS PASSED ===\n");
    return 0;
}
