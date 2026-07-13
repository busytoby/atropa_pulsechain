#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// Yul Opcode definition
typedef enum {
    YUL_OP_DEPLOY_CONTRACT = 0x55,
    YUL_OP_QUERY_READ = 0x30
} YulOpcode;

// Represents our 2-Channel data payload on the coaxial bus
typedef struct {
    uint64_t b_channel_0; // Sender / Caller address
    uint64_t b_channel_1; // Target contract address / ID
    uint64_t value;       // Auxiliary deployment length/data
    char     contract_name[32];
    YulOpcode d_channel;  // Control / Opcode
} TwoChannelTx;

// Unified peer registry structures
#define MAX_PEERS 16
typedef struct {
    char uri[64];
    bool is_thunk;
    bool is_active;
} UnifiedPeer;

UnifiedPeer g_peer_registry[MAX_PEERS];
size_t g_peer_count = 0;

// Global state variables simulating database
uint64_t g_balances[100]; 
bool g_seen_clients[100];
const uint64_t UNIVERSAL_GAS_FEE = 15;
const uint64_t DEFAULT_BOOTSTRAP_ALLOWANCE = 50;
const uint64_t DEPLOYMENT_GAS_COST = 100; // Contract deployment requires 100 gas

uint32_t get_balance_slot(uint64_t address) {
    return address % 100;
}

uint64_t get_balance(uint64_t address) {
    return g_balances[get_balance_slot(address)];
}

void set_balance(uint64_t address, uint64_t balance) {
    g_balances[get_balance_slot(address)] = balance;
}

// Pre-Flight Gas Auditing with Bootstrap
bool preflight_gas_check(uint64_t caller) {
    uint32_t slot = get_balance_slot(caller);
    if (!g_seen_clients[slot]) {
        g_seen_clients[slot] = true;
        set_balance(caller, DEFAULT_BOOTSTRAP_ALLOWANCE);
    }
    return true;
}

// Register a deployed contract as a thunk in the registry
bool register_contract_thunk(const char *name) {
    if (g_peer_count >= MAX_PEERS) return false;
    
    UnifiedPeer *peer = &g_peer_registry[g_peer_count++];
    snprintf(peer->uri, sizeof(peer->uri), "peer_contract://%s", name);
    peer->is_thunk = true;
    peer->is_active = true;
    printf("   [Registry] Deployed contract thunk successfully: %s\n", peer->uri);
    return true;
}

// Mock ZMM VM Coaxial Executor
bool process_coaxial_transaction(TwoChannelTx *tx, const char **out_err) {
    uint64_t caller = tx->b_channel_0;
    preflight_gas_check(caller);
    
    uint64_t balance = get_balance(caller);
    
    if (tx->d_channel == YUL_OP_DEPLOY_CONTRACT) {
        // Deployment Gas Check
        if (balance < DEPLOYMENT_GAS_COST) {
            *out_err = "REVERT: INSUFFICIENT_GAS_FOR_DEPLOYMENT";
            return false;
        }
        
        // Deduct deployment fee
        set_balance(caller, balance - DEPLOYMENT_GAS_COST);
        
        // Load contract as a thunk
        if (!register_contract_thunk(tx->contract_name)) {
            *out_err = "REVERT: REGISTRY_FULL";
            return false;
        }
        
        printf("   [ZMM] Loaded contract '%s' as a dynamic thunk (Charged: %lu Gas)\n",
               tx->contract_name, DEPLOYMENT_GAS_COST);
    } else if (tx->d_channel == YUL_OP_QUERY_READ) {
        if (balance < UNIVERSAL_GAS_FEE) {
            *out_err = "REVERT: INSUFFICIENT_GAS_BALANCE";
            return false;
        }
        set_balance(caller, balance - UNIVERSAL_GAS_FEE);
        printf("   [READ QUERY] Checked register of 0x%lx (Charged: %lu Gas)\n", tx->b_channel_1, UNIVERSAL_GAS_FEE);
    } else {
        *out_err = "REVERT: INVALID_OPCODE";
        return false;
    }
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: CONTRACT DEPLOYMENT GAS VERIFICATION\n");
    printf("=============================================================\n");

    // Initialize databases
    memset(g_balances, 0, sizeof(g_balances));
    memset(g_seen_clients, 0, sizeof(g_seen_clients));
    g_peer_count = 0;

    uint64_t user_alice = 0x99AABBCC;
    const char *err = NULL;

    // 1. Attempt to deploy TESTERC20 from new unknown client (Initial gas: 50)
    printf("1. Deploying 'TESTERC20' from new client 0x%lx (Cost: %lu Gas)...\n",
           user_alice, DEPLOYMENT_GAS_COST);
           
    TwoChannelTx tx_deploy = {
        .b_channel_0 = user_alice,
        .b_channel_1 = 0,
        .value = 512, // Code length mock
        .d_channel = YUL_OP_DEPLOY_CONTRACT
    };
    strncpy(tx_deploy.contract_name, "TESTERC20", sizeof(tx_deploy.contract_name) - 1);

    bool res = process_coaxial_transaction(&tx_deploy, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: INSUFFICIENT_GAS_FOR_DEPLOYMENT") == 0);
    assert(get_balance(user_alice) == 50); // Balance stays at default 50
    printf("   ✓ Deployment rejected successfully. Balance unchanged: %lu Gas\n\n", get_balance(user_alice));

    // 2. Fund Alice with enough gas to meet the deployment threshold
    printf("2. Funding client 0x%lx with additional 100 Gas tokens...\n", user_alice);
    set_balance(user_alice, 150); // Total balance = 150
    printf("   ✓ Client 0x%lx balance set to: %lu Gas\n\n", user_alice, get_balance(user_alice));

    // 3. Re-submit deployment transaction
    printf("3. Re-submitting deployment transaction for 'TESTERC20'...\n");
    res = process_coaxial_transaction(&tx_deploy, &err);
    assert(res == true);
    assert(get_balance(user_alice) == 50); // 150 - 100 deployment cost
    assert(g_peer_count == 1);
    assert(strcmp(g_peer_registry[0].uri, "peer_contract://TESTERC20") == 0);
    printf("   ✓ Deployment succeeded. Alice Remaining Balance: %lu Gas\n\n", get_balance(user_alice));

    printf("=============================================================\n");
    printf("AUNCIENT CONTRACT DEPLOYMENT GAS TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
