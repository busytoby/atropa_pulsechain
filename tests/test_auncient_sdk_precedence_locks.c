#include "auncient_sdk.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// -------------------------------------------------------------
// SDK Advanced Lock & Security Tests
// -------------------------------------------------------------
void test_sdk_security_clearance_levels(void) {
    printf("[TEST_ADV] Checking security clearance gate enforcement...\n");
    fflush(stdout);

    sdk_coaxial_env_t env;
    bool ok = auncient_sdk_init_coaxial(&env);
    assert(ok == true);

    sdk_kermit_cache_t cache = { .cached_value = 0, .cached_ts = { 0, 0 }, .is_warm = false };

    // Case 1: Context with Confidential Clearance (Level 1) attempting TopSecret write (> 900000)
    sdk_cics_context_t ctx_low = {
        .env = &env,
        .cache = &cache,
        .quorum_type = SDK_QUORUM_MAJORITY,
        .writer_id = 10,
        .security_clearance = 1 // Low clearance
    };

    bool approvals[SDK_NUM_NODES] = { true, true, true, false };
    uint32_t result = 0;
    ok = auncient_sdk_alu_execute(&ctx_low, ALU_OP_WRITE_ABD, 950000, approvals, &result);
    assert(ok == false); // Should reject due to clearance violation
    printf("   ✓ Level 1 clearance successfully blocked from writing TopSecret value.\n");
    fflush(stdout);

    // Case 2: Context with TopSecret Clearance (Level 3) attempting TopSecret write (> 900000)
    sdk_cics_context_t ctx_high = {
        .env = &env,
        .cache = &cache,
        .quorum_type = SDK_QUORUM_MAJORITY,
        .writer_id = 11,
        .security_clearance = 3 // High clearance
    };

    ok = auncient_sdk_alu_execute(&ctx_high, ALU_OP_WRITE_ABD, 950000, approvals, &result);
    assert(ok == true); // Should pass
    assert(result == 950000);
    printf("   ✓ Level 3 clearance successfully permitted TopSecret write.\n");
    fflush(stdout);

    auncient_sdk_close_coaxial(&env);
}

void test_sdk_autodin_precedence_locks(void) {
    printf("[TEST_ADV] Checking AUTODIN precedence-aware spin-locks...\n");
    fflush(stdout);

    sdk_coaxial_env_t env;
    bool ok = auncient_sdk_init_coaxial(&env);
    assert(ok == true);

    sdk_kermit_cache_t cache = { .cached_value = 0, .cached_ts = { 0, 0 }, .is_warm = false };
    sdk_cics_context_t ctx = {
        .env = &env,
        .cache = &cache,
        .quorum_type = SDK_QUORUM_MAJORITY,
        .writer_id = 20,
        .security_clearance = 3
    };

    // Acquire lock with Routine precedence ('R')
    printf("[TEST_ADV] Acquiring Routine precedence lock...\n");
    fflush(stdout);
    ok = auncient_sdk_autodin_spin_lock(&ctx, 0x111, 'R');
    assert(ok == true);

    // Release lock
    auncient_sdk_autodin_spin_unlock(&ctx, 0x111);
    printf("   ✓ Routine lock and release cycle complete.\n");
    fflush(stdout);

    // Acquire lock with Flash precedence ('F')
    printf("[TEST_ADV] Acquiring Flash precedence lock...\n");
    fflush(stdout);
    ok = auncient_sdk_autodin_spin_lock(&ctx, 0x222, 'F');
    assert(ok == true);

    auncient_sdk_autodin_spin_unlock(&ctx, 0x222);
    printf("   ✓ Flash lock and release cycle complete.\n");
    fflush(stdout);

    auncient_sdk_close_coaxial(&env);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SDK ADVANCED SECURITY & LOCK SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    test_sdk_security_clearance_levels();
    test_sdk_autodin_precedence_locks();

    printf("=============================================================\n");
    printf("SDK ADVANCED TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
