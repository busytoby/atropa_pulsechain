#include "auncient_sdk.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// Resilient recovery callback for testing
static void test_recovery_callback(sdk_cics_context_t *ctx, sdk_status_code_t error_code) {
    if (error_code == SDK_STATUS_ERR_SECURITY || error_code == SDK_STATUS_ERR_TRANSACTION) {
        // Recover by elevating security clearance level to satisfy pre-conditions
        ctx->security_clearance = 3;
    }
}

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
    const char *log_dat_bin_path = "tests/blame_log.dat.bin";

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
        .has_lock = false,
        .current_lock_precedence = '\0',
        .lock_stack_depth = 0,
        .state = SDK_STATE_UNLOCKED,
        .is_contract_checking = false,
        .last_blame = SDK_BLAME_NONE,
        .recovery_handler = NULL,
        .trace_index = 0,
        .trace_count = 0
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

    // 9. Test History Constraints (Liskov-Wing Weight Conservation)
    printf("[TEST] Testing history constraints (weight sum conservation)...\n");
    fflush(stdout);
    sdk_coaxial_env_t old_env;
    sdk_coaxial_env_t new_env;
    for (int i = 0; i < SDK_NUM_NODES; i++) {
        old_env.weights[i] = 1;
        new_env.weights[i] = 1;
    }
    // Conserved sum
    ok = auncient_sdk_validate_history_constraints(&old_env, &new_env);
    assert(ok == true);
    // Violated sum
    new_env.weights[0] = 5;
    ok = auncient_sdk_validate_history_constraints(&old_env, &new_env);
    assert(ok == false);
    printf("   ✓ History constraints validated correctly.\n");
    fflush(stdout);

    // 10. Test Frame Conditions (Modify Clauses)
    printf("[TEST] Testing frame conditions (modify clause gate)...\n");
    fflush(stdout);
    // Node 0 is approved in approvals list
    ok = auncient_sdk_validate_frame_conditions(ALU_OP_WRITE_ABD, approvals, 0);
    assert(ok == true);
    // Node 3 is unapproved in approvals list (should fail write access)
    ok = auncient_sdk_validate_frame_conditions(ALU_OP_WRITE_ABD, approvals, 3);
    assert(ok == false);
    printf("   ✓ Frame conditions modification clause validated correctly.\n");
    fflush(stdout);

    // 11. Test Typestate Lifecycle transitions
    printf("[TEST] Testing typestate lifecycle transitions...\n");
    fflush(stdout);
    low_clearance_ctx.state = SDK_STATE_UNLOCKED;
    // Direct ALU execute of ABD write should be blocked in SDK_STATE_UNLOCKED state
    ok = auncient_sdk_alu_execute(&low_clearance_ctx, ALU_OP_WRITE_ABD, 222, approvals, &results[0]);
    assert(ok == false);
    // Move to executing state
    ok = auncient_sdk_transition_typestate(&low_clearance_ctx, SDK_STATE_LOCKED);
    assert(ok == true);
    ok = auncient_sdk_transition_typestate(&low_clearance_ctx, SDK_STATE_EXECUTING);
    assert(ok == true);
    // Write should pass now under executing state (if lock is active)
    low_clearance_ctx.has_lock = true;
    ok = auncient_sdk_alu_execute(&low_clearance_ctx, ALU_OP_WRITE_ABD, 556, approvals, &results[0]);
    assert(ok == true);
    printf("   ✓ Typestate transition lifecycle verified successfully.\n");
    fflush(stdout);

    // 12. Test Dependent Types boundary conditions (Grid Quorum requires Even parity values)
    printf("[TEST] Testing dependent types boundary conditions...\n");
    fflush(stdout);
    low_clearance_ctx.quorum_type = SDK_QUORUM_GRID;
    // Even value 222 (valid dependent type under Grid Quorum)
    ok = auncient_sdk_validate_dependent_types(&low_clearance_ctx, ALU_OP_WRITE_ABD, 222);
    assert(ok == true);
    // Odd value 223 (invalid dependent type under Grid Quorum)
    ok = auncient_sdk_validate_dependent_types(&low_clearance_ctx, ALU_OP_WRITE_ABD, 223);
    assert(ok == false);
    printf("   ✓ Dependent type grid parity checks verified successfully.\n");
    fflush(stdout);

    // 13. Test Purity auditing (Side-Effect-Free verification checks)
    printf("[TEST] Testing purity audits (write block on check flag)...\n");
    fflush(stdout);
    low_clearance_ctx.is_contract_checking = true;
    ok = auncient_sdk_validate_purity(&low_clearance_ctx, ALU_OP_WRITE_ABD);
    assert(ok == false); // Write should trigger purity violation
    ok = auncient_sdk_validate_purity(&low_clearance_ctx, ALU_OP_READ_KERMIT);
    assert(ok == true);  // Read is pure/allowed
    low_clearance_ctx.is_contract_checking = false;
    printf("   ✓ Purity validations confirmed: writes blocked during contracts.\n");
    fflush(stdout);

    // 14. Test Contract Refinement checking & Invariant Strengthening (Behavioral subtyping refinement)
    printf("[TEST] Testing contract refinement & invariant strengthening...\n");
    fflush(stdout);
    sdk_cics_context_t parent_ctx = low_clearance_ctx;
    parent_ctx.security_clearance = 3;
    parent_ctx.quorum_type = SDK_QUORUM_WEIGHTED;
    parent_ctx.has_lock = true;

    sdk_cics_context_t child_ctx = low_clearance_ctx;
    child_ctx.security_clearance = 2; // Weakened pre-condition (requires LESS clearance)
    child_ctx.quorum_type = SDK_QUORUM_WEIGHTED;
    child_ctx.has_lock = true;

    // Conforming refinement & invariant strengthening
    ok = auncient_sdk_validate_contract_refinement(&parent_ctx, &child_ctx);
    assert(ok == true);
    ok = auncient_sdk_validate_invariant_strengthening(&parent_ctx, &child_ctx);
    assert(ok == true);

    // Non-conforming refinement (child requires MORE clearance, strengthening pre-condition)
    child_ctx.security_clearance = 4;
    ok = auncient_sdk_validate_contract_refinement(&parent_ctx, &child_ctx);
    assert(ok == false);

    // Non-conforming invariant strengthening (child degrades quorum to Majority, violating invariant rules)
    child_ctx.security_clearance = 2;
    child_ctx.quorum_type = SDK_QUORUM_MAJORITY;
    ok = auncient_sdk_validate_invariant_strengthening(&parent_ctx, &child_ctx);
    assert(ok == false);
    printf("   ✓ Contract refinement and invariant strengthening verified successfully.\n");
    fflush(stdout);

    // 15. Test Blame Assignment semantics & PLD logging
    printf("[TEST] Testing Findler-Felleisen blame assignments & logging...\n");
    fflush(stdout);
    low_clearance_ctx.last_blame = SDK_BLAME_NONE;
    low_clearance_ctx.security_clearance = 1; // Low clearance
    low_clearance_ctx.recovery_handler = NULL;
    low_clearance_ctx.quorum_type = SDK_QUORUM_MAJORITY;
    low_clearance_ctx.has_lock = true;
    
    // High-clearance value write (950000) fails pre-condition: blames the Caller
    ok = auncient_sdk_alu_execute(&low_clearance_ctx, ALU_OP_WRITE_ABD, 950000, approvals, &results[0]);
    assert(ok == false);
    assert(auncient_pld_verify_blame(&low_clearance_ctx, SDK_BLAME_CALLER) == true);
    
    // Broadcast blame over physical driver socket
    ok = auncient_pld_broadcast_blame(&low_clearance_ctx);
    assert(ok == true);

    // Log blame to disk (.dat.bin)
    ok = auncient_pld_log_blame_to_disk(&low_clearance_ctx, log_dat_bin_path);
    assert(ok == true);

    // Read back log from disk and verify its structure
    FILE *log_f = fopen(log_dat_bin_path, "rb");
    assert(log_f != NULL);
    sdk_blame_record_t read_record;
    size_t read_bytes = fread(&read_record, sizeof(sdk_blame_record_t), 1, log_f);
    fclose(log_f);
    assert(read_bytes == 1);
    assert(read_record.writer_id == low_clearance_ctx.writer_id);
    assert(read_record.blame_target == SDK_BLAME_CALLER);

    // Clear blame via driver query
    auncient_pld_clear_blame(&low_clearance_ctx);
    assert(auncient_pld_verify_blame(&low_clearance_ctx, SDK_BLAME_NONE) == true);
    printf("   ✓ Blame correctly verified, logged to disk, and cleared via PLD driver functions.\n");
    fflush(stdout);

    // 16. Test Resilient Contract Recovery Handlers
    printf("[TEST] Testing resilient contract recovery handlers...\n");
    fflush(stdout);
    low_clearance_ctx.security_clearance = 1; // Reset to low clearance
    low_clearance_ctx.recovery_handler = test_recovery_callback; // Set recovery hook
    
    // High-clearance value write should succeed because the recovery handler catches the clearance failure
    // and promotes clearance level to 3
    ok = auncient_sdk_alu_execute(&low_clearance_ctx, ALU_OP_WRITE_ABD, 950000, approvals, &results[0]);
    assert(ok == true);
    assert(low_clearance_ctx.security_clearance == 3);
    printf("   ✓ Resilient recovery handler executed and recovered state successfully.\n");
    fflush(stdout);

    // 17. Test Dependent Typestates (Grid Quorum locks transition unless all registers are even values)
    printf("[TEST] Testing dependent typestate transition rules...\n");
    fflush(stdout);
    low_clearance_ctx.quorum_type = SDK_QUORUM_GRID;
    low_clearance_ctx.state = SDK_STATE_EXECUTING;
    // Set node register to odd value 555
    env.registers[0].value = 555;
    // Attempting transition to COMMITTED state should be rejected
    ok = auncient_sdk_transition_typestate(&low_clearance_ctx, SDK_STATE_COMMITTED);
    assert(ok == false);

    // Set node register to even value 556
    env.registers[0].value = 556;
    // Attempting transition to COMMITTED state should succeed now
    ok = auncient_sdk_transition_typestate(&low_clearance_ctx, SDK_STATE_COMMITTED);
    assert(ok == true);
    printf("   ✓ Dependent typestate transition constraints validated correctly.\n");
    fflush(stdout);

    // 18. Test Autonomic Transactional Retries & Isolated Sandboxing
    printf("[TEST] Testing autonomic transactional retries & isolated sandboxing...\n");
    fflush(stdout);
    // Set clearance to 1 (will trigger pre-condition fail on TopSecret sub-binary execution)
    low_clearance_ctx.security_clearance = 1;
    low_clearance_ctx.quorum_type = SDK_QUORUM_MAJORITY;
    low_clearance_ctx.state = SDK_STATE_UNLOCKED;
    low_clearance_ctx.has_lock = false;
    low_clearance_ctx.current_lock_precedence = '\0';
    low_clearance_ctx.lock_stack_depth = 0;
    low_clearance_ctx.recovery_handler = test_recovery_callback; // Handler will elevate clearance on retry

    // Execute dat.bin containing TopSecret write instructions.
    // It will fail on the first run, rollback to snapshot, execute recovery handler, and succeed on the retry!
    ok = auncient_sdk_execute_dat_bin(&low_clearance_ctx, sub_bin_path, NULL, 0);
    assert(ok == true);
    assert(low_clearance_ctx.security_clearance == 3);

    // Verify Isolated Sandboxing pollution protection:
    // Create a sub-binary that writes to node 0, but then fails clearance.
    // Parent will execute with low clearance and NO recovery handler to force a final failure.
    FILE *sub_sandbox_src = fopen(sub_src_path, "w");
    assert(sub_sandbox_src != NULL);
    fprintf(sub_sandbox_src, "WRITE_ABD 888 1 0 0 0\n");      // Sandbox write 888 to node 0
    fprintf(sub_sandbox_src, "WRITE_ABD 950000 1 0 0 0\n");   // TopSecret write (fails clearance)
    fclose(sub_sandbox_src);

    const char *sub_sandbox_bin_path = "tests/sub_88.dat.bin";
    ok = auncient_sdk_compile_xpl_to_dat_bin(sub_src_path, sub_sandbox_bin_path);
    assert(ok == true);

    FILE *parent_sandbox_src = fopen(src_path, "w");
    assert(parent_sandbox_src != NULL);
    fprintf(parent_sandbox_src, "LOAD_SUB_XPL 88\n");
    fclose(parent_sandbox_src);

    ok = auncient_sdk_compile_xpl_to_dat_bin(src_path, bin_path);
    assert(ok == true);

    // Execute parent sandbox under clearance 1 (no recovery handler)
    env.registers[0].value = 400; // Baseline
    low_clearance_ctx.security_clearance = 1;
    low_clearance_ctx.recovery_handler = NULL;
    low_clearance_ctx.state = SDK_STATE_UNLOCKED;

    ok = auncient_sdk_execute_dat_bin(&low_clearance_ctx, bin_path, NULL, 0);
    assert(ok == false); // Must fail

    // Under isolated sandboxing, node 0 must NOT be polluted with 888 on sub-binary failure!
    assert(env.registers[0].value == 400); // Should remain 400
    printf("   ✓ Isolated sandboxing successfully isolated state. Register value remained: %u.\n", env.registers[0].value);
    fflush(stdout);

    // 19. Test Historical Trace Invariant Auditing
    printf("[TEST] Testing historical trace invariant auditing...\n");
    fflush(stdout);
    low_clearance_ctx.trace_index = 0;
    low_clearance_ctx.trace_count = 0;

    // Simulate trace pushing
    low_clearance_ctx.writer_id = 1;
    env.registers[0].value = 100;
    auncient_sdk_push_trace_entry(&low_clearance_ctx); // Sum 100

    env.registers[0].value = 200;
    auncient_sdk_push_trace_entry(&low_clearance_ctx); // Sum 200

    // Conforming trace: register sums are non-decreasing (100 -> 200)
    ok = auncient_sdk_validate_trace_invariants(&low_clearance_ctx);
    assert(ok == true);

    // Corrupted trace: register sum regresses (200 -> 150)
    env.registers[0].value = 150;
    auncient_sdk_push_trace_entry(&low_clearance_ctx); // Sum 150
    ok = auncient_sdk_validate_trace_invariants(&low_clearance_ctx);
    assert(ok == false);
    printf("   ✓ Historical trace checks trapped trajectory regressions correctly.\n");
    fflush(stdout);

    // 20. Test Lock Precedence & LIFO Stack Auditing
    printf("[TEST] Testing lock precedence hierarchy audits & LIFO stack validation...\n");
    fflush(stdout);
    low_clearance_ctx.has_lock = false;
    low_clearance_ctx.current_lock_precedence = '\0';
    low_clearance_ctx.lock_stack_depth = 0;

    // Acquire lock priority 'I' (Immediate, value 3)
    ok = auncient_sdk_autodin_spin_lock(&low_clearance_ctx, 0x123, 'I');
    assert(ok == true);
    assert(low_clearance_ctx.current_lock_precedence == 'I');

    // Attempting to acquire lock priority 'F' (Flash, value 4) should succeed via Priority Inheritance
    ok = auncient_sdk_autodin_spin_lock(&low_clearance_ctx, 0x124, 'F');
    assert(ok == true);
    assert(low_clearance_ctx.current_lock_precedence == 'F'); // Promoted
    assert(low_clearance_ctx.lock_stack_depth == 2);

    // LIFO Release Order: unlocking 0x123 (out-of-order) must fail
    ok = auncient_sdk_autodin_spin_unlock(&low_clearance_ctx, 0x123);
    assert(ok == false);

    // Unlocking 0x124 (top of stack) must succeed
    ok = auncient_sdk_autodin_spin_unlock(&low_clearance_ctx, 0x124);
    assert(ok == true);
    assert(low_clearance_ctx.current_lock_precedence == 'I'); // Restored to previous in stack

    // Unlocking 0x123 (now top of stack) must succeed
    ok = auncient_sdk_autodin_spin_unlock(&low_clearance_ctx, 0x123);
    assert(ok == true);
    assert(low_clearance_ctx.has_lock == false);
    assert(low_clearance_ctx.lock_stack_depth == 0);
    printf("   ✓ Priority inheritance and LIFO lock stack release audits verified successfully.\n");
    fflush(stdout);

    // 21. Test Load-Time Static Purity Checks
    printf("[TEST] Testing load-time static purity checks (write trap during contract checks)...\n");
    fflush(stdout);
    
    // Create a sub-binary that contains a write (sub_88.dat.bin)
    // When executing with is_contract_checking set to true (indicating a pure contract check block),
    // load-time static checks must reject this subtyping library.
    low_clearance_ctx.is_contract_checking = true;
    low_clearance_ctx.security_clearance = 3;

    // Load-time check must reject sub_88.dat.bin because it contains WRITE_ABD (not pure)
    ok = auncient_sdk_execute_dat_bin(&low_clearance_ctx, bin_path, NULL, 0);
    assert(ok == false);
    low_clearance_ctx.is_contract_checking = false;
    printf("   ✓ Static purity auditor successfully rejected state-modifying subtyping library.\n");
    fflush(stdout);

    // 22. Test Transition Invariants (Pre/Post Relation Constraints)
    printf("[TEST] Testing pre/post relation transition invariants...\n");
    fflush(stdout);
    env.registers[0].value = 500; // Baseline state
    // Conforming value transition (increasing/no regression)
    ok = auncient_sdk_validate_transition_invariant(&env, 0, 600);
    assert(ok == true);
    // Non-conforming value transition (regressing value from 500 to 499)
    ok = auncient_sdk_validate_transition_invariant(&env, 0, 499);
    assert(ok == false);
    printf("   ✓ Transition invariants trapped regression updates correctly.\n");
    fflush(stdout);

    auncient_sdk_close_coaxial(&env);

    // Clean up temporary files
    remove(src_path);
    remove(bin_path);
    remove(sub_src_path);
    remove(sub_bin_path);
    remove(sub_sandbox_bin_path);
    remove(log_dat_bin_path);

    printf("=============================================================\n");
    printf("XPL COMPILER & EXECUTION TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
