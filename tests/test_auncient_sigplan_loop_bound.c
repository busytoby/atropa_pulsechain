#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// Simulated loop representation inside the compiler AST
typedef struct {
    bool is_constant_bound;
    uint32_t bound_value;
    bool has_termination_proof;
} sigplan_loop_node_t;

// APDL Compiler Static Loop Auditor
typedef struct {
    sigplan_loop_node_t loop;
    bool sigplan_approved;
} sigplan_auditor_t;

// -------------------------------------------------------------
// SIGPLAN '73 Loop Termination Bounds Verification
// -------------------------------------------------------------
bool verify_sigplan_loop_bounds(sigplan_auditor_t *auditor, uint32_t max_allowed_limit) {
    auditor->sigplan_approved = false;

    // 1. Invariant check: Must have a proven termination condition
    if (!auditor->loop.has_termination_proof) {
        return false; // Loop cannot be proven to terminate
    }

    // 2. Bound check: Must be a constant (not dynamic or unknown)
    if (!auditor->loop.is_constant_bound) {
        return false; // Dynamic loop bounds cannot be statically verified
    }

    // 3. Limit check: Must not exceed hypervisor Wortman iteration clamp
    if (auditor->loop.bound_value > max_allowed_limit) {
        return false; // Exceeds safety envelope limit
    }

    auditor->sigplan_approved = true;
    return true; // Loop is verified safe under SIGPLAN termination specs
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SIGPLAN LOOP BOUND COMPILER AUDITOR SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Compliant Loop: Constant bound (5), within limit (10), proven termination
    sigplan_auditor_t safe_auditor = {
        .loop = { .is_constant_bound = true, .bound_value = 5, .has_termination_proof = true },
        .sigplan_approved = false
    };

    printf("[TEST] Auditing compliant loop with constant bound...\n");
    fflush(stdout);
    bool ok = verify_sigplan_loop_bounds(&safe_auditor, 10);
    assert(ok == true);
    assert(safe_auditor.sigplan_approved == true);
    printf("   ✓ Loop approved. Static safety properties verified.\n");
    fflush(stdout);

    // 2. Non-compliant Loop: Dynamic bound -> Should fail
    sigplan_auditor_t dynamic_auditor = {
        .loop = { .is_constant_bound = false, .bound_value = 0, .has_termination_proof = true },
        .sigplan_approved = false
    };

    printf("[TEST] Auditing loop with dynamic bounds...\n");
    fflush(stdout);
    ok = verify_sigplan_loop_bounds(&dynamic_auditor, 10);
    assert(ok == false);
    assert(dynamic_auditor.sigplan_approved == false);
    printf("   ✓ Dynamic loop bound rejected successfully.\n");
    fflush(stdout);

    // 3. Out-of-envelope Loop: Bound (15) exceeds Wortman limit (10) -> Should fail
    sigplan_auditor_t excessive_auditor = {
        .loop = { .is_constant_bound = true, .bound_value = 15, .has_termination_proof = true },
        .sigplan_approved = false
    };

    printf("[TEST] Auditing loop exceeding Wortman limit...\n");
    fflush(stdout);
    ok = verify_sigplan_loop_bounds(&excessive_auditor, 10);
    assert(ok == false);
    assert(excessive_auditor.sigplan_approved == false);
    printf("   ✓ Excessive loop bound rejected successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("SIGPLAN LOOP BOUND AUDITOR TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
