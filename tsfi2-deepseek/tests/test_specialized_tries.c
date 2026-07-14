#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_tst.h"
#include "tsfi_radix_g2p.h"

int main(void) {
    printf("=== TSFi Specialized Tries Verification ===\n");

    // ----------------------------------------------------
    // TEST 1: Ternary Search Tree (TST) RPC Router
    // ----------------------------------------------------
    printf("[TST] Initializing Ternary Search Tree RPC Router...\n");
    tsfi_tst_node *tst_root = NULL;

    tst_root = tsfi_tst_insert(tst_root, "eth_blockNumber", "eth_blockNumber_handler");
    tst_root = tsfi_tst_insert(tst_root, "eth_sendRawTransaction", "eth_sendRawTransaction_handler");
    tst_root = tsfi_tst_insert(tst_root, "eth_getBalance", "eth_getBalance_handler");
    tst_root = tsfi_tst_insert(tst_root, "zmm_executeThunk", "zmm_executeThunk_handler");

    printf("[TST] Resolving RPC methods...\n");
    const char *h1 = tsfi_tst_search(tst_root, "eth_getBalance");
    assert(h1 != NULL && strcmp(h1, "eth_getBalance_handler") == 0);
    printf("  [PASS] 'eth_getBalance' -> %s\n", h1);

    const char *h2 = tsfi_tst_search(tst_root, "zmm_executeThunk");
    assert(h2 != NULL && strcmp(h2, "zmm_executeThunk_handler") == 0);
    printf("  [PASS] 'zmm_executeThunk' -> %s\n", h2);

    const char *h_none = tsfi_tst_search(tst_root, "invalidMethod");
    assert(h_none == NULL);
    printf("  [PASS] Non-existent method resolved to NULL correctly.\n");

    tsfi_tst_destroy(tst_root);
    printf("[TST] Successfully verified TST RPC Routing.\n");

    // ----------------------------------------------------
    // TEST 2: Context-Sensitive G2P Rule Radix Trie
    // ----------------------------------------------------
    printf("[RADIX] Initializing Context-Sensitive G2P Radix Trie...\n");
    tsfi_radix_node *radix_root = tsfi_radix_create_node(NULL);

    // Rule: 'c' followed by 'e' -> /s/, otherwise -> /k/
    tsfi_radix_insert_rule(radix_root, "c", "k", "e", "s");
    // Rule: 'c' followed by 'i' -> /s/
    tsfi_radix_insert_rule(radix_root, "c", "k", "i", "s");
    // Rule: 'ph' -> /f/
    tsfi_radix_insert_rule(radix_root, "ph", "f", NULL, NULL);

    printf("[RADIX] Translating context-sensitive text segments...\n");
    int len = 0;
    
    // Test 'c' followed by 'a' (no match for 'e' or 'i') -> should resolve to default "k", matching 1 char
    const char *p1 = tsfi_radix_translate(radix_root, "cat", &len);
    assert(p1 != NULL && strcmp(p1, "k") == 0);
    assert(len == 1);
    printf("  [PASS] 'cat' segment matched '%s' (len=%d) -> Phoneme: /%s/\n", "c", len, p1);

    // Test 'c' followed by 'e' -> should resolve to context "s", matching 2 chars ("ce")
    const char *p2 = tsfi_radix_translate(radix_root, "cell", &len);
    assert(p2 != NULL && strcmp(p2, "s") == 0);
    assert(len == 2);
    printf("  [PASS] 'cell' segment matched '%s' (len=%d) -> Phoneme: /%s/\n", "ce", len, p2);

    // Test 'ph' -> should resolve to "f", matching 2 chars ("ph")
    const char *p3 = tsfi_radix_translate(radix_root, "phone", &len);
    assert(p3 != NULL && strcmp(p3, "f") == 0);
    assert(len == 2);
    printf("  [PASS] 'phone' segment matched '%s' (len=%d) -> Phoneme: /%s/\n", "ph", len, p3);

    tsfi_radix_destroy(radix_root);
    printf("[RADIX] Successfully verified Context-Sensitive Radix G2P translation.\n");

    printf("=== ALL SPECIALIZED TRIES TESTS PASSED ===\n");
    return 0;
}
