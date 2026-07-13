#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// Yul Opcode definition
typedef enum {
    YUL_OP_GAS_TRANSFER = 0x20,
    YUL_OP_DEPLOY_CONTRACT = 0x55,
    YUL_OP_QUERY_READ = 0x30
} YulOpcode;

// ACL Level definitions
typedef enum {
    ACL_ANONYMOUS = 0,
    ACL_USER = 1,
    ACL_ADMIN = 2
} AclLevel;

// Represents our 2-Channel data payload on the coaxial bus
typedef struct {
    uint64_t b_channel_0; // Sender / Caller address
    uint64_t b_channel_1; // Recipient address or Target key
    uint64_t value;       // Transfer amount or auxiliary data
    char     contract_name[32];
    YulOpcode d_channel;  // Control / Opcode
} TwoChannelTx;

// Global state variables simulating database
uint64_t g_balances[100]; 
bool g_seen_clients[100];
AclLevel g_acl_registry[100]; // Maps hashed address index -> AclLevel

const uint64_t UNIVERSAL_GAS_FEE = 15;
const uint64_t DEFAULT_BOOTSTRAP_ALLOWANCE = 50;
const uint64_t DEPLOYMENT_GAS_COST = 100;

uint32_t get_balance_slot(uint64_t address) {
    return address % 100;
}

uint64_t get_balance(uint64_t address) {
    return g_balances[get_balance_slot(address)];
}

void set_balance(uint64_t address, uint64_t balance) {
    g_balances[get_balance_slot(address)] = balance;
}

AclLevel get_acl_level(uint64_t address) {
    uint32_t slot = get_balance_slot(address);
    if (!g_seen_clients[slot]) {
        // Default support for anonymous clients
        return ACL_ANONYMOUS;
    }
    return g_acl_registry[slot];
}

void set_acl_level(uint64_t address, AclLevel level) {
    uint32_t slot = get_balance_slot(address);
    g_acl_registry[slot] = level;
}

// Pre-Flight Gas Auditing with Bootstrap
bool preflight_gas_check(uint64_t caller) {
    uint32_t slot = get_balance_slot(caller);
    if (!g_seen_clients[slot]) {
        g_seen_clients[slot] = true;
        set_balance(caller, DEFAULT_BOOTSTRAP_ALLOWANCE);
        set_acl_level(caller, ACL_ANONYMOUS); // Default anonymous role assigned
    }
    return true;
}

// Check ACL permission against target opcode
bool check_acl_permission(AclLevel level, YulOpcode op) {
    switch (op) {
        case YUL_OP_QUERY_READ:
            // All levels (including Anonymous) can read
            return true;
        case YUL_OP_GAS_TRANSFER:
            // Only User and Admin can transfer tokens
            return (level >= ACL_USER);
        case YUL_OP_DEPLOY_CONTRACT:
            // Only Admin can deploy contracts
            return (level >= ACL_ADMIN);
        default:
            return false;
    }
}

// Mock ZMM VM Coaxial Executor with ACL checks
bool process_coaxial_transaction(TwoChannelTx *tx, const char **out_err) {
    uint64_t caller = tx->b_channel_0;
    preflight_gas_check(caller);
    
    AclLevel caller_acl = get_acl_level(caller);
    uint64_t balance = get_balance(caller);
    
    // 1. Enforce ACL Authorization Gate
    if (!check_acl_permission(caller_acl, tx->d_channel)) {
        *out_err = "REVERT: ACL_PERMISSION_DENIED";
        return false;
    }
    
    // 2. Process Opcode under specific gas rules
    switch (tx->d_channel) {
        case YUL_OP_QUERY_READ: {
            if (balance < UNIVERSAL_GAS_FEE) {
                *out_err = "REVERT: INSUFFICIENT_GAS_BALANCE";
                return false;
            }
            set_balance(caller, balance - UNIVERSAL_GAS_FEE);
            printf("   [READ QUERY] Checked slot 0x%lx (Charged: %lu Gas)\n", tx->b_channel_1, UNIVERSAL_GAS_FEE);
            break;
        }
        case YUL_OP_GAS_TRANSFER: {
            if (balance < UNIVERSAL_GAS_FEE) {
                *out_err = "REVERT: INSUFFICIENT_GAS_BALANCE";
                return false;
            }
            uint64_t recipient = tx->b_channel_1;
            uint64_t amount = tx->value;
            uint64_t usable_bal = balance - UNIVERSAL_GAS_FEE;
            
            if (usable_bal < amount) {
                *out_err = "REVERT: INSUFFICIENT_BALANCE_FOR_TRANSFER";
                return false;
            }
            set_balance(caller, usable_bal - amount);
            set_balance(recipient, get_balance(recipient) + amount);
            printf("   [TRANSFER] Sent %lu tokens to 0x%lx (Charged: %lu Gas)\n", amount, recipient, UNIVERSAL_GAS_FEE);
            break;
        }
        case YUL_OP_DEPLOY_CONTRACT: {
            if (balance < DEPLOYMENT_GAS_COST) {
                *out_err = "REVERT: INSUFFICIENT_GAS_FOR_DEPLOYMENT";
                return false;
            }
            set_balance(caller, balance - DEPLOYMENT_GAS_COST);
            printf("   [DEPLOY] Loaded contract '%s' (Charged: %lu Gas)\n", tx->contract_name, DEPLOYMENT_GAS_COST);
            break;
        }
        default:
            *out_err = "REVERT: INVALID_OPCODE";
            return false;
    }
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: MULTIPLE ACL AND GAS VERIFICATION\n");
    printf("=============================================================\n");

    // Initialize databases
    memset(g_balances, 0, sizeof(g_balances));
    memset(g_seen_clients, 0, sizeof(g_seen_clients));
    memset(g_acl_registry, 0, sizeof(g_acl_registry));

    uint64_t peer_anon = 0x01A0B2C3;
    uint64_t peer_user = 0x02D0E4F5;
    uint64_t peer_admin = 0x03FF;
    const char *err = NULL;

    // Register User and Admin peers in registry (seeding balances)
    g_seen_clients[get_balance_slot(peer_user)] = true;
    set_balance(peer_user, 100);
    set_acl_level(peer_user, ACL_USER);

    g_seen_clients[get_balance_slot(peer_admin)] = true;
    set_balance(peer_admin, 200);
    set_acl_level(peer_admin, ACL_ADMIN);

    // 1. Test Case 1: Anonymous Peer Read Query (Allowed)
    printf("1. Anonymous peer 0x%lx attempting Read Query...\n", peer_anon);
    TwoChannelTx tx_read = {
        .b_channel_0 = peer_anon,
        .b_channel_1 = peer_anon,
        .d_channel = YUL_OP_QUERY_READ
    };
    bool res = process_coaxial_transaction(&tx_read, &err);
    assert(res == true);
    assert(get_balance(peer_anon) == 35); // 50 (bootstrap) - 15 fee
    printf("   ✓ Anonymous read allowed. Balance: %lu Gas\n\n", get_balance(peer_anon));

    // 2. Test Case 2: Anonymous Peer Transfer (Denied by ACL)
    printf("2. Anonymous peer 0x%lx attempting Gas Transfer...\n", peer_anon);
    TwoChannelTx tx_transfer = {
        .b_channel_0 = peer_anon,
        .b_channel_1 = peer_user,
        .value = 10,
        .d_channel = YUL_OP_GAS_TRANSFER
    };
    res = process_coaxial_transaction(&tx_transfer, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: ACL_PERMISSION_DENIED") == 0);
    printf("   ✓ Anonymous transfer rejected correctly: %s\n\n", err);

    // 3. Test Case 3: User Peer Transfer (Allowed)
    printf("3. User peer 0x%lx attempting Gas Transfer...\n", peer_user);
    TwoChannelTx tx_user_transfer = {
        .b_channel_0 = peer_user,
        .b_channel_1 = peer_anon,
        .value = 20,
        .d_channel = YUL_OP_GAS_TRANSFER
    };
    res = process_coaxial_transaction(&tx_user_transfer, &err);
    assert(res == true);
    assert(get_balance(peer_user) == 65);  // 100 - 15 fee - 20 amount
    assert(get_balance(peer_anon) == 55);  // 35 + 20 transfer
    printf("   ✓ User transfer allowed. User Bal: %lu, Recipient Bal: %lu\n\n",
           get_balance(peer_user), get_balance(peer_anon));

    // 4. Test Case 4: User Peer Contract Deployment (Denied by ACL)
    printf("4. User peer 0x%lx attempting Contract Deployment...\n", peer_user);
    TwoChannelTx tx_user_deploy = {
        .b_channel_0 = peer_user,
        .b_channel_1 = 0,
        .value = 512,
        .d_channel = YUL_OP_DEPLOY_CONTRACT
    };
    strncpy(tx_user_deploy.contract_name, "TESTERC20", 31);
    res = process_coaxial_transaction(&tx_user_deploy, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: ACL_PERMISSION_DENIED") == 0);
    printf("   ✓ User deployment rejected correctly: %s\n\n", err);

    // 5. Test Case 5: Admin Peer Contract Deployment (Allowed)
    printf("5. Admin peer 0x%lx attempting Contract Deployment...\n", peer_admin);
    TwoChannelTx tx_admin_deploy = {
        .b_channel_0 = peer_admin,
        .b_channel_1 = 0,
        .value = 512,
        .d_channel = YUL_OP_DEPLOY_CONTRACT
    };
    strncpy(tx_admin_deploy.contract_name, "TESTERC20", 31);
    res = process_coaxial_transaction(&tx_admin_deploy, &err);
    assert(res == true);
    assert(get_balance(peer_admin) == 100); // 200 - 100 deploy cost
    printf("   ✓ Admin deployment allowed. Admin Bal: %lu Gas\n\n", get_balance(peer_admin));

    printf("=============================================================\n");
    printf("AUNCIENT MULTIPLE ACL TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
