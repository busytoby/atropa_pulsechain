#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "tsfi_hogan.h"

int main(void) {
    printf("=============================================================\n");
    printf("HOGAN SYSTEMS UMBRELLA TRANSACTION SYSTEM - END TO END TESTS\n");
    printf("=============================================================\n");

    hogan_umbrella_system sys;
    tsfi_hogan_init(&sys);

    // 1. Setup multi-account environment
    printf("[E2E] Registering initial account set...\n");
    assert(tsfi_hogan_register_account(&sys, 1001, 10000) == 0); // Alice
    assert(tsfi_hogan_register_account(&sys, 2002, 5000) == 0);  // Bob
    assert(tsfi_hogan_register_account(&sys, 3003, 2000) == 0);  // Charlie
    
    // Verify initial total supply
    uint64_t initial_supply = 10000 + 5000 + 2000;
    
    // 2. Dispatch sequence of valid multi-VM transactions
    printf("[E2E] Dispatching valid transactions...\n");
    assert(tsfi_hogan_dispatch_tx(&sys, 1001, 2002, 3000, VM_EVM) == 0);   // Alice -> Bob: 3000 (EVM)
    assert(tsfi_hogan_dispatch_tx(&sys, 2002, 3003, 1500, VM_ZMM) == 0);   // Bob -> Charlie: 1500 (ZMM)
    assert(tsfi_hogan_dispatch_tx(&sys, 3003, 1001, 500, VM_RAMAC) == 0);  // Charlie -> Alice: 500 (RAMAC)
    
    // Verify live tx queue limits
    assert(sys.tx_count == 3);
    assert(sys.accounts[0].balance == 10000); // Balances remain unmodified during live phase
    
    // 3. Dispatch invalid transactions to test resilience
    printf("[E2E] Dispatching invalid/adversarial transactions...\n");
    assert(tsfi_hogan_dispatch_tx(&sys, 3003, 2002, 10000, VM_EVM) == 0);  // Charlie -> Bob: 10000 (Exceeds balance)
    assert(tsfi_hogan_dispatch_tx(&sys, 9999, 1001, 100, VM_ZMM) == 0);    // Non-existent sender
    
    // 4. Run Overnight Batch Reconciliation (Epoch 1 -> 2)
    printf("[E2E] Running overnight batch reconciliation...\n");
    assert(tsfi_hogan_overnight_reconciliation(&sys, "hogan_lfs.dat.bin") == 0);
    assert(sys.current_epoch == 2);
    
    // 5. Verify final balance state & conservation
    printf("[E2E] Verifying final ledger states...\n");
    // Expected balances:
    // Alice: 10000 - 3000 (sent) + 500 (received) = 7500
    // Bob: 5000 + 3000 (received) - 1500 (sent) = 6500
    // Charlie: 2000 + 1500 (received) - 500 (sent) = 3000
    // Charlie's overdrawn tx of 10000 should have failed silently during reconciliation, retaining Charlie at 3000.
    // Non-existent sender tx should have failed, leaving Alice at 7500.
    
    assert(sys.accounts[0].balance == 7500);
    assert(sys.accounts[1].balance == 6500);
    assert(sys.accounts[2].balance == 3000);
    
    uint64_t final_supply = sys.accounts[0].balance + sys.accounts[1].balance + sys.accounts[2].balance;
    assert(final_supply == initial_supply);
    printf("  [PASS] Conservation of funds verified. Total supply: %lu\n", final_supply);
    
    // Verify acab_epoch_root updated
    uint8_t zero_hash[32] = {0};
    assert(memcmp(sys.acab_epoch_root, zero_hash, 32) != 0);
    printf("  [PASS] ACAB Epoch Sync root successfully hashed.\n");
    
    // 6. Test Logical File System (LFS) Reload
    printf("[E2E] Verifying LFS database restore state...\n");
    hogan_umbrella_system restored_sys;
    tsfi_hogan_init(&restored_sys);
    assert(tsfi_hogan_lfs_load(&restored_sys, "hogan_lfs.dat.bin") == 0);
    
    assert(restored_sys.current_epoch == 2);
    assert(restored_sys.accounts[0].account_id == 1001 && restored_sys.accounts[0].balance == 7500);
    assert(restored_sys.accounts[1].account_id == 2002 && restored_sys.accounts[1].balance == 6500);
    assert(restored_sys.accounts[2].account_id == 3003 && restored_sys.accounts[2].balance == 3000);
    assert(memcmp(restored_sys.acab_epoch_root, sys.acab_epoch_root, 32) == 0);
    printf("  [PASS] LFS binary state fully restored from hogan_lfs.dat.bin.\n");

    // 7. Enforce Rule 13 (Only .dat.bin formats)
    printf("[E2E] Verifying Rule 13 file format restriction...\n");
    assert(tsfi_hogan_lfs_save(&sys, "hogan_lfs.json") == -3);
    printf("  [PASS] Invalid file extension properly rejected by LFS interface.\n");

    // 8. Test Sequential Block-Record Queue (BLACK Box)
    printf("[E2E] Testing Sequential Block-Record Queue...\n");
    const char *seq_filepath = "hogan_seq.dat.bin";
    remove(seq_filepath); // ensure clean start
    
    uint8_t payload1[12] = {0};
    uint32_t acc_id1 = 5005;
    uint64_t bal1 = 99999;
    memcpy(payload1, &acc_id1, sizeof(uint32_t));
    memcpy(payload1 + 4, &bal1, sizeof(uint64_t));
    
    assert(tsfi_hogan_write_seq_record(seq_filepath, payload1, 12) == 0);
    assert(tsfi_hogan_write_seq_record("hogan_seq.json", payload1, 12) == -3); // Rule 13 check
    
    uint8_t payload2[12] = {0};
    uint32_t acc_id2 = 6006;
    uint64_t bal2 = 88888;
    memcpy(payload2, &acc_id2, sizeof(uint32_t));
    memcpy(payload2 + 4, &bal2, sizeof(uint64_t));
    
    assert(tsfi_hogan_write_seq_record(seq_filepath, payload2, 12) == 0);
    printf("  [PASS] Sequential records written successfully.\n");
    
    // 9. Test Runtime Record Dictionary Resolver (RED Box)
    printf("[E2E] Testing Runtime Record Dictionary Resolver...\n");
    hogan_record_dict dict;
    tsfi_hogan_init_dict(&dict);
    assert(tsfi_hogan_add_field(&dict, "acc_id", 0, FIELD_TYPE_UINT32) == 0);
    assert(tsfi_hogan_add_field(&dict, "balance", 4, FIELD_TYPE_UINT64) == 0);
    
    uint8_t read_payload[12] = {0};
    size_t read_size = 0;
    
    // Read and resolve record at index 0 (acc_id1, bal1)
    assert(tsfi_hogan_read_seq_record(seq_filepath, 0, read_payload, &read_size) == 0);
    assert(read_size == 12);
    
    uint32_t resolved_acc_id = 0;
    uint64_t resolved_bal = 0;
    assert(tsfi_hogan_resolve_uint32(&dict, read_payload, "acc_id", &resolved_acc_id) == 0);
    assert(tsfi_hogan_resolve_uint64(&dict, read_payload, "balance", &resolved_bal) == 0);
    assert(resolved_acc_id == 5005);
    assert(resolved_bal == 99999);
    
    // Read and resolve record at index 1 (acc_id2, bal2)
    assert(tsfi_hogan_read_seq_record(seq_filepath, 1, read_payload, &read_size) == 0);
    assert(read_size == 12);
    
    assert(tsfi_hogan_resolve_uint32(&dict, read_payload, "acc_id", &resolved_acc_id) == 0);
    assert(tsfi_hogan_resolve_uint64(&dict, read_payload, "balance", &resolved_bal) == 0);
    assert(resolved_acc_id == 6006);
    assert(resolved_bal == 88888);
    
    printf("  [PASS] Dict resolver extracted values: acc_id=%u, balance=%lu\n", resolved_acc_id, resolved_bal);
    
    remove(seq_filepath); // clean up

    // 10. Test Sequential Journal Recovery (Transaction Playback)
    printf("[E2E] Testing Sequential Journal Recovery...\n");
    const char *journal_path = "hogan_journal.dat.bin";
    remove(journal_path); // ensure clean start
    
    // Write transactions to journal
    hogan_transaction tx1 = { 1, 1001, 2002, 1000, VM_EVM, 0 };
    hogan_transaction tx2 = { 2, 2002, 3003, 500, VM_ZMM, 0 };
    
    assert(tsfi_hogan_write_journal(journal_path, &tx1) == 0);
    assert(tsfi_hogan_write_journal(journal_path, &tx2) == 0);
    
    // Reset/Reinitialize state to simulate system crash recovery
    hogan_umbrella_system recovery_sys;
    tsfi_hogan_init(&recovery_sys);
    assert(tsfi_hogan_register_account(&recovery_sys, 1001, 7500) == 0);
    assert(tsfi_hogan_register_account(&recovery_sys, 2002, 6500) == 0);
    assert(tsfi_hogan_register_account(&recovery_sys, 3003, 3000) == 0);
    
    // Replay journal log
    assert(tsfi_hogan_replay_journal(&recovery_sys, journal_path) == 0);
    
    // Assert balances are correctly recovered
    // Alice: 7500 - 1000 = 6500
    // Bob: 6500 + 1000 - 500 = 7000
    // Charlie: 3000 + 500 = 3500
    assert(recovery_sys.accounts[0].balance == 6500);
    assert(recovery_sys.accounts[1].balance == 7000);
    assert(recovery_sys.accounts[2].balance == 3500);
    
    remove(journal_path); // clean up
    printf("  [PASS] Journal transaction playback and system state recovery verified.\n");

    printf("ALL HOGAN SYSTEMS E2E C TESTS COMPLETED SUCCESSFULLY!\n");
    return 0;
}
