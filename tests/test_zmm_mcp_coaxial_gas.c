#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// Standard enums matching the Auncient VM registers
typedef enum {
    HELMHOLTZ_OP_IDENTITY = 0,
    HELMHOLTZ_OP_RESONANCE = 4
} HelmholtzOpType;

// Yul Opcode definition
typedef enum {
    YUL_OP_GAS_TRANSFER = 0x20,
    YUL_OP_GAS_BURN = 0x21,
    YUL_OP_QUERY_READ = 0x30,
    YUL_OP_QUERY_WRITE = 0x31
} YulOpcode;

// Represents our 2-Channel data payload on the coaxial bus
typedef struct {
    uint64_t b_channel_0; // Sender / Caller address
    uint64_t b_channel_1; // Recipient address or Query key
    uint64_t value;       // Transfer amount or Write value
    YulOpcode d_channel;  // Control / Opcode
} TwoChannelTx;

// Peer entry for registry mocking
typedef struct {
    char uri[64];
    uint32_t port;
    bool is_active;
} McpPeer;

// Global state variables simulating storage
uint64_t g_balances[100]; // Map hashed address index -> balance
uint64_t g_gas_burned = 0;
McpPeer g_mcp_server;

// Simplified hash function mapping address to slot indices 0-99
uint32_t get_balance_slot(uint64_t address) {
    return address % 100;
}

// Read balance query
uint64_t get_balance(uint64_t address) {
    return g_balances[get_balance_slot(address)];
}

// Write balance mutation
void set_balance(uint64_t address, uint64_t balance) {
    g_balances[get_balance_slot(address)] = balance;
}

// Mock ZMM VM dispatcher representing our 2-channel operator
bool process_coaxial_transaction(TwoChannelTx *tx, const char **out_error) {
    uint64_t caller = tx->b_channel_0;
    uint64_t caller_bal = get_balance(caller);
    
    // Constant gas fee applied universally to BOTH read and write queries
    const uint64_t gas_fee = 15;
    
    // 1. Gas checking and burning
    if (caller_bal < gas_fee) {
        *out_error = "REVERT: INSUFFICIENT_GAS";
        return false;
    }
    
    // Deduct gas immediately (No distinction for view/write queries)
    set_balance(caller, caller_bal - gas_fee);
    g_gas_burned += gas_fee;
    
    // Refresh updated balance
    caller_bal = get_balance(caller);

    // 2. Opcode evaluation
    switch (tx->d_channel) {
        case YUL_OP_GAS_TRANSFER: {
            uint64_t recipient = tx->b_channel_1;
            uint64_t amount = tx->value;
            if (caller_bal < amount) {
                *out_error = "REVERT: INSUFFICIENT_BALANCE_FOR_TRANSFER";
                return false;
            }
            set_balance(caller, caller_bal - amount);
            set_balance(recipient, get_balance(recipient) + amount);
            printf("   [TX] Transfer Successful: %lu tokens from 0x%lx to 0x%lx\n", amount, caller, recipient);
            break;
        }
        case YUL_OP_QUERY_READ: {
            uint64_t target_key = tx->b_channel_1;
            uint64_t value = get_balance(target_key);
            printf("   [READ QUERY] Read Balance of 0x%lx -> Value: %lu (Charged %lu Gas)\n", target_key, value, gas_fee);
            break;
        }
        case YUL_OP_QUERY_WRITE: {
            uint64_t target_key = tx->b_channel_1;
            uint64_t new_val = tx->value;
            set_balance(target_key, new_val);
            printf("   [WRITE QUERY] Updated Balance of 0x%lx to %lu (Charged %lu Gas)\n", target_key, new_val, gas_fee);
            break;
        }
        default:
            *out_error = "REVERT: INVALID_OPCODE";
            return false;
    }
    return true;
}

// Mock network dispatcher sending transactions to MCP server port
bool send_network_tx_to_mcp(TwoChannelTx *tx, const char **out_error) {
    if (!g_mcp_server.is_active) {
        *out_error = "NETWORK_ERROR: MCP_SERVER_UNREACHABLE";
        return false;
    }
    
    printf("[NETWORK] Dispatching tx to MCP Server (%s:%u)...\n", g_mcp_server.uri, g_mcp_server.port);
    // Route tx to internal coaxial system
    return process_coaxial_transaction(tx, out_error);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: MCP NETWORK COAXIAL GAS UNIT TESTS\n");
    printf("=============================================================\n");

    // Initialize mock database and registers
    memset(g_balances, 0, sizeof(g_balances));
    g_gas_burned = 0;
    
    uint64_t user_alice = 0x01A0B2C3;
    uint64_t user_bob = 0x02D0E4F5;
    
    // Seed initial Alice balance (100 tokens)
    set_balance(user_alice, 100);
    set_balance(user_bob, 50);

    // 1. Initialize MCP Peer Server Connection
    printf("1. Registering and activating MCP peer connection...\n");
    strncpy(g_mcp_server.uri, "peer_mcp://127.0.0.1", sizeof(g_mcp_server.uri) - 1);
    g_mcp_server.port = 3000;
    g_mcp_server.is_active = true;
    printf("   ✓ Registered connection: %s:%u\n\n", g_mcp_server.uri, g_mcp_server.port);

    // 2. Test Case 1: Write Query (OP_QUERY_WRITE) - Charges 15 Gas
    printf("2. Executing Write Query (OP_QUERY_WRITE)...\n");
    TwoChannelTx tx_write = {
        .b_channel_0 = user_alice,
        .b_channel_1 = user_bob,
        .value = 80,
        .d_channel = YUL_OP_QUERY_WRITE
    };
    
    const char *err = NULL;
    bool res = send_network_tx_to_mcp(&tx_write, &err);
    assert(res == true);
    assert(get_balance(user_bob) == 80);
    assert(get_balance(user_alice) == 85); // 100 - 15 gas fee
    printf("   ✓ Write query succeeded. Alice Balance: %lu (Charged 15 Gas)\n\n", get_balance(user_alice));

    // 3. Test Case 2: Read Query (OP_QUERY_READ) - Also charges 15 Gas (No view-only exemption)
    printf("3. Executing Read Query (OP_QUERY_READ)...\n");
    TwoChannelTx tx_read = {
        .b_channel_0 = user_alice,
        .b_channel_1 = user_bob,
        .value = 0,
        .d_channel = YUL_OP_QUERY_READ
    };
    
    res = send_network_tx_to_mcp(&tx_read, &err);
    assert(res == true);
    assert(get_balance(user_alice) == 70); // 85 - 15 gas fee
    printf("   ✓ Read query succeeded. Alice Balance: %lu (Charged 15 Gas)\n\n", get_balance(user_alice));

    // 4. Test Case 3: Out of Gas Rejection
    printf("4. Executing Out of Gas Rejection test...\n");
    // Reduce Alice's balance to 10 tokens (less than the 15 gas fee)
    set_balance(user_alice, 10);
    
    TwoChannelTx tx_failed = {
        .b_channel_0 = user_alice,
        .b_channel_1 = user_bob,
        .value = 0,
        .d_channel = YUL_OP_QUERY_READ
    };
    
    res = send_network_tx_to_mcp(&tx_failed, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: INSUFFICIENT_GAS") == 0);
    printf("   ✓ Reversion successful: %s\n\n", err);

    printf("=============================================================\n");
    printf("AUNCIENT ZMM MCP COAXIAL GAS TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
