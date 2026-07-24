#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#define ACCOUNTS_COUNT 4

typedef struct {
    uint32_t balances[ACCOUNTS_COUNT];
    uint32_t total_supply;
} token_contract_state_t;

// -------------------------------------------------------------
// SIGPLAN Design-by-Contract (DbC) Invariant Checker
// -------------------------------------------------------------
bool check_contract_invariant(const token_contract_state_t *state) {
    uint64_t sum = 0;
    for (int i = 0; i < ACCOUNTS_COUNT; i++) {
        sum += state->balances[i];
    }
    // Invariant: The sum of all balances must exactly equal the declared total supply
    return (sum == state->total_supply);
}

// Executes a transfer under strict pre-conditions and post-conditions
bool execute_contract_transfer(token_contract_state_t *state, uint32_t from, uint32_t to, uint32_t amount) {
    // Pre-condition 1: Account indices must be valid
    if (from >= ACCOUNTS_COUNT || to >= ACCOUNTS_COUNT) {
        return false;
    }

    // Pre-condition 2: Sender must have sufficient balance
    if (state->balances[from] < amount) {
        return false;
    }

    // Backup state for recovery on rollback
    token_contract_state_t rollback_state = *state;

    // Execute state modification
    state->balances[from] -= amount;
    
    // Simulating potential overflow / unauthorized state modification check
    uint32_t prev_balance = state->balances[to];
    state->balances[to] += amount;

    // Check for integer overflow (Post-condition 1)
    if (state->balances[to] < prev_balance) {
        *state = rollback_state; // Rollback
        return false;
    }

    // Verify contract invariant (Post-condition 2)
    if (!check_contract_invariant(state)) {
        *state = rollback_state; // Rollback
        return false;
    }

    return true; // Transaction committed
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SIGPLAN CONTRACT INVARIANT MONITORS\n");
    printf("=============================================================\n");
    fflush(stdout);

    token_contract_state_t state = {
        .balances = { 100, 200, 50, 0 },
        .total_supply = 350
    };

    // Assert initial invariant holds
    assert(check_contract_invariant(&state) == true);

    // 1. Valid Transfer: Account 1 -> Account 3 (amount = 50) -> Should pass
    printf("[TEST] Executing valid contract transfer...\n");
    fflush(stdout);
    bool ok = execute_contract_transfer(&state, 1, 3, 50);
    assert(ok == true);
    assert(state.balances[1] == 150);
    assert(state.balances[3] == 50);
    assert(check_contract_invariant(&state) == true);
    printf("   ✓ Transfer committed. Contract invariant preserved.\n");
    fflush(stdout);

    // 2. Invalid Transfer: Account 2 -> Account 0 (amount = 100) -> Should fail pre-condition
    printf("[TEST] Executing invalid contract transfer (insufficient balance)...\n");
    fflush(stdout);
    ok = execute_contract_transfer(&state, 2, 0, 100);
    assert(ok == false); // Should reject
    assert(state.balances[2] == 50); // Unchanged
    printf("   ✓ Transfer rejected successfully by pre-conditions.\n");
    fflush(stdout);

    // 3. Simulating arithmetic overflow rollback (Post-condition trap)
    printf("[TEST] Executing contract transfer with overflow condition...\n");
    fflush(stdout);
    // Artificially configure Account 0 to max to force overflow on transfer
    state.balances[0] = 0xFFFFFFFF;
    state.total_supply = 0xFFFFFFFF + 150 + 50 + 50; // Temporarily update supply to maintain invariant
    
    ok = execute_contract_transfer(&state, 1, 0, 10);
    assert(ok == false); // Should trap overflow and roll back
    assert(state.balances[1] == 150); // Restored
    assert(state.balances[0] == 0xFFFFFFFF); // Restored
    printf("   ✓ Overflow caught by post-conditions. State rolled back successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("SIGPLAN INVARIANT MONITOR TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
