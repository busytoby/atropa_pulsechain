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

    // 5. Cleanup
    tsfi_trie_destroy(rpc_router);
    tsfi_trie_destroy(abi_router);
    tsfi_trie_destroy(scsi_router);

    printf("=== ALL MULTI-PURPOSE TRIE ROUTING TESTS PASSED ===\n");
    return 0;
}
