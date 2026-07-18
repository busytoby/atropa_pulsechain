#include "tsfi_hogan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int tsfi_hogan_update_atm_limit(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_limit, uint32_t authority_id) {
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3;
        }
    }
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
        return -1;
    }
    hogan_atm_limit_entry entry = { account_id, acc->atm_daily_limit, new_limit, authority_id };
    acc->atm_daily_limit = new_limit;
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_atm_limit_entry));
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_mcc_block(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint16_t mcc, uint8_t is_blocked, uint32_t authority_id) {
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3;
        }
    }
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
        return -1;
    }
    if (is_blocked) {
        uint8_t exists = 0;
        for (uint8_t i = 0; i < acc->blocked_mcc_count; i++) {
            if (acc->blocked_mccs[i] == mcc) {
                exists = 1;
                break;
            }
        }
        if (!exists && acc->blocked_mcc_count < 8) {
            acc->blocked_mccs[acc->blocked_mcc_count++] = mcc;
        }
    } else {
        int idx = -1;
        for (uint8_t i = 0; i < acc->blocked_mcc_count; i++) {
            if (acc->blocked_mccs[i] == mcc) {
                idx = i;
                break;
            }
        }
        if (idx >= 0) {
            for (uint8_t i = idx; i < acc->blocked_mcc_count - 1; i++) {
                acc->blocked_mccs[i] = acc->blocked_mccs[i + 1];
            }
            acc->blocked_mcc_count--;
        }
    }
    hogan_mcc_block_entry entry = { account_id, mcc, is_blocked, authority_id };
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_mcc_block_entry));
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_overdraft_grace_zone(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint64_t new_grace, uint32_t authority_id) {
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3;
        }
    }
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
        return -1;
    }
    hogan_overdraft_grace_entry entry = { account_id, acc->overdraft_grace_zone, new_grace, authority_id };
    acc->overdraft_grace_zone = new_grace;
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_overdraft_grace_entry));
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_account_currency(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint8_t new_currency, uint32_t authority_id) {
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3;
        }
    }
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
        return -1;
    }
    hogan_currency_entry entry = { account_id, acc->currency_code, new_currency, authority_id };
    acc->currency_code = new_currency;
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_currency_entry));
    sys->live_processing_enabled = original_live_state;
    return write_res;
}

int tsfi_hogan_update_fx_spread(hogan_umbrella_system *sys, const char *filepath, uint32_t account_id, uint32_t new_spread_bps, uint32_t authority_id) {
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3;
        }
    }
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
        return -1;
    }
    hogan_fx_spread_entry entry = { account_id, acc->fx_spread_bps, new_spread_bps, authority_id };
    acc->fx_spread_bps = new_spread_bps;
    int write_res = tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_fx_spread_entry));
    sys->live_processing_enabled = original_live_state;
    return write_res;
}
