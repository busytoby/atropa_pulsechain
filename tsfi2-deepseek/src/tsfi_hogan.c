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
        }
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
        if (fread(&acc_id, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        if (fread(&bal, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].account_id = acc_id;
        sys->accounts[i].balance = bal;
        sys->accounts[i].active = 1;
    }
    
    fclose(f);
    return 0;
}

int tsfi_hogan_overnight_reconciliation(hogan_umbrella_system *sys, const char *lfs_filepath) {
    // 1. Pause live transaction dispatching
    sys->live_processing_enabled = 0;
    
    // 2. Process all pending transactions in log
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
        
        if (sender && recipient && sender->balance >= tx->amount) {
            sender->balance -= tx->amount;
            recipient->balance += tx->amount;
            tx->processed = 1;
        } else {
            // Insufficient balance or missing account, mark as processed/failed
            tx->processed = 1;
        }
    }
    
    // Clear transaction log for next day
    sys->tx_count = 0;
    
    // 3. Compute new acab_epoch_root using OpenSSL EVP hash
    unsigned int hash_len = 32;
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    
    // Hash current accounts and epoch metadata
    EVP_DigestUpdate(mdctx, &sys->current_epoch, sizeof(uint32_t));
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active) {
            EVP_DigestUpdate(mdctx, &sys->accounts[i].account_id, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].balance, sizeof(uint64_t));
        }
    }
    EVP_DigestFinal_ex(mdctx, sys->acab_epoch_root, &hash_len);
    EVP_MD_CTX_free(mdctx);
    
    // Increment epoch
    sys->current_epoch++;
    
    // 4. Save state to Logical File System
    int save_res = tsfi_hogan_lfs_save(sys, lfs_filepath);
    
    // 5. Resume live transaction dispatching
    sys->live_processing_enabled = 1;
    
    return save_res;
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
