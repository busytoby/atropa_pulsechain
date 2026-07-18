#include "tsfi_hogan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int tsfi_hogan_write_journal(const char *filepath, const hogan_transaction *tx) {
    return tsfi_hogan_write_seq_record(filepath, (const uint8_t *)tx, sizeof(hogan_transaction));
}

int tsfi_hogan_replay_journal(hogan_umbrella_system *sys, const char *filepath) {
    uint8_t buf[sizeof(hogan_transaction)];
    size_t size = 0;
    size_t index = 0;
    
    // Disable live queue during replay recovery
    uint8_t original_live_state = sys->live_processing_enabled;
    sys->live_processing_enabled = 0;
    
    while (tsfi_hogan_read_seq_record(filepath, index, buf, &size) == 0) {
        if (size != sizeof(hogan_transaction)) {
            sys->live_processing_enabled = original_live_state;
            return -2; // Corrupt record size
        }
        
        const hogan_transaction *tx = (const hogan_transaction *)buf;
        
        // Find sender and recipient accounts and apply transaction directly
        hogan_account *sender = NULL;
        hogan_account *recipient = NULL;
        for (int j = 0; j < HOGAN_MAX_ACCOUNTS; j++) {
            if (sys->accounts[j].active) {
                if (sys->accounts[j].account_id == tx->sender_id) sender = &sys->accounts[j];
                if (sys->accounts[j].account_id == tx->recipient_id) recipient = &sys->accounts[j];
            }
        }
        
        if (sender && recipient && sender->balance >= tx->amount) {
            sender->balance -= tx->amount;
            recipient->balance += tx->amount;
        }
        
        index++;
    }
    
    sys->live_processing_enabled = original_live_state;
    return 0;
}

int tsfi_hogan_write_audit_log(const char *filepath, uint32_t epoch, uint32_t account_id, const char *activity_type, uint8_t status) {
    hogan_audit_entry entry;
    entry.epoch = epoch;
    entry.account_id = account_id;
    strncpy(entry.activity_type, activity_type, sizeof(entry.activity_type) - 1);
    entry.activity_type[sizeof(entry.activity_type) - 1] = '\0';
    entry.status = status;
    
    return tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_audit_entry));
}

int tsfi_hogan_print_audit_trail(const char *filepath, size_t *entries_count_out) {
    uint8_t buf[sizeof(hogan_audit_entry)];
    size_t size = 0;
    size_t index = 0;
    
    printf("--- HOGAN SYSTEMS MAINFRAME AUDIT TRAIL REPORT ---\n");
    while (tsfi_hogan_read_seq_record(filepath, index, buf, &size) == 0) {
        if (size != sizeof(hogan_audit_entry)) {
            return -2; // Corrupt record size
        }
        
        const hogan_audit_entry *entry = (const hogan_audit_entry *)buf;
        printf("  [Epoch %03u] Account: %04u | Activity: %-12s | Status: %s\n",
               entry->epoch, entry->account_id, entry->activity_type,
               entry->status ? "SUCCESS" : "FAILED");
               
        index++;
    }
    
    *entries_count_out = index;
    return 0;
}

int tsfi_hogan_write_ledger_entry(const char *filepath, uint32_t account_id, uint64_t amount, uint8_t is_credit, const char *description) {
    hogan_ledger_entry entry;
    entry.account_id = account_id;
    entry.amount = amount;
    entry.is_credit = is_credit;
    strncpy(entry.description, description, sizeof(entry.description) - 1);
    entry.description[sizeof(entry.description) - 1] = '\0';
    
    return tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_ledger_entry));
}

int tsfi_hogan_print_statement(const char *filepath, uint32_t account_id, size_t *entries_count_out) {
    uint8_t buf[sizeof(hogan_ledger_entry)];
    size_t size = 0;
    size_t index = 0;
    size_t matches = 0;
    
    printf("--- ACCOUNT LEDGER STATEMENT REPORT FOR ACC: %04u ---\n", account_id);
    while (tsfi_hogan_read_seq_record(filepath, index, buf, &size) == 0) {
        if (size != sizeof(hogan_ledger_entry)) {
            return -2; // Corrupt record size
        }
        
        const hogan_ledger_entry *entry = (const hogan_ledger_entry *)buf;
        if (entry->account_id == account_id) {
            printf("  [Tx %03lu] Type: %-6s | Amount: %-6lu | Desc: %s\n",
                   matches + 1, entry->is_credit ? "CREDIT" : "DEBIT",
                   entry->amount, entry->description);
            matches++;
        }
        index++;
    }
    
    *entries_count_out = matches;
    return 0;
}
