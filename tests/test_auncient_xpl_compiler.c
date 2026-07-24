#include "auncient_sdk.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// -------------------------------------------------------------
// .xpl to .dat.bin Compiler & Execution Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT .XPL SOURCE COMPILER & EXECUTION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    const char *src_path = "tests/test_source.xpl";
    const char *bin_path = "tests/test_compiled.dat.bin";
    const char *sub_src_path = "tests/sub_source.xpl";
    const char *sub_bin_path = "tests/sub_99.dat.bin";

    // 1. Create a dummy sub .xpl source text file (sets value to 1234)
    FILE *sub_src = fopen(sub_src_path, "w");
    assert(sub_src != NULL);
    fprintf(sub_src, "WRITE_ABD 1234 1 1 1 0\n");
    fclose(sub_src);

    // 2. Compile sub .xpl
    bool ok = auncient_sdk_compile_xpl_to_dat_bin(sub_src_path, sub_bin_path);
    assert(ok == true);

    // 3. Create a parent .xpl source text file (loads sub 99, then reads)
    FILE *src = fopen(src_path, "w");
    assert(src != NULL);
    fprintf(src, "LOAD_SUB_XPL 99\n");
    fprintf(src, "READ_KERMIT\n");
    fclose(src);

    printf("[TEST] Compiling parent .xpl source to binary .dat.bin stream...\n");
    fflush(stdout);

    ok = auncient_sdk_compile_xpl_to_dat_bin(src_path, bin_path);
    assert(ok == true);
    printf("   ✓ Parent compiler executed successfully.\n");
    fflush(stdout);

    // 4. Execute using the primary executable loader
    // This instantiates its own dedicated AUTODIN lock environment in memory,
    // while the loaded sub_99.dat.bin execution runs on the parent's environment.
    printf("[TEST] Executing parent compiled binary as primary .bin via SDK...\n");
    fflush(stdout);

    uint32_t results[2] = { 0 };
    ok = auncient_sdk_execute_primary_bin(bin_path, results, 2);
    assert(ok == true);
    assert(results[0] == 99);   // Sub-load trace
    assert(results[1] == 1234); // Value read
    printf("   ✓ Primary execution succeeded. Result 0 (sub-load ID): %u, Result 1 (read value): %u.\n", results[0], results[1]);
    fflush(stdout);

    // 5. Test Behavioral Subtyping Violation (Sub-binary requires TopSecret 950000 but context has level 2 clearance)
    printf("[TEST] Testing behavioral subtyping violations on load...\n");
    fflush(stdout);

    FILE *sub_violator = fopen(sub_src_path, "w");
    assert(sub_violator != NULL);
    fprintf(sub_violator, "WRITE_ABD 950000 1 1 1 0\n"); // TopSecret write
    fclose(sub_violator);

    ok = auncient_sdk_compile_xpl_to_dat_bin(sub_src_path, sub_bin_path);
    assert(ok == true);

    sdk_coaxial_env_t env;
    ok = auncient_sdk_init_coaxial(&env);
    assert(ok == true);
    // Initialize node value to 444
    env.registers[0].value = 444;

    sdk_kermit_cache_t cache = { .cached_value = 444, .cached_ts = { 0, 0 }, .is_warm = true };
    sdk_cics_context_t low_clearance_ctx = {
        .env = &env,
        .cache = &cache,
        .quorum_type = SDK_QUORUM_MAJORITY,
        .writer_id = 88,
        .security_clearance = 2, // Insufficient for TopSecret write
        .has_lock = false
    };

    // Attempting load should fail behavioral subtyping verification
    ok = auncient_sdk_execute_dat_bin(&low_clearance_ctx, bin_path, results, 2);
    assert(ok == false);
    printf("   ✓ Load failed successfully due to subtyping violation.\n");
    fflush(stdout);

    // Verify Cascaded Rollback (register remains 444, not corrupted by failed execution)
    assert(env.registers[0].value == 444);
    printf("   ✓ Cascaded rollback confirmed: register value preserved at 444.\n");
    fflush(stdout);

    // 6. Test Temporal Invariant Enforcement (attempt to write without holding lock)
    printf("[TEST] Testing temporal invariant checks (write without lock block)...\n");
    fflush(stdout);
    ok = auncient_sdk_validate_temporal_invariants(&low_clearance_ctx, ALU_OP_WRITE_ABD, 888);
    assert(ok == false); // Should fail temporal lock validation
    printf("   ✓ Write without active lock rejected by temporal check.\n");
    fflush(stdout);

    // 7. Test Invariant Monotonicity Auditing (Welch Sequence Rules)
    printf("[TEST] Testing monotonicity invariants (sequence rollback)...\n");
    fflush(stdout);
    env.registers[0].ts.counter = 50; // Current timestamp
    ok = auncient_sdk_validate_monotonicity(&env, 0, 45); // Stale sequence target
    assert(ok == false);
    ok = auncient_sdk_validate_monotonicity(&env, 0, 55); // Valid sequential sequence
    assert(ok == true);
    printf("   ✓ Monotonicity constraints validated correctly.\n");
    fflush(stdout);

    // 8. Test Post-Condition Oracle Verification
    printf("[TEST] Testing post-condition oracle validations...\n");
    fflush(stdout);
    bool approvals[SDK_NUM_NODES] = { true, true, true, false };
    // Majority quorum approvals (3 of 4) evaluates mathematically to 1 (true)
    ok = auncient_sdk_verify_postcondition_oracle(&low_clearance_ctx, ALU_OP_EVAL_ACKERMAN, approvals, 1);
    assert(ok == true);
    // Supplying a result mismatch (0 instead of 1) should be trapped by the oracle
    ok = auncient_sdk_verify_postcondition_oracle(&low_clearance_ctx, ALU_OP_EVAL_ACKERMAN, approvals, 0);
    assert(ok == false);
    printf("   ✓ Post-condition oracle verification trapped discrepancies successfully.\n");
    fflush(stdout);

    auncient_sdk_close_coaxial(&env);

    // Clean up temporary files
    remove(src_path);
    remove(bin_path);
    remove(sub_src_path);
    remove(sub_bin_path);

    printf("=============================================================\n");
    printf("XPL COMPILER & EXECUTION TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
