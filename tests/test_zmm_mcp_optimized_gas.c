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

// Bytecode Command Compaction (Tokens)
typedef enum {
    CMD_EPOCH_INIT = 0x81,
    CMD_EPOCH_STATE = 0x82,
    CMD_RESONATE = 0x83
} CompactCommand;

// Yul Opcode definition
typedef enum {
    YUL_OP_COMPACT_EXEC = 0x77
} YulOpcode;

// Represents our 2-Channel data payload on the coaxial bus (Zero-Copy compatible)
typedef struct {
    uint64_t b_channel_0; // Sender / Caller address
    uint64_t b_channel_1; // Recipient address or Query key
    uint64_t value;       // Transfer amount or Write value
    uint8_t  compact_cmd; // Compressed instruction byte (Bytecode Compaction)
    YulOpcode d_channel;  // Control / Opcode
} TwoChannelTx;

// Mock WaveSystem structure representing our state
typedef struct {
    int id;
    int version;
    bool ftw;
    int counter;
    char current_directive[256];
    HelmholtzOpType active_op;
} WaveSystem;

// Global state variables simulating database
uint64_t g_balances[100]; 
const uint64_t UNIVERSAL_GAS_FEE = 15;

uint32_t get_balance_slot(uint64_t address) {
    return address % 100;
}

uint64_t get_balance(uint64_t address) {
    return g_balances[get_balance_slot(address)];
}

void set_balance(uint64_t address, uint64_t balance) {
    g_balances[get_balance_slot(address)] = balance;
}

// 1. Pre-Flight Gas Auditing
bool preflight_gas_check(uint64_t caller, const char **out_err) {
    uint64_t balance = get_balance(caller);
    if (balance < UNIVERSAL_GAS_FEE) {
        *out_err = "PREFLIGHT_REJECT: INSUFFICIENT_GAS_BALANCE";
        return false;
    }
    return true;
}

// Decompresses the compact bytecode command into standard directive string
const char* decompress_command(uint8_t compact_cmd) {
    switch (compact_cmd) {
        case CMD_EPOCH_INIT:  return "EPOCH_INIT";
        case CMD_EPOCH_STATE: return "EPOCH_STATE";
        case CMD_RESONATE:    return "HELMHOLTZ_RESONATE";
        default:              return "UNKNOWN";
    }
}

// 2. Zero-Copy Command Processing
// Note: Accepts a direct pointer to the transaction buffer (no copying)
bool process_coaxial_transaction_zero_copy(WaveSystem *ws, TwoChannelTx *tx, const char **out_err) {
    uint64_t caller = tx->b_channel_0;
    
    // Deduct gas
    uint64_t balance = get_balance(caller);
    set_balance(caller, balance - UNIVERSAL_GAS_FEE);
    
    // 3. Bytecode Compaction Decompression
    const char *directive = decompress_command(tx->compact_cmd);
    printf("   [TAC Executor] Executing decompressed directive: %s\n", directive);
    
    // Route to direct Helmholtz main loop transitions
    strncpy(ws->current_directive, directive, sizeof(ws->current_directive) - 1);
    if (strcmp(directive, "EPOCH_INIT") == 0) {
        ws->version = 2026;
        ws->ftw = false;
    } else if (strcmp(directive, "EPOCH_STATE") == 0) {
        ws->ftw = true;
        ws->counter++;
    } else if (strcmp(directive, "HELMHOLTZ_RESONATE") == 0) {
        ws->active_op = HELMHOLTZ_OP_RESONANCE;
        ws->counter = 762;
    } else {
        *out_err = "REVERT: INVALID_COMPRESSED_DIRECTIVE";
        return false;
    }
    return true;
}

// Dispatcher integrating all 3 optimizations
bool dispatch_mcp_transaction(WaveSystem *ws, TwoChannelTx *tx, const char **out_err) {
    // Optimization 1: Pre-Flight Gas Auditing
    if (!preflight_gas_check(tx->b_channel_0, out_err)) {
        printf("   [Pre-flight] Transaction rejected early: %s\n", *out_err);
        return false;
    }
    printf("   [Pre-flight] Gas check passed.\n");
    
    // Optimization 2: Zero-Copy processing (Passes direct pointer 'tx')
    return process_coaxial_transaction_zero_copy(ws, tx, out_err);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: OPTIMIZED MCP COAXIAL GAS UNIT TESTS\n");
    printf("=============================================================\n");

    // Initialize state
    memset(g_balances, 0, sizeof(g_balances));
    uint64_t user_alice = 0x01A0B2C3;
    set_balance(user_alice, 100);

    WaveSystem ws = {
        .id = 762,
        .version = 0,
        .ftw = false,
        .counter = 0,
        .active_op = HELMHOLTZ_OP_IDENTITY
    };
    memset(ws.current_directive, 0, sizeof(ws.current_directive));

    // Test Case 1: Successful optimized transaction
    printf("1. Running successful optimized transition (Alice Balance: 100)...\n");
    TwoChannelTx tx_success = {
        .b_channel_0 = user_alice,
        .b_channel_1 = 0,
        .value = 0,
        .compact_cmd = CMD_EPOCH_INIT, // Compressed bytecode token
        .d_channel = YUL_OP_COMPACT_EXEC
    };
    
    const char *err = NULL;
    bool res = dispatch_mcp_transaction(&ws, &tx_success, &err);
    assert(res == true);
    assert(ws.version == 2026);
    assert(get_balance(user_alice) == 85); // 100 - 15 gas fee
    printf("   ✓ Test Case 1 Passed. Decompressed Directive: %s | Alice Balance: %lu\n\n",
           ws.current_directive, get_balance(user_alice));

    // Test Case 2: Pre-flight rejection (Insufficient gas)
    printf("2. Running pre-flight gas rejection (Alice Balance: 10, Fee: 15)...\n");
    set_balance(user_alice, 10);
    
    TwoChannelTx tx_fail = {
        .b_channel_0 = user_alice,
        .b_channel_1 = 0,
        .value = 0,
        .compact_cmd = CMD_EPOCH_STATE,
        .d_channel = YUL_OP_COMPACT_EXEC
    };
    
    res = dispatch_mcp_transaction(&ws, &tx_fail, &err);
    assert(res == false);
    assert(strcmp(err, "PREFLIGHT_REJECT: INSUFFICIENT_GAS_BALANCE") == 0);
    assert(get_balance(user_alice) == 10); // Balance should NOT change since it was rejected pre-flight
    printf("   ✓ Test Case 2 Passed. Pre-flight rejected successfully. Balance unchanged: %lu\n\n",
           get_balance(user_alice));

    printf("=============================================================\n");
    printf("AUNCIENT OPTIMIZED MCP COAXIAL GAS TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
