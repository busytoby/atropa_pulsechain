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
    auncient_timeline_process(events, 2, 0.1f, &ctx);
    assert(!events[0].triggered);
    assert(!events[1].triggered);
    printf("   ✓ Timeline boundary guard verified.\n");

    // Process at time 0.8 (first event should trigger)
    auncient_timeline_process(events, 2, 0.8f, &ctx);
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

    printf("=============================================================\n");
    printf("AUNCIENT INTEGRATION TEST COMPLETE\n");
    printf("=============================================================\n");
    return 0;
}
