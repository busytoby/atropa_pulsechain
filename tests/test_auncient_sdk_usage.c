#include "auncient_sdk.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// -------------------------------------------------------------
// SDK Usage Unit Tests
// -------------------------------------------------------------
void test_sdk_grid_quorum(void) {
    printf("[TEST_USAGE] Verifying Grid Quorum using CICS Exec...\n");
    fflush(stdout);

    sdk_coaxial_env_t env;
    bool ok = auncient_sdk_init_coaxial(&env);
    assert(ok == true);

    sdk_kermit_cache_t cache = { .cached_value = 0, .cached_ts = { 0, 0 }, .is_warm = false };
    sdk_cics_context_t ctx = {
        .env = &env,
        .cache = &cache,
        .quorum_type = SDK_QUORUM_GRID,
        .writer_id = 1
    };

    // Case 1: Insufficient approvals for Grid (Node 0 and Node 1 approve -> Row 0, but no Col)
    bool approvals_fail[SDK_NUM_NODES] = { true, true, false, false };
    ok = auncient_sdk_cics_exec(&ctx, 8888, approvals_fail);
    assert(ok == false); // Should reject and abort
    printf("   ✓ Row-only approvals rejected successfully.\n");
    fflush(stdout);

    // Case 2: Sufficient approvals for Grid (Nodes 0, 1, 2 approve -> Row 0 [0,1] + Col 0 [0,2])
    bool approvals_pass[SDK_NUM_NODES] = { true, true, true, false };
    ok = auncient_sdk_cics_exec(&ctx, 9999, approvals_pass);
    assert(ok == true); // Should commit

    uint32_t val = 0;
    auncient_sdk_alu_execute(&ctx, ALU_OP_READ_KERMIT, 0, NULL, &val);
    assert(val == 9999);
    printf("   ✓ Row + Column approvals committed successfully.\n");
    fflush(stdout);

    auncient_sdk_close_coaxial(&env);
}

void test_sdk_weighted_quorum(void) {
    printf("[TEST_USAGE] Verifying Weighted Quorum using CICS Exec...\n");
    fflush(stdout);

    sdk_coaxial_env_t env;
    bool ok = auncient_sdk_init_coaxial(&env);
    assert(ok == true);

    // Node weights: Node 0 = 3, Node 1 = 1, Node 2 = 1, Node 3 = 1
    sdk_kermit_cache_t cache = { .cached_value = 0, .cached_ts = { 0, 0 }, .is_warm = false };
    sdk_cics_context_t ctx = {
        .env = &env,
        .cache = &cache,
        .quorum_type = SDK_QUORUM_WEIGHTED,
        .writer_id = 2
    };

    // Case 1: Node 0 (wt 3) + Node 1 (wt 1) = 4 -> Threshold met
    bool approvals_pass[SDK_NUM_NODES] = { true, true, false, false };
    ok = auncient_sdk_cics_exec(&ctx, 5555, approvals_pass);
    assert(ok == true);
    printf("   ✓ Weighted threshold met (Node 0 + Node 1). Write committed.\n");
    fflush(stdout);

    // Case 2: Node 1 (wt 1) + Node 2 (wt 1) = 2 -> Under threshold 4
    bool approvals_fail[SDK_NUM_NODES] = { false, true, true, false };
    ok = auncient_sdk_cics_exec(&ctx, 7777, approvals_fail);
    assert(ok == false);
    printf("   ✓ Weighted threshold missed (Node 1 + Node 2). Write aborted.\n");
    fflush(stdout);

    auncient_sdk_close_coaxial(&env);
}

void test_sdk_kermit_cache_invalidation(void) {
    printf("[TEST_USAGE] Verifying KERMIT Cache warming and direct ALU execution...\n");
    fflush(stdout);

    sdk_coaxial_env_t env;
    bool ok = auncient_sdk_init_coaxial(&env);
    assert(ok == true);

    sdk_kermit_cache_t cache = { .cached_value = 0, .cached_ts = { 0, 0 }, .is_warm = false };
    sdk_cics_context_t ctx = {
        .env = &env,
        .cache = &cache,
        .quorum_type = SDK_QUORUM_MAJORITY,
        .writer_id = 3
    };

    // Direct ALU Write instruction (Node 0, 1, 2 approve)
    bool approvals[SDK_NUM_NODES] = { true, true, true, false };
    uint32_t result = 0;
    ok = auncient_sdk_alu_execute(&ctx, ALU_OP_WRITE_ABD, 777, approvals, &result);
    assert(ok == true);
    assert(result == 777);
    assert(cache.is_warm == true);
    assert(cache.cached_value == 777);
    printf("   ✓ Direct ALU_WRITE_ABD executed. Cache warmed successfully.\n");
    fflush(stdout);

    // Modify backend register directly to simulate asynchronous updates
    env.registers[0].value = 999;
    env.registers[0].ts.counter = 100; // Much higher timestamp

    // Cache is still warm, so ALU read should return cached value (777) rather than backend value (999)
    uint32_t val = 0;
    auncient_sdk_alu_execute(&ctx, ALU_OP_READ_KERMIT, 0, NULL, &val);
    assert(val == 777);
    printf("   ✓ Cache read hit returned cached value 777 (asynchronous bypass).\n");
    fflush(stdout);

    // Invalidate cache
    cache.is_warm = false;

    // Read again - cache miss forces backend reload, which should fetch the updated value (999)
    auncient_sdk_alu_execute(&ctx, ALU_OP_READ_KERMIT, 0, NULL, &val);
    assert(val == 999);
    assert(cache.is_warm == true);
    assert(cache.cached_value == 999);
    printf("   ✓ Cache invalidation forced reload. Read returned updated value 999.\n");
    fflush(stdout);

    auncient_sdk_close_coaxial(&env);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SDK DETAILED USAGE VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    test_sdk_grid_quorum();
    test_sdk_weighted_quorum();
    test_sdk_kermit_cache_invalidation();

    printf("=============================================================\n");
    printf("SDK USAGE TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
