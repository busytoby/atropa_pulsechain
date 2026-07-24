#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// Coaxial Token Resource representation under linear type rules
typedef struct {
    uint32_t token_id;
    uint32_t active_references; // Number of nodes concurrently holding this token
} linear_token_t;

// -------------------------------------------------------------
// SIGPLAN Linear Type Linearity Auditor
// -------------------------------------------------------------
bool verify_token_linearity(const linear_token_t *token) {
    // Linear Type Rule: A linear resource must be used exactly once.
    // 1. Duplication check: Cannot be held by more than one node (references > 1)
    if (token->active_references > 1) {
        return false; // Aliasing / duplication violation
    }

    // 2. Leak check: Cannot be discarded or unreferenced (references == 0)
    if (token->active_references == 0) {
        return false; // Resource leak violation
    }

    return true; // Linearity verified (exactly one active reference)
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SIGPLAN LINEAR RESOURCE AUDITOR SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Compliant state: Exactly one reference holding the token
    linear_token_t active_token = {
        .token_id = 0xAA,
        .active_references = 1
    };

    printf("[TEST] Checking compliant token linearity...\n");
    fflush(stdout);
    bool ok = verify_token_linearity(&active_token);
    assert(ok == true);
    printf("   ✓ Linearity verified. Token held by exactly one node.\n");
    fflush(stdout);

    // 2. Duplication violation: Two nodes holding the token simultaneously
    linear_token_t duplicated_token = {
        .token_id = 0xAA,
        .active_references = 2
    };

    printf("[TEST] Checking duplicated token violation...\n");
    fflush(stdout);
    ok = verify_token_linearity(&duplicated_token);
    assert(ok == false);
    printf("   ✓ Duplicate token reference trapped successfully.\n");
    fflush(stdout);

    // 3. Leak violation: Token discarded/lost
    linear_token_t leaked_token = {
        .token_id = 0xAA,
        .active_references = 0
    };

    printf("[TEST] Checking leaked token violation...\n");
    fflush(stdout);
    ok = verify_token_linearity(&leaked_token);
    assert(ok == false);
    printf("   ✓ Leaked token resource trapped successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("SIGPLAN LINEARITY TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
