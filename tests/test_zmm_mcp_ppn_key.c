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

// PPN Definition: [Project, Programmer]
typedef struct {
    uint16_t project;
    uint16_t programmer;
} PPN;

// Represents a registered key entry mapped to a PPN
#define MAX_KEYS 16
typedef struct {
    uint32_t key_id;
    PPN      ppn;
    AclLevel acl;
    bool     is_active;
} KeyRegistryEntry;

KeyRegistryEntry g_key_registry[MAX_KEYS];
size_t g_key_count = 0;

// Global state variables simulating database
uint64_t g_balances[100]; 
const uint64_t UNIVERSAL_GAS_FEE = 15;
const uint64_t DEPLOYMENT_GAS_COST = 100;

// 2-Channel transaction payload including PPN and Key
typedef struct {
    PPN      ppn;         // Project/Programmer ID
    uint32_t key_id;      // PKI Key ID
    uint64_t recipient;   // Target account/PPN (hashed)
    uint64_t value;       // Transfer amount
    YulOpcode d_channel;  // Control / Opcode
} TwoChannelTx;

uint32_t get_ppn_slot(PPN ppn) {
    return (ppn.project + ppn.programmer) % 100;
}

uint64_t get_ppn_balance(PPN ppn) {
    return g_balances[get_ppn_slot(ppn)];
}

void set_ppn_balance(PPN ppn, uint64_t balance) {
    g_balances[get_ppn_slot(ppn)] = balance;
}

// Registry helpers
void registry_init(void) {
    g_key_count = 0;
    memset(g_key_registry, 0, sizeof(g_key_registry));
}

bool register_key(uint32_t key_id, uint16_t project, uint16_t programmer, AclLevel acl) {
    if (g_key_count >= MAX_KEYS) return false;
    
    // Add entry
    KeyRegistryEntry *entry = &g_key_registry[g_key_count++];
    entry->key_id = key_id;
    entry->ppn.project = project;
    entry->ppn.programmer = programmer;
    entry->acl = acl;
    entry->is_active = true;
    
    printf("   [Registry] Key ID %u registered to PPN [%u, %u] with ACL: %d\n",
           key_id, project, programmer, acl);
    return true;
}

// Verifies if a key is authorized to act on behalf of a PPN
bool verify_ppn_key_acl(PPN ppn, uint32_t key_id, YulOpcode op, AclLevel *out_acl, const char **out_err) {
    for (size_t i = 0; i < g_key_count; i++) {
        KeyRegistryEntry *entry = &g_key_registry[i];
        if (entry->is_active && entry->key_id == key_id) {
            // Confirm the key is registered for the specified PPN
            if (entry->ppn.project != ppn.project || entry->ppn.programmer != ppn.programmer) {
                *out_err = "REVERT: KEY_NOT_AUTHORIZED_FOR_THIS_PPN";
                return false;
            }
            
            // Check operations permission
            *out_acl = entry->acl;
            switch (op) {
                case YUL_OP_QUERY_READ:
                    return true;
                case YUL_OP_GAS_TRANSFER:
                    return (entry->acl >= ACL_USER);
                case YUL_OP_DEPLOY_CONTRACT:
                    return (entry->acl >= ACL_ADMIN);
                default:
                    *out_err = "REVERT: INVALID_OPCODE";
                    return false;
            }
        }
    }
    // Default support for anonymous keys (if key not in registry)
    *out_acl = ACL_ANONYMOUS;
    if (op == YUL_OP_QUERY_READ) {
        return true;
    }
    *out_err = "REVERT: KEY_NOT_REGISTERED_ACL_DENIED";
    return false;
}

// Coaxial transaction executor
bool process_coaxial_transaction(TwoChannelTx *tx, const char **out_err) {
    AclLevel acl;
    if (!verify_ppn_key_acl(tx->ppn, tx->key_id, tx->d_channel, &acl, out_err)) {
        return false;
    }
    
    uint64_t balance = get_ppn_balance(tx->ppn);
    
    switch (tx->d_channel) {
        case YUL_OP_QUERY_READ: {
            if (balance < UNIVERSAL_GAS_FEE) {
                *out_err = "REVERT: INSUFFICIENT_GAS_BALANCE";
                return false;
            }
            set_ppn_balance(tx->ppn, balance - UNIVERSAL_GAS_FEE);
            printf("   [READ] PPN [%u,%u] read registry. Gas charged: %lu\n",
                   tx->ppn.project, tx->ppn.programmer, UNIVERSAL_GAS_FEE);
            break;
        }
        case YUL_OP_GAS_TRANSFER: {
            if (balance < UNIVERSAL_GAS_FEE) {
                *out_err = "REVERT: INSUFFICIENT_GAS_BALANCE";
                return false;
            }
            uint64_t fee_deducted = balance - UNIVERSAL_GAS_FEE;
            if (fee_deducted < tx->value) {
                *out_err = "REVERT: INSUFFICIENT_BALANCE_FOR_TRANSFER";
                return false;
            }
            set_ppn_balance(tx->ppn, fee_deducted - tx->value);
            printf("   [TRANSFER] PPN [%u,%u] transferred %lu tokens. Gas charged: %lu\n",
                   tx->ppn.project, tx->ppn.programmer, tx->value, UNIVERSAL_GAS_FEE);
            break;
        }
        case YUL_OP_DEPLOY_CONTRACT: {
            if (balance < DEPLOYMENT_GAS_COST) {
                *out_err = "REVERT: INSUFFICIENT_GAS_FOR_DEPLOYMENT";
                return false;
            }
            set_ppn_balance(tx->ppn, balance - DEPLOYMENT_GAS_COST);
            printf("   [DEPLOY] PPN [%u,%u] deployed contract. Gas charged: %lu\n",
                   tx->ppn.project, tx->ppn.programmer, DEPLOYMENT_GAS_COST);
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
    printf("AUNCIENT ZMM VM: PPN AND KEY-BASED ACL VERIFICATION\n");
    printf("=============================================================\n");

    registry_init();
    memset(g_balances, 0, sizeof(g_balances));

    // Register our PPN and Keys
    PPN admin_ppn = { .project = 1, .programmer = 1 };
    PPN user_ppn = { .project = 10, .programmer = 5 };

    // Set balances
    set_ppn_balance(admin_ppn, 200);
    set_ppn_balance(user_ppn, 100);

    // Register multiple keys for admin_ppn:
    // Key 11 (default 11 key) and Key 12 are both authorized for PPN [1,1]
    printf("1. Registering keys to PPNs...\n");
    assert(register_key(11, 1, 1, ACL_ADMIN) == true); // Default 11 key (Admin)
    assert(register_key(12, 1, 1, ACL_ADMIN) == true); // Backup key (Admin)
    assert(register_key(22, 10, 5, ACL_USER) == true);  // User key for PPN [10,5]
    printf("   ✓ Keys registered.\n\n");

    const char *err = NULL;

    // Test Case 1: Deploy contract using Default 11 Key (Authorized Admin)
    printf("2. Deploying contract with Default 11 Key...\n");
    TwoChannelTx tx_deploy1 = {
        .ppn = admin_ppn,
        .key_id = 11,
        .d_channel = YUL_OP_DEPLOY_CONTRACT
    };
    bool res = process_coaxial_transaction(&tx_deploy1, &err);
    assert(res == true);
    assert(get_ppn_balance(admin_ppn) == 100); // 200 - 100 cost
    printf("   ✓ Deployment succeeded using Key 11. Remaining PPN balance: %lu Gas\n\n",
           get_ppn_balance(admin_ppn));

    // Test Case 2: Deploy contract using Backup Key 12 (Authorized Admin for same PPN)
    printf("3. Deploying contract with Backup Key 12...\n");
    TwoChannelTx tx_deploy2 = {
        .ppn = admin_ppn,
        .key_id = 12,
        .d_channel = YUL_OP_DEPLOY_CONTRACT
    };
    res = process_coaxial_transaction(&tx_deploy2, &err);
    assert(res == true);
    assert(get_ppn_balance(admin_ppn) == 0); // 100 - 100 cost
    printf("   ✓ Deployment succeeded using Key 12. Remaining PPN balance: %lu Gas\n\n",
           get_ppn_balance(admin_ppn));

    // Test Case 3: Unauthorized Key access to PPN [1,1]
    printf("4. Attempting deploy with Key 22 on PPN [1,1] (Key registered to [10,5])...\n");
    TwoChannelTx tx_unauth = {
        .ppn = admin_ppn,
        .key_id = 22,
        .d_channel = YUL_OP_DEPLOY_CONTRACT
    };
    res = process_coaxial_transaction(&tx_unauth, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: KEY_NOT_AUTHORIZED_FOR_THIS_PPN") == 0);
    printf("   ✓ Transaction rejected successfully: %s\n\n", err);

    // Test Case 4: Anonymous unregistered key (Allow read only, reject transfer)
    printf("5. Testing unregistered Key 99 for PPN [10,5]...\n");
    TwoChannelTx tx_anon_read = {
        .ppn = user_ppn,
        .key_id = 99,
        .d_channel = YUL_OP_QUERY_READ
    };
    res = process_coaxial_transaction(&tx_anon_read, &err);
    assert(res == true);
    assert(get_ppn_balance(user_ppn) == 85); // 100 - 15 fee
    printf("   ✓ Read query allowed for anonymous Key 99.\n");

    TwoChannelTx tx_anon_transfer = {
        .ppn = user_ppn,
        .key_id = 99,
        .d_channel = YUL_OP_GAS_TRANSFER,
        .value = 10
    };
    res = process_coaxial_transaction(&tx_anon_transfer, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: KEY_NOT_REGISTERED_ACL_DENIED") == 0);
    printf("   ✓ Transfer query rejected for anonymous Key 99: %s\n\n", err);

    printf("=============================================================\n");
    printf("AUNCIENT PPN KEY-BASED ACL TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
