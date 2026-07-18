#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "tsfi_hogan.h"
#include "tsfi_zmm_vm.h"
#include "tsfi_ramac_layout.h"
#include "tsfi_wire_firmware.h"

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

    // 47. Test Account Dormancy and Activity Status Manager (Dormancy)
    printf("[E2E] Testing Account Dormancy and Activity Status Manager...\n");
    const char *dorm_path = "hogan_dorm.dat.bin";
    remove(dorm_path); // ensure clean start
    
    hogan_umbrella_system dorm_sys;
    tsfi_hogan_init(&dorm_sys);
    assert(tsfi_hogan_register_account(&dorm_sys, 1001, 10000) == 0); // Alice: 10000 balance
    assert(tsfi_hogan_register_account(&dorm_sys, 2002, 500) == 0);   // Bob: 500 balance
    
    // Set Alice's dormancy threshold to 3 epochs
    assert(tsfi_hogan_set_dormancy_threshold(&dorm_sys, dorm_path, 1001, 3, 999) == 0);
    assert(tsfi_hogan_set_dormancy_threshold(&dorm_sys, "hogan_dorm.json", 1001, 3, 999) == -3); // Rule 13 check
    
    // Dispatch a transaction in epoch 1 (succeeds)
    assert(tsfi_hogan_dispatch_tx(&dorm_sys, 1001, 2002, 100, VM_EVM) == 0);
    assert(tsfi_hogan_overnight_reconciliation(&dorm_sys, "hogan_lfs.dat.bin") == 0);
    assert(dorm_sys.accounts[0].balance == 9900);
    
    // Advance epoch to 5 (dorm_sys.current_epoch = 5, Alice last active at 1. Inactivity = 4 epochs > threshold 3)
    dorm_sys.current_epoch = 5;
    
    // Try to dispatch a transaction (should be blocked due to dormancy since epoch gap 4 > threshold 3)
    assert(tsfi_hogan_dispatch_tx(&dorm_sys, 1001, 2002, 100, VM_EVM) == 0);
    assert(tsfi_hogan_overnight_reconciliation(&dorm_sys, "hogan_lfs.dat.bin") == 0);
    assert(dorm_sys.accounts[0].balance == 9900); // Alice's balance remains unchanged
    
    // Reactivate dormant account administratively
    assert(tsfi_hogan_reactivate_dormant_account(&dorm_sys, dorm_path, 1001, 999) == 0);
    
    // Verify transaction now succeeds
    assert(tsfi_hogan_dispatch_tx(&dorm_sys, 1001, 2002, 100, VM_EVM) == 0);
    assert(tsfi_hogan_overnight_reconciliation(&dorm_sys, "hogan_lfs.dat.bin") == 0);
    assert(dorm_sys.accounts[0].balance == 9800); // Alice's balance correctly decremented
    
    // Read override log files and verify reactivation entries
    uint8_t read_dormbuf[sizeof(hogan_dormancy_entry)];
    size_t dorm_size = 0;
    assert(tsfi_hogan_read_seq_record(dorm_path, 0, read_dormbuf, &dorm_size) == 0);
    assert(dorm_size == sizeof(hogan_dormancy_entry));
    
    // The first record in dorm_path is the threshold setting
    const hogan_dormancy_entry *th_entry = (const hogan_dormancy_entry *)read_dormbuf;
    assert(th_entry->account_id == 1001);
    assert(th_entry->previous_dormant_state == 0); // initial threshold 0
    assert(th_entry->new_dormant_state == 3);      // new threshold 3
    
    // The second record in dorm_path is the reactivation
    assert(tsfi_hogan_read_seq_record(dorm_path, 1, read_dormbuf, &dorm_size) == 0);
    const hogan_dormancy_entry *react_entry = (const hogan_dormancy_entry *)read_dormbuf;
    assert(react_entry->account_id == 1001);
    assert(react_entry->previous_dormant_state == 1); // was dormant
    assert(react_entry->new_dormant_state == 0);      // now active
    
    remove(dorm_path); // clean up
    printf("  [PASS] Account dormancy thresholds, gap validations, reactivations, and override logs verified.\n");

    // 48. Test Batch Interest Minimum Posting Threshold Manager (Interest Threshold)
    printf("[E2E] Testing Batch Interest Minimum Posting Threshold Manager...\n");
    const char *inthreshold_path = "hogan_inthreshold.dat.bin";
    remove(inthreshold_path); // ensure clean start
    
    hogan_umbrella_system th_sys;
    tsfi_hogan_init(&th_sys);
    assert(tsfi_hogan_register_account(&th_sys, 1001, 1000) == 0); // Alice: 1000 balance
    
    // Set Alice's minimum interest posting threshold to 5 units
    assert(tsfi_hogan_update_interest_threshold(&th_sys, inthreshold_path, 1001, 5, 999) == 0);
    assert(tsfi_hogan_update_interest_threshold(&th_sys, "hogan_inthreshold.json", 1001, 5, 999) == -3); // Rule 13 check
    
    // Apply batch interest posting at 30 bps (interest calculated = 1000 * 30 / 10000 = 3 units)
    // 3 units < 5 threshold => Interest should be suppressed (Alice balance remains 1000)
    assert(tsfi_hogan_apply_interest(&th_sys, "hogan_interests.dat.bin", 30) == 0);
    assert(th_sys.accounts[0].balance == 1000);
    remove("hogan_interests.dat.bin");
    
    // Apply batch interest posting at 60 bps (interest calculated = 1000 * 60 / 10000 = 6 units)
    // 6 units >= 5 threshold => Interest should post (Alice balance becomes 1006)
    assert(tsfi_hogan_apply_interest(&th_sys, "hogan_interests.dat.bin", 60) == 0);
    assert(th_sys.accounts[0].balance == 1006);
    remove("hogan_interests.dat.bin");
    
    // Read override log files and verify entries
    uint8_t read_thbuf[sizeof(hogan_interest_threshold_entry)];
    size_t th_size = 0;
    assert(tsfi_hogan_read_seq_record(inthreshold_path, 0, read_thbuf, &th_size) == 0);
    assert(th_size == sizeof(hogan_interest_threshold_entry));
    const hogan_interest_threshold_entry *thentry = (const hogan_interest_threshold_entry *)read_thbuf;
    assert(thentry->account_id == 1001);
    assert(thentry->previous_threshold == 0);
    assert(thentry->new_threshold == 5);
    assert(thentry->authority_id == 999);
    
    remove(inthreshold_path); // clean up
    printf("  [PASS] Batch interest posting thresholds, suppression, and override logs verified.\n");

    // 49. Test Batch Fee Minimum Balance Waive Manager (Fee Waive Threshold)
    printf("[E2E] Testing Batch Fee Minimum Balance Waive Manager...\n");
    const char *fwpath = "hogan_feewaive.dat.bin";
    remove(fwpath); // ensure clean start
    
    hogan_umbrella_system fw_sys;
    tsfi_hogan_init(&fw_sys);
    assert(tsfi_hogan_register_account(&fw_sys, 1001, 100) == 0); // Alice: 100 balance
    
    // Set Alice's minimum balance fee waive threshold to 150 units
    assert(tsfi_hogan_update_fee_waive_threshold(&fw_sys, fwpath, 1001, 150, 999) == 0);
    assert(tsfi_hogan_update_fee_waive_threshold(&fw_sys, "hogan_feewaive.json", 1001, 150, 999) == -3); // Rule 13 check
    
    // Apply batch maintenance fees of 30 units
    // Since Alice's balance (100) is below the threshold (150), the fee should be waived (balance remains 100)
    assert(tsfi_hogan_apply_fees(&fw_sys, "hogan_fees.dat.bin", 30) == 0);
    assert(fw_sys.accounts[0].balance == 100);
    remove("hogan_fees.dat.bin");
    
    // Set Alice's balance to 200 (above threshold 150)
    fw_sys.accounts[0].balance = 200;
    
    // Apply batch maintenance fees of 30 units (now Alice is charged, balance becomes 170)
    assert(tsfi_hogan_apply_fees(&fw_sys, "hogan_fees.dat.bin", 30) == 0);
    assert(fw_sys.accounts[0].balance == 170);
    remove("hogan_fees.dat.bin");
    
    // Read override log files and verify entries
    uint8_t read_fwbuf[sizeof(hogan_fee_waive_threshold_entry)];
    size_t fw_size = 0;
    assert(tsfi_hogan_read_seq_record(fwpath, 0, read_fwbuf, &fw_size) == 0);
    assert(fw_size == sizeof(hogan_fee_waive_threshold_entry));
    const hogan_fee_waive_threshold_entry *fwentry = (const hogan_fee_waive_threshold_entry *)read_fwbuf;
    assert(fwentry->account_id == 1001);
    assert(fwentry->previous_threshold == 0);
    assert(fwentry->new_threshold == 150);
    assert(fwentry->authority_id == 999);
    
    remove(fwpath); // clean up
    printf("  [PASS] Batch fee minimum balance waiving, validations, and override logs verified.\n");

    // 50. Test Card PIN Change Failure Count Manager (PIN Change Lockout)
    printf("[E2E] Testing Card PIN Change Failure Count Manager...\n");
    const char *pclock_path = "hogan_pclock.dat.bin";
    const char *pclimit_path = "hogan_pclimit.dat.bin";
    remove(pclock_path); // ensure clean start
    remove(pclimit_path);
    
    hogan_umbrella_system pc_sys;
    tsfi_hogan_init(&pc_sys);
    assert(tsfi_hogan_register_account(&pc_sys, 1001, 1000) == 0);
    pc_sys.accounts[0].card_pin = 1234;
    
    // Set Alice's PIN change fail limit to 2 attempts
    assert(tsfi_hogan_set_pin_change_fail_limit(&pc_sys, pclimit_path, 1001, 2, 999) == 0);
    assert(tsfi_hogan_set_pin_change_fail_limit(&pc_sys, "hogan_pclimit.json", 1001, 2, 999) == -3); // Rule 13 check
    
    // Attempt 1: Failed PIN change (wrong old PIN 1111)
    assert(tsfi_hogan_change_card_pin(&pc_sys, pclock_path, 1001, 1111, 4321) == -2);
    assert(pc_sys.accounts[0].card_pin == 1234);
    assert(pc_sys.accounts[0].pin_change_fail_count == 1);
    
    // Attempt 2: Failed PIN change (wrong old PIN 2222, reaches limit of 2)
    assert(tsfi_hogan_change_card_pin(&pc_sys, pclock_path, 1001, 2222, 4321) == -2);
    assert(pc_sys.accounts[0].card_pin == 1234);
    assert(pc_sys.accounts[0].pin_change_fail_count == 2);
    
    // Attempt 3: Blocked PIN change due to lockout (-4)
    assert(tsfi_hogan_change_card_pin(&pc_sys, pclock_path, 1001, 1234, 4321) == -4);
    assert(pc_sys.accounts[0].card_pin == 1234);
    
    // Verify override log files
    uint8_t read_pclbuf[sizeof(hogan_pin_change_limit_entry)];
    size_t pcl_size = 0;
    assert(tsfi_hogan_read_seq_record(pclimit_path, 0, read_pclbuf, &pcl_size) == 0);
    assert(pcl_size == sizeof(hogan_pin_change_limit_entry));
    const hogan_pin_change_limit_entry *pcentry = (const hogan_pin_change_limit_entry *)read_pclbuf;
    assert(pcentry->account_id == 1001);
    assert(pcentry->previous_limit == 0);
    assert(pcentry->new_limit == 2);
    assert(pcentry->authority_id == 999);
    
    remove(pclock_path); // clean up
    remove(pclimit_path);
    printf("  [PASS] Card PIN change fail counts, lockouts, and override logs verified.\n");

    // 51. Test Batch Fee Exemption Expiry Manager (Fee Exemption Expiry)
    printf("[E2E] Testing Batch Fee Exemption Expiry Manager...\n");
    const char *feexp_path = "hogan_feexp.dat.bin";
    remove(feexp_path); // ensure clean start
    
    hogan_umbrella_system fe_sys;
    tsfi_hogan_init(&fe_sys);
    assert(tsfi_hogan_register_account(&fe_sys, 1001, 1000) == 0); // Alice: 1000 balance
    fe_sys.accounts[0].fee_exempt = 1; // initially exempt
    
    // Set Alice's fee exemption expiry epoch to 2
    assert(tsfi_hogan_update_fee_exempt_expiry(&fe_sys, feexp_path, 1001, 2, 999) == 0);
    assert(tsfi_hogan_update_fee_exempt_expiry(&fe_sys, "hogan_feexp.json", 1001, 2, 999) == -3); // Rule 13 check
    
    // Epoch is 1 (system starts at 1, <= 2 expiry): Fee is skipped (Alice balance remains 1000)
    assert(tsfi_hogan_apply_fees(&fe_sys, "hogan_fees.dat.bin", 50) == 0);
    assert(fe_sys.accounts[0].balance == 1000);
    remove("hogan_fees.dat.bin");
    
    // Advance epoch to 3 (> 2 expiry)
    fe_sys.current_epoch = 3;
    
    // Apply batch maintenance fee of 50 units (exemption expires, fee is charged, balance becomes 950)
    assert(tsfi_hogan_apply_fees(&fe_sys, "hogan_fees.dat.bin", 50) == 0);
    assert(fe_sys.accounts[0].balance == 950);
    assert(fe_sys.accounts[0].fee_exempt == 0); // verify flag is cleared
    remove("hogan_fees.dat.bin");
    
    // Read override log files and verify entries
    uint8_t read_feexbuf[sizeof(hogan_fee_exempt_expiry_entry)];
    size_t feex_size = 0;
    assert(tsfi_hogan_read_seq_record(feexp_path, 0, read_feexbuf, &feex_size) == 0);
    assert(feex_size == sizeof(hogan_fee_exempt_expiry_entry));
    const hogan_fee_exempt_expiry_entry *feentry = (const hogan_fee_exempt_expiry_entry *)read_feexbuf;
    assert(feentry->account_id == 1001);
    assert(feentry->previous_expiry == 0);
    assert(feentry->new_expiry == 2);
    assert(feentry->authority_id == 999);
    
    remove(feexp_path); // clean up
    printf("  [PASS] Batch fee exemption expirations, validations, and override logs verified.\n");

    // 52. Test Card Merchant Exception Whitelist Manager (Merchant Exception)
    printf("[E2E] Testing Card Merchant Exception Whitelist Manager...\n");
    const char *mex_path = "hogan_merchant_exceptions.dat.bin";
    remove(mex_path); // ensure clean start
    
    hogan_umbrella_system me_sys;
    tsfi_hogan_init(&me_sys);
    assert(tsfi_hogan_register_account(&me_sys, 1001, 1000) == 0); // Alice: 1000 balance
    
    // Block merchant 7777 for Alice
    assert(tsfi_hogan_update_merchant_block(&me_sys, "hogan_mblock.dat.bin", 1001, 7777, 1, 999) == 0);
    remove("hogan_mblock.dat.bin");
    
    // Auth should fail because merchant 7777 is blocked
    assert(tsfi_hogan_authorize_card(&me_sys, "hogan_holds.dat.bin", 2002, 1001, 7777, 100) == -5);
    
    // Whitelist merchant 7777 as an exception for Alice
    assert(tsfi_hogan_add_merchant_exception(&me_sys, mex_path, 1001, 7777, 999) == 0);
    assert(tsfi_hogan_add_merchant_exception(&me_sys, "hogan_mwhitelist.json", 1001, 7777, 999) == -3); // Rule 13 check
    
    // Auth should now succeed with whitelist exception bypassing blocked merchant check
    assert(tsfi_hogan_authorize_card(&me_sys, "hogan_holds.dat.bin", 2002, 1001, 7777, 100) == 0);
    remove("hogan_holds.dat.bin");
    
    // Read override log files and verify entries
    uint8_t read_mebuf[sizeof(hogan_merchant_exception_entry)];
    size_t me_size = 0;
    assert(tsfi_hogan_read_seq_record(mex_path, 0, read_mebuf, &me_size) == 0);
    assert(me_size == sizeof(hogan_merchant_exception_entry));
    const hogan_merchant_exception_entry *me_entry = (const hogan_merchant_exception_entry *)read_mebuf;
    assert(me_entry->account_id == 1001);
    assert(me_entry->merchant_id == 7777);
    assert(me_entry->was_added == 1);
    assert(me_entry->authority_id == 999);
    
    remove(mex_path); // clean up
    printf("  [PASS] Card merchant block whitelist overrides and override logs verified.\n");

    // 53. Test Account Posting Restriction Manager (Posting Restriction)
    printf("[E2E] Testing Account Posting Restriction Manager...\n");
    const char *pr_path = "hogan_post_restrictions.dat.bin";
    remove(pr_path); // ensure clean start
    
    hogan_umbrella_system pr_sys;
    tsfi_hogan_init(&pr_sys);
    assert(tsfi_hogan_register_account(&pr_sys, 1001, 1000) == 0); // Alice: 1000 balance
    assert(tsfi_hogan_register_account(&pr_sys, 2002, 1000) == 0); // Bob: 1000 balance
    
    // Set posting restriction on Alice to restrict debits (1)
    assert(tsfi_hogan_update_posting_restriction(&pr_sys, pr_path, 1001, 1, 999) == 0);
    assert(tsfi_hogan_update_posting_restriction(&pr_sys, "hogan_pr.json", 1001, 1, 999) == -3); // Rule 13 check
    
    // Card auth on Alice should fail with -10 (Debit restricted)
    assert(tsfi_hogan_authorize_card(&pr_sys, "hogan_holds.dat.bin", 5005, 1001, 8888, 100) == -10);
    
    // Queue transfer from Alice to Bob (outbound from restricted account)
    assert(tsfi_hogan_dispatch_tx(&pr_sys, 1001, 2002, 100, VM_EVM) == 0);
    
    // Run reconciliation to process transactions
    assert(tsfi_hogan_overnight_reconciliation_ex(&pr_sys, "hogan_lfs.dat.bin", NULL) == 0);
    remove("hogan_lfs.dat.bin");
    
    // Verify transaction was marked as processed (but failed/blocked inside dispatch)
    assert(pr_sys.tx_log[0].processed == 1);
    assert(pr_sys.accounts[0].balance == 1000); // balance unchanged
    assert(pr_sys.accounts[1].balance == 1000);
    
    // Set posting restriction on Bob to restrict credits (2)
    assert(tsfi_hogan_update_posting_restriction(&pr_sys, pr_path, 2002, 2, 999) == 0);
    
    // Clear Alice restriction so she can debit
    assert(tsfi_hogan_update_posting_restriction(&pr_sys, pr_path, 1001, 0, 999) == 0);
    
    // Queue transfer from Alice to Bob (inbound to restricted account Bob)
    assert(tsfi_hogan_dispatch_tx(&pr_sys, 1001, 2002, 100, VM_EVM) == 0);
    
    // Run reconciliation to process transactions
    assert(tsfi_hogan_overnight_reconciliation_ex(&pr_sys, "hogan_lfs.dat.bin", NULL) == 0);
    remove("hogan_lfs.dat.bin");
    
    assert(pr_sys.tx_log[0].processed == 1); // should block Bob receiving credits
    assert(pr_sys.accounts[0].balance == 1000);
    assert(pr_sys.accounts[1].balance == 1000);
    
    // Read override log files and verify entries
    uint8_t read_prbuf[sizeof(hogan_posting_restriction_entry)];
    size_t pr_size = 0;
    // The last override update was Bob's restriction code being set to 2, followed by Alice being set to 0.
    // Let's read the third entry (logical index 2) which is Alice being cleared to 0.
    assert(tsfi_hogan_read_seq_record(pr_path, 2, read_prbuf, &pr_size) == 0);
    assert(pr_size == sizeof(hogan_posting_restriction_entry));
    const hogan_posting_restriction_entry *pr_entry = (const hogan_posting_restriction_entry *)read_prbuf;
    assert(pr_entry->account_id == 1001);
    assert(pr_entry->previous_restriction == 1);
    assert(pr_entry->new_restriction == 0);
    assert(pr_entry->authority_id == 999);
    
    remove(pr_path); // clean up
    printf("  [PASS] Account posting restriction blocks and override logs verified.\n");

    // 54. Test Account Dormancy Fee Surcharge Manager (Dormancy Fee Surcharge)
    printf("[E2E] Testing Account Dormancy Fee Surcharge Manager...\n");
    const char *ds_path = "hogan_dormancy_surcharges.dat.bin";
    remove(ds_path); // ensure clean start
    
    hogan_umbrella_system ds_sys;
    tsfi_hogan_init(&ds_sys);
    assert(tsfi_hogan_register_account(&ds_sys, 1001, 1000) == 0); // Alice: 1000 balance
    
    // Set Alice as dormant
    ds_sys.accounts[0].is_dormant = 1;
    
    // Update dormancy fee surcharge to 50
    assert(tsfi_hogan_update_dormancy_surcharge(&ds_sys, ds_path, 1001, 50, 999) == 0);
    assert(tsfi_hogan_update_dormancy_surcharge(&ds_sys, "hogan_ds.json", 1001, 50, 999) == -3); // Rule 13 check
    
    // Apply batch fees: flat fee of 10. Dormant Alice should be charged 10 (flat) + 50 (surcharge) = 60.
    const char *fee_log_path = "hogan_fees_test.dat.bin";
    remove(fee_log_path);
    assert(tsfi_hogan_apply_fees(&ds_sys, fee_log_path, 10) == 0);
    remove(fee_log_path);
    
    assert(ds_sys.accounts[0].balance == 940); // 1000 - 60 = 940
    
    // Read override log files and verify entries
    uint8_t read_dsbuf[sizeof(hogan_dormancy_surcharge_entry)];
    size_t ds_size = 0;
    assert(tsfi_hogan_read_seq_record(ds_path, 0, read_dsbuf, &ds_size) == 0);
    assert(ds_size == sizeof(hogan_dormancy_surcharge_entry));
    const hogan_dormancy_surcharge_entry *ds_entry = (const hogan_dormancy_surcharge_entry *)read_dsbuf;
    assert(ds_entry->account_id == 1001);
    assert(ds_entry->previous_surcharge == 0);
    assert(ds_entry->new_surcharge == 50);
    assert(ds_entry->authority_id == 999);
    
    remove(ds_path); // clean up
    printf("  [PASS] Account dormancy fee surcharge calculations and override logs verified.\n");

    // 55. Test Batch Interest Rate Tier Manager (Interest Rate Tiering)
    printf("[E2E] Testing Batch Interest Rate Tier Manager...\n");
    const char *it_path = "hogan_interest_tiers.dat.bin";
    remove(it_path); // ensure clean start
    
    hogan_umbrella_system it_sys;
    tsfi_hogan_init(&it_sys);
    assert(tsfi_hogan_register_account(&it_sys, 1001, 20000) == 0); // Alice: 20000 balance
    
    // Set Alice's tier: if balance >= 15000, rate is 500 bps (5%)
    assert(tsfi_hogan_update_interest_tier(&it_sys, it_path, 1001, 15000, 500, 999) == 0);
    assert(tsfi_hogan_update_interest_tier(&it_sys, "hogan_it.json", 1001, 15000, 500, 999) == -3); // Rule 13 check
    
    // Apply batch interest: base rate is 100 bps (1%). Alice has 20000, so she matches the tier and gets 500 bps.
    // Interest: 20000 * 500 / 10000 = 1000.
    const char *int_log_path = "hogan_interest_test.dat.bin";
    remove(int_log_path);
    assert(tsfi_hogan_apply_interest(&it_sys, int_log_path, 100) == 0);
    remove(int_log_path);
    
    assert(it_sys.accounts[0].balance == 21000); // 20000 + 1000 = 21000
    
    // Read override log files and verify entries
    uint8_t read_itbuf[sizeof(hogan_interest_tier_entry)];
    size_t it_size = 0;
    assert(tsfi_hogan_read_seq_record(it_path, 0, read_itbuf, &it_size) == 0);
    assert(it_size == sizeof(hogan_interest_tier_entry));
    const hogan_interest_tier_entry *it_entry = (const hogan_interest_tier_entry *)read_itbuf;
    assert(it_entry->account_id == 1001);
    assert(it_entry->threshold == 15000);
    assert(it_entry->rate_bps == 500);
    assert(it_entry->authority_id == 999);
    
    remove(it_path); // clean up
    printf("  [PASS] Batch interest rate tiering calculations and override logs verified.\n");

    // 56. Test Batch Fee Tier Manager (Fee Tiering)
    printf("[E2E] Testing Batch Fee Tier Manager...\n");
    const char *ft_path = "hogan_fee_tiers.dat.bin";
    remove(ft_path); // ensure clean start
    
    hogan_umbrella_system ft_sys;
    tsfi_hogan_init(&ft_sys);
    assert(tsfi_hogan_register_account(&ft_sys, 1001, 1000) == 0); // Alice: 1000 balance
    
    // Set Alice's tier: if balance < 2000, fee is 45 instead of flat fee (e.g. 10)
    assert(tsfi_hogan_update_fee_tier(&ft_sys, ft_path, 1001, 2000, 45, 999) == 0);
    assert(tsfi_hogan_update_fee_tier(&ft_sys, "hogan_ft.json", 1001, 2000, 45, 999) == -3); // Rule 13 check
    
    // Apply batch fees: flat fee of 10. Alice's balance (1000) is below the threshold (2000), so she is charged 45.
    remove(fee_log_path);
    assert(tsfi_hogan_apply_fees(&ft_sys, fee_log_path, 10) == 0);
    remove(fee_log_path);
    
    assert(ft_sys.accounts[0].balance == 955); // 1000 - 45 = 955
    
    // Read override log files and verify entries
    uint8_t read_ftbuf[sizeof(hogan_fee_tier_entry)];
    size_t ft_size = 0;
    assert(tsfi_hogan_read_seq_record(ft_path, 0, read_ftbuf, &ft_size) == 0);
    assert(ft_size == sizeof(hogan_fee_tier_entry));
    const hogan_fee_tier_entry *ft_entry = (const hogan_fee_tier_entry *)read_ftbuf;
    assert(ft_entry->account_id == 1001);
    assert(ft_entry->threshold == 2000);
    assert(ft_entry->fee_amount == 45);
    assert(ft_entry->authority_id == 999);
    
    remove(ft_path); // clean up
    printf("  [PASS] Batch fee tiering calculations and override logs verified.\n");

    // 57. Test Transfer Outbound Transaction Fee Manager (Transfer Outbound Transaction Fee)
    printf("[E2E] Testing Transfer Outbound Transaction Fee Manager...\n");
    const char *tf_path = "hogan_transfer_fees.dat.bin";
    remove(tf_path); // ensure clean start
    
    hogan_umbrella_system tf_sys;
    tsfi_hogan_init(&tf_sys);
    assert(tsfi_hogan_register_account(&tf_sys, 1001, 5000) == 0); // Alice: 5000 balance
    assert(tsfi_hogan_register_account(&tf_sys, 2002, 500) == 0);  // Bob: 500 balance
    
    // Set Alice's outbound transfer fee: 100 bps (1%) and flat fee of 5
    assert(tsfi_hogan_update_transfer_fee(&tf_sys, tf_path, 1001, 100, 5, 999) == 0);
    assert(tsfi_hogan_update_transfer_fee(&tf_sys, "hogan_tf.json", 1001, 100, 5, 999) == -3); // Rule 13 check
    
    // Dispatch transfer of 1000 from Alice to Bob
    assert(tsfi_hogan_dispatch_tx(&tf_sys, 1001, 2002, 1000, VM_EVM) == 0);
    
    // Run overnight reconciliation
    assert(tsfi_hogan_overnight_reconciliation(&tf_sys, "hogan_lfs.dat.bin") == 0);
    remove("hogan_lfs.dat.bin");
    
    // Verify balances (Alice: 5000 - 1000 transfer - (10 + 5 fee) = 3985)
    assert(tf_sys.accounts[0].balance == 3985);
    assert(tf_sys.accounts[1].balance == 1500);
    
    // Read override log files and verify entries
    uint8_t read_tfbuf[sizeof(hogan_transfer_fee_entry)];
    size_t tf_size = 0;
    assert(tsfi_hogan_read_seq_record(tf_path, 0, read_tfbuf, &tf_size) == 0);
    assert(tf_size == sizeof(hogan_transfer_fee_entry));
    const hogan_transfer_fee_entry *tf_entry = (const hogan_transfer_fee_entry *)read_tfbuf;
    assert(tf_entry->account_id == 1001);
    assert(tf_entry->fee_bps == 100);
    assert(tf_entry->fee_flat == 5);
    assert(tf_entry->authority_id == 999);
    
    remove(tf_path); // clean up
    printf("  [PASS] Transfer outbound transaction fee calculation, application, and override logs verified.\n");

    // 58. Test Hogan Multi-Epoch Simulation and RAMAC Disk Mapping Integration
    printf("[E2E] Testing Hogan Multi-Epoch Simulation & RAMAC Disk Mapping...\n");
    hogan_umbrella_system sim_sys;
    tsfi_hogan_init(&sim_sys);
    assert(tsfi_hogan_register_account(&sim_sys, 1001, 5000) == 0);
    assert(tsfi_hogan_register_account(&sim_sys, 2002, 1000) == 0);
    
    sim_sys.accounts[0].min_balance = 1000;
    sim_sys.accounts[0].min_balance_fee_waive_threshold = 2000;
    sim_sys.accounts[0].card_expiry_epoch = 5;
    
    tsfi_ramac_record *ramac_disk = (tsfi_ramac_record*)calloc(RAMAC_CYLINDERS * RAMAC_HEADS * RAMAC_SECTORS, sizeof(tsfi_ramac_record));
    assert(ramac_disk != NULL);
    
    for (uint32_t epoch = 1; epoch <= 10; epoch++) {
        if (epoch <= 3) {
            uint64_t tx_amount = 100 * epoch;
            assert(tsfi_hogan_dispatch_tx(&sim_sys, 1001, 2002, tx_amount, VM_RAMAC) == 0);
        } else if (epoch == 4) {
            assert(tsfi_hogan_apply_fees(&sim_sys, "hogan_fees.dat.bin", 50) == 0);
            remove("hogan_fees.dat.bin");
        } else if (epoch == 6) {
            assert(tsfi_hogan_authorize_card(&sim_sys, "hogan_tx.dat.bin", 5005, 1001, 9009, 100) == -7);
            remove("hogan_tx.dat.bin");
        } else if (epoch == 7) {
            assert(tsfi_hogan_update_card_expiry(&sim_sys, "hogan_expiry.dat.bin", 1001, 10, 999) == 0);
            remove("hogan_expiry.dat.bin");
            assert(tsfi_hogan_authorize_card(&sim_sys, "hogan_tx.dat.bin", 5005, 1001, 9009, 100) == 0);
            remove("hogan_tx.dat.bin");
        } else if (epoch == 8) {
            assert(tsfi_hogan_update_deposit_limit(&sim_sys, "hogan_dep.dat.bin", 2002, 50, 999) == 0);
            remove("hogan_dep.dat.bin");
            assert(tsfi_hogan_dispatch_tx(&sim_sys, 1001, 2002, 100, VM_RAMAC) == 0);
        } else if (epoch == 9) {
            sim_sys.accounts[1].dormancy_threshold_epochs = 3;
            sim_sys.accounts[1].last_activity_epoch = 5;
        } else if (epoch == 10) {
            assert(tsfi_hogan_reactivate_dormant_account(&sim_sys, "hogan_react.dat.bin", 2002, 999) == 0);
            remove("hogan_react.dat.bin");
        }

        assert(tsfi_hogan_overnight_reconciliation(&sim_sys, "hogan_lfs.dat.bin") == 0);
        remove("hogan_lfs.dat.bin");
        
        if (epoch == 8) {
            assert(sim_sys.accounts[0].balance == 4350);
            assert(sim_sys.accounts[1].balance == 1550);
        }
        
        char key_alice[32], val_alice[32];
        char key_bob[32], val_bob[32];
        snprintf(key_alice, sizeof(key_alice), "acc_1001_epoch_%d", epoch);
        snprintf(val_alice, sizeof(val_alice), "%lu", sim_sys.accounts[0].balance);
        snprintf(key_bob, sizeof(key_bob), "acc_2002_epoch_%d", epoch);
        snprintf(val_bob, sizeof(val_bob), "%lu", sim_sys.accounts[1].balance);
        
        double seek1 = 0, seek2 = 0;
        int slot1 = tsfi_ramac_insert_record(ramac_disk, key_alice, val_alice, epoch, &seek1);
        int slot2 = tsfi_ramac_insert_record(ramac_disk, key_bob, val_bob, epoch, &seek2);
        assert(slot1 >= 0);
        assert(slot2 >= 0);
    }
    free(ramac_disk);
    printf("  [PASS] Hogan multi-epoch simulation and RAMAC disk layout mapping completed successfully.\n");

    // 59. Test Hogan ZMM VM State Machine Integration
    printf("[E2E] Testing Hogan ZMM VM State Machine Integration...\n");
    // Clean previous storage state files
    remove("evm_storage.dat.bin");
    remove("tsfi2-deepseek/evm_storage.dat.bin");
    
    tsfi_wire_firmware_init();
    
    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);
    
    const char *yul_path = "solidity/bin/ramacSystem.yul";
    FILE *f_yul = fopen(yul_path, "r");
    if (f_yul) {
        fclose(f_yul);
    } else {
        yul_path = "../solidity/bin/ramacSystem.yul";
    }
    
    char init_cmd[256];
    snprintf(init_cmd, sizeof(init_cmd), "YULINIT \"ramacSystem\", \"%s\", 1024", yul_path);
    tsfi_zmm_vm_exec(&vm, init_cmd);
    
    // Execute WRT Inquiry key_test_123 -> val_test_999
    const char *wrt_cmd = "YULEXEC \"ramacSystem\", \"e28e404f"
                          "0000000000000000000000000000000000000000000000000000000000000020"
                          "0000000000000000000000000000000000000000000000000000000000000044"
                          "57525420"
                          "6b65795f746573745f3132330000000000000000000000000000000000000000"
                          "76616c5f746573745f3939390000000000000000000000000000000000000000\"";
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, wrt_cmd);
    
    // Execute QRY Inquiry to retrieve key_test_123
    const char *qry_cmd = "YULEXEC \"ramacSystem\", \"e28e404f"
                          "0000000000000000000000000000000000000000000000000000000000000020"
                          "0000000000000000000000000000000000000000000000000000000000000024"
                          "51525920"
                          "6b65795f746573745f3132330000000000000000000000000000000000000000\"";
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, qry_cmd);
    assert(strstr(vm.output_buffer, "76616c5f746573745f393939") != NULL);
    
    // WinchesterMQ Socket Bridge Telemetry loops to ZMM
    tsfi_winchester_socket_bridge route_bridge;
    tsfi_winchester_socket_init(&route_bridge, 9999);
    
    LauTelemetryState route_telem;
    memset(&route_telem, 0, sizeof(route_telem));
    vm.telem = &route_telem;
    
    // Verify keycode 32 (d/D) is routed over Auncient virtual SCSI hardware loopback bridge
    uint8_t route_packet[1] = { 32 };
    int route_res = tsfi_winchester_socket_route_to_zmm(&route_bridge, route_packet, 1, &vm);
    assert(route_res == 0);
    assert(route_telem.zmm_val == 32);
    
    tsfi_zmm_vm_destroy(&vm);
    remove("evm_storage.dat.bin");
    remove("tsfi2-deepseek/evm_storage.dat.bin");
    printf("  [PASS] Auncient ZMM VM Yul execution, dynamic contract query, and WinchesterMQ hardware handshakes verified.\n");

    // 60. Test PEP ACH Direct Deposit, ILS Loans, DDO Sweeper, and HSM PIN verification
    printf("[E2E] Testing Hogan PEP ACH, ILS Loans, DDO Sweeper, and HSM verification...\n");
    hogan_umbrella_system test_sys;
    tsfi_hogan_init(&test_sys);
    
    // Register test accounts
    assert(tsfi_hogan_register_account(&test_sys, 7001, 1000) == 0);
    assert(tsfi_hogan_register_account(&test_sys, 7002, 500) == 0);
    assert(tsfi_hogan_register_account(&test_sys, 7003, 5000) == 0);
    
    // 1. ACH direct deposit registration
    assert(tsfi_hogan_register_ach_direct_deposit(&test_sys, 7001, 987654, 300, 2) == 0);
    
    // 2. Setup Loan
    assert(tsfi_hogan_setup_loan(&test_sys, 7002, 2000, 1000, 500, 5) == 0); // principal 2000, interest 10%, payment 500, 5 epochs
    
    // 3. Verify HSM PIN
    assert(tsfi_hogan_verify_hsm_pin_block(&test_sys, 7001, 1234, 1234 ^ 0xABCDEF, 0xABCDEF) == 0);
    assert(tsfi_hogan_verify_hsm_pin_block(&test_sys, 7001, 9999, 1234 ^ 0xABCDEF, 0xABCDEF) == -2);
    
    // Overnight reconciliation passes to advance epochs and trigger batch integration processors
    // Epoch 1 -> 2
    char lfs_temp[] = "test_temp_lfs.dat.bin";
    remove(lfs_temp);
    assert(tsfi_hogan_overnight_reconciliation_ex(&test_sys, lfs_temp, NULL) == 0);
    
    // Run reconciliation again to clear ACH at clearing_epoch 2
    assert(tsfi_hogan_overnight_reconciliation_ex(&test_sys, lfs_temp, NULL) == 0);
    
    assert(test_sys.accounts[0].balance == 1300); // 7001
    assert(test_sys.accounts[1].balance == 0);    // 7002
    
    // 4. Add Sweep Rule now that initial setup and deposit is completed
    assert(tsfi_hogan_add_sweep_rule(&test_sys, 7003, 7001, 1200, 1500) == 0); // source 7003, target 7001, trigger 1200, target_bal 1500

    // Let's transfer 200 out of 7001 to drop it below 1200
    test_sys.accounts[0].balance = 1100;
    
    // Run overnight reconciliation epoch 3:
    assert(tsfi_hogan_overnight_reconciliation_ex(&test_sys, lfs_temp, NULL) == 0);
    assert(test_sys.accounts[0].balance == 1500);
    assert(test_sys.accounts[2].balance == 4600);
    
    remove(lfs_temp);
    printf("  [PASS] Hogan PEP ACH, ILS Loans, DDO Sweeps, and HSM PIN verification E2E verified.\n");

    // 5. Test RAMAC cylinder seek emulator
    tsfi_ramac_record *test_ramac = (tsfi_ramac_record*)calloc(RAMAC_CYLINDERS * RAMAC_HEADS * RAMAC_SECTORS, sizeof(tsfi_ramac_record));
    assert(test_ramac != NULL);
    double latency = tsfi_hogan_simulate_ramac_lfs_latency(&test_sys, test_ramac);
    assert(latency > 0.0);
    free(test_ramac);
    printf("  [PASS] RAMAC Cylinder Seek/Latency Emulator verified.\n");

    // 6. Test APPC/SNA LU6.2 Transaction Propagation
    uint8_t appc_buf[512];
    size_t appc_len = 0;
    assert(tsfi_hogan_propagate_appc_transaction(&test_sys, 7001, 7002, 250, appc_buf, &appc_len) == 0);
    assert(appc_len > sizeof(tsfi_sna_fmh));
    printf("  [PASS] APPC/SNA LU6.2 Transaction Propagation verified.\n");

    // 7. Test HSM PIN Translation & CVV Matrices
    char cvv_out[4];
    assert(tsfi_hogan_hsm_translate_pin_and_cvv(&test_sys, 7001, 45, "CVV_MASTER_KEY", cvv_out) == 0);
    assert(strlen(cvv_out) == 3);
    printf("  [PASS] HSM PIN Translation & CVV Matrices verified.\n");

    // 8. Test Vulkan Batch Status Console
    char batch_console[512];
    assert(tsfi_hogan_render_vulkan_batch_status(&test_sys, batch_console, sizeof(batch_console)) == 0);
    assert(strstr(batch_console, "HOGAN BATCH STATUS") != NULL);
    printf("  [PASS] Vulkan Batch Status Console layout verified.\n");

    // 9. Test Double-Entry General Ledger (GL) Reconciliation
    int64_t gl_variance = 0;
    assert(tsfi_hogan_gl_reconciliation(&test_sys, "gl_reconciliation_report.dat.bin", &gl_variance) == 0);
    remove("gl_reconciliation_report.dat.bin");
    printf("  [PASS] Double-Entry GL Reconciliation verified.\n");

    // 10. Test FDIC Insurance Warning System
    char alert_buf[1024];
    test_sys.accounts[0].balance = 300000; // Above 250,000 threshold
    assert(tsfi_hogan_fdic_insurance_warnings(&test_sys, alert_buf, sizeof(alert_buf)) == 0);
    assert(strstr(alert_buf, "[FDIC WARNING]") != NULL);
    printf("  [PASS] FDIC Insurance Warning System verified.\n");

    // 11. Test COBOL Copybook Serializer/Parser
    uint8_t copybook_buf[64];
    hogan_account test_acc;
    memset(&test_acc, 0, sizeof(hogan_account));
    test_acc.account_id = 99123;
    test_acc.balance = 56789;
    test_acc.currency_code = 2; // GBP
    test_acc.card_expiry_epoch = 123456;
    
    int cobol_len = tsfi_hogan_to_cobol_copybook(&test_acc, copybook_buf, sizeof(copybook_buf));
    assert(cobol_len == 17);
    
    hogan_account parsed_acc;
    memset(&parsed_acc, 0, sizeof(parsed_acc));
    assert(tsfi_hogan_from_cobol_copybook(&parsed_acc, copybook_buf, cobol_len) == 0);
    assert(parsed_acc.account_id == 99123);
    assert(parsed_acc.balance == 56789);
    assert(parsed_acc.currency_code == 2);
    assert(parsed_acc.card_expiry_epoch == 123456);
    printf("  [PASS] COBOL Copybook Serializer/Parser verified.\n");

    // 12. Test Compound Interest & Daily Limit Alerts
    test_sys.accounts[0].balance = 10000;
    test_sys.accounts[0].has_custom_rate = 1;
    test_sys.accounts[0].custom_interest_rate_bps = 250; // 2.5%
    test_sys.accounts[0].daily_transferred = 60000; // Trigger alert threshold (>50,000)

    tsfi_hogan_process_integrations_overnight(&test_sys);
    
    // Balance should have interest added: 10000 + 250 = 10250
    assert(test_sys.accounts[0].balance == 10250);
    printf("  [PASS] Compound Interest & Daily Limit Alerts verified.\n");

    printf("ALL HOGAN SYSTEMS E2E C TESTS COMPLETED SUCCESSFULLY!\n");
    return 0;
}
