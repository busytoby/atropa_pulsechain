#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// -------------------------------------------------------------
// Base Contract Interface Specifications
// -------------------------------------------------------------
bool base_precondition(uint32_t amount) {
    // Base rule: Must withdraw at least 1 unit
    return (amount >= 1);
}

bool base_postcondition(uint32_t old_balance, uint32_t new_balance, uint32_t amount) {
    // Base rule: New balance must decrease exactly by the withdrawn amount
    return (new_balance == old_balance - amount);
}

// -------------------------------------------------------------
// Behavioral Subtyping Analyzer (Liskov Substitution Checker)
// -------------------------------------------------------------
// A sub-contract is compliant ONLY if:
// 1. Its pre-condition is WEAKER than (or equal to) the base pre-condition.
//    (If base allows it, sub-contract MUST allow it).
// 2. Its post-condition is STRONGER than (or equal to) the base post-condition.
//    (Must guarantee at least what the base guarantees).
bool verify_precondition_subtyping(bool (*sub_pre)(uint32_t), bool (*base_pre)(uint32_t), uint32_t test_val) {
    // If base allows test_val, sub-contract must also allow it.
    // i.e., base_pre(test_val) => sub_pre(test_val)
    if (base_pre(test_val) && !sub_pre(test_val)) {
        return false; // Violates: pre-condition was strengthened!
    }
    return true;
}

// -------------------------------------------------------------
// Sub-Contract Implementations
// -------------------------------------------------------------
// Compliant Sub-Contract (Weaker Pre-condition: allows all values including 0)
bool compliant_sub_precondition(uint32_t amount) {
    (void)amount;
    return true;
}

// Non-Compliant Sub-Contract (Strengthened Pre-condition: restricts to >= 10)
bool bad_sub_precondition(uint32_t amount) {
    return (amount >= 10);
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SIGPLAN BEHAVIORAL SUBTYPING ANALYZER\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Verify Compliant Sub-contract Pre-condition
    // Since compliant_sub_precondition allows >= 0 (weaker than >= 1), it should pass subtyping check
    printf("[TEST] Checking compliant sub-contract subtyping...\n");
    fflush(stdout);
    bool ok = true;
    for (uint32_t val = 0; val < 100; val++) {
        if (!verify_precondition_subtyping(compliant_sub_precondition, base_precondition, val)) {
            ok = false;
            break;
        }
    }
    assert(ok == true);
    printf("   ✓ Compliant subtyping confirmed (pre-condition weakened).\n");
    fflush(stdout);

    // 2. Verify Non-compliant Sub-contract Pre-condition
    // bad_sub_precondition requires >= 10 (stronger than >= 1).
    // A value of 5 is valid for base, but rejected by sub-contract -> Violates Liskov!
    printf("[TEST] Checking non-compliant sub-contract (strengthened pre-condition)...\n");
    fflush(stdout);
    bool compliance_failed = false;
    for (uint32_t val = 0; val < 100; val++) {
        if (!verify_precondition_subtyping(bad_sub_precondition, base_precondition, val)) {
            compliance_failed = true;
            printf("   ✓ Subtyping violation trapped at value: %u\n", val);
            break;
        }
    }
    assert(compliance_failed == true);
    printf("   ✓ Non-compliant subtyping successfully flagged.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("BEHAVIORAL SUBTYPING TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
