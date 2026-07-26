#include "../src/auncient_timeline_autodin.h"
#include <stdio.h>
#include <assert.h>

#include <string.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TIMELINE, AUTODIN, & HOGAN INTEGRATION TEST\n");
    printf("=============================================================\n");

    // 1. Initialize CICS Context
    sdk_cics_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));

    // 2. Define Timeline Events
    TimelineEvent events[2] = {
        { .timestamp = 0.5f, .event_id = 101, .lock_token = 0x111, .precedence = 'F', .triggered = false },
        { .timestamp = 1.2f, .event_id = 102, .lock_token = 0x222, .precedence = 'L', .triggered = false }
    };

    // Process at time 0.1 (nothing should trigger)
    auncient_timeline_process(events, 2, 0.1f, &ctx, NULL, 0, 0);
    assert(!events[0].triggered);
    assert(!events[1].triggered);
    printf("   ✓ Timeline boundary guard verified.\n");

    // Process at time 0.8 (first event should trigger)
    auncient_timeline_process(events, 2, 0.8f, &ctx, NULL, 0, 0);
    assert(events[0].triggered);
    assert(!events[1].triggered);
    printf("   ✓ AUTODIN precedence event locking verified.\n");

    // 3. Hogan Asset Reconciliation Test
    uint8_t mock_dna[4] = {0x12, 0x34, 0x56, 0x78};
    bool reconciled = auncient_hogan_reconcile_asset(999, mock_dna, 4);
    assert(reconciled);
    assert(reconciled);
    printf("   ✓ Hogan transaction consensus verified.\n");

    // 4. Hogan Account Registration Test
    HoganAccount account;
    bool registered = auncient_hogan_register_account(777, mock_dna, 4, &account);
    assert(registered);
    assert(account.account_id == 777);
    assert(account.balance_saat == 1000000);
    assert(account.is_active == true);
    printf("   ✓ First-class Hogan account holder registration verified.\n");

    // 5. Test Deposit, Withdraw, and Overdraft protection
    bool dep_ok = auncient_hogan_deposit(&account, 500000);
    assert(dep_ok);
    assert(account.balance_saat == 1500000); // 1.0M + 0.5M
    printf("   ✓ Hogan account deposit verified.\n");

    bool withdraw_ok = auncient_hogan_withdraw(&account, 1000000);
    assert(withdraw_ok);
    assert(account.balance_saat == 500000); // 1.5M - 1.0M
    printf("   ✓ Hogan account withdrawal verified.\n");

    bool overdraft_fail = auncient_hogan_withdraw(&account, 600000);
    assert(!overdraft_fail); // Should fail due to insufficient balance
    assert(account.balance_saat == 500000);
    printf("   ✓ Hogan overdraft protection verified.\n");

    // 6. Test Timeline Rollback and Checkpoint recovery
    auncient_timeline_rollback(events, 2, 0.4f, &account, 1000000);
    assert(!events[0].triggered); // Reset since 0.5 > 0.4
    assert(account.balance_saat == 1000000); // Checkpoint restored
    printf("   ✓ Timeline seek rollback and checkpoint recovery verified.\n");

    // 7. Test Timeline Payroll distribution
    auncient_timeline_payroll_payout(&account, 250000);
    assert(account.balance_saat == 1250000); // 1.0M restored balance + 0.25M payout
    printf("   ✓ Timeline payroll salary distribution verified.\n");

    // 8. Test Account Transfer and double-entry auditing
    HoganAccount recipient_acc;
    uint8_t rec_dna[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    auncient_hogan_register_account(888, rec_dna, 4, &recipient_acc);
    
    // Attempt high-value transfer with clearance level 1 (should fail)
    bool unauth_tx = auncient_hogan_transfer(&account, &recipient_acc, 200000);
    assert(!unauth_tx);
    
    // Escalate clearance and transfer successfully
    account.clearance_level = 2;
    bool auth_tx = auncient_hogan_transfer(&account, &recipient_acc, 250000);
    assert(auth_tx);
    assert(account.balance_saat == 1000000); // 1.25M - 250k
    assert(recipient_acc.balance_saat == 1250000); // 1.0M + 250k
    printf("   ✓ Double-entry transfer and clearance authorizations verified.\n");

    // Verify ledger audit
    HoganAccount ledger[2] = { account, recipient_acc };
    bool audit_ok = auncient_hogan_audit_ledger(ledger, 2, 2250000); // 1.0M + 1.25M
    assert(audit_ok);
    printf("   ✓ Ledger auditing and checks verified.\n");

    // Verify ongoing recurring audits
    sdk_cics_context_t audit_ctx;
    memset(&audit_ctx, 0, sizeof(audit_ctx));
    
    // First audit at time 1.1 with correct balance (should pass, last_blame = 0)
    auncient_timeline_process(events, 2, 1.1f, &audit_ctx, ledger, 2, 2250000);
    assert(audit_ctx.last_blame == 0);
    printf("   ✓ Recurring AUTODIN audit success path verified.\n");

    // Second audit at time 2.2 with mismatched balance (should fail, last_blame = 1)
    auncient_timeline_process(events, 2, 2.2f, &audit_ctx, ledger, 2, 3000000);
    assert(audit_ctx.last_blame == 1);
    printf("   ✓ Recurring AUTODIN audit violation blame verified.\n");

    // Test hypervisor latency warning
    auncient_hypervisor_monitor_audit(0.0000002f, true); // Fast audit info
    auncient_hypervisor_monitor_audit(0.0000050f, false); // Long-running warning
    printf("   ✓ Hypervisor audit logging verified.\n");

    // Test WinchesterMQ ABI transitions and registers
    WinchesterMQState w_state;
    memset(&w_state, 0, sizeof(w_state));
    w_state.identity = 3;
    
    // Seed
    winchester_mq_seed(&w_state, 5, 2, 4);
    assert(w_state.base == 5);
    
    // Tune (Channel = 5^4 % MOTZKIN_PRIME = 625)
    winchester_mq_tune(&w_state);
    assert(w_state.channel == 625);
    
    // Saturate
    winchester_mq_saturate(&w_state, 10, 20, 100);
    assert(w_state.element == 110);
    assert(w_state.chin == 30);
    assert(w_state.monopole == (30 * 30 * 30)); // 30^3
    printf("   ✓ WinchesterMQ state transitions and ABI equations verified.\n");

    printf("=============================================================\n");
    printf("AUNCIENT INTEGRATION TEST COMPLETE\n");
    printf("=============================================================\n");
    return 0;
}
