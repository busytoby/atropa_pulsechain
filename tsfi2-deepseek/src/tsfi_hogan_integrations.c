#include "tsfi_hogan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int tsfi_hogan_register_ach_direct_deposit(hogan_umbrella_system *sys, uint32_t account_id, uint32_t routing_number, uint64_t amount, uint32_t clearing_epoch) {
    if (sys->ach_queue_count >= 32) {
        return -1; // queue full
    }
    
    // Verify account exists
    uint8_t exists = 0;
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active && sys->accounts[i].account_id == account_id) {
            exists = 1;
            break;
        }
    }
    if (!exists) return -2; // Account missing

    hogan_ach_entry *entry = &sys->ach_queue[sys->ach_queue_count++];
    entry->account_id = account_id;
    entry->routing_number = routing_number;
    entry->amount = amount;
    entry->clearing_epoch = clearing_epoch;
    entry->processed = 0;
    
    return 0;
}

int tsfi_hogan_setup_loan(hogan_umbrella_system *sys, uint32_t account_id, uint64_t principal, uint32_t interest_rate_bps, uint64_t next_payment, uint32_t term_epochs) {
    if (sys->loan_count >= 16) {
        return -1; // loan slots full
    }

    uint8_t exists = 0;
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active && sys->accounts[i].account_id == account_id) {
            exists = 1;
            break;
        }
    }
    if (!exists) return -2;

    hogan_loan *loan = &sys->loans[sys->loan_count++];
    loan->account_id = account_id;
    loan->principal_remaining = principal;
    loan->interest_rate_bps = interest_rate_bps;
    loan->next_payment_amount = next_payment;
    loan->term_epochs_remaining = term_epochs;
    loan->active = 1;
    
    return 0;
}

int tsfi_hogan_add_sweep_rule(hogan_umbrella_system *sys, uint32_t source_id, uint32_t target_id, uint64_t trigger_threshold, uint64_t target_balance) {
    if (sys->sweep_rule_count >= 16) {
        return -1; // sweep slots full
    }

    uint8_t source_exists = 0, target_exists = 0;
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active) {
            if (sys->accounts[i].account_id == source_id) source_exists = 1;
            if (sys->accounts[i].account_id == target_id) target_exists = 1;
        }
    }
    if (!source_exists || !target_exists) return -2;

    hogan_sweep_rule *rule = &sys->sweep_rules[sys->sweep_rule_count++];
    rule->source_account_id = source_id;
    rule->target_account_id = target_id;
    rule->trigger_threshold = trigger_threshold;
    rule->target_balance = target_balance;
    rule->active = 1;
    
    return 0;
}

int tsfi_hogan_verify_hsm_pin_block(hogan_umbrella_system *sys, uint32_t account_id, uint32_t entered_pin, uint32_t pin_block, uint32_t encrypted_key) {
    // Auncient hardware HSM key translation bridge simulation
    hogan_account *acc = NULL;
    for (int i = 0; i < HOGAN_MAX_ACCOUNTS; i++) {
        if (sys->accounts[i].active && sys->accounts[i].account_id == account_id) {
            acc = &sys->accounts[i];
            break;
        }
    }
    if (!acc) return -1;

    // Emulated PIN block translation logic
    uint32_t translated_pin = pin_block ^ encrypted_key;
    if (translated_pin == entered_pin) {
        acc->pin_fail_count = 0;
        return 0;
    } else {
        acc->pin_fail_count++;
        return -2;
    }
}

void tsfi_hogan_process_integrations_overnight(hogan_umbrella_system *sys) {
    // 1. Process PEP ACH Direct Deposits
    for (uint32_t i = 0; i < sys->ach_queue_count; i++) {
        hogan_ach_entry *ach = &sys->ach_queue[i];
        if (!ach->processed && sys->current_epoch >= ach->clearing_epoch) {
            for (int j = 0; j < HOGAN_MAX_ACCOUNTS; j++) {
                if (sys->accounts[j].active && sys->accounts[j].account_id == ach->account_id) {
                    sys->accounts[j].balance += ach->amount;
                    ach->processed = 1;
                    break;
                }
            }
        }
    }

    // 2. Process ILS Loan Interest Accrual and Dynamic Amortization Payments
    for (uint32_t i = 0; i < sys->loan_count; i++) {
        hogan_loan *loan = &sys->loans[i];
        if (loan->active && loan->principal_remaining > 0) {
            hogan_account *acc = NULL;
            for (int j = 0; j < HOGAN_MAX_ACCOUNTS; j++) {
                if (sys->accounts[j].active && sys->accounts[j].account_id == loan->account_id) {
                    acc = &sys->accounts[j];
                    break;
                }
            }
            if (acc) {
                // Apply dynamic interest accrual
                uint64_t interest = (loan->principal_remaining * loan->interest_rate_bps) / 10000;
                loan->principal_remaining += interest;
                
                // Process payment sweep
                uint64_t payment = loan->next_payment_amount;
                if (payment > loan->principal_remaining) {
                    payment = loan->principal_remaining;
                }
                
                if (acc->balance >= payment) {
                    acc->balance -= payment;
                    loan->principal_remaining -= payment;
                    if (loan->term_epochs_remaining > 0) {
                        loan->term_epochs_remaining--;
                    }
                }
                
                if (loan->principal_remaining == 0 || loan->term_epochs_remaining == 0) {
                    loan->active = 0;
                }
            }
        }
    }

    // 3. Process DDO Hierarchical Sweep Rules
    for (uint32_t i = 0; i < sys->sweep_rule_count; i++) {
        hogan_sweep_rule *rule = &sys->sweep_rules[i];
        if (rule->active) {
            hogan_account *source = NULL;
            hogan_account *target = NULL;
            for (int j = 0; j < HOGAN_MAX_ACCOUNTS; j++) {
                if (sys->accounts[j].active) {
                    if (sys->accounts[j].account_id == rule->source_account_id) source = &sys->accounts[j];
                    if (sys->accounts[j].account_id == rule->target_account_id) target = &sys->accounts[j];
                }
            }
            if (source && target && target->balance < rule->trigger_threshold) {
                uint64_t needed = rule->target_balance - target->balance;
                if (source->balance >= needed) {
                    source->balance -= needed;
                    target->balance += needed;
                } else {
                    uint64_t partial = source->balance;
                    source->balance = 0;
                    target->balance += partial;
                }
            }
        }
    }
}
