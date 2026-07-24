#include "auncient_sdk.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT CICS-ALU COAXIAL SDK INTEGRATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    sdk_coaxial_env_t env;
    bool ok = auncient_sdk_init_coaxial(&env);
    assert(ok == true);
    printf("   ✓ Coaxial access link initialized successfully.\n");
    fflush(stdout);

    sdk_kermit_cache_t cache = { .cached_value = 0, .cached_ts = { 0, 0 }, .is_warm = false };
    sdk_cics_context_t ctx = {
        .env = &env,
        .cache = &cache,
        .quorum_type = SDK_QUORUM_MAJORITY,
        .writer_id = 99
    };

    // 1. Transaction execution with valid majority approvals (Nodes 0, 1, 2 approve)
    bool approvals_majority[SDK_NUM_NODES] = { true, true, true, false };
    printf("[TEST] Running CICS transaction with Majority Quorum...\n");
    fflush(stdout);
    ok = auncient_sdk_cics_exec(&ctx, 12345, approvals_majority);
    assert(ok == true);

    uint32_t val = 0;
    auncient_sdk_alu_execute(&ctx, ALU_OP_READ_KERMIT, 0, NULL, &val);
    assert(val == 12345);
    printf("   ✓ Transaction committed. Read returned consistent value 12345.\n");
    fflush(stdout);

    // 2. Transaction execution with stale/insufficient approvals
    bool approvals_fail[SDK_NUM_NODES] = { false, true, false, false };
    printf("[TEST] Running CICS transaction with invalid quorum (expected fail)...\n");
    fflush(stdout);
    ok = auncient_sdk_cics_exec(&ctx, 67890, approvals_fail);
    assert(ok == false); // Should fail quorum validation
    printf("   ✓ Invalid transaction successfully aborted and rolled back.\n");
    fflush(stdout);

    auncient_sdk_close_coaxial(&env);
    printf("=============================================================\n");
    printf("SDK INTEGRATION TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
