#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_qing_graph.h"

int main(void) {
    printf("=== TSFi Dysnomia Networks Undirected Qing Graph Verification ===\n");

    tsfi_qing_graph_node nodes[TSFI_NET_COUNT];
    tsfi_qing_graph_init(nodes);

    // Build the undirected network topology links:
    // ZMM <-> Anvil_EVM <-> PulseChain
    tsfi_qing_graph_add_edge(&nodes[TSFI_NET_ZMM], &nodes[TSFI_NET_ANVIL_EVM]);
    tsfi_qing_graph_add_edge(&nodes[TSFI_NET_ANVIL_EVM], &nodes[TSFI_NET_PULSECHAIN]);

    // Insert mock Qings into separate network BSTs
    CachedContract c1;
    strcpy(c1.name, "zmm_qing_wbtc");
    c1.virtual_address = 0x85e1ULL;
    nodes[TSFI_NET_ZMM].bst_root = tsfi_qing_bst_insert(nodes[TSFI_NET_ZMM].bst_root, c1.virtual_address, &c1);

    CachedContract c2;
    strcpy(c2.name, "anvil_qing_usdc");
    c2.virtual_address = 0x39e9ULL;
    nodes[TSFI_NET_ANVIL_EVM].bst_root = tsfi_qing_bst_insert(nodes[TSFI_NET_ANVIL_EVM].bst_root, c2.virtual_address, &c2);

    CachedContract c3;
    strcpy(c3.name, "pulsechain_qing_wpls");
    c3.virtual_address = 0x8157ULL;
    nodes[TSFI_NET_PULSECHAIN].bst_root = tsfi_qing_bst_insert(nodes[TSFI_NET_PULSECHAIN].bst_root, c3.virtual_address, &c3);

    // 1. Search locally on ZMM
    printf("[ROUTE] Searching for 0x85e1 (ZMM native) starting from ZMM...\n");
    CachedContract *res1 = tsfi_qing_graph_route_find(nodes, TSFI_NET_ZMM, 0x85e1ULL);
    assert(res1 != NULL);
    assert(strcmp(res1->name, "zmm_qing_wbtc") == 0);
    printf("  [PASS] Found locally: %s\n", res1->name);

    // 2. Search routed from ZMM to Anvil EVM (1 hop)
    printf("[ROUTE] Searching for 0x39e9 (Anvil native) starting from ZMM...\n");
    CachedContract *res2 = tsfi_qing_graph_route_find(nodes, TSFI_NET_ZMM, 0x39e9ULL);
    assert(res2 != NULL);
    assert(strcmp(res2->name, "anvil_qing_usdc") == 0);
    printf("  [PASS] Routed successfully: %s\n", res2->name);

    // 3. Search routed from ZMM to PulseChain (2 hops: ZMM -> Anvil_EVM -> PulseChain)
    printf("[ROUTE] Searching for 0x8157 (PulseChain native) starting from ZMM...\n");
    CachedContract *res3 = tsfi_qing_graph_route_find(nodes, TSFI_NET_ZMM, 0x8157ULL);
    assert(res3 != NULL);
    assert(strcmp(res3->name, "pulsechain_qing_wpls") == 0);
    printf("  [PASS] Routed successfully: %s\n", res3->name);

    // 4. Verify search fails for non-existent addresses
    CachedContract *res_none = tsfi_qing_graph_route_find(nodes, TSFI_NET_ZMM, 0xdeadbeefULL);
    assert(res_none == NULL);
    printf("  [PASS] Routed search for 0xdeadbeef correctly returned NULL.\n");

    // Cleanup
    tsfi_qing_graph_destroy(nodes);
    printf("=== ALL DYSNOMIA NETWORK GRAPH TESTS PASSED ===\n");
    return 0;
}
