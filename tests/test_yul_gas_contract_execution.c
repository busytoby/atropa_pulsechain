#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// Mocking the Yul GasContract storage slots
// Slot 0: Owner address
// Slot 1: Total Supply
// Slot 2: Balances mapping offset (base)
// Slot 3: Burned mapping offset (base)

uint64_t g_yul_storage[1000]; // Simulated storage array

// Helper to compute storage slot index for mapping balance: keccak256(account, slot)
uint32_t get_balance_slot_yul(uint64_t account) {
    // Simple mock hash mapping (simulating the keccak256 offset computation)
    return (account + 2) % 1000;
}

uint32_t get_burned_slot_yul(uint64_t account) {
    return (account + 3) % 1000;
}

// Simulated Yul functions translating GasContract.yul strict-assembly directly
void yul_constructor(uint64_t owner) {
    g_yul_storage[0] = owner;          // sstore(0, owner)
    g_yul_storage[1] = 1000000;        // sstore(1, initial supply: 1,000,000)
    
    // Set initial balance of owner to total supply
    uint32_t owner_bal_slot = get_balance_slot_yul(owner);
    g_yul_storage[owner_bal_slot] = 1000000;
}

uint64_t yul_balanceOf(uint64_t account) {
    uint32_t slot = get_balance_slot_yul(account);
    return g_yul_storage[slot]; // sload(slot)
}

bool yul_transfer(uint64_t sender, uint64_t recipient, uint64_t amount, const char **out_err) {
    uint32_t sender_slot = get_balance_slot_yul(sender);
    uint32_t recipient_slot = get_balance_slot_yul(recipient);
    
    uint64_t sender_bal = g_yul_storage[sender_slot];
    if (sender_bal < amount) {
        *out_err = "REVERT: INSUFFICIENT_BALANCE";
        return false;
    }
    
    g_yul_storage[sender_slot] = sender_bal - amount;
    g_yul_storage[recipient_slot] = g_yul_storage[recipient_slot] + amount;
    return true;
}

bool yul_burn(uint64_t account, uint64_t amount, const char **out_err) {
    uint32_t bal_slot = get_balance_slot_yul(account);
    uint32_t burn_slot = get_burned_slot_yul(account);
    
    uint64_t balance = g_yul_storage[bal_slot];
    if (balance < amount) {
        *out_err = "REVERT: INSUFFICIENT_BALANCE_TO_BURN";
        return false;
    }
    
    g_yul_storage[bal_slot] = balance - amount;
    g_yul_storage[burn_slot] = g_yul_storage[burn_slot] + amount;
    g_yul_storage[1] = g_yul_storage[1] - amount; // Reduce total supply
    return true;
}

bool yul_mint(uint64_t caller, uint64_t recipient, uint64_t amount, const char **out_err) {
    uint64_t owner = g_yul_storage[0];
    if (caller != owner) {
        *out_err = "REVERT: ONLY_OWNER_CAN_MINT";
        return false;
    }
    
    uint32_t dest_slot = get_balance_slot_yul(recipient);
    g_yul_storage[dest_slot] = g_yul_storage[dest_slot] + amount;
    g_yul_storage[1] = g_yul_storage[1] + amount; // Increase total supply
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: YUL GAS CONTRACT SIMULATOR VERIFICATION\n");
    printf("=============================================================\n");

    memset(g_yul_storage, 0, sizeof(g_yul_storage));

    uint64_t owner = 0x01A0B2C3;
    uint64_t user_alice = 0x02D0E4F5;
    uint64_t user_bob = 0x03FF;
    const char *err = NULL;

    // 1. Constructor execution
    printf("1. Running constructor (Owner: 0x%lx)...\n", owner);
    yul_constructor(owner);
    assert(g_yul_storage[0] == owner);
    assert(g_yul_storage[1] == 1000000);
    assert(yul_balanceOf(owner) == 1000000);
    printf("   ✓ Constructor logic initialized successfully. Owner Balance: %lu\n\n", yul_balanceOf(owner));

    // 2. Transfer validation
    printf("2. Transferring 50,000 tokens from Owner to Alice...\n");
    bool res = yul_transfer(owner, user_alice, 50000, &err);
    assert(res == true);
    assert(yul_balanceOf(owner) == 950000);
    assert(yul_balanceOf(user_alice) == 50000);
    printf("   ✓ Transfer succeeded. Alice Balance: %lu\n\n", yul_balanceOf(user_alice));

    // 3. Reverted transfer validation
    printf("3. Attempting transfer exceeding balance (Bob tries to send 10 tokens, balance is 0)...\n");
    res = yul_transfer(user_bob, user_alice, 10, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: INSUFFICIENT_BALANCE") == 0);
    printf("   ✓ Transfer rejected successfully: %s\n\n", err);

    // 4. Burn validation
    printf("4. Burning 10,000 tokens from Alice's account...\n");
    res = yul_burn(user_alice, 10000, &err);
    assert(res == true);
    assert(yul_balanceOf(user_alice) == 40000);
    assert(g_yul_storage[get_burned_slot_yul(user_alice)] == 10000);
    assert(g_yul_storage[1] == 990000); // Total supply updated
    printf("   ✓ Burn succeeded. Alice Balance: %lu | Total Supply: %lu\n\n",
           yul_balanceOf(user_alice), g_yul_storage[1]);

    // 5. Mint validation (Owner only)
    printf("5. Minting 20,000 tokens to Bob (Caller: Owner)...\n");
    res = yul_mint(owner, user_bob, 20000, &err);
    assert(res == true);
    assert(yul_balanceOf(user_bob) == 20000);
    assert(g_yul_storage[1] == 1010000);
    printf("   ✓ Mint succeeded. Bob Balance: %lu | Total Supply: %lu\n\n",
           yul_balanceOf(user_bob), g_yul_storage[1]);

    printf("6. Minting attempt by non-owner (Caller: Alice)...\n");
    res = yul_mint(user_alice, user_bob, 5000, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: ONLY_OWNER_CAN_MINT") == 0);
    printf("   ✓ Non-owner mint rejected successfully: %s\n\n", err);

    printf("=============================================================\n");
    printf("AUNCIENT YUL GAS CONTRACT TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
