#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_qing_bst.h"

int main(void) {
    printf("=== TSFi ZMM Qing BST Indexer Verification ===\n");

    // Mock contract cache setup
    g_cached_contracts_count = 0;

    // Simulate 3 loaded Qings on the ZMM VM
    CachedContract c1;
    strcpy(c1.name, "qing_wbtc");
    c1.virtual_address = 0x85e14bb47b22cf82ULL;
    g_cached_contracts[g_cached_contracts_count++] = c1;

    CachedContract c2;
    strcpy(c2.name, "qing_usdc");
    c2.virtual_address = 0x39e9e8edf0165247ULL;
    g_cached_contracts[g_cached_contracts_count++] = c2;

    CachedContract c3;
    strcpy(c3.name, "qing_wpls");
    c3.virtual_address = 0x8157a5ac755d7021ULL;
    g_cached_contracts[g_cached_contracts_count++] = c3;

    // Build the binary search tree
    tsfi_qing_bst_node *root = tsfi_qing_bst_populate();
    assert(root != NULL);

    // Search and verify lookups
    CachedContract *res1 = tsfi_qing_bst_find(root, 0x85e14bb47b22cf82ULL);
    assert(res1 != NULL);
    assert(strcmp(res1->name, "qing_wbtc") == 0);
    printf("[PASS] Found qing_wbtc by address 0x85e14bb47b22cf82 successfully.\n");

    CachedContract *res2 = tsfi_qing_bst_find(root, 0x39e9e8edf0165247ULL);
    assert(res2 != NULL);
    assert(strcmp(res2->name, "qing_usdc") == 0);
    printf("[PASS] Found qing_usdc by address 0x39e9e8edf0165247 successfully.\n");

    // Search for non-existent key
    CachedContract *res_none = tsfi_qing_bst_find(root, 0xdeadbeefdeadbeefULL);
    assert(res_none == NULL);
    printf("[PASS] Non-existent address lookup returned NULL as expected.\n");

    tsfi_qing_bst_destroy(root);
    printf("=== ALL BST TESTS PASSED ===\n");
    return 0;
}
