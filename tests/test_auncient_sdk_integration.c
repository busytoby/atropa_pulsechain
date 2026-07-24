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
        .writer_id = 99,
        .security_clearance = 2
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

    // 3. Test dynamic weights configuration
    uint32_t new_weights[SDK_NUM_NODES] = { 2, 2, 0, 0 };
    printf("[TEST] Configuring new dynamic node weights...\n");
    fflush(stdout);
    ok = auncient_sdk_configure_weights(&env, new_weights);
    assert(ok == true);
    assert(env.weights[0] == 2);
    printf("   ✓ Dynamic node weights updated successfully.\n");
    fflush(stdout);

    // 4. Test batched operations execution
    printf("[TEST] Running batched ALU operations execution...\n");
    fflush(stdout);
    sdk_batched_op_t batch_ops[2] = {
        { .opcode = ALU_OP_WRITE_ABD, .value = 54321, .approvals = { true, true, false, false } },
        { .opcode = ALU_OP_READ_KERMIT, .value = 0, .approvals = { 0 } }
    };
    uint32_t batch_results[2] = { 0 };
    ok = auncient_sdk_batch_exec(&ctx, batch_ops, 2, batch_results);
    assert(ok == true);
    assert(batch_results[0] == 54321);
    assert(batch_results[1] == 54321); // Read should match what was just written
    printf("   ✓ Batched operations committed and verified successfully.\n");
    fflush(stdout);

    auncient_sdk_close_coaxial(&env);
    printf("=============================================================\n");
    printf("SDK INTEGRATION TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
