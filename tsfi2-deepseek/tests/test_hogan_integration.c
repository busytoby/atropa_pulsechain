#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "tsfi_hogan.h"

int main(void) {
    printf("=============================================================\n");
    printf("HOGAN SYSTEMS UMBRELLA, LFS, AND BATCH SYNC INTEGRATION TESTS\n");
    printf("=============================================================\n");

    hogan_umbrella_system sys;
    tsfi_hogan_init(&sys);

    // 1. Unified Umbrella Architecture tests
    printf("[Test] Registering accounts...\n");
    int res = tsfi_hogan_register_account(&sys, 101, 5000);
    assert(res == 0);
    res = tsfi_hogan_register_account(&sys, 202, 1000);
    assert(res == 0);
    res = tsfi_hogan_register_account(&sys, 101, 2000); // duplicate
    assert(res == -1);

    printf("[Test] Dispatching transactions...\n");
    res = tsfi_hogan_dispatch_tx(&sys, 101, 202, 500, VM_EVM);
    assert(res == 0);
    assert(sys.tx_count == 1);
    assert(sys.accounts[0].balance == 5000); // Balance shouldn't change until batch reconciliation

    // 2. Overnight Batch Reconciliation tests
    printf("[Test] Running overnight batch reconciliation...\n");
    res = tsfi_hogan_overnight_reconciliation(&sys, "hogan_lfs.dat.bin");
    assert(res == 0);
    assert(sys.current_epoch == 2);
    assert(sys.accounts[0].balance == 4500); // 5000 - 500
    assert(sys.accounts[1].balance == 1500); // 1000 + 500
    
    // Check that acab_epoch_root is not all zeros
    uint8_t zero_hash[32] = {0};
    assert(memcmp(sys.acab_epoch_root, zero_hash, 32) != 0);
    printf("  [PASS] Batch reconciliation and epoch root generation successful.\n");

    // 3. Logical File System (LFS) tests
    printf("[Test] Loading LFS state into fresh system...\n");
    hogan_umbrella_system new_sys;
    tsfi_hogan_init(&new_sys);
    
    res = tsfi_hogan_lfs_load(&new_sys, "hogan_lfs.dat.bin");
    assert(res == 0);
    assert(new_sys.current_epoch == 2);
    assert(new_sys.accounts[0].account_id == 101);
    assert(new_sys.accounts[0].balance == 4500);
    assert(new_sys.accounts[1].account_id == 202);
    assert(new_sys.accounts[1].balance == 1500);
    assert(memcmp(new_sys.acab_epoch_root, sys.acab_epoch_root, 32) == 0);
    printf("  [PASS] LFS serialization / deserialization successful.\n");

    // Rule 13: Enforce .dat.bin extension check
    printf("[Test] Verifying Rule 13 file extension enforcement...\n");
    res = tsfi_hogan_lfs_save(&sys, "hogan_lfs.json");
    assert(res == -3); // Should return error code indicating extension ban
    printf("  [PASS] Non-.dat.bin file formats correctly rejected.\n");

    printf("ALL HOGAN SYSTEMS INTEGRATION TESTS PASSED!\n");
    return 0;
}
