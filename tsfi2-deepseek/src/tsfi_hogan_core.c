#include "tsfi_hogan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>

static uint64_t convert_currency(uint64_t amount, uint8_t from_currency, uint8_t to_currency) {
    if (from_currency == to_currency) return amount;
    uint64_t factors[] = {10000, 11000, 13000, 500};
    uint64_t factor_from = (from_currency < 4) ? factors[from_currency] : 10000;
    uint64_t factor_to = (to_currency < 4) ? factors[to_currency] : 10000;
    return (amount * factor_from) / factor_to;
}


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
            fwrite(&sys->accounts[i].interest_tier_threshold, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].interest_tier_rate_bps, sizeof(uint32_t), 1, f);
            fwrite(&sys->accounts[i].fee_tier_threshold, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].fee_tier_amount, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].transfer_fee_bps, sizeof(uint32_t), 1, f);
            fwrite(&sys->accounts[i].transfer_fee_flat, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].atm_daily_limit, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].atm_withdrawn_today, sizeof(uint64_t), 1, f);
            fwrite(sys->accounts[i].blocked_mccs, sizeof(uint16_t), 8, f);
            fwrite(&sys->accounts[i].blocked_mcc_count, sizeof(uint8_t), 1, f);
            fwrite(&sys->accounts[i].overdraft_grace_zone, sizeof(uint64_t), 1, f);
            fwrite(&sys->accounts[i].currency_code, sizeof(uint8_t), 1, f);
            fwrite(&sys->accounts[i].fx_spread_bps, sizeof(uint32_t), 1, f);
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
        uint64_t tier_th = 0;
        if (fread(&tier_th, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].interest_tier_threshold = tier_th;
        uint32_t tier_r = 0;
        if (fread(&tier_r, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].interest_tier_rate_bps = tier_r;
        uint64_t f_tier_th = 0;
        if (fread(&f_tier_th, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].fee_tier_threshold = f_tier_th;
        uint64_t f_tier_amt = 0;
        if (fread(&f_tier_amt, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].fee_tier_amount = f_tier_amt;
        uint32_t t_fee_bps = 0;
        if (fread(&t_fee_bps, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].transfer_fee_bps = t_fee_bps;
        uint64_t t_fee_flat = 0;
        if (fread(&t_fee_flat, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].transfer_fee_flat = t_fee_flat;
        uint64_t atm_lim = 0;
        if (fread(&atm_lim, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        uint64_t atm_w = 0;
        if (fread(&atm_w, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        uint16_t blk_mccs[8];
        if (fread(blk_mccs, sizeof(uint16_t), 8, f) != 8) {
            fclose(f);
            return -2;
        }
        uint8_t blk_mcc_cnt = 0;
        if (fread(&blk_mcc_cnt, sizeof(uint8_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        uint64_t od_grace = 0;
        if (fread(&od_grace, sizeof(uint64_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        uint8_t curr_c = 0;
        if (fread(&curr_c, sizeof(uint8_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        uint32_t fx_spr = 0;
        if (fread(&fx_spr, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2;
        }
        sys->accounts[i].atm_daily_limit = atm_lim;
        sys->accounts[i].atm_withdrawn_today = atm_w;
        memcpy(sys->accounts[i].blocked_mccs, blk_mccs, sizeof(blk_mccs));
        sys->accounts[i].blocked_mcc_count = blk_mcc_cnt;
        sys->accounts[i].overdraft_grace_zone = od_grace;
        sys->accounts[i].currency_code = curr_c;
        sys->accounts[i].fx_spread_bps = fx_spr;
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

int tsfi_hogan_overnight_reconciliation(hogan_umbrella_system *sys, const char *lfs_filepath) {
    return tsfi_hogan_overnight_reconciliation_ex(sys, lfs_filepath, NULL);
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
                } else {
                    uint64_t fee = sender->transfer_fee_flat + (tx->amount * (uint64_t)sender->transfer_fee_bps) / 10000;
                    uint64_t fx_fee = 0;
                    if (sender->currency_code != recipient->currency_code) {
                        fx_fee = (tx->amount * (uint64_t)sender->fx_spread_bps) / 10000;
                    }
                    uint64_t total_needed = tx->amount + fee + fx_fee;
                    uint64_t converted_credit = convert_currency(tx->amount, sender->currency_code, recipient->currency_code);

                    if (sender->balance >= total_needed + sender->min_balance) {
                        sender->balance -= total_needed;
                        recipient->balance += converted_credit;
                        sender->daily_transferred += tx->amount;
                        sender->acc_spent_today_transfers += tx->amount;
                        sender->daily_tx_count++;
                        recipient->daily_deposited += converted_credit;
                        sender->last_activity_epoch = sys->current_epoch;
                        recipient->last_activity_epoch = sys->current_epoch;
                        tx->processed = 1;
                    } else {
                        uint64_t required = total_needed + sender->min_balance;
                        uint64_t deficit = required - sender->balance;
                        
                        if (deficit <= sender->overdraft_grace_zone) {
                            if (sender->overdraft_limit > 0 && sender->overdraft_drawn + deficit > sender->overdraft_limit) {
                                tx->processed = 1;
                            } else {
                                sender->balance = 0;
                                sender->overdraft_drawn += deficit;
                                recipient->balance += converted_credit;
                                sender->daily_transferred += tx->amount;
                                sender->acc_spent_today_transfers += tx->amount;
                                sender->daily_tx_count++;
                                recipient->daily_deposited += converted_credit;
                                sender->last_activity_epoch = sys->current_epoch;
                                recipient->last_activity_epoch = sys->current_epoch;
                                tx->processed = 1;
                            }
                        } else if (sender->has_backup) {
                            if (sender->overdraft_limit > 0 && sender->overdraft_drawn + deficit > sender->overdraft_limit) {
                                tx->processed = 1;
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
                                    sender->balance -= total_needed;
                                    recipient->balance += converted_credit;
                                    sender->daily_transferred += tx->amount;
                                    sender->acc_spent_today_transfers += tx->amount;
                                    sender->daily_tx_count++;
                                    recipient->daily_deposited += converted_credit;
                                    sender->overdraft_drawn += deficit;
                                    sender->last_activity_epoch = sys->current_epoch;
                                    recipient->last_activity_epoch = sys->current_epoch;
                                    tx->processed = 1;
                                    
                                    if (overdraft_filepath) {
                                        hogan_overdraft_entry o_entry = { sender->account_id, backup->account_id, deficit, 1 };
                                        tsfi_hogan_write_seq_record(overdraft_filepath, (const uint8_t *)&o_entry, sizeof(hogan_overdraft_entry));
                                    }
                                    
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
                                }
                            }
                        } else {
                            tx->processed = 1;
                        }
                    }
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
            sys->accounts[j].atm_withdrawn_today = 0;
        }
    }
    
    // Process PEP ACH, ILS Loans, and DDO Sweeps
    tsfi_hogan_process_integrations_overnight(sys);
    
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
            EVP_DigestUpdate(mdctx, &sys->accounts[i].interest_tier_threshold, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].interest_tier_rate_bps, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].fee_tier_threshold, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].fee_tier_amount, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].transfer_fee_bps, sizeof(uint32_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].transfer_fee_flat, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].atm_daily_limit, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].atm_withdrawn_today, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, sys->accounts[i].blocked_mccs, sizeof(uint16_t) * 8);
            EVP_DigestUpdate(mdctx, &sys->accounts[i].blocked_mcc_count, sizeof(uint8_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].overdraft_grace_zone, sizeof(uint64_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].currency_code, sizeof(uint8_t));
            EVP_DigestUpdate(mdctx, &sys->accounts[i].fx_spread_bps, sizeof(uint32_t));
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
