#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_trie_dispatcher.h"

int main(void) {
    printf("=== TSFi Multi-Purpose Trie Dispatchers Verification ===\n");

    // 1. Initialize Routers
    tsfi_trie_node *rpc_router = tsfi_trie_init_rpc_router();
    tsfi_trie_node *abi_router = tsfi_trie_init_abi_router();
    tsfi_trie_node *scsi_router = tsfi_trie_init_scsi_router();

    assert(rpc_router != NULL);
    assert(abi_router != NULL);
    assert(scsi_router != NULL);

    // 2. Verify RPC Router
    printf("[ROUTER] Verifying JSON-RPC Method Dispatcher...\n");
    int rpc_val1 = tsfi_trie_resolve_rpc(rpc_router, "wave512.run");
    assert(rpc_val1 == 1);
    printf("  [PASS] 'wave512.run' resolved to type code: %d\n", rpc_val1);

    int rpc_val2 = tsfi_trie_resolve_rpc(rpc_router, "manifold.swap_asset");
    assert(rpc_val2 == 20);
    printf("  [PASS] 'manifold.swap_asset' resolved to type code: %d\n", rpc_val2);

    int rpc_fail = tsfi_trie_resolve_rpc(rpc_router, "nonexistent.method");
    assert(rpc_fail == 0);
    printf("  [PASS] Invalid RPC returned fallback type code: %d\n", rpc_fail);

    // 3. Verify ABI Router
    printf("[ROUTER] Verifying Yul ABI selector Dispatcher...\n");
    const char *abi_name1 = tsfi_trie_resolve_abi(abi_router, "18c1ab9a");
    assert(abi_name1 != NULL);
    assert(strcmp(abi_name1, "predictMelQuantized") == 0);
    printf("  [PASS] '18c1ab9a' selector resolved to function: %s\n", abi_name1);

    const char *abi_name2 = tsfi_trie_resolve_abi(abi_router, "5f60cdf4");
    assert(abi_name2 != NULL);
    assert(strcmp(abi_name2, "registerSpeaker") == 0);
    printf("  [PASS] '5f60cdf4' selector resolved to function: %s\n", abi_name2);

    const char *abi_fail = tsfi_trie_resolve_abi(abi_router, "deadbeef");
    assert(abi_fail == NULL);
    printf("  [PASS] Invalid ABI selector correctly returned NULL.\n");

    // 4. Verify SCSI Prefix Router
    printf("[ROUTER] Verifying SCSI command prefix Dispatcher...\n");
    const char *scsi_action1 = tsfi_trie_resolve_scsi(scsi_router, "SCSI_INQUIRY_LUN0");
    assert(scsi_action1 != NULL);
    assert(strcmp(scsi_action1, "handshake_inquiry") == 0);
    printf("  [PASS] 'SCSI_INQUIRY_LUN0' prefix matched to action: %s\n", scsi_action1);

    const char *scsi_action2 = tsfi_trie_resolve_scsi(scsi_router, "SCSI_READ_BLOCK_100");
    assert(scsi_action2 != NULL);
    assert(strcmp(scsi_action2, "handshake_read") == 0);
    printf("  [PASS] 'SCSI_READ_BLOCK_100' prefix matched to action: %s\n", scsi_action2);

    const char *scsi_fail = tsfi_trie_resolve_scsi(scsi_router, "SCSI_UNKNOWN_OPCODE");
    assert(scsi_fail == NULL);
    printf("  [PASS] Unmatched SCSI command prefix returned NULL.\n");

    // 5. Verify IP CIDR Router
    printf("[ROUTER] Verifying IP CIDR Router...\n");
    tsfi_trie_node *cidr_router = tsfi_trie_init_cidr_router();
    assert(cidr_router != NULL);
    tsfi_trie_add_cidr_route(cidr_router, "192.168.1.0/24", "AS_LOCAL");
    tsfi_trie_add_cidr_route(cidr_router, "10.0.0.0/8", "AS_BACKBONE");

    const char *route1 = tsfi_trie_route_ip(cidr_router, "192.168.1.53");
    assert(route1 != NULL && strcmp(route1, "AS_LOCAL") == 0);
    printf("  [PASS] IP '192.168.1.53' correctly routed to: %s\n", route1);

    const char *route2 = tsfi_trie_route_ip(cidr_router, "10.250.0.1");
    assert(route2 != NULL && strcmp(route2, "AS_BACKBONE") == 0);
    printf("  [PASS] IP '10.250.0.1' correctly routed to: %s\n", route2);

    const char *route_fail = tsfi_trie_route_ip(cidr_router, "8.8.8.8");
    assert(route_fail == NULL);
    printf("  [PASS] Unmatched IP '8.8.8.8' correctly returned NULL.\n");

    // 6. Verify Contract Namespace Router
    printf("[ROUTER] Verifying Contract Namespace Router...\n");
    tsfi_trie_node *namespace_router = tsfi_trie_init_contract_namespace_router();
    assert(namespace_router != NULL);

    const char *ns1 = tsfi_trie_resolve_contract_namespace(namespace_router, "dynamic_0x7f03a45a");
    assert(ns1 != NULL && strcmp(ns1, "dynamic_contract") == 0);
    printf("  [PASS] Contract 'dynamic_0x7f03a45a' prefix matched to: %s\n", ns1);

    const char *ns2 = tsfi_trie_resolve_contract_namespace(namespace_router, "sys_get_time");
    assert(ns2 != NULL && strcmp(ns2, "system_contract") == 0);
    printf("  [PASS] Contract 'sys_get_time' prefix matched to: %s\n", ns2);

    const char *ns_fail = tsfi_trie_resolve_contract_namespace(namespace_router, "random_addr");
    assert(ns_fail == NULL);
    printf("  [PASS] Unmatched contract 'random_addr' prefix correctly returned NULL.\n");

    // 7. Cleanup
    tsfi_trie_destroy(rpc_router);
    tsfi_trie_destroy(abi_router);
    tsfi_trie_destroy(scsi_router);
    tsfi_trie_destroy(cidr_router);
    tsfi_trie_destroy(namespace_router);

    printf("=== ALL MULTI-PURPOSE TRIE ROUTING TESTS PASSED ===\n");
    return 0;
}
