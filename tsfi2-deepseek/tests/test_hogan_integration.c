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

    // 11. Test Sequential Activity Audit Log (System Auditing)
    printf("[E2E] Testing Sequential Activity Audit Log...\n");
    const char *audit_path = "hogan_audit.dat.bin";
    remove(audit_path); // ensure clean start
    
    assert(tsfi_hogan_write_audit_log(audit_path, 1, 1001, "REGISTER", 1) == 0);
    assert(tsfi_hogan_write_audit_log(audit_path, 1, 2002, "REGISTER", 1) == 0);
    assert(tsfi_hogan_write_audit_log(audit_path, 1, 1001, "TRANSFER", 1) == 0);
    assert(tsfi_hogan_write_audit_log(audit_path, 1, 3003, "TRANSFER", 0) == 0); // failed
    
    assert(tsfi_hogan_write_audit_log("hogan_audit.json", 1, 1001, "REGISTER", 1) == -3); // Rule 13 check
    
    size_t count = 0;
    assert(tsfi_hogan_print_audit_trail(audit_path, &count) == 0);
    assert(count == 4);
    
    remove(audit_path); // clean up
    printf("  [PASS] Mainframe audit log trails correctly written and formatted.\n");

    // 12. Test Master Account Index Resolver (Storage Record Pointer Addressing)
    printf("[E2E] Testing Master Account Index Resolver...\n");
    const char *index_path = "hogan_index.dat.bin";
    remove(index_path); // ensure clean start
    
    assert(tsfi_hogan_write_account_index(index_path, 1001, 50) == 0);
    assert(tsfi_hogan_write_account_index(index_path, 2002, 120) == 0);
    assert(tsfi_hogan_write_account_index(index_path, 3003, 300) == 0);
    assert(tsfi_hogan_write_account_index("hogan_index.json", 3003, 300) == -3); // Rule 13 check
    
    uint32_t resolved_offset = 0;
    assert(tsfi_hogan_lookup_account_offset(index_path, 2002, &resolved_offset) == 0);
    assert(resolved_offset == 120);
    
    assert(tsfi_hogan_lookup_account_offset(index_path, 3003, &resolved_offset) == 0);
    assert(resolved_offset == 300);
    
    assert(tsfi_hogan_lookup_account_offset(index_path, 9999, &resolved_offset) == -1); // not found
    
    remove(index_path); // clean up
    printf("  [PASS] Master Account Index lookups and pointer resolutions verified.\n");

    // 13. Test Account Ledger Statement Generator (Statement Reporting)
    printf("[E2E] Testing Account Ledger Statement Generator...\n");
    const char *ledger_path = "hogan_ledger.dat.bin";
    remove(ledger_path); // ensure clean start
    
    assert(tsfi_hogan_write_ledger_entry(ledger_path, 1001, 3000, 0, "EVM TRANSFER") == 0);
    assert(tsfi_hogan_write_ledger_entry(ledger_path, 1001, 500, 1, "RAMAC DEPOSIT") == 0);
    assert(tsfi_hogan_write_ledger_entry(ledger_path, 2002, 3000, 1, "EVM TRANSFER") == 0);
    assert(tsfi_hogan_write_ledger_entry("hogan_ledger.json", 1001, 3000, 0, "EVM TRANSFER") == -3); // Rule 13 check
    
    size_t stmt_count = 0;
    assert(tsfi_hogan_print_statement(ledger_path, 1001, &stmt_count) == 0);
    assert(stmt_count == 2);
    
    assert(tsfi_hogan_print_statement(ledger_path, 2002, &stmt_count) == 0);
    assert(stmt_count == 1);
    
    assert(tsfi_hogan_print_statement(ledger_path, 9999, &stmt_count) == 0);
    assert(stmt_count == 0);
    
    remove(ledger_path); // clean up
    printf("  [PASS] Account Ledger Statements correctly written, filtered, and generated.\n");

    // 14. Test Batch Interest Posting Engine (Ledger Calculation Updates)
    printf("[E2E] Testing Batch Interest Posting Engine...\n");
    const char *interest_path = "hogan_interest.dat.bin";
    remove(interest_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system interest_sys;
    tsfi_hogan_init(&interest_sys);
    assert(tsfi_hogan_register_account(&interest_sys, 1001, 10000) == 0); // 10,000 units
    assert(tsfi_hogan_register_account(&interest_sys, 2002, 5000) == 0);  // 5,000 units
    
    // Apply 500 basis points interest (5%)
    assert(tsfi_hogan_apply_interest(&interest_sys, interest_path, 500) == 0);
    assert(tsfi_hogan_apply_interest(&interest_sys, "hogan_interest.json", 500) == -3); // Rule 13 check
    
    // Verify updated balances
    assert(interest_sys.accounts[0].balance == 10500); // 10000 + 500
    assert(interest_sys.accounts[1].balance == 5250);  // 5000 + 250
    
    // Read sequential log file and verify entries
    uint8_t read_buf[sizeof(hogan_interest_entry)];
    size_t rsize = 0;
    
    // Check entry 0
    assert(tsfi_hogan_read_seq_record(interest_path, 0, read_buf, &rsize) == 0);
    assert(rsize == sizeof(hogan_interest_entry));
    const hogan_interest_entry *entry0 = (const hogan_interest_entry *)read_buf;
    assert(entry0->account_id == 1001);
    assert(entry0->original_balance == 10000);
    assert(entry0->interest_added == 500);
    assert(entry0->new_balance == 10500);
    
    // Check entry 1
    assert(tsfi_hogan_read_seq_record(interest_path, 1, read_buf, &rsize) == 0);
    assert(rsize == sizeof(hogan_interest_entry));
    const hogan_interest_entry *entry1 = (const hogan_interest_entry *)read_buf;
    assert(entry1->account_id == 2002);
    assert(entry1->original_balance == 5000);
    assert(entry1->interest_added == 250);
    assert(entry1->new_balance == 5250);
    
    remove(interest_path); // clean up
    printf("  [PASS] Batch interest rates calculated, applied, and logged successfully.\n");

    // 15. Test Batch Fee Application Engine (Ledger Maintenance Deductions)
    printf("[E2E] Testing Batch Fee Application Engine...\n");
    const char *fees_path = "hogan_fees.dat.bin";
    remove(fees_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system fees_sys;
    tsfi_hogan_init(&fees_sys);
    assert(tsfi_hogan_register_account(&fees_sys, 1001, 1000) == 0); // 1,000 units
    assert(tsfi_hogan_register_account(&fees_sys, 2002, 30) == 0);   // 30 units (insufficient for full fee)
    
    // Apply 50 units flat maintenance fee
    assert(tsfi_hogan_apply_fees(&fees_sys, fees_path, 50) == 0);
    assert(tsfi_hogan_apply_fees(&fees_sys, "hogan_fees.json", 50) == -3); // Rule 13 check
    
    // Verify updated balances
    assert(fees_sys.accounts[0].balance == 950); // 1000 - 50
    assert(fees_sys.accounts[1].balance == 0);   // 30 - 30 (clamped)
    
    // Read sequential log file and verify entries
    uint8_t read_fbuf[sizeof(hogan_fee_entry)];
    size_t fsize = 0;
    
    // Check entry 0
    assert(tsfi_hogan_read_seq_record(fees_path, 0, read_fbuf, &fsize) == 0);
    assert(fsize == sizeof(hogan_fee_entry));
    const hogan_fee_entry *fentry0 = (const hogan_fee_entry *)read_fbuf;
    assert(fentry0->account_id == 1001);
    assert(fentry0->original_balance == 1000);
    assert(fentry0->fee_deducted == 50);
    assert(fentry0->new_balance == 950);
    
    // Check entry 1
    assert(tsfi_hogan_read_seq_record(fees_path, 1, read_fbuf, &fsize) == 0);
    assert(fsize == sizeof(hogan_fee_entry));
    const hogan_fee_entry *fentry1 = (const hogan_fee_entry *)read_fbuf;
    assert(fentry1->account_id == 2002);
    assert(fentry1->original_balance == 30);
    assert(fentry1->fee_deducted == 30); // clamped fee
    assert(fentry1->new_balance == 0);
    
    remove(fees_path); // clean up
    printf("  [PASS] Batch maintenance fees calculated, applied, and logged successfully.\n");

    // 16. Test Overdraft Protection Engine (Backup Balance Linkage)
    printf("[E2E] Testing Overdraft Protection Engine...\n");
    const char *overdraft_path = "hogan_overdraft.dat.bin";
    remove(overdraft_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system overdraft_sys;
    tsfi_hogan_init(&overdraft_sys);
    assert(tsfi_hogan_register_account(&overdraft_sys, 1001, 1000) == 0); // Alice: 1,000 units
    assert(tsfi_hogan_register_account(&overdraft_sys, 2002, 5000) == 0); // Bob: 5,000 units
    assert(tsfi_hogan_register_account(&overdraft_sys, 3003, 0) == 0);    // Charlie: 0 units
    
    // Link Bob (2002) as Alice's (1001) backup
    assert(tsfi_hogan_link_backup(&overdraft_sys, 1001, 2002) == 0);
    
    // Alice transfers 3000 to Charlie (requires 2000 from Bob)
    assert(tsfi_hogan_dispatch_tx(&overdraft_sys, 1001, 3003, 3000, VM_EVM) == 0);
    
    // Run reconciliation with overdraft file output
    assert(tsfi_hogan_overnight_reconciliation_ex(&overdraft_sys, "hogan_lfs.dat.bin", overdraft_path) == 0);
    assert(tsfi_hogan_overnight_reconciliation_ex(&overdraft_sys, "hogan_lfs.dat.bin", "hogan_overdraft.json") == -3); // Rule 13 check
    
    // Verify updated balances
    assert(overdraft_sys.accounts[0].balance == 0);    // Alice: 1000 + 2000 - 3000 = 0
    assert(overdraft_sys.accounts[1].balance == 3000); // Bob: 5000 - 2000 = 3000
    assert(overdraft_sys.accounts[2].balance == 3000); // Charlie: 0 + 3000 = 3000
    
    // Read sequential log file and verify entries
    uint8_t read_obuf[sizeof(hogan_overdraft_entry)];
    size_t osize = 0;
    assert(tsfi_hogan_read_seq_record(overdraft_path, 0, read_obuf, &osize) == 0);
    assert(osize == sizeof(hogan_overdraft_entry));
    
    const hogan_overdraft_entry *oentry = (const hogan_overdraft_entry *)read_obuf;
    assert(oentry->primary_account_id == 1001);
    assert(oentry->backup_account_id == 2002);
    assert(oentry->amount_transferred == 2000);
    assert(oentry->success == 1);
    
    remove(overdraft_path); // clean up
    printf("  [PASS] Overdraft linkage, automatic balance transfer, and logging verified.\n");

    // 17. Test Card Authorization Engine (Temporary Balance Holds)
    printf("[E2E] Testing Card Authorization Engine...\n");
    const char *card_path = "hogan_card.dat.bin";
    remove(card_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system card_sys;
    tsfi_hogan_init(&card_sys);
    assert(tsfi_hogan_register_account(&card_sys, 1001, 1000) == 0); // Alice: 1,000 units
    
    // Authorize card transaction of 400 (leaves 600 available, held: 400)
    assert(tsfi_hogan_authorize_card(&card_sys, card_path, 9901, 1001, 888, 400) == 0);
    assert(tsfi_hogan_authorize_card(&card_sys, "hogan_card.json", 9901, 1001, 888, 400) == -3); // Rule 13 check
    
    // Try to authorize transaction of 700 (should be declined, balance_held remains 400)
    assert(tsfi_hogan_authorize_card(&card_sys, card_path, 9902, 1001, 888, 700) == -1);
    
    // Verify holds
    assert(card_sys.accounts[0].balance == 1000);
    assert(card_sys.accounts[0].balance_held == 400);
    
    // Read sequential log file and verify entries
    uint8_t read_cbuf[sizeof(hogan_card_entry)];
    size_t csize = 0;
    
    // Check entry 0 (Approved hold of 400)
    assert(tsfi_hogan_read_seq_record(card_path, 0, read_cbuf, &csize) == 0);
    assert(csize == sizeof(hogan_card_entry));
    const hogan_card_entry *centry0 = (const hogan_card_entry *)read_cbuf;
    assert(centry0->card_id == 9901);
    assert(centry0->account_id == 1001);
    assert(centry0->merchant_id == 888);
    assert(centry0->amount == 400);
    assert(centry0->approved == 1);
    
    // Check entry 1 (Declined hold of 700)
    assert(tsfi_hogan_read_seq_record(card_path, 1, read_cbuf, &csize) == 0);
    assert(csize == sizeof(hogan_card_entry));
    const hogan_card_entry *centry1 = (const hogan_card_entry *)read_cbuf;
    assert(centry1->card_id == 9902);
    assert(centry1->account_id == 1001);
    assert(centry1->merchant_id == 888);
    assert(centry1->amount == 700);
    assert(centry1->approved == 0);
    
    remove(card_path); // clean up
    printf("  [PASS] Card balance holds, limits, and authorization logging verified.\n");

    // 18. Test Administrative Account Stop Manager (Status Overrides)
    printf("[E2E] Testing Administrative Account Stop Manager...\n");
    const char *stops_path = "hogan_stops.dat.bin";
    remove(stops_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system stop_sys;
    tsfi_hogan_init(&stop_sys);
    assert(tsfi_hogan_register_account(&stop_sys, 1001, 5000) == 0); // Alice: 5,000 units
    assert(tsfi_hogan_register_account(&stop_sys, 2002, 1000) == 0); // Bob: 1,000 units
    
    // Lock Alice's account with STATUS_STOP_DEBIT
    assert(tsfi_hogan_apply_account_stop(&stop_sys, stops_path, 1001, STATUS_STOP_DEBIT, 777) == 0);
    assert(tsfi_hogan_apply_account_stop(&stop_sys, "hogan_stops.json", 1001, STATUS_STOP_DEBIT, 777) == -3); // Rule 13 check
    
    // Alice attempts to transfer 1000 to Bob (should fail during reconciliation due to STOP_DEBIT status)
    assert(tsfi_hogan_dispatch_tx(&stop_sys, 1001, 2002, 1000, VM_EVM) == 0);
    
    // Bob attempts to transfer 500 to Alice (should succeed since incoming works on STOP_DEBIT)
    assert(tsfi_hogan_dispatch_tx(&stop_sys, 2002, 1001, 500, VM_EVM) == 0);
    
    // Run reconciliation
    assert(tsfi_hogan_overnight_reconciliation(&stop_sys, "hogan_lfs.dat.bin") == 0);
    
    // Verify updated balances:
    // Alice: 5000 (transfer of 1000 blocked) + 500 (incoming worked) = 5500
    // Bob: 1000 - 500 = 500
    assert(stop_sys.accounts[0].balance == 5500);
    assert(stop_sys.accounts[1].balance == 500);
    
    // Now lock Alice's account with STATUS_STOP_ALL
    assert(tsfi_hogan_apply_account_stop(&stop_sys, stops_path, 1001, STATUS_STOP_ALL, 777) == 0);
    
    // Bob attempts to transfer another 100 to Alice (should fail now since status is STOP_ALL)
    assert(tsfi_hogan_dispatch_tx(&stop_sys, 2002, 1001, 100, VM_EVM) == 0);
    
    assert(tsfi_hogan_overnight_reconciliation(&stop_sys, "hogan_lfs.dat.bin") == 0);
    
    // Verify balance remains unchanged (incoming 100 blocked)
    assert(stop_sys.accounts[0].balance == 5500);
    assert(stop_sys.accounts[1].balance == 500);
    
    // Read sequential log file and verify entries
    uint8_t read_sbuf[sizeof(hogan_stop_entry)];
    size_t ssize = 0;
    
    // Check entry 0
    assert(tsfi_hogan_read_seq_record(stops_path, 0, read_sbuf, &ssize) == 0);
    assert(ssize == sizeof(hogan_stop_entry));
    const hogan_stop_entry *sentry0 = (const hogan_stop_entry *)read_sbuf;
    assert(sentry0->account_id == 1001);
    assert(sentry0->previous_status == STATUS_ACTIVE);
    assert(sentry0->new_status == STATUS_STOP_DEBIT);
    assert(sentry0->authority_id == 777);
    
    remove(stops_path); // clean up
    printf("  [PASS] Administrative status overrides, locks, and logging verified.\n");

    // 19. Test Card Hold Release Engine (Balance Hold Settlements)
    printf("[E2E] Testing Card Hold Release Engine...\n");
    const char *release_path = "hogan_releases.dat.bin";
    remove(release_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system release_sys;
    tsfi_hogan_init(&release_sys);
    assert(tsfi_hogan_register_account(&release_sys, 1001, 1000) == 0); // Alice: 1,000 units
    
    // Authorize card hold of 400 (held: 400)
    assert(tsfi_hogan_authorize_card(&release_sys, "hogan_dummy_card.dat.bin", 9901, 1001, 888, 400) == 0);
    remove("hogan_dummy_card.dat.bin"); // clean up
    
    // Release hold of 150 (held becomes 250)
    assert(tsfi_hogan_release_hold(&release_sys, release_path, 9901, 1001, 150) == 0);
    assert(tsfi_hogan_release_hold(&release_sys, "hogan_releases.json", 9901, 1001, 150) == -3); // Rule 13 check
    
    assert(release_sys.accounts[0].balance_held == 250);
    
    // Release hold of 300 (exceeds held, clamps to 0)
    assert(tsfi_hogan_release_hold(&release_sys, release_path, 9901, 1001, 300) == 0);
    
    assert(release_sys.accounts[0].balance_held == 0);
    
    // Read sequential log file and verify entries
    uint8_t read_rbuf[sizeof(hogan_release_entry)];
    size_t rel_size = 0;
    
    // Check entry 0
    assert(tsfi_hogan_read_seq_record(release_path, 0, read_rbuf, &rel_size) == 0);
    assert(rel_size == sizeof(hogan_release_entry));
    const hogan_release_entry *rentry0 = (const hogan_release_entry *)read_rbuf;
    assert(rentry0->card_id == 9901);
    assert(rentry0->account_id == 1001);
    assert(rentry0->amount_released == 150);
    assert(rentry0->success == 1);
    
    remove(release_path); // clean up
    printf("  [PASS] Card balance hold settlements and release logs verified.\n");

    // 20. Test Daily Spending Limit Manager (Credit Limit Overrides)
    printf("[E2E] Testing Daily Spending Limit Manager...\n");
    const char *limits_path = "hogan_limits.dat.bin";
    remove(limits_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system limit_sys;
    tsfi_hogan_init(&limit_sys);
    assert(tsfi_hogan_register_account(&limit_sys, 1001, 10000) == 0); // Alice: 10,000 units
    
    // Set Alice's daily limit to 500
    assert(tsfi_hogan_update_daily_limit(&limit_sys, limits_path, 1001, 500, 777) == 0);
    assert(tsfi_hogan_update_daily_limit(&limit_sys, "hogan_limits.json", 1001, 500, 777) == -3); // Rule 13 check
    
    // Try to authorize card transaction of 400 (succeeds, spent: 400)
    assert(tsfi_hogan_authorize_card(&limit_sys, "hogan_dummy_card.dat.bin", 9901, 1001, 888, 400) == 0);
    
    // Try to authorize card transaction of 200 (should be declined, exceeds 500 daily spent limit)
    assert(tsfi_hogan_authorize_card(&limit_sys, "hogan_dummy_card.dat.bin", 9902, 1001, 888, 200) == -1);
    
    // Verify spending hold totals
    assert(limit_sys.accounts[0].daily_spent == 400);
    
    // Run overnight reconciliation (should reset daily_spent to 0)
    assert(tsfi_hogan_overnight_reconciliation(&limit_sys, "hogan_lfs.dat.bin") == 0);
    
    // Verify spent counter was reset to 0
    assert(limit_sys.accounts[0].daily_spent == 0);
    
    // Now authorize card transaction of 200 (should succeed now spent is reset)
    assert(tsfi_hogan_authorize_card(&limit_sys, "hogan_dummy_card.dat.bin", 9903, 1001, 888, 200) == 0);
    remove("hogan_dummy_card.dat.bin"); // clean up
    
    // Read sequential log file and verify entries
    uint8_t read_lbuf[sizeof(hogan_limit_entry)];
    size_t lim_size = 0;
    assert(tsfi_hogan_read_seq_record(limits_path, 0, read_lbuf, &lim_size) == 0);
    assert(lim_size == sizeof(hogan_limit_entry));
    
    const hogan_limit_entry *lentry = (const hogan_limit_entry *)read_lbuf;
    assert(lentry->account_id == 1001);
    assert(lentry->previous_limit == 0);
    assert(lentry->new_limit == 500);
    assert(lentry->authority_id == 777);
    
    remove(limits_path); // clean up
    printf("  [PASS] Daily spending limit overrides, validations, and resets verified.\n");

    // 21. Test Account Closure and Liquidation Engine (Status Termination)
    printf("[E2E] Testing Account Closure and Liquidation Engine...\n");
    const char *closure_path = "hogan_closures.dat.bin";
    remove(closure_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system closure_sys;
    tsfi_hogan_init(&closure_sys);
    assert(tsfi_hogan_register_account(&closure_sys, 1001, 1000) == 0); // Alice: 1,000 units
    
    // Place a hold of 100 (held: 100)
    assert(tsfi_hogan_authorize_card(&closure_sys, "hogan_dummy_card.dat.bin", 9901, 1001, 888, 100) == 0);
    remove("hogan_dummy_card.dat.bin"); // clean up
    
    // Try to close account (should fail with -2 due to active holds)
    assert(tsfi_hogan_close_account(&closure_sys, closure_path, 1001, 999) == -2);
    
    // Release card hold (held: 0)
    assert(tsfi_hogan_release_hold(&closure_sys, "hogan_dummy_rel.dat.bin", 9901, 1001, 100) == 0);
    remove("hogan_dummy_rel.dat.bin"); // clean up
    
    // Close account (should succeed and liquidate 1000 balance)
    assert(tsfi_hogan_close_account(&closure_sys, closure_path, 1001, 999) == 0);
    assert(tsfi_hogan_close_account(&closure_sys, "hogan_closures.json", 1001, 999) == -3); // Rule 13 check
    
    // Verify account is closed
    assert(closure_sys.accounts[0].active == 0);
    assert(closure_sys.accounts[0].balance == 0);
    
    // Read sequential log file and verify entries
    uint8_t read_clbuf[sizeof(hogan_closure_entry)];
    size_t cl_size = 0;
    assert(tsfi_hogan_read_seq_record(closure_path, 0, read_clbuf, &cl_size) == 0);
    assert(cl_size == sizeof(hogan_closure_entry));
    
    const hogan_closure_entry *clentry = (const hogan_closure_entry *)read_clbuf;
    assert(clentry->account_id == 1001);
    assert(clentry->liquidated_balance == 1000);
    assert(clentry->authority_id == 999);
    
    remove(closure_path); // clean up
    printf("  [PASS] Account liquidation, hold verification, and closure logs verified.\n");

    // 22. Test Daily Transfer Limit Manager (Outbound Transfer Limits)
    printf("[E2E] Testing Daily Transfer Limit Manager...\n");
    const char *tlimits_path = "hogan_tlimits.dat.bin";
    remove(tlimits_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system tlimit_sys;
    tsfi_hogan_init(&tlimit_sys);
    assert(tsfi_hogan_register_account(&tlimit_sys, 1001, 10000) == 0); // Alice: 10,000 units
    assert(tsfi_hogan_register_account(&tlimit_sys, 2002, 1000) == 0);  // Bob: 1,000 units
    
    // Set Alice's daily transfer limit to 2000
    assert(tsfi_hogan_update_transfer_limit(&tlimit_sys, tlimits_path, 1001, 2000, 777) == 0);
    assert(tsfi_hogan_update_transfer_limit(&tlimit_sys, "hogan_tlimits.json", 1001, 2000, 777) == -3); // Rule 13 check
    
    // Dispatch transfer of 1500 from Alice to Bob (succeeds)
    assert(tsfi_hogan_dispatch_tx(&tlimit_sys, 1001, 2002, 1500, VM_EVM) == 0);
    
    // Dispatch transfer of 1000 from Alice to Bob (declined, exceeds 2000 daily limit)
    assert(tsfi_hogan_dispatch_tx(&tlimit_sys, 1001, 2002, 1000, VM_EVM) == 0);
    
    // Run reconciliation
    assert(tsfi_hogan_overnight_reconciliation(&tlimit_sys, "hogan_lfs.dat.bin") == 0);
    
    // Verify balances (only 1500 transfer succeeded, 1000 transfer was blocked)
    assert(tlimit_sys.accounts[0].balance == 8500); // 10000 - 1500
    assert(tlimit_sys.accounts[1].balance == 2500); // 1000 + 1500
    
    // Verify transfer metric reset overnight
    assert(tlimit_sys.accounts[0].daily_transferred == 0);
    
    // Read sequential log file and verify entries
    uint8_t read_tlbuf[sizeof(hogan_trans_limit_entry)];
    size_t tlim_size = 0;
    assert(tsfi_hogan_read_seq_record(tlimits_path, 0, read_tlbuf, &tlim_size) == 0);
    assert(tlim_size == sizeof(hogan_trans_limit_entry));
    
    const hogan_trans_limit_entry *tlentry = (const hogan_trans_limit_entry *)read_tlbuf;
    assert(tlentry->account_id == 1001);
    assert(tlentry->previous_limit == 0);
    assert(tlentry->new_limit == 2000);
    assert(tlentry->authority_id == 777);
    
    remove(tlimits_path); // clean up
    printf("  [PASS] Daily transfer limits, validations, and overnight resets verified.\n");

    // 23. Test Standing Order Scheduler (Scheduled Recurring Transfers)
    printf("[E2E] Testing Standing Order Scheduler...\n");
    const char *standing_path = "hogan_standing.dat.bin";
    remove(standing_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system standing_sys;
    tsfi_hogan_init(&standing_sys);
    assert(tsfi_hogan_register_account(&standing_sys, 1001, 10000) == 0); // Alice: 10,000 units
    assert(tsfi_hogan_register_account(&standing_sys, 2002, 1000) == 0);  // Bob: 1,000 units
    
    // Register standing order: Alice transfers 2000 to Bob recurringly
    assert(tsfi_hogan_register_standing_order(standing_path, 1001, 2002, 2000) == 0);
    assert(tsfi_hogan_register_standing_order("hogan_standing.json", 1001, 2002, 2000) == -3); // Rule 13 check
    
    // Execute standing orders (dispatches pending transaction from Alice to Bob)
    assert(tsfi_hogan_execute_standing_orders(&standing_sys, standing_path) == 0);
    assert(tsfi_hogan_execute_standing_orders(&standing_sys, "hogan_standing.json") == -3); // Rule 13 check
    
    // Verify a transaction was dispatched into the system queue
    assert(standing_sys.tx_count == 1);
    assert(standing_sys.tx_log[0].sender_id == 1001);
    assert(standing_sys.tx_log[0].recipient_id == 2002);
    assert(standing_sys.tx_log[0].amount == 2000);
    
    // Run overnight reconciliation to apply the transfer
    assert(tsfi_hogan_overnight_reconciliation(&standing_sys, "hogan_lfs.dat.bin") == 0);
    
    // Verify updated balances
    assert(standing_sys.accounts[0].balance == 8000); // 10000 - 2000
    assert(standing_sys.accounts[1].balance == 3000); // 1000 + 2000
    
    // Read sequential log file and verify entries
    uint8_t read_stbuf[sizeof(hogan_standing_order)];
    size_t st_size = 0;
    assert(tsfi_hogan_read_seq_record(standing_path, 0, read_stbuf, &st_size) == 0);
    assert(st_size == sizeof(hogan_standing_order));
    
    const hogan_standing_order *stentry = (const hogan_standing_order *)read_stbuf;
    assert(stentry->sender_id == 1001);
    assert(stentry->recipient_id == 2002);
    assert(stentry->amount == 2000);
    
    remove(standing_path); // clean up
    printf("  [PASS] Standing order registration, execution dispatch, and logging verified.\n");

    // 24. Test Account Compliance Freeze Manager (Legal Holds)
    printf("[E2E] Testing Account Compliance Freeze Manager...\n");
    const char *freezes_path = "hogan_freezes.dat.bin";
    remove(freezes_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system freeze_sys;
    tsfi_hogan_init(&freeze_sys);
    assert(tsfi_hogan_register_account(&freeze_sys, 1001, 10000) == 0); // Alice: 10,000 units
    assert(tsfi_hogan_register_account(&freeze_sys, 2002, 1000) == 0);  // Bob: 1,000 units
    
    // Freeze Alice's account
    assert(tsfi_hogan_apply_account_freeze(&freeze_sys, freezes_path, 1001, 1, 999) == 0);
    assert(tsfi_hogan_apply_account_freeze(&freeze_sys, "hogan_freezes.json", 1001, 1, 999) == -3); // Rule 13 check
    
    // Alice attempts to transfer 1500 to Bob (should fail because Alice is frozen)
    assert(tsfi_hogan_dispatch_tx(&freeze_sys, 1001, 2002, 1500, VM_EVM) == 0);
    
    // Bob attempts to transfer 500 to Alice (should fail because Alice is frozen - blocks incoming credits too)
    assert(tsfi_hogan_dispatch_tx(&freeze_sys, 2002, 1001, 500, VM_EVM) == 0);
    
    // Run reconciliation
    assert(tsfi_hogan_overnight_reconciliation(&freeze_sys, "hogan_lfs.dat.bin") == 0);
    
    // Verify balances remain unchanged (both transfers blocked)
    assert(freeze_sys.accounts[0].balance == 10000);
    assert(freeze_sys.accounts[1].balance == 1000);
    
    // Unfreeze Alice's account
    assert(tsfi_hogan_apply_account_freeze(&freeze_sys, freezes_path, 1001, 0, 999) == 0);
    
    // Alice attempts to transfer 1500 to Bob again (should succeed)
    assert(tsfi_hogan_dispatch_tx(&freeze_sys, 1001, 2002, 1500, VM_EVM) == 0);
    
    assert(tsfi_hogan_overnight_reconciliation(&freeze_sys, "hogan_lfs.dat.bin") == 0);
    
    // Verify updated balances
    assert(freeze_sys.accounts[0].balance == 8500); // 10000 - 1500
    assert(freeze_sys.accounts[1].balance == 2500); // 1000 + 1500
    
    // Read sequential log file and verify entries
    uint8_t read_frbuf[sizeof(hogan_freeze_entry)];
    size_t fr_size = 0;
    
    // Check entry 0 (Frozen)
    assert(tsfi_hogan_read_seq_record(freezes_path, 0, read_frbuf, &fr_size) == 0);
    assert(fr_size == sizeof(hogan_freeze_entry));
    const hogan_freeze_entry *frentry0 = (const hogan_freeze_entry *)read_frbuf;
    assert(frentry0->account_id == 1001);
    assert(frentry0->is_frozen == 1);
    assert(frentry0->authority_id == 999);
    
    // Check entry 1 (Unfrozen)
    assert(tsfi_hogan_read_seq_record(freezes_path, 1, read_frbuf, &fr_size) == 0);
    assert(fr_size == sizeof(hogan_freeze_entry));
    const hogan_freeze_entry *frentry1 = (const hogan_freeze_entry *)read_frbuf;
    assert(frentry1->account_id == 1001);
    assert(frentry1->is_frozen == 0);
    assert(frentry1->authority_id == 999);
    
    remove(freezes_path); // clean up
    printf("  [PASS] Compliance freezes, legal holds, and logging verified.\n");

    // 25. Test Card Status Compliance Manager (Card Locks)
    printf("[E2E] Testing Card Status Compliance Manager...\n");
    const char *cardlocks_path = "hogan_cardlocks.dat.bin";
    remove(cardlocks_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system card_comp_sys;
    tsfi_hogan_init(&card_comp_sys);
    assert(tsfi_hogan_register_account(&card_comp_sys, 1001, 10000) == 0); // Alice: 10,000 units
    
    // Block card 9901
    assert(tsfi_hogan_apply_card_status(&card_comp_sys, cardlocks_path, 9901, CARD_STATUS_BLOCKED, 888) == 0);
    assert(tsfi_hogan_apply_card_status(&card_comp_sys, "hogan_cardlocks.json", 9901, CARD_STATUS_BLOCKED, 888) == -3); // Rule 13 check
    
    // Attempt card authorization on blocked card 9901 (should fail with -4)
    assert(tsfi_hogan_authorize_card(&card_comp_sys, "hogan_dummy_auth.dat.bin", 9901, 1001, 777, 1000) == -4);
    
    // Unblock card 9901
    assert(tsfi_hogan_apply_card_status(&card_comp_sys, cardlocks_path, 9901, CARD_STATUS_ACTIVE, 888) == 0);
    
    // Attempt card authorization on active card 9901 (should now succeed)
    assert(tsfi_hogan_authorize_card(&card_comp_sys, "hogan_dummy_auth.dat.bin", 9901, 1001, 777, 1000) == 0);
    remove("hogan_dummy_auth.dat.bin"); // clean up
    
    // Read sequential log file and verify entries
    uint8_t read_cardlbuf[sizeof(hogan_card_status_entry)];
    size_t cardl_size = 0;
    
    // Check entry 0 (Blocked)
    assert(tsfi_hogan_read_seq_record(cardlocks_path, 0, read_cardlbuf, &cardl_size) == 0);
    assert(cardl_size == sizeof(hogan_card_status_entry));
    const hogan_card_status_entry *clentry0 = (const hogan_card_status_entry *)read_cardlbuf;
    assert(clentry0->card_id == 9901);
    assert(clentry0->previous_status == CARD_STATUS_ACTIVE);
    assert(clentry0->new_status == CARD_STATUS_BLOCKED);
    assert(clentry0->authority_id == 888);
    
    // Check entry 1 (Activated)
    assert(tsfi_hogan_read_seq_record(cardlocks_path, 1, read_cardlbuf, &cardl_size) == 0);
    assert(cardl_size == sizeof(hogan_card_status_entry));
    const hogan_card_status_entry *clentry1 = (const hogan_card_status_entry *)read_cardlbuf;
    assert(clentry1->card_id == 9901);
    assert(clentry1->previous_status == CARD_STATUS_BLOCKED);
    assert(clentry1->new_status == CARD_STATUS_ACTIVE);
    assert(clentry1->authority_id == 888);
    
    remove(cardlocks_path); // clean up
    printf("  [PASS] Card status compliance blocks and logging verified.\n");

    // 26. Test Batch Fee Exemption Manager (Fee Waivers)
    printf("[E2E] Testing Batch Fee Exemption Manager...\n");
    const char *exempts_path = "hogan_exempts.dat.bin";
    remove(exempts_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system exempt_sys;
    tsfi_hogan_init(&exempt_sys);
    assert(tsfi_hogan_register_account(&exempt_sys, 1001, 1000) == 0); // Alice: 1,000 units
    assert(tsfi_hogan_register_account(&exempt_sys, 2002, 1000) == 0); // Bob: 1,000 units
    
    // Exempt Alice from fee charging
    assert(tsfi_hogan_apply_fee_exemption(&exempt_sys, exempts_path, 1001, 1, 999) == 0);
    assert(tsfi_hogan_apply_fee_exemption(&exempt_sys, "hogan_exempts.json", 1001, 1, 999) == -3); // Rule 13 check
    
    // Apply batch maintenance fee of 150 flat
    assert(tsfi_hogan_apply_fees(&exempt_sys, "hogan_dummy_fees.dat.bin", 150) == 0);
    remove("hogan_dummy_fees.dat.bin"); // clean up
    
    // Verify balances (Alice balance remains 1000, Bob balance is 850)
    assert(exempt_sys.accounts[0].balance == 1000);
    assert(exempt_sys.accounts[1].balance == 850);
    
    // Read sequential log file and verify entries
    uint8_t read_exbuf[sizeof(hogan_exemption_entry)];
    size_t ex_size = 0;
    assert(tsfi_hogan_read_seq_record(exempts_path, 0, read_exbuf, &ex_size) == 0);
    assert(ex_size == sizeof(hogan_exemption_entry));
    
    const hogan_exemption_entry *exentry = (const hogan_exemption_entry *)read_exbuf;
    assert(exentry->account_id == 1001);
    assert(exentry->fee_exempt == 1);
    assert(exentry->authority_id == 999);
    
    remove(exempts_path); // clean up
    printf("  [PASS] Fee waivers, waivers bypassing charging, and logs verified.\n");

    // 27. Test Interest Rate Override Manager (Custom Rates)
    printf("[E2E] Testing Interest Rate Override Manager...\n");
    const char *overrides_path = "hogan_int_overrides.dat.bin";
    remove(overrides_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system override_sys;
    tsfi_hogan_init(&override_sys);
    assert(tsfi_hogan_register_account(&override_sys, 1001, 10000) == 0); // Alice: 10,000 units
    assert(tsfi_hogan_register_account(&override_sys, 2002, 10000) == 0); // Bob: 10,000 units
    
    // Set custom interest rate on Alice to 500 bps (5%)
    assert(tsfi_hogan_apply_interest_override(&override_sys, overrides_path, 1001, 500, 777) == 0);
    assert(tsfi_hogan_apply_interest_override(&override_sys, "hogan_int_overrides.json", 1001, 500, 777) == -3); // Rule 13 check
    
    // Apply batch interest rate of 200 bps (2%)
    assert(tsfi_hogan_apply_interest(&override_sys, "hogan_dummy_interest.dat.bin", 200) == 0);
    remove("hogan_dummy_interest.dat.bin"); // clean up
    
    // Verify balances (Alice balance gains 5% = 500 units, Bob balance gains 2% = 200 units)
    assert(override_sys.accounts[0].balance == 10500);
    assert(override_sys.accounts[1].balance == 10200);
    
    // Read sequential log file and verify entries
    uint8_t read_ovbuf[sizeof(hogan_int_override_entry)];
    size_t ov_size = 0;
    assert(tsfi_hogan_read_seq_record(overrides_path, 0, read_ovbuf, &ov_size) == 0);
    assert(ov_size == sizeof(hogan_int_override_entry));
    
    const hogan_int_override_entry *oventry = (const hogan_int_override_entry *)read_ovbuf;
    assert(oventry->account_id == 1001);
    assert(oventry->previous_rate_bps == 0);
    assert(oventry->new_rate_bps == 500);
    assert(oventry->authority_id == 777);
    
    remove(overrides_path); // clean up
    printf("  [PASS] Custom interest rate overrides, custom rate postings, and override logs verified.\n");

    // 28. Test Minimum Balance Compliance Manager (Threshold Enforcement)
    printf("[E2E] Testing Minimum Balance Compliance Manager...\n");
    const char *minb_path = "hogan_minb.dat.bin";
    remove(minb_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system minb_sys;
    tsfi_hogan_init(&minb_sys);
    assert(tsfi_hogan_register_account(&minb_sys, 1001, 1000) == 0); // Alice: 1,000 units
    assert(tsfi_hogan_register_account(&minb_sys, 2002, 1000) == 0); // Bob: 1,000 units
    
    // Set Alice's minimum balance threshold to 300 units
    assert(tsfi_hogan_update_min_balance(&minb_sys, minb_path, 1001, 300, 888) == 0);
    assert(tsfi_hogan_update_min_balance(&minb_sys, "hogan_minb.json", 1001, 300, 888) == -3); // Rule 13 check
    
    // Alice transfers 600 to Bob (succeeds, remaining balance 400 >= 300)
    assert(tsfi_hogan_dispatch_tx(&minb_sys, 1001, 2002, 600, VM_EVM) == 0);
    
    // Alice transfers 200 to Bob (should fail because remaining balance would be 200 < 300)
    assert(tsfi_hogan_dispatch_tx(&minb_sys, 1001, 2002, 200, VM_EVM) == 0);
    
    // Run reconciliation
    assert(tsfi_hogan_overnight_reconciliation(&minb_sys, "hogan_lfs.dat.bin") == 0);
    
    // Verify balances (only 600 transfer processed, 200 transfer blocked)
    assert(minb_sys.accounts[0].balance == 400);
    assert(minb_sys.accounts[1].balance == 1600);
    
    // Read sequential log file and verify entries
    uint8_t read_mbbuf[sizeof(hogan_min_balance_entry)];
    size_t mb_size = 0;
    assert(tsfi_hogan_read_seq_record(minb_path, 0, read_mbbuf, &mb_size) == 0);
    assert(mb_size == sizeof(hogan_min_balance_entry));
    
    const hogan_min_balance_entry *mbentry = (const hogan_min_balance_entry *)read_mbbuf;
    assert(mbentry->account_id == 1001);
    assert(mbentry->previous_min_balance == 0);
    assert(mbentry->new_min_balance == 300);
    assert(mbentry->authority_id == 888);
    
    remove(minb_path); // clean up
    printf("  [PASS] Minimum balance compliance, validations, and override logs verified.\n");

    // 29. Test Daily Transaction Count Manager (Velocity Capping)
    printf("[E2E] Testing Daily Transaction Count Manager...\n");
    const char *txcounts_path = "hogan_txcounts.dat.bin";
    remove(txcounts_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system velocity_sys;
    tsfi_hogan_init(&velocity_sys);
    assert(tsfi_hogan_register_account(&velocity_sys, 1001, 10000) == 0); // Alice: 10,000 units
    assert(tsfi_hogan_register_account(&velocity_sys, 2002, 1000) == 0);  // Bob: 1,000 units
    
    // Set Alice's daily tx limit to 1
    assert(tsfi_hogan_update_tx_count_limit(&velocity_sys, txcounts_path, 1001, 1, 999) == 0);
    assert(tsfi_hogan_update_tx_count_limit(&velocity_sys, "hogan_txcounts.json", 1001, 1, 999) == -3); // Rule 13 check
    
    // Alice transfers 500 to Bob (succeeds, daily_tx_count becomes 1)
    assert(tsfi_hogan_dispatch_tx(&velocity_sys, 1001, 2002, 500, VM_EVM) == 0);
    
    // Alice transfers 500 to Bob again (declined, exceeds 1 daily tx limit)
    assert(tsfi_hogan_dispatch_tx(&velocity_sys, 1001, 2002, 500, VM_EVM) == 0);
    
    // Run reconciliation
    assert(tsfi_hogan_overnight_reconciliation(&velocity_sys, "hogan_lfs.dat.bin") == 0);
    
    // Verify balances (only 500 transfer processed, second one blocked)
    assert(velocity_sys.accounts[0].balance == 9500);
    assert(velocity_sys.accounts[1].balance == 1500);
    
    // Verify daily tx count resets overnight
    assert(velocity_sys.accounts[0].daily_tx_count == 0);
    
    // Read sequential log file and verify entries
    uint8_t read_vbuf[sizeof(hogan_tx_count_entry)];
    size_t v_size = 0;
    assert(tsfi_hogan_read_seq_record(txcounts_path, 0, read_vbuf, &v_size) == 0);
    assert(v_size == sizeof(hogan_tx_count_entry));
    
    const hogan_tx_count_entry *ventry = (const hogan_tx_count_entry *)read_vbuf;
    assert(ventry->account_id == 1001);
    assert(ventry->previous_tx_limit == 0);
    assert(ventry->new_tx_limit == 1);
    assert(ventry->authority_id == 999);
    
    remove(txcounts_path); // clean up
    printf("  [PASS] Daily transaction count velocity caps, overnight resets, and logs verified.\n");

    // 30. Test Account Priority Routing Manager (Priority Queuing)
    printf("[E2E] Testing Account Priority Routing Manager...\n");
    const char *priority_path = "hogan_priority.dat.bin";
    remove(priority_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system prio_sys;
    tsfi_hogan_init(&prio_sys);
    assert(tsfi_hogan_register_account(&prio_sys, 1001, 1000) == 0); // Alice (Standard): 1,000 units
    assert(tsfi_hogan_register_account(&prio_sys, 2002, 1000) == 0); // Bob (High Priority): 1,000 units
    assert(tsfi_hogan_register_account(&prio_sys, 3003, 100) == 0);  // Charlie: 100 units
    
    // Elevate Bob's priority to 1 (High Priority / VIP)
    assert(tsfi_hogan_update_priority_tier(&prio_sys, priority_path, 2002, 1, 777) == 0);
    assert(tsfi_hogan_update_priority_tier(&prio_sys, "hogan_priority.json", 2002, 1, 777) == -3); // Rule 13 check
    
    // Alice (Standard) dispatches transfer of 100 from Charlie first
    assert(tsfi_hogan_dispatch_tx(&prio_sys, 3003, 1001, 100, VM_EVM) == 0);
    
    // Bob (High Priority) dispatches transfer of 100 from Charlie second
    assert(tsfi_hogan_dispatch_tx(&prio_sys, 3003, 2002, 100, VM_EVM) == 0);
    
    // Run overnight reconciliation.
    // Because Charlie has only 100 balance, the high priority transfer (Bob's) should execute first,
    // causing Alice's standard priority transfer to fail due to insufficient funds.
    assert(tsfi_hogan_overnight_reconciliation(&prio_sys, "hogan_lfs.dat.bin") == 0);
    
    // Verify balances (Bob succeeded, Alice remained unchanged)
    assert(prio_sys.accounts[0].balance == 1000); // Alice: unchanged
    assert(prio_sys.accounts[1].balance == 1100); // Bob: 1000 + 100
    assert(prio_sys.accounts[2].balance == 0);    // Charlie: 0
    
    // Read sequential log file and verify entries
    uint8_t read_pbuf[sizeof(hogan_priority_entry)];
    size_t p_size = 0;
    assert(tsfi_hogan_read_seq_record(priority_path, 0, read_pbuf, &p_size) == 0);
    assert(p_size == sizeof(hogan_priority_entry));
    
    const hogan_priority_entry *pentry = (const hogan_priority_entry *)read_pbuf;
    assert(pentry->account_id == 2002);
    assert(pentry->previous_priority == 0);
    assert(pentry->new_priority == 1);
    assert(pentry->authority_id == 777);
    
    remove(priority_path); // clean up
    printf("  [PASS] Priority queuing, double-pass execution, and override logs verified.\n");

    // 31. Test Maximum Balance Compliance Manager (Deposit Caps)
    printf("[E2E] Testing Maximum Balance Compliance Manager...\n");
    const char *maxb_path = "hogan_maxb.dat.bin";
    remove(maxb_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system maxb_sys;
    tsfi_hogan_init(&maxb_sys);
    assert(tsfi_hogan_register_account(&maxb_sys, 1001, 1000) == 0); // Alice: 1,000 units
    assert(tsfi_hogan_register_account(&maxb_sys, 2002, 1000) == 0); // Bob: 1,000 units
    
    // Set Bob's maximum balance threshold to 1500 units
    assert(tsfi_hogan_update_max_balance(&maxb_sys, maxb_path, 2002, 1500, 888) == 0);
    assert(tsfi_hogan_update_max_balance(&maxb_sys, "hogan_maxb.json", 2002, 1500, 888) == -3); // Rule 13 check
    
    // Alice transfers 400 to Bob (succeeds, Bob balance becomes 1400 <= 1500)
    assert(tsfi_hogan_dispatch_tx(&maxb_sys, 1001, 2002, 400, VM_EVM) == 0);
    
    // Alice transfers 200 to Bob (should fail because Bob balance would be 1600 > 1500)
    assert(tsfi_hogan_dispatch_tx(&maxb_sys, 1001, 2002, 200, VM_EVM) == 0);
    
    // Run reconciliation
    assert(tsfi_hogan_overnight_reconciliation(&maxb_sys, "hogan_lfs.dat.bin") == 0);
    
    // Verify balances (only 400 transfer processed, 200 transfer blocked)
    assert(maxb_sys.accounts[0].balance == 600);
    assert(maxb_sys.accounts[1].balance == 1400);
    
    // Read sequential log file and verify entries
    uint8_t read_maxbuf[sizeof(hogan_max_balance_entry)];
    size_t max_size = 0;
    assert(tsfi_hogan_read_seq_record(maxb_path, 0, read_maxbuf, &max_size) == 0);
    assert(max_size == sizeof(hogan_max_balance_entry));
    
    const hogan_max_balance_entry *maxentry = (const hogan_max_balance_entry *)read_maxbuf;
    assert(maxentry->account_id == 2002);
    assert(maxentry->previous_max_balance == 0);
    assert(maxentry->new_max_balance == 1500);
    assert(maxentry->authority_id == 888);
    
    remove(maxb_path); // clean up
    printf("  [PASS] Deposit cap compliance, validations, and override logs verified.\n");

    // 32. Test Daily Deposit Limit Manager (Inbound Velocity Capping)
    printf("[E2E] Testing Daily Deposit Limit Manager...\n");
    const char *depb_path = "hogan_depb.dat.bin";
    remove(depb_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system depb_sys;
    tsfi_hogan_init(&depb_sys);
    assert(tsfi_hogan_register_account(&depb_sys, 1001, 10000) == 0); // Alice: 10,000 units
    assert(tsfi_hogan_register_account(&depb_sys, 2002, 1000) == 0);  // Bob: 1,000 units
    
    // Set Bob's daily deposit limit to 500 units
    assert(tsfi_hogan_update_deposit_limit(&depb_sys, depb_path, 2002, 500, 777) == 0);
    assert(tsfi_hogan_update_deposit_limit(&depb_sys, "hogan_depb.json", 2002, 500, 777) == -3); // Rule 13 check
    
    // Alice transfers 400 to Bob (succeeds, Bob daily_deposited becomes 400)
    assert(tsfi_hogan_dispatch_tx(&depb_sys, 1001, 2002, 400, VM_EVM) == 0);
    
    // Alice transfers 200 to Bob (should fail because Bob daily_deposited would be 600 > 500)
    assert(tsfi_hogan_dispatch_tx(&depb_sys, 1001, 2002, 200, VM_EVM) == 0);
    
    // Run reconciliation
    assert(tsfi_hogan_overnight_reconciliation(&depb_sys, "hogan_lfs.dat.bin") == 0);
    
    // Verify balances (only 400 transfer processed, 200 transfer blocked)
    assert(depb_sys.accounts[0].balance == 9600);
    assert(depb_sys.accounts[1].balance == 1400);
    
    // Verify daily_deposited resets overnight
    assert(depb_sys.accounts[1].daily_deposited == 0);
    
    // Read sequential log file and verify entries
    uint8_t read_depbuf[sizeof(hogan_deposit_limit_entry)];
    size_t dep_size = 0;
    assert(tsfi_hogan_read_seq_record(depb_path, 0, read_depbuf, &dep_size) == 0);
    assert(dep_size == sizeof(hogan_deposit_limit_entry));
    
    const hogan_deposit_limit_entry *depentry = (const hogan_deposit_limit_entry *)read_depbuf;
    assert(depentry->account_id == 2002);
    assert(depentry->previous_deposit_limit == 0);
    assert(depentry->new_deposit_limit == 500);
    assert(depentry->authority_id == 777);
    
    remove(depb_path); // clean up
    printf("  [PASS] Daily deposit limit velocity caps, resets, and override logs verified.\n");

    // 33. Test Overdraft Limit Compliance Manager (Credit Line Capping)
    printf("[E2E] Testing Overdraft Limit Compliance Manager...\n");
    const char *ovlimit_path = "hogan_ovlimits.dat.bin";
    remove(ovlimit_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system ov_sys;
    tsfi_hogan_init(&ov_sys);
    assert(tsfi_hogan_register_account(&ov_sys, 1001, 1000) == 0); // Alice primary: 1,000 units
    assert(tsfi_hogan_register_account(&ov_sys, 2002, 10000) == 0); // Bob backup: 10,000 units
    
    // Link Bob as Alice's backup
    assert(tsfi_hogan_link_backup(&ov_sys, 1001, 2002) == 0);
    
    // Set Alice's overdraft credit line limit to 500 units
    assert(tsfi_hogan_update_overdraft_limit(&ov_sys, ovlimit_path, 1001, 500, 999) == 0);
    assert(tsfi_hogan_update_overdraft_limit(&ov_sys, "hogan_ovlimits.json", 1001, 500, 999) == -3); // Rule 13 check
    
    // Alice transfers 1400 to Bob (deficit is 400 <= 500 limit; succeeds)
    assert(tsfi_hogan_dispatch_tx(&ov_sys, 1001, 2002, 1400, VM_EVM) == 0);
    
    // Alice transfers another 200 to Bob (deficit 200 exceeds remaining 100 limit space; fails)
    assert(tsfi_hogan_dispatch_tx(&ov_sys, 1001, 2002, 200, VM_EVM) == 0);
    
    // Run reconciliation
    assert(tsfi_hogan_overnight_reconciliation(&ov_sys, "hogan_lfs.dat.bin") == 0);
    
    // Verify balances (Alice balance 0, Bob balance 10000 - 400 + 1400 = 11000)
    assert(ov_sys.accounts[0].balance == 0);
    assert(ov_sys.accounts[1].balance == 11000);
    
    // Verify overdraft drawn metric on Alice is 400
    assert(ov_sys.accounts[0].overdraft_drawn == 400);
    
    // Read sequential log file and verify entries
    uint8_t read_ovlbuf[sizeof(hogan_overdraft_limit_entry)];
    size_t ovl_size = 0;
    assert(tsfi_hogan_read_seq_record(ovlimit_path, 0, read_ovlbuf, &ovl_size) == 0);
    assert(ovl_size == sizeof(hogan_overdraft_limit_entry));
    
    const hogan_overdraft_limit_entry *ovlentry = (const hogan_overdraft_limit_entry *)read_ovlbuf;
    assert(ovlentry->account_id == 1001);
    assert(ovlentry->previous_overdraft_limit == 0);
    assert(ovlentry->new_overdraft_limit == 500);
    assert(ovlentry->authority_id == 999);
    
    remove(ovlimit_path); // clean up
    printf("  [PASS] Overdraft credit line capping, validations, and override logs verified.\n");

    // 34. Test Card Daily Spend Manager (Spend Velocity Capping)
    printf("[E2E] Testing Card Daily Spend Manager...\n");
    const char *cardsp_path = "hogan_cardsp.dat.bin";
    remove(cardsp_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system cardsp_sys;
    tsfi_hogan_init(&cardsp_sys);
    assert(tsfi_hogan_register_account(&cardsp_sys, 1001, 10000) == 0); // Alice: 10,000 units
    
    // Set Alice's daily card spend limit to 500 units
    assert(tsfi_hogan_update_card_spend_limit(&cardsp_sys, cardsp_path, 1001, 500, 888) == 0);
    assert(tsfi_hogan_update_card_spend_limit(&cardsp_sys, "hogan_cardsp.json", 1001, 500, 888) == -3); // Rule 13 check
    
    // Authorize card spending of 400 (succeeds, Alice card_spent_today becomes 400)
    assert(tsfi_hogan_authorize_card(&cardsp_sys, "hogan_tx.dat.bin", 5005, 1001, 9009, 400) == 0);
    
    // Authorize card spending of 200 (should fail because Alice daily card spent would be 600 > 500)
    assert(tsfi_hogan_authorize_card(&cardsp_sys, "hogan_tx.dat.bin", 5005, 1001, 9009, 200) == -1);
    
    // Run reconciliation
    assert(tsfi_hogan_overnight_reconciliation(&cardsp_sys, "hogan_lfs.dat.bin") == 0);
    
    // Verify daily card spent resets overnight
    assert(cardsp_sys.accounts[0].card_spent_today == 0);
    
    // Read sequential log file and verify entries
    uint8_t read_cspbuf[sizeof(hogan_card_spend_limit_entry)];
    size_t csp_size = 0;
    assert(tsfi_hogan_read_seq_record(cardsp_path, 0, read_cspbuf, &csp_size) == 0);
    assert(csp_size == sizeof(hogan_card_spend_limit_entry));
    
    const hogan_card_spend_limit_entry *cspentry = (const hogan_card_spend_limit_entry *)read_cspbuf;
    assert(cspentry->account_id == 1001);
    assert(cspentry->previous_spend_limit == 0);
    assert(cspentry->new_spend_limit == 500);
    assert(cspentry->authority_id == 888);
    
    remove(cardsp_path); // clean up
    remove("hogan_tx.dat.bin"); // clean up
    printf("  [PASS] Card daily spend limit velocity caps, resets, and override logs verified.\n");

    // 35. Test Merchant ID Block Manager (Merchant Restrictions)
    printf("[E2E] Testing Merchant ID Block Manager...\n");
    const char *merc_path = "hogan_merchant.dat.bin";
    remove(merc_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system merc_sys;
    tsfi_hogan_init(&merc_sys);
    assert(tsfi_hogan_register_account(&merc_sys, 1001, 10000) == 0); // Alice: 10,000 units
    
    // Block merchant ID 9009 on Alice's account
    assert(tsfi_hogan_update_merchant_block(&merc_sys, merc_path, 1001, 9009, 1, 999) == 0);
    assert(tsfi_hogan_update_merchant_block(&merc_sys, "hogan_merchant.json", 1001, 9009, 1, 999) == -3); // Rule 13 check
    
    // Authorize card spending of 400 to merchant 8008 (succeeds)
    assert(tsfi_hogan_authorize_card(&merc_sys, "hogan_tx.dat.bin", 5005, 1001, 8008, 400) == 0);
    
    // Authorize card spending of 200 to merchant 9009 (fails, blocked merchant returns -5)
    assert(tsfi_hogan_authorize_card(&merc_sys, "hogan_tx.dat.bin", 5005, 1001, 9009, 200) == -5);
    
    // Read sequential log file and verify entries
    uint8_t read_mblocks[sizeof(hogan_merchant_block_entry)];
    size_t m_size = 0;
    assert(tsfi_hogan_read_seq_record(merc_path, 0, read_mblocks, &m_size) == 0);
    assert(m_size == sizeof(hogan_merchant_block_entry));
    
    const hogan_merchant_block_entry *mentry = (const hogan_merchant_block_entry *)read_mblocks;
    assert(mentry->account_id == 1001);
    assert(mentry->merchant_id == 9009);
    assert(mentry->is_blocked == 1);
    assert(mentry->authority_id == 999);
    
    remove(merc_path); // clean up
    remove("hogan_tx.dat.bin"); // clean up
    printf("  [PASS] Card merchant restrictions, validations, and override logs verified.\n");

    // 36. Test Card Transaction Velocity Manager (Authorization Capping)
    printf("[E2E] Testing Card Transaction Velocity Manager...\n");
    const char *cardvel_path = "hogan_cardvel.dat.bin";
    remove(cardvel_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system cardvel_sys;
    tsfi_hogan_init(&cardvel_sys);
    assert(tsfi_hogan_register_account(&cardvel_sys, 1001, 10000) == 0); // Alice: 10,000 units
    
    // Set Alice's daily card transaction count limit to 1 transaction
    assert(tsfi_hogan_update_card_tx_limit(&cardvel_sys, cardvel_path, 1001, 1, 777) == 0);
    assert(tsfi_hogan_update_card_tx_limit(&cardvel_sys, "hogan_cardvel.json", 1001, 1, 777) == -3); // Rule 13 check
    
    // First card authorization (succeeds)
    assert(tsfi_hogan_authorize_card(&cardvel_sys, "hogan_tx.dat.bin", 5005, 1001, 9009, 100) == 0);
    
    // Second card authorization (should fail because Alice card tx count would be 2 > 1 limit; returns -6)
    assert(tsfi_hogan_authorize_card(&cardvel_sys, "hogan_tx.dat.bin", 5005, 1001, 9009, 100) == -6);
    
    // Run reconciliation
    assert(tsfi_hogan_overnight_reconciliation(&cardvel_sys, "hogan_lfs.dat.bin") == 0);
    
    // Verify daily card tx count resets overnight
    assert(cardvel_sys.accounts[0].card_tx_count_today == 0);
    
    // Read sequential log file and verify entries
    uint8_t read_cvlbuf[sizeof(hogan_card_tx_limit_entry)];
    size_t cvl_size = 0;
    assert(tsfi_hogan_read_seq_record(cardvel_path, 0, read_cvlbuf, &cvl_size) == 0);
    assert(cvl_size == sizeof(hogan_card_tx_limit_entry));
    
    const hogan_card_tx_limit_entry *cvlentry = (const hogan_card_tx_limit_entry *)read_cvlbuf;
    assert(cvlentry->account_id == 1001);
    assert(cvlentry->previous_tx_limit == 0);
    assert(cvlentry->new_tx_limit == 1);
    assert(cvlentry->authority_id == 777);
    
    remove(cardvel_path); // clean up
    remove("hogan_tx.dat.bin"); // clean up
    printf("  [PASS] Card daily transaction velocity caps, resets, and override logs verified.\n");

    // 37. Test Card Expiry Compliance Manager (Expiration Validation)
    printf("[E2E] Testing Card Expiry Compliance Manager...\n");
    const char *expiry_path = "hogan_expiry.dat.bin";
    remove(expiry_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system exp_sys;
    tsfi_hogan_init(&exp_sys);
    assert(tsfi_hogan_register_account(&exp_sys, 1001, 10000) == 0); // Alice: 10,000 units
    
    // Set Alice's card expiry to epoch 1
    assert(tsfi_hogan_update_card_expiry(&exp_sys, expiry_path, 1001, 1, 999) == 0);
    assert(tsfi_hogan_update_card_expiry(&exp_sys, "hogan_expiry.json", 1001, 1, 999) == -3); // Rule 13 check
    
    // First card authorization (succeeds because current_epoch is 1 <= expiry 1)
    assert(tsfi_hogan_authorize_card(&exp_sys, "hogan_tx.dat.bin", 5005, 1001, 9009, 100) == 0);
    
    // Advance epoch to 2 by running overnight reconciliation
    assert(tsfi_hogan_overnight_reconciliation(&exp_sys, "hogan_lfs.dat.bin") == 0);
    assert(exp_sys.current_epoch == 2);
    
    // Second card authorization (should fail because current_epoch 2 > expiry 1; returns -7)
    assert(tsfi_hogan_authorize_card(&exp_sys, "hogan_tx.dat.bin", 5005, 1001, 9009, 100) == -7);
    
    // Read sequential log file and verify entries
    uint8_t read_expbuf[sizeof(hogan_card_expiry_entry)];
    size_t exp_size = 0;
    assert(tsfi_hogan_read_seq_record(expiry_path, 0, read_expbuf, &exp_size) == 0);
    assert(exp_size == sizeof(hogan_card_expiry_entry));
    
    const hogan_card_expiry_entry *expentry = (const hogan_card_expiry_entry *)read_expbuf;
    assert(expentry->account_id == 1001);
    assert(expentry->previous_expiry_epoch == 0);
    assert(expentry->new_expiry_epoch == 1);
    assert(expentry->authority_id == 999);
    
    remove(expiry_path); // clean up
    remove("hogan_tx.dat.bin"); // clean up
    printf("  [PASS] Card expiry validations, epoch advances, and override logs verified.\n");

    // 38. Test Account Grace Period Interest Rate Manager (Grace Period Restriction)
    printf("[E2E] Testing Account Grace Period Interest Rate Manager...\n");
    const char *gp_path = "hogan_grace.dat.bin";
    remove(gp_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system gp_sys;
    tsfi_hogan_init(&gp_sys);
    assert(tsfi_hogan_register_account(&gp_sys, 1001, 10000) == 0); // Alice: 10,000 units
    
    // Set Alice's grace period to epoch 2
    assert(tsfi_hogan_update_grace_period(&gp_sys, gp_path, 1001, 2, 888) == 0);
    assert(tsfi_hogan_update_grace_period(&gp_sys, "hogan_grace.json", 1001, 2, 888) == -3); // Rule 13 check
    
    // Apply batch interest rate of 10% (1000 bps). At current_epoch 1, Alice is in grace period (1 <= 2), so interest added should be 0.
    assert(tsfi_hogan_apply_interest(&gp_sys, "hogan_interest.dat.bin", 1000) == 0);
    assert(gp_sys.accounts[0].balance == 10000);
    remove("hogan_interest.dat.bin");
    
    // Advance epoch to 2. Alice is still in grace period (2 <= 2), so interest should still be 0.
    assert(tsfi_hogan_overnight_reconciliation(&gp_sys, "hogan_lfs.dat.bin") == 0);
    assert(gp_sys.current_epoch == 2);
    assert(tsfi_hogan_apply_interest(&gp_sys, "hogan_interest.dat.bin", 1000) == 0);
    assert(gp_sys.accounts[0].balance == 10000);
    remove("hogan_interest.dat.bin");
    
    // Advance epoch to 3. Alice is now past grace period (3 > 2), so 10% interest should be applied (10,000 * 10% = 1,000 added).
    assert(tsfi_hogan_overnight_reconciliation(&gp_sys, "hogan_lfs.dat.bin") == 0);
    assert(gp_sys.current_epoch == 3);
    assert(tsfi_hogan_apply_interest(&gp_sys, "hogan_interest.dat.bin", 1000) == 0);
    assert(gp_sys.accounts[0].balance == 11000);
    remove("hogan_interest.dat.bin");
    
    // Read sequential log file and verify entries
    uint8_t read_gpbuf[sizeof(hogan_grace_period_entry)];
    size_t gp_size = 0;
    assert(tsfi_hogan_read_seq_record(gp_path, 0, read_gpbuf, &gp_size) == 0);
    assert(gp_size == sizeof(hogan_grace_period_entry));
    
    const hogan_grace_period_entry *gpentry = (const hogan_grace_period_entry *)read_gpbuf;
    assert(gpentry->account_id == 1001);
    assert(gpentry->previous_grace_period == 0);
    assert(gpentry->new_grace_period == 2);
    assert(gpentry->authority_id == 888);
    
    remove(gp_path); // clean up
    printf("  [PASS] Account interest grace periods, suppression checks, and override logs verified.\n");

    // 39. Test Card Minimum Transaction Amount Validator (Minimum Amount Validation)
    printf("[E2E] Testing Card Minimum Transaction Amount Validator...\n");
    const char *minauth_path = "hogan_minauth.dat.bin";
    remove(minauth_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system min_sys;
    tsfi_hogan_init(&min_sys);
    assert(tsfi_hogan_register_account(&min_sys, 1001, 10000) == 0); // Alice: 10,000 units
    
    // Set Alice's card minimum authorization amount to 50
    assert(tsfi_hogan_update_min_card_auth(&min_sys, minauth_path, 1001, 50, 999) == 0);
    assert(tsfi_hogan_update_min_card_auth(&min_sys, "hogan_minauth.json", 1001, 50, 999) == -3); // Rule 13 check
    
    // First card authorization for 100 (succeeds because 100 >= 50 threshold)
    assert(tsfi_hogan_authorize_card(&min_sys, "hogan_tx.dat.bin", 5005, 1001, 9009, 100) == 0);
    
    // Second card authorization for 20 (should fail because 20 < 50 threshold; returns -8)
    assert(tsfi_hogan_authorize_card(&min_sys, "hogan_tx.dat.bin", 5005, 1001, 9009, 20) == -8);
    
    // Read sequential log file and verify entries
    uint8_t read_minbuf[sizeof(hogan_min_card_auth_entry)];
    size_t min_size = 0;
    assert(tsfi_hogan_read_seq_record(minauth_path, 0, read_minbuf, &min_size) == 0);
    assert(min_size == sizeof(hogan_min_card_auth_entry));
    
    const hogan_min_card_auth_entry *minentry = (const hogan_min_card_auth_entry *)read_minbuf;
    assert(minentry->account_id == 1001);
    assert(minentry->previous_min_amount == 0);
    assert(minentry->new_min_amount == 50);
    assert(minentry->authority_id == 999);
    
    remove(minauth_path); // clean up
    remove("hogan_tx.dat.bin"); // clean up
    printf("  [PASS] Card minimum authorization thresholds, amount checks, and override logs verified.\n");

    // 40. Test Card Single Transaction Maximum Limit Manager (Maximum Amount Validation)
    printf("[E2E] Testing Card Single Transaction Maximum Limit Manager...\n");
    const char *maxauth_path = "hogan_maxauth.dat.bin";
    remove(maxauth_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system max_sys;
    tsfi_hogan_init(&max_sys);
    assert(tsfi_hogan_register_account(&max_sys, 1001, 10000) == 0); // Alice: 10,000 units
    
    // Set Alice's card maximum authorization amount to 1000
    assert(tsfi_hogan_update_max_card_auth(&max_sys, maxauth_path, 1001, 1000, 999) == 0);
    assert(tsfi_hogan_update_max_card_auth(&max_sys, "hogan_maxauth.json", 1001, 1000, 999) == -3); // Rule 13 check
    
    // First card authorization for 100 (succeeds because 100 <= 1000 threshold)
    assert(tsfi_hogan_authorize_card(&max_sys, "hogan_tx.dat.bin", 5005, 1001, 9009, 100) == 0);
    
    // Second card authorization for 2000 (should fail because 2000 > 1000 threshold; returns -9)
    assert(tsfi_hogan_authorize_card(&max_sys, "hogan_tx.dat.bin", 5005, 1001, 9009, 2000) == -9);
    
    // Read sequential log file and verify entries
    uint8_t read_mcabuf[sizeof(hogan_max_card_auth_entry)];
    size_t mca_size = 0;
    assert(tsfi_hogan_read_seq_record(maxauth_path, 0, read_mcabuf, &mca_size) == 0);
    assert(mca_size == sizeof(hogan_max_card_auth_entry));
    
    const hogan_max_card_auth_entry *mcaentry = (const hogan_max_card_auth_entry *)read_mcabuf;
    assert(mcaentry->account_id == 1001);
    assert(mcaentry->previous_max_amount == 0);
    assert(mcaentry->new_max_amount == 1000);
    assert(mcaentry->authority_id == 999);
    
    remove(maxauth_path); // clean up
    remove("hogan_tx.dat.bin"); // clean up
    printf("  [PASS] Card maximum authorization thresholds, amount checks, and override logs verified.\n");

    // 41. Test Card Daily Failures Lock Manager (Daily Failure Lock)
    printf("[E2E] Testing Card Daily Failures Lock Manager...\n");
    const char *faillock_path = "hogan_faillock.dat.bin";
    remove(faillock_path); // ensure clean start
    remove("hogan_fail_locks.dat.bin"); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system fail_sys;
    tsfi_hogan_init(&fail_sys);
    assert(tsfi_hogan_register_account(&fail_sys, 1001, 10000) == 0); // Alice: 10,000 units
    
    // Set Alice's card failure limit to 2 failures
    assert(tsfi_hogan_update_card_fail_limit(&fail_sys, faillock_path, 1001, 2, 777) == 0);
    assert(tsfi_hogan_update_card_fail_limit(&fail_sys, "hogan_faillock.json", 1001, 2, 777) == -3); // Rule 13 check
    
    // Trigger first authorization failure (insufficient funds, request 20000)
    assert(tsfi_hogan_authorize_card(&fail_sys, "hogan_tx.dat.bin", 5005, 1001, 9009, 20000) == -1);
    
    // Trigger second authorization failure (insufficient funds, request 30000)
    // This second failure should trigger an automatic block (returns -1 on auth, card becomes blocked)
    assert(tsfi_hogan_authorize_card(&fail_sys, "hogan_tx.dat.bin", 5005, 1001, 9009, 30000) == -1);
    
    // Third authorization attempt should be blocked by status lock (returns -4)
    assert(tsfi_hogan_authorize_card(&fail_sys, "hogan_tx.dat.bin", 5005, 1001, 9009, 100) == -4);
    
    // Read failure lock entry and verify override log
    uint8_t read_flbuf[sizeof(hogan_card_fail_limit_entry)];
    size_t fl_size = 0;
    assert(tsfi_hogan_read_seq_record(faillock_path, 0, read_flbuf, &fl_size) == 0);
    assert(fl_size == sizeof(hogan_card_fail_limit_entry));
    const hogan_card_fail_limit_entry *flentry = (const hogan_card_fail_limit_entry *)read_flbuf;
    assert(flentry->account_id == 1001);
    assert(flentry->previous_fail_limit == 0);
    assert(flentry->new_fail_limit == 2);
    assert(flentry->authority_id == 777);
    
    // Read auto lock sequential log entries
    uint8_t read_albuf[sizeof(hogan_card_fail_limit_entry)];
    size_t al_size = 0;
    assert(tsfi_hogan_read_seq_record("hogan_fail_locks.dat.bin", 0, read_albuf, &al_size) == 0);
    assert(al_size == sizeof(hogan_card_fail_limit_entry));
    const hogan_card_fail_limit_entry *alentry = (const hogan_card_fail_limit_entry *)read_albuf;
    assert(alentry->account_id == 1001);
    assert(alentry->new_fail_limit == 5005); // card_id is stored in new_fail_limit field for system auto lock representation
    assert(alentry->authority_id == 0);
    
    remove(faillock_path); // clean up
    remove("hogan_fail_locks.dat.bin"); // clean up
    remove("hogan_tx.dat.bin"); // clean up
    printf("  [PASS] Card failure count limits, auto-blocking, and lock logs verified.\n");

    // 42. Test Account Overdraft Fee Accumulator Manager (Overdraft Fee Booking)
    printf("[E2E] Testing Account Overdraft Fee Accumulator Manager...\n");
    const char *odfee_path = "hogan_odfee.dat.bin";
    remove(odfee_path); // ensure clean start
    
    hogan_umbrella_system odfee_sys;
    tsfi_hogan_init(&odfee_sys);
    assert(tsfi_hogan_register_account(&odfee_sys, 1001, 1000) == 0); // Alice: 1000 balance
    assert(tsfi_hogan_register_account(&odfee_sys, 2002, 500) == 0);  // Bob: 500 balance
    assert(tsfi_hogan_register_account(&odfee_sys, 3003, 10000) == 0); // Backup: 10,000 balance
    
    // Set Alice's backup to account 3003
    odfee_sys.accounts[0].backup_account_id = 3003;
    odfee_sys.accounts[0].has_backup = 1;
    
    // Set Alice's overdraft fee amount to 15 units
    assert(tsfi_hogan_update_overdraft_fee(&odfee_sys, odfee_path, 1001, 15, 888) == 0);
    assert(tsfi_hogan_update_overdraft_fee(&odfee_sys, "hogan_odfee.json", 1001, 15, 888) == -3); // Rule 13 check
    
    // Dispatch overnight transfer transaction for 1200 units (requires 200 overdraft deficit)
    assert(tsfi_hogan_dispatch_tx(&odfee_sys, 1001, 2002, 1200, VM_EVM) == 0);
    
    // Run overnight reconciliation
    assert(tsfi_hogan_overnight_reconciliation(&odfee_sys, "hogan_lfs.dat.bin") == 0);
    
    // Verify backup balance (10,000 - 200 = 9800)
    assert(odfee_sys.accounts[2].balance == 9800);
    // Verify Alice's balance (1000 + 200 - 1200 = 0. Then minus 15 fee = 0, with 15 fee added to overdraft_drawn)
    assert(odfee_sys.accounts[0].balance == 0);
    assert(odfee_sys.accounts[0].overdraft_drawn == 215); // 200 deficit + 15 fee
    
    // Read sequential log file and verify entries
    uint8_t read_ofbuf[sizeof(hogan_overdraft_fee_entry)];
    size_t of_size = 0;
    assert(tsfi_hogan_read_seq_record(odfee_path, 0, read_ofbuf, &of_size) == 0);
    assert(of_size == sizeof(hogan_overdraft_fee_entry));
    
    const hogan_overdraft_fee_entry *ofentry = (const hogan_overdraft_fee_entry *)read_ofbuf;
    assert(ofentry->account_id == 1001);
    assert(ofentry->previous_fee_amount == 0);
    assert(ofentry->new_fee_amount == 15);
    assert(ofentry->authority_id == 888);
    
    remove(odfee_path); // clean up
    printf("  [PASS] Overdraft transaction fees, deductions, and override logs verified.\n");

    // 43. Test Batch Interest Limit Cap Manager (Interest Cap Restriction)
    printf("[E2E] Testing Batch Interest Limit Cap Manager...\n");
    const char *intcap_path = "hogan_intcap.dat.bin";
    remove(intcap_path); // ensure clean start
    
    // Set up system state
    hogan_umbrella_system cap_sys;
    tsfi_hogan_init(&cap_sys);
    assert(tsfi_hogan_register_account(&cap_sys, 1001, 10000) == 0); // Alice: 10,000 units
    
    // Set Alice's maximum interest per epoch to 100 units
    assert(tsfi_hogan_update_interest_cap(&cap_sys, intcap_path, 1001, 100, 999) == 0);
    assert(tsfi_hogan_update_interest_cap(&cap_sys, "hogan_intcap.json", 1001, 100, 999) == -3); // Rule 13 check
    
    // Apply batch interest of 10% (1000 bps). Alice would get 1000 interest, but capped at 100.
    assert(tsfi_hogan_apply_interest(&cap_sys, "hogan_interest.dat.bin", 1000) == 0);
    assert(cap_sys.accounts[0].balance == 10100);
    remove("hogan_interest.dat.bin");
    
    // Read sequential log file and verify entries
    uint8_t read_icbuf[sizeof(hogan_interest_cap_entry)];
    size_t ic_size = 0;
    assert(tsfi_hogan_read_seq_record(intcap_path, 0, read_icbuf, &ic_size) == 0);
    assert(ic_size == sizeof(hogan_interest_cap_entry));
    
    const hogan_interest_cap_entry *icentry = (const hogan_interest_cap_entry *)read_icbuf;
    assert(icentry->account_id == 1001);
    assert(icentry->previous_interest_cap == 0);
    assert(icentry->new_interest_cap == 100);
    assert(icentry->authority_id == 999);
    
    remove(intcap_path); // clean up
    printf("  [PASS] Batch interest caps, limit validations, and override logs verified.\n");

    // 44. Test Account Cumulative Transaction Amount Cap Manager (Account spend caps)
    printf("[E2E] Testing Account Cumulative Transaction Amount Cap Manager...\n");
    const char *acc_spend_path = "hogan_acc_spend.dat.bin";
    remove(acc_spend_path); // ensure clean start
    
    hogan_umbrella_system spend_sys;
    tsfi_hogan_init(&spend_sys);
    assert(tsfi_hogan_register_account(&spend_sys, 1001, 10000) == 0); // Alice: 10000 balance
    assert(tsfi_hogan_register_account(&spend_sys, 2002, 500) == 0);   // Bob: 500 balance
    
    // Set Alice's account cumulative transaction spend limit to 3000 units
    assert(tsfi_hogan_update_acc_spend_limit(&spend_sys, acc_spend_path, 1001, 3000, 999) == 0);
    assert(tsfi_hogan_update_acc_spend_limit(&spend_sys, "hogan_acc_spend.json", 1001, 3000, 999) == -3); // Rule 13 check
    
    // Dispatch standard transfer for 2000 units (succeeds because 2000 <= 3000 limit)
    assert(tsfi_hogan_dispatch_tx(&spend_sys, 1001, 2002, 2000, VM_EVM) == 0);
    
    // Dispatch standard transfer for 1500 units (should fail because total debits 3500 > 3000 limit)
    assert(tsfi_hogan_dispatch_tx(&spend_sys, 1001, 2002, 1500, VM_EVM) == 0);
    
    // Run overnight reconciliation
    assert(tsfi_hogan_overnight_reconciliation(&spend_sys, "hogan_lfs.dat.bin") == 0);
    
    // Verify Alice's balance (10000 - 2000 = 8000. Second tx was rejected)
    assert(spend_sys.accounts[0].balance == 8000);
    
    // Read sequential log file and verify entries
    uint8_t read_asbuf[sizeof(hogan_acc_spend_limit_entry)];
    size_t as_size = 0;
    assert(tsfi_hogan_read_seq_record(acc_spend_path, 0, read_asbuf, &as_size) == 0);
    assert(as_size == sizeof(hogan_acc_spend_limit_entry));
    
    const hogan_acc_spend_limit_entry *asentry = (const hogan_acc_spend_limit_entry *)read_asbuf;
    assert(asentry->account_id == 1001);
    assert(asentry->previous_spend_limit == 0);
    assert(asentry->new_spend_limit == 3000);
    assert(asentry->authority_id == 999);
    
    remove(acc_spend_path); // clean up
    printf("  [PASS] Account cumulative transaction amount caps, volume validation, and override logs verified.\n");

    // 45. Test Card PIN Attempt Limit Lockout Manager (PIN locks)
    printf("[E2E] Testing Card PIN Attempt Limit Lockout Manager...\n");
    const char *pin_path = "hogan_pin.dat.bin";
    const char *pin_limit_path = "hogan_pin_limits.dat.bin";
    remove(pin_path); // ensure clean start
    remove(pin_limit_path); // ensure clean start
    remove("hogan_pin_locks.dat.bin"); // ensure clean start
    
    hogan_umbrella_system pin_sys;
    tsfi_hogan_init(&pin_sys);
    assert(tsfi_hogan_register_account(&pin_sys, 1001, 10000) == 0); // Alice: 10000 balance
    
    // Set Alice's card PIN to 4321
    assert(tsfi_hogan_update_card_pin(&pin_sys, pin_path, 1001, 4321, 999) == 0);
    assert(tsfi_hogan_update_card_pin(&pin_sys, "hogan_pin.json", 1001, 4321, 999) == -3); // Rule 13 check
    
    // Set Alice's card PIN attempt limit to 2 attempts
    assert(tsfi_hogan_update_pin_fail_limit(&pin_sys, pin_limit_path, 1001, 2, 999) == 0);
    assert(tsfi_hogan_update_pin_fail_limit(&pin_sys, "hogan_pin_limits.json", 1001, 2, 999) == -3); // Rule 13 check
    
    // Verify incorrect PIN (entered 1111) returning -2
    assert(tsfi_hogan_validate_card_pin(&pin_sys, "hogan_attempts.dat.bin", 5005, 1001, 1111) == -2);
    
    // Verify second incorrect PIN (entered 2222) returning -2, which triggers auto block
    assert(tsfi_hogan_validate_card_pin(&pin_sys, "hogan_attempts.dat.bin", 5005, 1001, 2222) == -2);
    
    // Verify third PIN attempt is blocked with status block returning -4
    assert(tsfi_hogan_validate_card_pin(&pin_sys, "hogan_attempts.dat.bin", 5005, 1001, 4321) == -4);
    
    // Read override log files and verify entries
    uint8_t read_pinbuf[sizeof(hogan_card_pin_entry)];
    size_t pin_size = 0;
    assert(tsfi_hogan_read_seq_record(pin_path, 0, read_pinbuf, &pin_size) == 0);
    assert(pin_size == sizeof(hogan_card_pin_entry));
    const hogan_card_pin_entry *pinentry = (const hogan_card_pin_entry *)read_pinbuf;
    assert(pinentry->account_id == 1001);
    assert(pinentry->previous_pin == 0);
    assert(pinentry->new_pin == 4321);
    assert(pinentry->authority_id == 999);
    
    // Read auto lock sequential log entries
    uint8_t read_plbuf[sizeof(hogan_pin_fail_limit_entry)];
    size_t pl_size = 0;
    assert(tsfi_hogan_read_seq_record("hogan_pin_locks.dat.bin", 0, read_plbuf, &pl_size) == 0);
    assert(pl_size == sizeof(hogan_pin_fail_limit_entry));
    const hogan_pin_fail_limit_entry *plentry = (const hogan_pin_fail_limit_entry *)read_plbuf;
    assert(plentry->account_id == 1001);
    assert(plentry->previous_pin_limit == 2); // pin_fail_limit
    assert(plentry->new_pin_limit == 5005);   // card_id
    
    remove(pin_path); // clean up
    remove(pin_limit_path); // clean up
    remove("hogan_attempts.dat.bin"); // clean up
    remove("hogan_pin_locks.dat.bin"); // clean up
    printf("  [PASS] Card PIN validations, fail counts, auto-blocking lockouts, and override logs verified.\n");

    // 46. Test Account Daily Fee Cap Manager (Fee Caps)
    printf("[E2E] Testing Account Daily Fee Cap Manager...\n");
    const char *feecap_path = "hogan_feecap.dat.bin";
    remove(feecap_path); // ensure clean start
    
    hogan_umbrella_system fcap_sys;
    tsfi_hogan_init(&fcap_sys);
    assert(tsfi_hogan_register_account(&fcap_sys, 1001, 1000) == 0); // Alice: 1000 balance
    
    // Set Alice's maintenance fee cap to 15 units
    assert(tsfi_hogan_update_fee_cap(&fcap_sys, feecap_path, 1001, 15, 999) == 0);
    assert(tsfi_hogan_update_fee_cap(&fcap_sys, "hogan_feecap.json", 1001, 15, 999) == -3); // Rule 13 check
    
    // Apply batch maintenance fee of 50 units (Alice's charge should be capped at 15)
    assert(tsfi_hogan_apply_fees(&fcap_sys, "hogan_fees.dat.bin", 50) == 0);
    assert(fcap_sys.accounts[0].balance == 985);
    remove("hogan_fees.dat.bin");
    
    // Read override log files and verify entries
    uint8_t read_fcbuf[sizeof(hogan_fee_cap_entry)];
    size_t fc_size = 0;
    assert(tsfi_hogan_read_seq_record(feecap_path, 0, read_fcbuf, &fc_size) == 0);
    assert(fc_size == sizeof(hogan_fee_cap_entry));
    const hogan_fee_cap_entry *fcentry = (const hogan_fee_cap_entry *)read_fcbuf;
    assert(fcentry->account_id == 1001);
    assert(fcentry->previous_fee_cap == 0);
    assert(fcentry->new_fee_cap == 15);
    assert(fcentry->authority_id == 999);
    
    remove(feecap_path); // clean up
    printf("  [PASS] Maintenance fee caps, cap validations, and override logs verified.\n");

    printf("ALL HOGAN SYSTEMS E2E C TESTS COMPLETED SUCCESSFULLY!\n");
    return 0;
}
