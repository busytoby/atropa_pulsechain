#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// Enums matching the Auncient VM registers
typedef enum {
    HELMHOLTZ_OP_IDENTITY = 0,
    HELMHOLTZ_OP_RESONANCE = 4
} HelmholtzOpType;

// Yul Opcode definition
typedef enum {
    YUL_OP_QUERY_READ = 0x30
} YulOpcode;

// Represents our 2-Channel data payload on the coaxial bus
typedef struct {
    uint64_t b_channel_0; // Sender / Caller address
    uint64_t b_channel_1; // Recipient address or Query key
    uint64_t value;       // Transfer amount or Write value
    YulOpcode d_channel;  // Control / Opcode
} TwoChannelTx;

// Global state variables simulating database
uint64_t g_balances[100]; 
bool g_seen_clients[100]; // Tracks if we have seen the client address slot before
const uint64_t UNIVERSAL_GAS_FEE = 15;
const uint64_t DEFAULT_BOOTSTRAP_ALLOWANCE = 50;

uint32_t get_balance_slot(uint64_t address) {
    return address % 100;
}

uint64_t get_balance(uint64_t address) {
    return g_balances[get_balance_slot(address)];
}

void set_balance(uint64_t address, uint64_t balance) {
    g_balances[get_balance_slot(address)] = balance;
}

// Pre-Flight Gas Auditing with Default Allowance Bootstrap
bool preflight_gas_check_with_bootstrap(uint64_t caller, const char **out_err) {
    uint32_t slot = get_balance_slot(caller);
    
    // Check if client is new/unknown
    if (!g_seen_clients[slot]) {
        g_seen_clients[slot] = true;
        set_balance(caller, DEFAULT_BOOTSTRAP_ALLOWANCE);
        printf("   [Bootstrap] New unknown client 0x%lx detected. Granted initial allowance: %lu Gas\n",
               caller, DEFAULT_BOOTSTRAP_ALLOWANCE);
    }
    
    uint64_t balance = get_balance(caller);
    if (balance < UNIVERSAL_GAS_FEE) {
        *out_err = "REVERT: INSUFFICIENT_GAS_BALANCE";
        return false;
    }
    return true;
}

// Executes a coaxial transaction
bool process_coaxial_transaction(TwoChannelTx *tx, const char **out_err) {
    uint64_t caller = tx->b_channel_0;
    
    // Pre-Flight check
    if (!preflight_gas_check_with_bootstrap(caller, out_err)) {
        return false;
    }
    
    // Deduct gas fee
    uint64_t balance = get_balance(caller);
    set_balance(caller, balance - UNIVERSAL_GAS_FEE);
    
    // Execute opcode
    if (tx->d_channel == YUL_OP_QUERY_READ) {
        uint64_t target_key = tx->b_channel_1;
        uint64_t val = get_balance(target_key);
        printf("   [READ QUERY] Balance of 0x%lx is %lu (Gas Charged: %lu)\n", target_key, val, UNIVERSAL_GAS_FEE);
    } else {
        *out_err = "REVERT: INVALID_OPCODE";
        return false;
    }
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: MCP UNKNOWN CLIENT BOOTSTRAP GAS TESTS\n");
    printf("=============================================================\n");

    // Initialize databases
    memset(g_balances, 0, sizeof(g_balances));
    memset(g_seen_clients, 0, sizeof(g_seen_clients));

    uint64_t unknown_client = 0x99AABBCC;
    const char *err = NULL;

    // 1. First query by unknown client (balance is initially 0)
    printf("1. Submitting read query from unknown client 0x%lx...\n", unknown_client);
    TwoChannelTx tx_first = {
        .b_channel_0 = unknown_client,
        .b_channel_1 = unknown_client, // Querying own balance
        .value = 0,
        .d_channel = YUL_OP_QUERY_READ
    };
    
    bool res = process_coaxial_transaction(&tx_first, &err);
    assert(res == true);
    
    // Balance should be: 50 (allowance) - 15 (gas fee) = 35
    uint64_t bal1 = get_balance(unknown_client);
    assert(bal1 == 35);
    printf("   ✓ First query succeeded. Current Balance: %lu Gas\n\n", bal1);

    // 2. Second query by same client (uses remaining balance)
    printf("2. Submitting second read query from client 0x%lx...\n", unknown_client);
    TwoChannelTx tx_second = {
        .b_channel_0 = unknown_client,
        .b_channel_1 = unknown_client,
        .value = 0,
        .d_channel = YUL_OP_QUERY_READ
    };
    
    res = process_coaxial_transaction(&tx_second, &err);
    assert(res == true);
    
    // Balance should be: 35 - 15 = 20
    uint64_t bal2 = get_balance(unknown_client);
    assert(bal2 == 20);
    printf("   ✓ Second query succeeded. Current Balance: %lu Gas\n\n", bal2);

    // 3. Exhausting gas balance
    printf("3. Exhausting gas balance to trigger reversion...\n");
    // Force balance down to 5 gas
    set_balance(unknown_client, 5);
    
    TwoChannelTx tx_third = {
        .b_channel_0 = unknown_client,
        .b_channel_1 = unknown_client,
        .value = 0,
        .d_channel = YUL_OP_QUERY_READ
    };
    
    res = process_coaxial_transaction(&tx_third, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: INSUFFICIENT_GAS_BALANCE") == 0);
    printf("   ✓ Reversion successful: %s\n\n", err);

    printf("=============================================================\n");
    printf("AUNCIENT UNKNOWN CLIENT BOOTSTRAP TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
