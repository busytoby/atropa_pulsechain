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
            sys->accounts[i].last_activity_epoch = sys->current_epoch;
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
            fwrite(&sys->accounts[i].priority_tier, sizeof(uint8_t), 1, f);
            fwrite(&sys->accounts[i].max_balance, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].daily_deposit_limit, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].daily_deposited, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].overdraft_limit, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].overdraft_drawn, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].card_spend_limit, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].card_spent_today, sizeof(uint64_t), 1, f);
            fwrite(sys->accounts[i].blocked_merchants, sizeof(uint32_t), 8, f);
            fwrite(&sys->accounts[i].blocked_merchant_count, sizeof(uint8_t), 1, f);
            fwrite(&sys->accounts[i].card_tx_limit, sizeof(uint32_t), 1, f);
            fwrite(&sys->accounts[i].card_tx_count_today, sizeof(uint32_t), 1, f);
            fwrite(&sys->accounts[i].card_expiry_epoch, sizeof(uint32_t), 1, f);
            fwrite(&sys->accounts[i].grace_period_epochs, sizeof(uint32_t), 1, f);
            fwrite(&sys->accounts[i].min_card_auth_amount, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].max_card_auth_amount, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].card_fail_count_today, sizeof(uint32_t), 1, f);
            fwrite(&sys->accounts[i].card_fail_limit, sizeof(uint32_t), 1, f);
            fwrite(&sys->accounts[i].overdraft_fee_amount, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].max_interest_per_epoch, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].acc_spend_limit_today, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].acc_spent_today_transfers, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].card_pin, sizeof(uint32_t), 1, f);
            fwrite(&sys->accounts[i].pin_fail_count, sizeof(uint32_t), 1, f);
            fwrite(&sys->accounts[i].pin_fail_limit, sizeof(uint32_t), 1, f);
            fwrite(&sys->accounts[i].max_fee_per_epoch, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].last_activity_epoch, sizeof(uint32_t), 1, f);
            fwrite(&sys->accounts[i].dormancy_threshold_epochs, sizeof(uint32_t), 1, f);
            fwrite(&sys->accounts[i].is_dormant, sizeof(uint8_t), 1, f);
            fwrite(&sys->accounts[i].min_interest_posting_threshold, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].min_balance_fee_waive_threshold, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].pin_change_fail_count, sizeof(uint32_t), 1, f);
            fwrite(&sys->accounts[i].pin_change_fail_limit, sizeof(uint32_t), 1, f);
            fwrite(&sys->accounts[i].fee_exempt_expiry_epoch, sizeof(uint32_t), 1, f);
            fwrite(&sys->accounts[i].merchant_exception_count, sizeof(uint8_t), 1, f);
            if (sys->accounts[i].merchant_exception_count > 0) {
                fwrite(sys->accounts[i].merchant_exceptions, sizeof(uint32_t), sys->accounts[i].merchant_exception_count, f);
            }
            fwrite(&sys->accounts[i].posting_restriction, sizeof(uint8_t), 1, f);
            fwrite(&sys->accounts[i].dormancy_fee_surcharge, sizeof(uint64_t), 1, f);
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
        uint8_t priority_val = 0;
        if (fread(&priority_val, sizeof(uint8_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].priority_tier = priority_val;
        uint64_t max_bal = 0;
        if (fread(&max_bal, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].max_balance = max_bal;
        uint64_t dep_lim = 0;
        if (fread(&dep_lim, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        uint64_t dep_amt = 0;
        if (fread(&dep_amt, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].daily_deposit_limit = dep_lim;
        sys->accounts[i].daily_deposited = dep_amt;
        uint64_t ov_lim = 0;
        if (fread(&ov_lim, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        uint64_t ov_drawn = 0;
        if (fread(&ov_drawn, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].overdraft_limit = ov_lim;
        sys->accounts[i].overdraft_drawn = ov_drawn;
        uint64_t card_lim = 0;
        if (fread(&card_lim, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        uint64_t card_spent = 0;
        if (fread(&card_spent, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].card_spend_limit = card_lim;
        sys->accounts[i].card_spent_today = card_spent;
        if (fread(sys->accounts[i].blocked_merchants, sizeof(uint32_t), 8, f) != 8) {
            fclose(f);
            return -2;
        }
        uint8_t blocked_count = 0;
        if (fread(&blocked_count, sizeof(uint8_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].blocked_merchant_count = blocked_count;
        uint32_t c_tx_lim = 0;
        if (fread(&c_tx_lim, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        uint32_t c_tx_cnt = 0;
        if (fread(&c_tx_cnt, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].card_tx_limit = c_tx_lim;
        sys->accounts[i].card_tx_count_today = c_tx_cnt;
        uint32_t c_exp = 0;
        if (fread(&c_exp, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].card_expiry_epoch = c_exp;
        uint32_t gp_epochs = 0;
        if (fread(&gp_epochs, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].grace_period_epochs = gp_epochs;
        uint64_t min_auth_amt = 0;
        if (fread(&min_auth_amt, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].min_card_auth_amount = min_auth_amt;
        uint64_t max_auth_amt = 0;
        if (fread(&max_auth_amt, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].max_card_auth_amount = max_auth_amt;
        uint32_t c_fail_cnt = 0;
        if (fread(&c_fail_cnt, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].card_fail_count_today = c_fail_cnt;
        uint32_t c_fail_lim = 0;
        if (fread(&c_fail_lim, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].card_fail_limit = c_fail_lim;
        uint64_t od_fee = 0;
        if (fread(&od_fee, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].overdraft_fee_amount = od_fee;
        uint64_t max_int_cap = 0;
        if (fread(&max_int_cap, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].max_interest_per_epoch = max_int_cap;
        uint64_t acc_spl_tod = 0;
        if (fread(&acc_spl_tod, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].acc_spend_limit_today = acc_spl_tod;
        uint64_t acc_sp_trans = 0;
        if (fread(&acc_sp_trans, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].acc_spent_today_transfers = acc_sp_trans;
        uint32_t c_pin = 0;
        if (fread(&c_pin, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].card_pin = c_pin;
        uint32_t p_fails = 0;
        if (fread(&p_fails, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].pin_fail_count = p_fails;
        uint32_t p_lim = 0;
        if (fread(&p_lim, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].pin_fail_limit = p_lim;
        uint64_t max_f_cap = 0;
        if (fread(&max_f_cap, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].max_fee_per_epoch = max_f_cap;
        uint32_t last_act = 0;
        if (fread(&last_act, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].last_activity_epoch = last_act;
        uint32_t dorm_th = 0;
        if (fread(&dorm_th, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].dormancy_threshold_epochs = dorm_th;
        uint8_t is_dorm = 0;
        if (fread(&is_dorm, sizeof(uint8_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].is_dormant = is_dorm;
        uint64_t min_int_th = 0;
        if (fread(&min_int_th, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].min_interest_posting_threshold = min_int_th;
        uint64_t min_bal_fw = 0;
        if (fread(&min_bal_fw, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].min_balance_fee_waive_threshold = min_bal_fw;
        uint32_t pc_fails = 0;
        if (fread(&pc_fails, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].pin_change_fail_count = pc_fails;
        uint32_t pc_lim = 0;
        if (fread(&pc_lim, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].pin_change_fail_limit = pc_lim;
        uint32_t fe_exp = 0;
        if (fread(&fe_exp, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].fee_exempt_expiry_epoch = fe_exp;
        uint8_t me_cnt = 0;
        if (fread(&me_cnt, sizeof(uint8_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].merchant_exception_count = me_cnt;
        if (me_cnt > 0) {
            if (fread(sys->accounts[i].merchant_exceptions, sizeof(uint32_t), me_cnt, f) != me_cnt) {
                fclose(f);
                return -2;
            }
        }
        uint8_t post_r = 0;
        if (fread(&post_r, sizeof(uint8_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].posting_restriction = post_r;
        uint64_t dorm_s = 0;
        if (fread(&dorm_s, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].dormancy_fee_surcharge = dorm_s;
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
            if (sys->accounts[i].grace_period_epochs > 0 && sys->current_epoch <= sys->accounts[i].grace_period_epochs) {
                active_rate = 0;
            }
            
            // Calculate interest: balance * active_rate / 10000
            uint64_t interest = (sys->accounts[i].balance * active_rate) / 10000;
            if (sys->accounts[i].max_interest_per_epoch > 0 && interest > sys->accounts[i].max_interest_per_epoch) {
                interest = sys->accounts[i].max_interest_per_epoch;
            }
            if (interest < sys->accounts[i].min_interest_posting_threshold) {
                interest = 0; // suppress interest posting below threshold
            }
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
                if (sys->accounts[i].fee_exempt_expiry_epoch > 0 && sys->current_epoch > sys->accounts[i].fee_exempt_expiry_epoch) {
                    sys->accounts[i].fee_exempt = 0; // expired exemption!
                } else {
                    continue; // skip fee charging if account is exempt
                }
            }
            if (sys->accounts[i].min_balance_fee_waive_threshold > 0 &&
                sys->accounts[i].balance < sys->accounts[i].min_balance_fee_waive_threshold) {
                continue; // waive fee because balance is below threshold
            }
            hogan_fee_entry entry;
            entry.account_id = sys->accounts[i].account_id;
            entry.original_balance = sys->accounts[i].balance;
            
            uint64_t actual_fee = flat_fee;
            if (sys->accounts[i].is_dormant) {
                actual_fee += sys->accounts[i].dormancy_fee_surcharge;
            }
            if (sys->accounts[i].max_fee_per_epoch > 0 && actual_fee > sys->accounts[i].max_fee_per_epoch) {
                actual_fee = sys->accounts[i].max_fee_per_epoch;
            }
            if (sys->accounts[i].balance < actual_fee) {
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
    
    // Process VIP transactions first (priority_tier == 1), then Standard transactions (priority_tier == 0)
    for (int pass = 0; pass < 2; pass++) {
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
                // If VIP pass, skip standard accounts. If standard pass, process standard.
                uint8_t is_vip = sender->priority_tier || recipient->priority_tier;
                if (pass == 0 && !is_vip) continue;
                if (pass == 1 && is_vip) continue;
                
                if (sender->dormancy_threshold_epochs > 0 && (sys->current_epoch - sender->last_activity_epoch > sender->dormancy_threshold_epochs)) {
                    sender->is_dormant = 1;
                }
                if (recipient->dormancy_threshold_epochs > 0 && (sys->current_epoch - recipient->last_activity_epoch > recipient->dormancy_threshold_epochs)) {
                    recipient->is_dormant = 1;
                }
                
                if (sender->is_dormant || recipient->is_dormant) {
                    tx->processed = 1; // transaction blocked due to account dormancy
                } else if (sender->is_frozen || recipient->is_frozen) {
                    tx->processed = 1; // transaction blocked by compliance hold / freeze
                } else if (sender->posting_restriction & 1) {
                    tx->processed = 1; // transaction blocked due to debit posting restriction
                } else if (recipient->posting_restriction & 2) {
                    tx->processed = 1; // transaction blocked due to credit posting restriction
            } else if (sender->status_code == STATUS_STOP_ALL || sender->status_code == STATUS_STOP_DEBIT || recipient->status_code == STATUS_STOP_ALL) {
                tx->processed = 1; // transaction blocked by administrative hold
            } else if (sender->daily_tx_limit > 0 && sender->daily_tx_count >= sender->daily_tx_limit) {
                tx->processed = 1; // blocked by daily velocity/tx count limit
                } else if (sender->daily_transfer_limit > 0 && sender->daily_transferred + tx->amount > sender->daily_transfer_limit) {
                    tx->processed = 1; // blocked by daily transfer limit
                } else if (sender->acc_spend_limit_today > 0 && sender->acc_spent_today_transfers + tx->amount > sender->acc_spend_limit_today) {
                    tx->processed = 1; // blocked by account cumulative transfer amount limit
                } else if (recipient->max_balance > 0 && recipient->balance + tx->amount > recipient->max_balance) {
                    tx->processed = 1; // blocked by recipient maximum balance limit
                } else if (recipient->daily_deposit_limit > 0 && recipient->daily_deposited + tx->amount > recipient->daily_deposit_limit) {
                    tx->processed = 1; // blocked by daily deposit limit
                } else if (sender->balance >= tx->amount + sender->min_balance) {
                sender->balance -= tx->amount;
                recipient->balance += tx->amount;
                sender->daily_transferred += tx->amount;
                sender->acc_spent_today_transfers += tx->amount;
                sender->daily_tx_count++;
                recipient->daily_deposited += tx->amount;
                sender->last_activity_epoch = sys->current_epoch;
                recipient->last_activity_epoch = sys->current_epoch;
                tx->processed = 1;
            } else if (sender->has_backup) {
                // Determine overdraft backup account
                uint64_t required = tx->amount + sender->min_balance;
                uint64_t deficit = required - sender->balance;
                
                // Enforce overdraft limit check
                if (sender->overdraft_limit > 0 && sender->overdraft_drawn + deficit > sender->overdraft_limit) {
                    tx->processed = 1; // blocked by overdraft limit cap
                    if (overdraft_filepath) {
                        hogan_overdraft_entry o_entry = { sender->account_id, sender->backup_account_id, deficit, 0 };
                        tsfi_hogan_write_seq_record(overdraft_filepath, (const uint8_t *)&o_entry, sizeof(hogan_overdraft_entry));
                    }
                } else {
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
                         sender->acc_spent_today_transfers += tx->amount;
                         sender->daily_tx_count++;
                         recipient->daily_deposited += tx->amount;
                         sender->overdraft_drawn += deficit;
                         sender->last_activity_epoch = sys->current_epoch;
                         recipient->last_activity_epoch = sys->current_epoch;
                         tx->processed = 1;
                        
                        if (overdraft_filepath) {
                            hogan_overdraft_entry o_entry = { sender->account_id, backup->account_id, deficit, 1 };
                            tsfi_hogan_write_seq_record(overdraft_filepath, (const uint8_t *)&o_entry, sizeof(hogan_overdraft_entry));
                        }
                        
                        // Charge overdraft fee if configured
                        if (sender->overdraft_fee_amount > 0) {
                            if (sender->balance >= sender->overdraft_fee_amount) {
                                sender->balance -= sender->overdraft_fee_amount;
                            } else {
                                uint64_t remaining_fee = sender->overdraft_fee_amount - sender->balance;
                                sender->balance = 0;
                                sender->overdraft_drawn += remaining_fee;
                            }
                        }
                    } else {
                        tx->processed = 1;
                        if (overdraft_filepath) {
                            hogan_overdraft_entry o_entry = { sender->account_id, sender->backup_account_id, deficit, 0 };
                            tsfi_hogan_write_seq_record(overdraft_filepath, (const uint8_t *)&o_entry, sizeof(hogan_overdraft_entry));
                        }
                    }
                }
            } else {
                tx->processed = 1; // Fails, insufficient funds and no backup
            }
        } else {
            tx->processed = 1; // Fails, sender or receiver missing
        }
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
            sys->accounts[j].daily_deposited = 0;
            sys->accounts[j].card_spent_today = 0;
            sys->accounts[j].card_tx_count_today = 0;
            sys->accounts[j].card_fail_count_today = 0;
            sys->accounts[j].acc_spent_today_transfers = 0;
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
            EVP_DigestUpdate(mdctx, &sys->accounts[i].priority_tier, sizeof(uint8_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].max_balance, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].daily_deposit_limit, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].daily_deposited, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].overdraft_limit, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].overdraft_drawn, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].card_spend_limit, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].card_spent_today, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].blocked_merchant_count, sizeof(uint8_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].card_tx_limit, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].card_tx_count_today, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].card_expiry_epoch, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].grace_period_epochs, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].min_card_auth_amount, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].max_card_auth_amount, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].card_fail_count_today, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].card_fail_limit, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].overdraft_fee_amount, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].max_interest_per_epoch, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].acc_spend_limit_today, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].acc_spent_today_transfers, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].card_pin, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].pin_fail_count, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].pin_fail_limit, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].max_fee_per_epoch, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].last_activity_epoch, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].dormancy_threshold_epochs, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].is_dormant, sizeof(uint8_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].min_interest_posting_threshold, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].min_balance_fee_waive_threshold, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].pin_change_fail_count, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].pin_change_fail_limit, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].fee_exempt_expiry_epoch, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].merchant_exception_count, sizeof(uint8_t));
            if (sys->accounts[i].merchant_exception_count > 0) {
                EVP_DigestUpdate(mdctx, sys->accounts[i].merchant_exceptions, sizeof(uint32_t) * sys->accounts[i].merchant_exception_count);
            }
            EVP_DigestUpdate(mdctx, &sys->accounts[i].posting_restriction, sizeof(uint8_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].dormancy_fee_surcharge, sizeof(uint64_t));
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
    
    if (acc) {
        uint8_t is_excepted = 0;
        for (uint8_t i = 0; i < acc->merchant_exception_count; i++) {
            if (acc->merchant_exceptions[i] == merchant_id) {
                is_excepted = 1;
                break;
            }
        }
        if (!is_excepted) {
            for (uint8_t i = 0; i < acc->blocked_merchant_count; i++) {
                if (acc->blocked_merchants[i] == merchant_id) {
                    sys->live_processing_enabled = original_live_state;
                    return -5; // Merchant is blocked
                }
            }
        }
        if (acc->posting_restriction & 1) {
            sys->live_processing_enabled = original_live_state;
            return -10; // Debits restricted
        }
        if (acc->card_tx_limit > 0 && acc->card_tx_count_today >= acc->card_tx_limit) {
            sys->live_processing_enabled = original_live_state;
            return -6; // Card transaction limit hit
        }
        if (acc->card_expiry_epoch > 0 && sys->current_epoch > acc->card_expiry_epoch) {
            sys->live_processing_enabled = original_live_state;
            return -7; // Card has expired
        }
        if (acc->min_card_auth_amount > 0 && amount < acc->min_card_auth_amount) {
            sys->live_processing_enabled = original_live_state;
            return -8; // Amount below minimum card authorization threshold
        }
        if (acc->max_card_auth_amount > 0 && amount > acc->max_card_auth_amount) {
            sys->live_processing_enabled = original_live_state;
            return -9; // Amount exceeds maximum card authorization threshold
        }
    }
    
    int decline_reason = 0;
    if (acc) {
        uint8_t is_excepted = 0;
        for (uint8_t i = 0; i < acc->merchant_exception_count; i++) {
            if (acc->merchant_exceptions[i] == merchant_id) {
                is_excepted = 1;
                break;
            }
        }
        if (!is_excepted) {
            for (uint8_t i = 0; i < acc->blocked_merchant_count; i++) {
                if (acc->blocked_merchants[i] == merchant_id) {
                    decline_reason = -5; // Merchant is blocked
                    break;
                }
            }
        }
        if (acc->posting_restriction & 1) {
            decline_reason = -10; // Debits restricted
        }
        if (decline_reason == 0 && acc->card_tx_limit > 0 && acc->card_tx_count_today >= acc->card_tx_limit) {
            decline_reason = -6; // Card transaction limit hit
        }
        if (decline_reason == 0 && acc->card_expiry_epoch > 0 && sys->current_epoch > acc->card_expiry_epoch) {
            decline_reason = -7; // Card has expired
        }
        if (decline_reason == 0 && acc->min_card_auth_amount > 0 && amount < acc->min_card_auth_amount) {
            decline_reason = -8; // Amount below minimum card authorization threshold
        }
        if (decline_reason == 0 && acc->max_card_auth_amount > 0 && amount > acc->max_card_auth_amount) {
            decline_reason = -9; // Amount exceeds maximum card authorization threshold
        }
    } else {
        decline_reason = -1; // Account missing
    }
    
    uint8_t approved = 0;
    if (decline_reason == 0) {
        if (acc->balance >= acc->balance_held + amount) {
            if (acc->daily_limit == 0 || (acc->daily_spent + amount <= acc->daily_limit)) {
                if (acc->card_spend_limit == 0 || (acc->card_spent_today + amount <= acc->card_spend_limit)) {
                    acc->balance_held += amount;
                    acc->daily_spent += amount;
                    acc->card_spent_today += amount;
                    acc->card_tx_count_today++;
                    approved = 1;
                } else {
                    decline_reason = -1; // card spend limit hit
                }
            } else {
                decline_reason = -1; // daily spend limit hit
            }
        } else {
            decline_reason = -1; // insufficient balance
        }
    }
    
    if (!approved && acc) {
        acc->card_fail_count_today++;
        if (acc->card_fail_limit > 0 && acc->card_fail_count_today >= acc->card_fail_limit) {
            // Auto block card!
            uint8_t already_blocked = 0;
            for (size_t idx = 0; idx < sys->blocked_card_count; idx++) {
                if (sys->blocked_cards[idx] == card_id) {
                    already_blocked = 1;
                    break;
                }
            }
            if (!already_blocked && sys->blocked_card_count < HOGAN_MAX_BLOCKED_CARDS) {
                sys->blocked_cards[sys->blocked_card_count++] = card_id;
                // Log the auto-lock to the sequential file "hogan_fail_locks.dat.bin"
                hogan_card_fail_limit_entry lock_entry = { account_id, acc->card_fail_limit, card_id, 0 }; // 0 for system autolock authority
                tsfi_hogan_write_seq_record("hogan_fail_locks.dat.bin", (const uint8_t *)&lock_entry, sizeof(hogan_card_fail_limit_entry));
            }
        }
    }
    
    if (decline_reason == 0 && approved) {
        decline_reason = 0;
    } else if (decline_reason == 0) {
        decline_reason = -1;
    }
    
    hogan_card_entry entry = { card_id, account_id, merchant_id, amount, approved };
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_card_entry));
    
    sys->live_processing_enabled = original_live_state;
    
    if (write_res != 0) return write_res;
    return decline_reason;
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

int tsfi_hogan_update_priority_tier(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint8_t new_priority, uint32_t authority_id) {
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
    
    hogan_priority_entry entry = { account_id, acc->priority_tier, new_priority, authority_id };
    acc->priority_tier = new_priority;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_priority_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_max_balance(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_max_balance, uint32_t authority_id) {
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
    
    hogan_max_balance_entry entry = { account_id, acc->max_balance, new_max_balance, authority_id };
    acc->max_balance = new_max_balance;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_max_balance_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_deposit_limit(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_limit, uint32_t authority_id) {
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
    
    hogan_deposit_limit_entry entry = { account_id, acc->daily_deposit_limit, new_limit, authority_id };
    acc->daily_deposit_limit = new_limit;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_deposit_limit_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_overdraft_limit(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_limit, uint32_t authority_id) {
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
    
    hogan_overdraft_limit_entry entry = { account_id, acc->overdraft_limit, new_limit, authority_id };
    acc->overdraft_limit = new_limit;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_overdraft_limit_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_card_spend_limit(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_limit, uint32_t authority_id) {
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
    
    hogan_card_spend_limit_entry entry = { account_id, acc->card_spend_limit, new_limit, authority_id };
    acc->card_spend_limit = new_limit;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_card_spend_limit_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_merchant_block(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t merchant_id, uint8_t is_blocked, uint32_t authority_id) {
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
    
    if (is_blocked) {
        // Add to blocked merchants if not full and not already present
        uint8_t exists = 0;
        for (uint8_t i = 0; i < acc->blocked_merchant_count; i++) {
            if (acc->blocked_merchants[i] == merchant_id) {
                exists = 1;
                break;
            }
        }
        if (!exists && acc->blocked_merchant_count < 8) {
            acc->blocked_merchants[acc->blocked_merchant_count++] = merchant_id;
        }
    } else {
        // Remove from blocked merchants
        int idx = -1;
        for (uint8_t i = 0; i < acc->blocked_merchant_count; i++) {
            if (acc->blocked_merchants[i] == merchant_id) {
                idx = i;
                break;
            }
        }
        if (idx >= 0) {
            for (uint8_t i = idx; i < acc->blocked_merchant_count - 1; i++) {
                acc->blocked_merchants[i] = acc->blocked_merchants[i + 1];
            }
            acc->blocked_merchant_count--;
        }
    }
    
    hogan_merchant_block_entry entry = { account_id, merchant_id, is_blocked, authority_id };
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_merchant_block_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_card_tx_limit(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t new_limit, uint32_t authority_id) {
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
    
    hogan_card_tx_limit_entry entry = { account_id, acc->card_tx_limit, new_limit, authority_id };
    acc->card_tx_limit = new_limit;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_card_tx_limit_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_card_expiry(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t new_expiry_epoch, uint32_t authority_id) {
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
    
    hogan_card_expiry_entry entry = { account_id, acc->card_expiry_epoch, new_expiry_epoch, authority_id };
    acc->card_expiry_epoch = new_expiry_epoch;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_card_expiry_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_grace_period(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t new_grace_period, uint32_t authority_id) {
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
    
    hogan_grace_period_entry entry = { account_id, acc->grace_period_epochs, new_grace_period, authority_id };
    acc->grace_period_epochs = new_grace_period;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_grace_period_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_min_card_auth(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_min_amount, uint32_t authority_id) {
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
    
    hogan_min_card_auth_entry entry = { account_id, acc->min_card_auth_amount, new_min_amount, authority_id };
    acc->min_card_auth_amount = new_min_amount;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_min_card_auth_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_max_card_auth(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_max_amount, uint32_t authority_id) {
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
    
    hogan_max_card_auth_entry entry = { account_id, acc->max_card_auth_amount, new_max_amount, authority_id };
    acc->max_card_auth_amount = new_max_amount;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_max_card_auth_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_card_fail_limit(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t new_fail_limit, uint32_t authority_id) {
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
    
    hogan_card_fail_limit_entry entry = { account_id, acc->card_fail_limit, new_fail_limit, authority_id };
    acc->card_fail_limit = new_fail_limit;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_card_fail_limit_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_overdraft_fee(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_fee_amount, uint32_t authority_id) {
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
    
    hogan_overdraft_fee_entry entry = { account_id, acc->overdraft_fee_amount, new_fee_amount, authority_id };
    acc->overdraft_fee_amount = new_fee_amount;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_overdraft_fee_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_interest_cap(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_interest_cap, uint32_t authority_id) {
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
    
    hogan_interest_cap_entry entry = { account_id, acc->max_interest_per_epoch, new_interest_cap, authority_id };
    acc->max_interest_per_epoch = new_interest_cap;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_interest_cap_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_acc_spend_limit(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_spend_limit, uint32_t authority_id) {
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
    
    hogan_acc_spend_limit_entry entry = { account_id, acc->acc_spend_limit_today, new_spend_limit, authority_id };
    acc->acc_spend_limit_today = new_spend_limit;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_acc_spend_limit_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_validate_card_pin(hogan_umbrella_system *sys, const char *filepath, uint32_t card_id, uint32_t account_id, uint32_t entered_pin) {
    // Enforce Rule 13: file extension must end with .dat.bin
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3; // Invalid extension
        }
    }

    // Disable live queue during validation
    uint8_t original_live_state = sys->live_processing_enabled;
    sys->live_processing_enabled = 0;

    // Check if card is blocked
    for (size_t idx = 0; idx < sys->blocked_card_count; idx++) {
        if (sys->blocked_cards[idx] == card_id) {
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

    if (!acc) {
        sys->live_processing_enabled = original_live_state;
        return -1; // Account missing
    }

    uint8_t correct = (acc->card_pin == entered_pin);
    if (correct) {
        acc->pin_fail_count = 0;
    } else {
        acc->pin_fail_count++;
        if (acc->pin_fail_limit > 0 && acc->pin_fail_count >= acc->pin_fail_limit) {
            // Auto lock the card!
            uint8_t already_blocked = 0;
            for (size_t idx = 0; idx < sys->blocked_card_count; idx++) {
                if (sys->blocked_cards[idx] == card_id) {
                    already_blocked = 1;
                    break;
                }
            }
            if (!already_blocked && sys->blocked_card_count < HOGAN_MAX_BLOCKED_CARDS) {
                sys->blocked_cards[sys->blocked_card_count++] = card_id;
                // Log to "hogan_pin_locks.dat.bin"
                hogan_pin_fail_limit_entry lock_entry = { account_id, acc->pin_fail_limit, card_id, 0 };
                tsfi_hogan_write_seq_record("hogan_pin_locks.dat.bin", (const uint8_t *)&lock_entry, sizeof(hogan_pin_fail_limit_entry));
            }
        }
    }

    // Log this attempt as card pin entry override record format (representing (previous_pin, new_pin, success) structure)
    hogan_card_pin_entry attempt_entry = { account_id, acc->card_pin, entered_pin, correct ? 1u : 0u };
    tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&attempt_entry, sizeof(hogan_card_pin_entry));

    sys->live_processing_enabled = original_live_state;
    return correct ? 0 : -2; // 0 on success, -2 on incorrect PIN
}

int tsfi_hogan_update_pin_fail_limit(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t new_pin_limit, uint32_t authority_id) {
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
    
    hogan_pin_fail_limit_entry entry = { account_id, acc->pin_fail_limit, new_pin_limit, authority_id };
    acc->pin_fail_limit = new_pin_limit;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_pin_fail_limit_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_card_pin(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t new_pin, uint32_t authority_id) {
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
    
    hogan_card_pin_entry entry = { account_id, acc->card_pin, new_pin, authority_id };
    acc->card_pin = new_pin;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_card_pin_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_fee_cap(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_fee_cap, uint32_t authority_id) {
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
    
    hogan_fee_cap_entry entry = { account_id, acc->max_fee_per_epoch, new_fee_cap, authority_id };
    acc->max_fee_per_epoch = new_fee_cap;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_fee_cap_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_reactivate_dormant_account(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t authority_id) {
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
    
    hogan_dormancy_entry entry = { account_id, acc->is_dormant, 0, authority_id };
    acc->is_dormant = 0;
    acc->last_activity_epoch = sys->current_epoch;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_dormancy_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_set_dormancy_threshold(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t threshold_epochs, uint32_t authority_id) {
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
    
    // Log previous dormancy threshold to "new" parameter placeholder, authority_id is logged
    hogan_dormancy_entry entry = { account_id, (uint8_t)acc->dormancy_threshold_epochs, (uint8_t)threshold_epochs, authority_id };
    acc->dormancy_threshold_epochs = threshold_epochs;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_dormancy_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_interest_threshold(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_threshold, uint32_t authority_id) {
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
    
    hogan_interest_threshold_entry entry = { account_id, acc->min_interest_posting_threshold, new_threshold, authority_id };
    acc->min_interest_posting_threshold = new_threshold;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_interest_threshold_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_fee_waive_threshold(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_threshold, uint32_t authority_id) {
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
    
    hogan_fee_waive_threshold_entry entry = { account_id, acc->min_balance_fee_waive_threshold, new_threshold, authority_id };
    acc->min_balance_fee_waive_threshold = new_threshold;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_fee_waive_threshold_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_change_card_pin(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t old_pin, uint32_t new_pin) {
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
    
    // If fail limit is defined (>0) and fail count has reached limit, reject attempt
    if (acc->pin_change_fail_limit > 0 && acc->pin_change_fail_count >= acc->pin_change_fail_limit) {
        sys->live_processing_enabled = original_live_state;
        return -4; // PIN change locked out
    }
    
    uint8_t success = 0;
    if (acc->card_pin == old_pin) {
        acc->card_pin = new_pin;
        acc->pin_change_fail_count = 0; // reset fail count on success
        success = 1;
    } else {
        acc->pin_change_fail_count++;
    }
    
    hogan_pin_change_entry entry = { account_id, old_pin, new_pin, success };
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_pin_change_entry));
    
    sys->live_processing_enabled = original_live_state;
    if (!success) {
        return -2; // invalid old PIN
    }
    return write_res;
}

int tsfi_hogan_set_pin_change_fail_limit(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t new_limit, uint32_t authority_id) {
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
    
    hogan_pin_change_limit_entry entry = { account_id, acc->pin_change_fail_limit, new_limit, authority_id };
    acc->pin_change_fail_limit = new_limit;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_pin_change_limit_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_fee_exempt_expiry(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t expiry_epoch, uint32_t authority_id) {
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
    
    hogan_fee_exempt_expiry_entry entry = { account_id, acc->fee_exempt_expiry_epoch, expiry_epoch, authority_id };
    acc->fee_exempt_expiry_epoch = expiry_epoch;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_fee_exempt_expiry_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_add_merchant_exception(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t merchant_id, uint32_t authority_id) {
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
    
    if (acc->merchant_exception_count >= 16) {
        sys->live_processing_enabled = original_live_state;
        return -2; // whitelist full
    }
    
    // Add to exceptions
    acc->merchant_exceptions[acc->merchant_exception_count++] = merchant_id;
    
    hogan_merchant_exception_entry entry = { account_id, merchant_id, 1, authority_id };
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_merchant_exception_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_posting_restriction(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint8_t restriction_code, uint32_t authority_id) {
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
    
    hogan_posting_restriction_entry entry = { account_id, acc->posting_restriction, restriction_code, authority_id };
    acc->posting_restriction = restriction_code;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_posting_restriction_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_dormancy_surcharge(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t surcharge_amount, uint32_t authority_id) {
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
    
    hogan_dormancy_surcharge_entry entry = { account_id, acc->dormancy_fee_surcharge, surcharge_amount, authority_id };
    acc->dormancy_fee_surcharge = surcharge_amount;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_dormancy_surcharge_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}
