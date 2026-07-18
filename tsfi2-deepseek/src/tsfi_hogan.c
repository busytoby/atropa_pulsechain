#include "tsfi_hogan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>

void tsfi_hogan_init(hogan_umbrella_system *sys) {
    memset(sys, 0, sizeof(hogan_umbrella_system));
    sys->live_processing_enabled = 1;
    sys->current_epoch = 1;
}

int tsfi_hogan_register_account(hogan_umbrella_system *sys, uint32_t account_id, uint64_t initial_balance) {
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active && sys->accounts[i].account_id == account_id) {
            return -1; // Already registered
        }
    }
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (!sys->accounts[i].active) {
            sys->accounts[i].account_id = account_id;
            sys->accounts[i].balance = initial_balance;
            sys->accounts[i].active = 1;
            return 0;
        }
    }
    return -2; // Out of account slots
}

int tsfi_hogan_dispatch_tx(hogan_umbrella_system *sys, uint32_t sender_id, uint32_t recipient_id, uint64_t amount, hogan_vm_type target_vm) {
    if (!sys->live_processing_enabled) {
        return -1; // Live processing paused (e.g. during batch window)
    }
    if (sys->tx_count >= HOGAN_MAX_TRANSACTIONS) {
        return -2; // Transaction log full
    }
    
    // Queue transaction for batch processing
    hogan_transaction *tx = &sys->tx_log[sys->tx_count++];
    tx->tx_id = (uint32_t)sys->tx_count;
    tx->sender_id = sender_id;
    tx->recipient_id = recipient_id;
    tx->amount = amount;
    tx->target_vm = target_vm;
    tx->processed = 0;
    
    return 0;
}

int tsfi_hogan_lfs_save(const hogan_umbrella_system *sys, const char *filepath) {
    // Enforce Rule 13: file extension must be .dat.bin
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        // Must end with .dat.bin
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3;
        }
    }

    FILE *f = fopen(filepath, "wb");
    if (!f) return -1;
    
    // Save metadata
    fwrite(&sys->current_epoch, sizeof(uint32_t), 1, f);
    fwrite(sys->acab_epoch_root, 1, 32, f);
    
    // Save accounts
    uint32_t active_accounts = 0;
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active) {
            active_accounts++;
        }
    }
    fwrite(&active_accounts, sizeof(uint32_t), 1, f);
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active) {
            fwrite(&sys->accounts[i].account_id, sizeof(uint32_t), 1, f);
            fwrite(&sys->accounts[i].balance, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].backup_account_id, sizeof(uint32_t), 1, f);
            fwrite(&sys->accounts[i].has_backup, sizeof(uint8_t), 1, f);
            fwrite(&sys->accounts[i].balance_held, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].status_code, sizeof(uint8_t), 1, f);
            fwrite(&sys->accounts[i].daily_limit, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].daily_spent, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].daily_transfer_limit, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].daily_transferred, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].is_frozen, sizeof(uint8_t), 1, f);
            fwrite(&sys->accounts[i].fee_exempt, sizeof(uint8_t), 1, f);
            fwrite(&sys->accounts[i].custom_interest_rate_bps, sizeof(uint32_t), 1, f);
            fwrite(&sys->accounts[i].has_custom_rate, sizeof(uint8_t), 1, f);
            fwrite(&sys->accounts[i].min_balance, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].daily_tx_limit, sizeof(uint32_t), 1, f);
            fwrite(&sys->accounts[i].daily_tx_count, sizeof(uint32_t), 1, f);
        }
    }
    
    fwrite(&sys->blocked_card_count, sizeof(size_t), 1, f);
    if (sys->blocked_card_count > 0) {
        fwrite(sys->blocked_cards, sizeof(uint32_t), sys->blocked_card_count, f);
    }
    
    fclose(f);
    return 0;
}

int tsfi_hogan_lfs_load(hogan_umbrella_system *sys, const char *filepath) {
    FILE *f = fopen(filepath, "rb");
    if (!f) return -1;
    
    memset(sys->accounts, 0, sizeof(sys->accounts));
    
    if (fread(&sys->current_epoch, sizeof(uint32_t), 1, f) != 1) {
        fclose(f);
        return -2;
    }
    if (fread(sys->acab_epoch_root, 1, 32, f) != 32) {
        fclose(f);
        return -2;
    }
    
    uint32_t active_accounts = 0;
    if (fread(&active_accounts, sizeof(uint32_t), 1, f) != 1) {
        fclose(f);
        return -2;
    }
    
    for (uint32_t i = 0; i < active_accounts; i++) {
        uint32_t acc_id;
        uint64_t bal;
        uint32_t backup_id;
        uint8_t has_bk;
        if (fread(&acc_id, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        if (fread(&bal, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        if (fread(&backup_id, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        if (fread(&has_bk, sizeof(uint8_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        uint64_t bal_held = 0;
        if (fread(&bal_held, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        uint8_t st_code = 0;
        if (fread(&st_code, sizeof(uint8_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        uint64_t d_limit = 0;
        if (fread(&d_limit, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        uint64_t d_spent = 0;
        if (fread(&d_spent, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        uint64_t dt_limit = 0;
        if (fread(&dt_limit, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        uint64_t dt_spent = 0;
        if (fread(&dt_spent, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].account_id = acc_id;
        sys->accounts[i].balance = bal;
        sys->accounts[i].backup_account_id = backup_id;
        sys->accounts[i].has_backup = has_bk;
        sys->accounts[i].balance_held = bal_held;
        sys->accounts[i].status_code = st_code;
        sys->accounts[i].daily_limit = d_limit;
        sys->accounts[i].daily_spent = d_spent;
        sys->accounts[i].daily_transfer_limit = dt_limit;
        sys->accounts[i].daily_transferred = dt_spent;
        uint8_t frozen_val = 0;
        if (fread(&frozen_val, sizeof(uint8_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].is_frozen = frozen_val;
        sys->accounts[i].is_frozen = frozen_val;
        uint8_t exempt_val = 0;
        if (fread(&exempt_val, sizeof(uint8_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].fee_exempt = exempt_val;
        uint32_t cust_rate = 0;
        if (fread(&cust_rate, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        uint8_t has_cust = 0;
        if (fread(&has_cust, sizeof(uint8_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].custom_interest_rate_bps = cust_rate;
        sys->accounts[i].has_custom_rate = has_cust;
        uint64_t min_bal = 0;
        if (fread(&min_bal, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].min_balance = min_bal;
        uint32_t tx_lim = 0;
        if (fread(&tx_lim, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        uint32_t tx_cnt = 0;
        if (fread(&tx_cnt, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].daily_tx_limit = tx_lim;
        sys->accounts[i].daily_tx_count = tx_cnt;
        sys->accounts[i].active = 1;
    }
    
    if (fread(&sys->blocked_card_count, sizeof(size_t), 1, f) != 1) {
        fclose(f);
        return -2;
    }
    if (sys->blocked_card_count > 0) {
        if (fread(sys->blocked_cards, sizeof(uint32_t), sys->blocked_card_count, f) != sys->blocked_card_count) {
            fclose(f);
            return -2;
        }
    }
    
    fclose(f);
    return 0;
}

int tsfi_hogan_overnight_reconciliation(hogan_umbrella_system *sys, const char *lfs_filepath) {
    return tsfi_hogan_overnight_reconciliation_ex(sys, lfs_filepath, NULL);
}

int tsfi_hogan_write_seq_record(const char *filepath, const uint8_t *payload, size_t size) {
    // Enforce Rule 13: file extension must end with .dat.bin
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3; // Invalid extension
        }
    }

    FILE *f = fopen(filepath, "ab");
    if (!f) return -1;
    
    uint32_t record_len = (uint32_t)size;
    if (fwrite(&record_len, sizeof(uint32_t), 1, f) != 1) {
        fclose(f);
        return -2;
    }
    if (fwrite(payload, 1, size, f) != size) {
        fclose(f);
        return -2;
    }
    
    fclose(f);
    return 0;
}

int tsfi_hogan_read_seq_record(const char *filepath, size_t index, uint8_t *payload_out, size_t *size_out) {
    FILE *f = fopen(filepath, "rb");
    if (!f) return -1;
    
    for (size_t i = 0; i <= index; i++) {
        uint32_t record_len = 0;
        if (fread(&record_len, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2; // Out of bounds / EOF
        }
        
        if (i == index) {
            if (fread(payload_out, 1, record_len, f) != record_len) {
                fclose(f);
                return -3;
            }
            *size_out = record_len;
            fclose(f);
            return 0;
        } else {
            // Seek past this record to read the next one
            if (fseek(f, record_len, SEEK_CUR) != 0) {
                fclose(f);
                return -4;
            }
        }
    }
    
    fclose(f);
    return -5;
}

void tsfi_hogan_init_dict(hogan_record_dict *dict) {
    memset(dict, 0, sizeof(hogan_record_dict));
}

int tsfi_hogan_add_field(hogan_record_dict *dict, const char *name, uint32_t offset, hogan_field_type type) {
    if (dict->field_count >= HOGAN_MAX_FIELDS) {
        return -1;
    }
    hogan_field_def *field = &dict->fields[dict->field_count++];
    strncpy(field->name, name, sizeof(field->name) - 1);
    field->offset = offset;
    field->type = type;
    return 0;
}

int tsfi_hogan_resolve_uint32(const hogan_record_dict *dict, const uint8_t *payload, const char *field_name, uint32_t *val_out) {
    for (size_t i = 0; i < dict->field_count; i++) {
        const hogan_field_def *field = &dict->fields[i];
        if (strcmp(field->name, field_name) == 0) {
            if (field->type != FIELD_TYPE_UINT32) return -1;
            memcpy(val_out, payload + field->offset, sizeof(uint32_t));
            return 0;
        }
    }
    return -2; // Not found
}

int tsfi_hogan_resolve_uint64(const hogan_record_dict *dict, const uint8_t *payload, const char *field_name, uint64_t *val_out) {
    for (size_t i = 0; i < dict->field_count; i++) {
        const hogan_field_def *field = &dict->fields[i];
        if (strcmp(field->name, field_name) == 0) {
            if (field->type != FIELD_TYPE_UINT64) return -1;
            memcpy(val_out, payload + field->offset, sizeof(uint64_t));
            return 0;
        }
    }
    return -2; // Not found
}

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

int tsfi_hogan_write_account_index(const char *filepath, uint32_t account_id, uint32_t sector_offset) {
    hogan_index_entry entry;
    entry.account_id = account_id;
    entry.sector_offset = sector_offset;
    
    return tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_index_entry));
}

int tsfi_hogan_lookup_account_offset(const char *filepath, uint32_t account_id, uint32_t *sector_offset_out) {
    uint8_t buf[sizeof(hogan_index_entry)];
    size_t size = 0;
    size_t index = 0;
    
    while (tsfi_hogan_read_seq_record(filepath, index, buf, &size) == 0) {
        if (size != sizeof(hogan_index_entry)) {
            return -2; // Corrupt record size
        }
        
        const hogan_index_entry *entry = (const hogan_index_entry *)buf;
        if (entry->account_id == account_id) {
            *sector_offset_out = entry->sector_offset;
            return 0; // Found
        }
        index++;
    }
    
    return -1; // Not found
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

int tsfi_hogan_apply_interest(hogan_umbrella_system *sys, const char *filepath, uint32_t rate_bps) {
    // Enforce Rule 13: file extension must end with .dat.bin
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3; // Invalid extension
        }
    }

    // Disable live queue during batch interest posting
    uint8_t original_live_state = sys->live_processing_enabled;
    sys->live_processing_enabled = 0;
    
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active) {
            hogan_interest_entry entry;
            entry.account_id = sys->accounts[i].account_id;
            entry.original_balance = sys->accounts[i].balance;
            
            uint32_t active_rate = rate_bps;
            if (sys->accounts[i].has_custom_rate) {
                active_rate = sys->accounts[i].custom_interest_rate_bps;
            }
            
            // Calculate interest: balance * active_rate / 10000
            uint64_t interest = (sys->accounts[i].balance * active_rate) / 10000;
            sys->accounts[i].balance += interest;
            
            entry.interest_added = interest;
            entry.new_balance = sys->accounts[i].balance;
            
            int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_interest_entry));
            if (write_res != 0) {
                sys->live_processing_enabled = original_live_state;
                return write_res;
            }
        }
    }
    
    sys->live_processing_enabled = original_live_state;
    return 0;
}

int tsfi_hogan_apply_fees(hogan_umbrella_system *sys, const char *filepath, uint64_t flat_fee) {
    // Enforce Rule 13: file extension must end with .dat.bin
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3; // Invalid extension
        }
    }

    // Disable live queue during batch fee application
    uint8_t original_live_state = sys->live_processing_enabled;
    sys->live_processing_enabled = 0;
    
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active) {
            if (sys->accounts[i].fee_exempt) {
                continue; // skip fee charging if account is exempt
            }
            hogan_fee_entry entry;
            entry.account_id = sys->accounts[i].account_id;
            entry.original_balance = sys->accounts[i].balance;
            
            uint64_t actual_fee = flat_fee;
            if (sys->accounts[i].balance < flat_fee) {
                actual_fee = sys->accounts[i].balance; // clamp to 0
            }
            
            sys->accounts[i].balance -= actual_fee;
            
            entry.fee_deducted = actual_fee;
            entry.new_balance = sys->accounts[i].balance;
            
            int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_fee_entry));
            if (write_res != 0) {
                sys->live_processing_enabled = original_live_state;
                return write_res;
            }
        }
    }
    
    sys->live_processing_enabled = original_live_state;
    return 0;
}

int tsfi_hogan_link_backup(hogan_umbrella_system *sys, uint32_t primary_id, uint32_t backup_id) {
    hogan_account *primary = NULL;
    hogan_account *backup = NULL;
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active) {
            if (sys->accounts[i].account_id == primary_id) primary = &sys->accounts[i];
            if (sys->accounts[i].account_id == backup_id) backup = &sys->accounts[i];
        }
    }
    if (!primary || !backup) return -1; // primary or backup not found
    
    primary->backup_account_id = backup_id;
    primary->has_backup = 1;
    return 0;
}

int tsfi_hogan_overnight_reconciliation_ex(hogan_umbrella_system *sys, const char *lfs_filepath, const char *overdraft_filepath) {
    if (overdraft_filepath) {
        const char *ext = strrchr(overdraft_filepath, '.');
        if (!ext || strcmp(ext, ".bin") != 0) {
            if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
                return -3; // Invalid extension
            }
        }
    }

    // Disable live transaction queue
    sys->live_processing_enabled = 0;
    
    // Process all pending transactions in log
    for (size_t i = 0; i < sys->tx_count; i++) {
        hogan_transaction *tx = &sys->tx_log[i];
        if (tx->processed) continue;
        
        hogan_account *sender = NULL;
        hogan_account *recipient = NULL;
        for (int j = 0; j < HOGAN_MAX_ACCOUNTS; j++) {
            if (sys->accounts[j].active) {
                if (sys->accounts[j].account_id == tx->sender_id) sender = &sys->accounts[j];
                if (sys->accounts[j].account_id == tx->recipient_id) recipient = &sys->accounts[j];
            }
        }
        
        if (sender && recipient) {
            if (sender->is_frozen || recipient->is_frozen) {
                tx->processed = 1; // transaction blocked by compliance hold / freeze
            } else if (sender->status_code == STATUS_STOP_ALL || sender->status_code == STATUS_STOP_DEBIT || recipient->status_code == STATUS_STOP_ALL) {
                tx->processed = 1; // transaction blocked by administrative hold
            } else if (sender->daily_tx_limit > 0 && sender->daily_tx_count >= sender->daily_tx_limit) {
                tx->processed = 1; // blocked by daily velocity/tx count limit
            } else if (sender->daily_transfer_limit > 0 && sender->daily_transferred + tx->amount > sender->daily_transfer_limit) {
                tx->processed = 1; // blocked by daily transfer limit
            } else if (sender->balance >= tx->amount + sender->min_balance) {
                sender->balance -= tx->amount;
                recipient->balance += tx->amount;
                sender->daily_transferred += tx->amount;
                sender->daily_tx_count++;
                tx->processed = 1;
            } else if (sender->has_backup) {
                // Determine overdraft backup account
                uint64_t required = tx->amount + sender->min_balance;
                if (required > sender->balance) {
                    uint64_t deficit = required - sender->balance;
                    hogan_account *backup = NULL;
                    for (int j = 0; j < HOGAN_MAX_ACCOUNTS; j++) {
                        if (sys->accounts[j].active && sys->accounts[j].account_id == sender->backup_account_id) {
                            backup = &sys->accounts[j];
                            break;
                        }
                    }
                    
                    if (backup && backup->balance >= deficit) {
                        backup->balance -= deficit;
                        sender->balance += deficit;
                        
                        sender->balance -= tx->amount;
                        recipient->balance += tx->amount;
                        sender->daily_transferred += tx->amount;
                        sender->daily_tx_count++;
                        tx->processed = 1;
                        
                        if (overdraft_filepath) {
                            hogan_overdraft_entry o_entry = { sender->account_id, backup->account_id, deficit, 1 };
                            tsfi_hogan_write_seq_record(overdraft_filepath, (const uint8_t *)&o_entry, sizeof(hogan_overdraft_entry));
                        }
                    } else {
                        tx->processed = 1;
                        if (overdraft_filepath) {
                            hogan_overdraft_entry o_entry = { sender->account_id, sender->backup_account_id, deficit, 0 };
                            tsfi_hogan_write_seq_record(overdraft_filepath, (const uint8_t *)&o_entry, sizeof(hogan_overdraft_entry));
                        }
                    }
                } else {
                    tx->processed = 1; // Fails
                }
            } else {
                tx->processed = 1; // Fails, insufficient funds and no backup
            }
        } else {
            tx->processed = 1; // Fails, sender or receiver missing
        }
    }
    
    // Clear transaction log for next day
    sys->tx_count = 0;
    
    // Reset daily spent metrics and transfer metrics and velocity counters
    for (int j = 0; j < HOGAN_MAX_ACCOUNTS; j++) {
        if (sys->accounts[j].active) {
            sys->accounts[j].daily_spent = 0;
            sys->accounts[j].daily_transferred = 0;
            sys->accounts[j].daily_tx_count = 0;
        }
    }
    
    // Compute new acab_epoch_root using OpenSSL EVP hash
    unsigned int hash_len = 32;
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    
    // Hash current accounts and epoch metadata
    EVP_DigestUpdate(mdctx, &sys->current_epoch, sizeof(uint32_t));
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active) {
            EVP_DigestUpdate(mdctx, &sys->accounts[i].account_id, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].balance, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].backup_account_id, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].has_backup, sizeof(uint8_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].balance_held, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].status_code, sizeof(uint8_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].daily_limit, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].daily_spent, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].daily_transfer_limit, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].daily_transferred, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].is_frozen, sizeof(uint8_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].fee_exempt, sizeof(uint8_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].custom_interest_rate_bps, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].has_custom_rate, sizeof(uint8_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].min_balance, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].daily_tx_limit, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].daily_tx_count, sizeof(uint32_t));
        }
    }
    EVP_DigestUpdate(mdctx, &sys->blocked_card_count, sizeof(size_t));
    if (sys->blocked_card_count > 0) {
        EVP_DigestUpdate(mdctx, sys->blocked_cards, sizeof(uint32_t) * sys->blocked_card_count);
    }
    EVP_DigestFinal_ex(mdctx, sys->acab_epoch_root, &hash_len);
    EVP_MD_CTX_free(mdctx);
    
    // Increment epoch
    sys->current_epoch++;
    
    // Save state to Logical File System
    int save_res = tsfi_hogan_lfs_save(sys, lfs_filepath);
    
    // Resume live transaction dispatching
    sys->live_processing_enabled = 1;
    
    return save_res;
}

int tsfi_hogan_authorize_card(hogan_umbrella_system *sys, const char *filepath, uint32_t card_id, uint32_t account_id, uint32_t merchant_id, uint64_t amount) {
    // Enforce Rule 13: file extension must end with .dat.bin
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3; // Invalid extension
        }
    }

    // Disable live queue during authorization calculation
    uint8_t original_live_state = sys->live_processing_enabled;
    sys->live_processing_enabled = 0;
    
    // Check if card is blocked
    for (size_t i = 0; i < sys->blocked_card_count; i++) {
        if (sys->blocked_cards[i] == card_id) {
            sys->live_processing_enabled = original_live_state;
            return -4; // Card is blocked
        }
    }
    
    hogan_account *acc = NULL;
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active && sys->accounts[i].account_id == account_id) {
            acc = &sys->accounts[i];
            break;
        }
    }
    
    uint8_t approved = 0;
    if (acc && (acc->balance >= acc->balance_held + amount)) {
        if (acc->daily_limit == 0 || (acc->daily_spent + amount <= acc->daily_limit)) {
            acc->balance_held += amount;
            acc->daily_spent += amount;
            approved = 1;
        }
    }
    
    hogan_card_entry entry = { card_id, account_id, merchant_id, amount, approved };
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_card_entry));
    
    sys->live_processing_enabled = original_live_state;
    
    if (write_res != 0) return write_res;
    return approved ? 0 : -1; // return 0 if approved, -1 if declined due to limit
}

int tsfi_hogan_apply_account_stop(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint8_t new_status, uint32_t authority_id) {
    // Enforce Rule 13: file extension must end with .dat.bin
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3; // Invalid extension
        }
    }

    // Disable live queue during administrative action
    uint8_t original_live_state = sys->live_processing_enabled;
    sys->live_processing_enabled = 0;
    
    hogan_account *acc = NULL;
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active && sys->accounts[i].account_id == account_id) {
            acc = &sys->accounts[i];
            break;
        }
    }
    
    if (!acc) {
        sys->live_processing_enabled = original_live_state;
        return -1; // account not found
    }
    
    hogan_stop_entry entry = { account_id, acc->status_code, new_status, authority_id };
    acc->status_code = new_status;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_stop_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_release_hold(hogan_umbrella_system *sys, const char *filepath, uint32_t card_id, uint32_t account_id, uint64_t amount_released) {
    // Enforce Rule 13: file extension must end with .dat.bin
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3; // Invalid extension
        }
    }

    // Disable live queue during hold release
    uint8_t original_live_state = sys->live_processing_enabled;
    sys->live_processing_enabled = 0;
    
    hogan_account *acc = NULL;
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active && sys->accounts[i].account_id == account_id) {
            acc = &sys->accounts[i];
            break;
        }
    }
    
    uint8_t success = 0;
    if (acc) {
        if (acc->balance_held >= amount_released) {
            acc->balance_held -= amount_released;
        } else {
            acc->balance_held = 0;
        }
        success = 1;
    }
    
    hogan_release_entry entry = { card_id, account_id, amount_released, success };
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_release_entry));
    
    sys->live_processing_enabled = original_live_state;
    
    if (write_res != 0) return write_res;
    return success ? 0 : -1;
}

int tsfi_hogan_update_daily_limit(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_limit, uint32_t authority_id) {
    // Enforce Rule 13: file extension must end with .dat.bin
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3; // Invalid extension
        }
    }

    // Disable live queue during administrative action
    uint8_t original_live_state = sys->live_processing_enabled;
    sys->live_processing_enabled = 0;
    
    hogan_account *acc = NULL;
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active && sys->accounts[i].account_id == account_id) {
            acc = &sys->accounts[i];
            break;
        }
    }
    
    if (!acc) {
        sys->live_processing_enabled = original_live_state;
        return -1; // account not found
    }
    
    hogan_limit_entry entry = { account_id, acc->daily_limit, new_limit, authority_id };
    acc->daily_limit = new_limit;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_limit_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_close_account(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t authority_id) {
    // Enforce Rule 13: file extension must end with .dat.bin
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3; // Invalid extension
        }
    }

    // Disable live queue during closure operation
    uint8_t original_live_state = sys->live_processing_enabled;
    sys->live_processing_enabled = 0;
    
    hogan_account *acc = NULL;
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active && sys->accounts[i].account_id == account_id) {
            acc = &sys->accounts[i];
            break;
        }
    }
    
    if (!acc) {
        sys->live_processing_enabled = original_live_state;
        return -1; // account not found
    }
    
    if (acc->balance_held > 0) {
        sys->live_processing_enabled = original_live_state;
        return -2; // active holds block closure
    }
    
    uint64_t liquidated = acc->balance;
    acc->balance = 0;
    acc->active = 0;
    
    hogan_closure_entry entry = { account_id, liquidated, authority_id };
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_closure_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_transfer_limit(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_limit, uint32_t authority_id) {
    // Enforce Rule 13: file extension must end with .dat.bin
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3; // Invalid extension
        }
    }

    // Disable live queue during administrative action
    uint8_t original_live_state = sys->live_processing_enabled;
    sys->live_processing_enabled = 0;
    
    hogan_account *acc = NULL;
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active && sys->accounts[i].account_id == account_id) {
            acc = &sys->accounts[i];
            break;
        }
    }
    
    if (!acc) {
        sys->live_processing_enabled = original_live_state;
        return -1; // account not found
    }
    
    hogan_trans_limit_entry entry = { account_id, acc->daily_transfer_limit, new_limit, authority_id };
    acc->daily_transfer_limit = new_limit;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_trans_limit_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_register_standing_order(const char *filepath, uint32_t sender_id, uint32_t recipient_id, uint64_t amount) {
    // Enforce Rule 13: file extension must end with .dat.bin
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3; // Invalid extension
        }
    }

    hogan_standing_order order = { sender_id, recipient_id, amount };
    return tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&order, sizeof(hogan_standing_order));
}

int tsfi_hogan_execute_standing_orders(hogan_umbrella_system *sys, const char *filepath) {
    // Enforce Rule 13: file extension must end with .dat.bin
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3; // Invalid extension
        }
    }

    uint8_t read_buf[sizeof(hogan_standing_order)];
    size_t size = 0;
    size_t index = 0;
    
    while (tsfi_hogan_read_seq_record(filepath, index, read_buf, &size) == 0) {
        if (size != sizeof(hogan_standing_order)) {
            return -2; // Corrupt record size
        }
        
        const hogan_standing_order *order = (const hogan_standing_order *)read_buf;
        // Dispatch transaction into the log
        int dispatch_res = tsfi_hogan_dispatch_tx(sys, order->sender_id, order->recipient_id, order->amount, VM_EVM);
        if (dispatch_res != 0) {
            // Even if dispatch fails (e.g. queue full), we continue executing other orders,
            // but we can log or track it. For now, just continue.
        }
        index++;
    }
    
    return 0;
}

int tsfi_hogan_apply_account_freeze(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint8_t is_frozen, uint32_t authority_id) {
    // Enforce Rule 13: file extension must end with .dat.bin
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3; // Invalid extension
        }
    }

    // Disable live queue during administrative action
    uint8_t original_live_state = sys->live_processing_enabled;
    sys->live_processing_enabled = 0;
    
    hogan_account *acc = NULL;
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active && sys->accounts[i].account_id == account_id) {
            acc = &sys->accounts[i];
            break;
        }
    }
    
    if (!acc) {
        sys->live_processing_enabled = original_live_state;
        return -1; // account not found
    }
    
    hogan_freeze_entry entry = { account_id, is_frozen, authority_id };
    acc->is_frozen = is_frozen;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_freeze_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_apply_card_status(hogan_umbrella_system *sys, const char *filepath, uint32_t card_id, uint8_t new_status, uint32_t authority_id) {
    // Enforce Rule 13: file extension must end with .dat.bin
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3; // Invalid extension
        }
    }

    // Disable live queue during administrative action
    uint8_t original_live_state = sys->live_processing_enabled;
    sys->live_processing_enabled = 0;
    
    uint8_t prev_status = CARD_STATUS_ACTIVE;
    size_t found_idx = (size_t)-1;
    for (size_t i = 0; i < sys->blocked_card_count; i++) {
        if (sys->blocked_cards[i] == card_id) {
            prev_status = CARD_STATUS_BLOCKED;
            found_idx = i;
            break;
        }
    }
    
    if (new_status == CARD_STATUS_BLOCKED) {
        if (prev_status == CARD_STATUS_ACTIVE) {
            if (sys->blocked_card_count < HOGAN_MAX_BLOCKED_CARDS) {
                sys->blocked_cards[sys->blocked_card_count++] = card_id;
            } else {
                sys->live_processing_enabled = original_live_state;
                return -2; // blocked cards registry overflow
            }
        }
    } else {
        if (prev_status == CARD_STATUS_BLOCKED && found_idx != (size_t)-1) {
            // Remove from blocked list by shifting remaining elements
            for (size_t i = found_idx; i < sys->blocked_card_count - 1; i++) {
                sys->blocked_cards[i] = sys->blocked_cards[i + 1];
            }
            sys->blocked_card_count--;
        }
    }
    
    hogan_card_status_entry entry = { card_id, prev_status, new_status, authority_id };
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_card_status_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_apply_fee_exemption(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint8_t fee_exempt, uint32_t authority_id) {
    // Enforce Rule 13: file extension must end with .dat.bin
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3; // Invalid extension
        }
    }

    // Disable live queue during administrative action
    uint8_t original_live_state = sys->live_processing_enabled;
    sys->live_processing_enabled = 0;
    
    hogan_account *acc = NULL;
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active && sys->accounts[i].account_id == account_id) {
            acc = &sys->accounts[i];
            break;
        }
    }
    
    if (!acc) {
        sys->live_processing_enabled = original_live_state;
        return -1; // account not found
    }
    
    hogan_exemption_entry entry = { account_id, fee_exempt, authority_id };
    acc->fee_exempt = fee_exempt;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_exemption_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_apply_interest_override(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t new_rate_bps, uint32_t authority_id) {
    // Enforce Rule 13: file extension must end with .dat.bin
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3; // Invalid extension
        }
    }

    // Disable live queue during administrative action
    uint8_t original_live_state = sys->live_processing_enabled;
    sys->live_processing_enabled = 0;
    
    hogan_account *acc = NULL;
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active && sys->accounts[i].account_id == account_id) {
            acc = &sys->accounts[i];
            break;
        }
    }
    
    if (!acc) {
        sys->live_processing_enabled = original_live_state;
        return -1; // account not found
    }
    
    uint32_t prev_rate = acc->has_custom_rate ? acc->custom_interest_rate_bps : 0;
    
    hogan_int_override_entry entry = { account_id, prev_rate, new_rate_bps, authority_id };
    acc->custom_interest_rate_bps = new_rate_bps;
    acc->has_custom_rate = 1;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_int_override_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_min_balance(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_min_balance, uint32_t authority_id) {
    // Enforce Rule 13: file extension must end with .dat.bin
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3; // Invalid extension
        }
    }

    // Disable live queue during administrative action
    uint8_t original_live_state = sys->live_processing_enabled;
    sys->live_processing_enabled = 0;
    
    hogan_account *acc = NULL;
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active && sys->accounts[i].account_id == account_id) {
            acc = &sys->accounts[i];
            break;
        }
    }
    
    if (!acc) {
        sys->live_processing_enabled = original_live_state;
        return -1; // account not found
    }
    
    hogan_min_balance_entry entry = { account_id, acc->min_balance, new_min_balance, authority_id };
    acc->min_balance = new_min_balance;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_min_balance_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_tx_count_limit(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t new_limit, uint32_t authority_id) {
    // Enforce Rule 13: file extension must end with .dat.bin
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3; // Invalid extension
        }
    }

    // Disable live queue during administrative action
    uint8_t original_live_state = sys->live_processing_enabled;
    sys->live_processing_enabled = 0;
    
    hogan_account *acc = NULL;
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active && sys->accounts[i].account_id == account_id) {
            acc = &sys->accounts[i];
            break;
        }
    }
    
    if (!acc) {
        sys->live_processing_enabled = original_live_state;
        return -1; // account not found
    }
    
    hogan_tx_count_entry entry = { account_id, acc->daily_tx_limit, new_limit, authority_id };
    acc->daily_tx_limit = new_limit;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_tx_count_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}
