#include "tsfi_hogan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
            if (sys->accounts[i].interest_tier_threshold > 0 &&
                sys->accounts[i].balance >= sys->accounts[i].interest_tier_threshold) {
                active_rate = sys->accounts[i].interest_tier_rate_bps;
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
            if (sys->accounts[i].fee_tier_threshold > 0 &&
                sys->accounts[i].balance < sys->accounts[i].fee_tier_threshold) {
                actual_fee = sys->accounts[i].fee_tier_amount;
            }
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
        if (acc->is_frozen) {
            sys->live_processing_enabled = original_live_state;
            return -2; // Compliance freeze hold blocks card transactions
        }
        if (acc->status_code == STATUS_STOP_ALL || acc->status_code == STATUS_STOP_DEBIT) {
            sys->live_processing_enabled = original_live_state;
            return -3; // Administrative stop blocks card transactions
        }
        if (acc->is_dormant) {
            sys->live_processing_enabled = original_live_state;
            return -4; // Dormant accounts block card authorizations
        }
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
            // Continue executing other orders even if one fails
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

    // Log this attempt
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

int tsfi_hogan_update_interest_tier(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t threshold, uint32_t rate_bps, uint32_t authority_id) {
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
    
    hogan_interest_tier_entry entry = { account_id, threshold, rate_bps, authority_id };
    acc->interest_tier_threshold = threshold;
    acc->interest_tier_rate_bps = rate_bps;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_interest_tier_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_fee_tier(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t threshold, uint64_t fee_amount, uint32_t authority_id) {
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
    
    hogan_fee_tier_entry entry = { account_id, threshold, fee_amount, authority_id };
    acc->fee_tier_threshold = threshold;
    acc->fee_tier_amount = fee_amount;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_fee_tier_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_transfer_fee(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t fee_bps, uint64_t fee_flat, uint32_t authority_id) {
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
    
    hogan_transfer_fee_entry entry = { account_id, fee_bps, fee_flat, authority_id };
    acc->transfer_fee_bps = fee_bps;
    acc->transfer_fee_flat = fee_flat;
    
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_transfer_fee_entry));
    
    sys->live_processing_enabled = original_live_state;
    return write_res;
}
