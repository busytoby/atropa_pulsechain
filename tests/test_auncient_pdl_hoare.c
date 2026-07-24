#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef struct {
    uint8_t tag;
    int32_t value;
} hoare_state_t;

#define TAG_KERNEL 0x01
#define TAG_USER   0x02

// -------------------------------------------------------------
// PDL Hoare-Style Proof Assistant: phi -> [pi]psi
// -------------------------------------------------------------
bool pdl_verify_hoare(const hoare_state_t *initial,
                      bool (*phi)(const hoare_state_t *),
                      void (*pi)(hoare_state_t *),
                      bool (*psi)(const hoare_state_t *)) {
    // If precondition phi is true, execute program pi and assert postcondition psi
    if (phi(initial)) {
        hoare_state_t state_copy = *initial;
        pi(&state_copy);
        
        if (!psi(&state_copy)) {
            return false; // Safety contract violated!
        }
    }
    return true; // Provably safe for this state
}

// Precondition: Tag must be KERNEL
bool precondition_kernel_only(const hoare_state_t *state) {
    return state->tag == TAG_KERNEL;
}

// Postcondition: Value must be non-negative (>= 0)
bool postcondition_non_negative(const hoare_state_t *state) {
    return state->value >= 0;
}

// Program 1 (Safe): Increments value by 10
void program_safe(hoare_state_t *state) {
    state->value += 10;
}

// Program 2 (Unsafe): Subtracts 50 (can cause negative values if initial value is small)
void program_unsafe(hoare_state_t *state) {
    state->value -= 50;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PDL HOARE PROOF ASSISTANT SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initial state setup: Value = 20
    hoare_state_t state = { .tag = TAG_KERNEL, .value = 20 };

    // 1. Verify Safe Program: precondition (KERNEL) -> [safe] postcondition (value >= 0)
    printf("[TEST] Verifying safe program under Hoare contract...\n");
    fflush(stdout);
    bool safe = pdl_verify_hoare(&state, precondition_kernel_only, program_safe, postcondition_non_negative);
    assert(safe == true);
    printf("   ✓ Program verified: safe under contract.\n");
    fflush(stdout);

    // 2. Verify Unsafe Program: precondition (KERNEL) -> [unsafe] postcondition (value >= 0)
    // Value will drop from 20 to -30, violating postcondition
    printf("[TEST] Verifying unsafe program (value will drop to negative)...\n");
    fflush(stdout);
    safe = pdl_verify_hoare(&state, precondition_kernel_only, program_unsafe, postcondition_non_negative);
    assert(safe == false); // Must detect contract violation
    printf("   ✓ Contract violation caught successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("PDL HOARE PROOF ASSISTANT TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
