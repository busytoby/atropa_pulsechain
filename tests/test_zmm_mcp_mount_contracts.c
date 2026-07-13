#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// Yul Opcode definition
typedef enum {
    YUL_OP_QUERY_READ = 0x30,
    YUL_OP_MOUNT_CONTRACT = 0x61,
    YUL_OP_UNMOUNT_CONTRACT = 0x62
} YulOpcode;

// PPN Definition: [Project, Programmer]
typedef struct {
    uint16_t project;
    uint16_t programmer;
} PPN;

// Session structure tracking mounted contracts per PPN slot
#define MAX_MOUNTS_PER_USER 4
typedef struct {
    char mounted_contract_names[MAX_MOUNTS_PER_USER][32];
    size_t mount_count;
} UserSession;

UserSession g_user_sessions[100]; // Map PPN slot -> UserSession

// Global database simulating balances
uint64_t g_balances[100]; 
const uint64_t UNIVERSAL_GAS_FEE = 15;

// Represents our 2-Channel data payload on the coaxial bus
typedef struct {
    PPN      ppn;
    uint32_t key_id;
    char     contract_name[32];
    YulOpcode d_channel;
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

// Check if a contract is currently mounted for a user session
bool is_contract_mounted(PPN ppn, const char *name) {
    uint32_t slot = get_ppn_slot(ppn);
    UserSession *session = &g_user_sessions[slot];
    for (size_t i = 0; i < session->mount_count; i++) {
        if (strcmp(session->mounted_contract_names[i], name) == 0) {
            return true;
        }
    }
    return false;
}

// Mount a contract to a user session
bool mount_contract(PPN ppn, const char *name, const char **out_err) {
    uint32_t slot = get_ppn_slot(ppn);
    UserSession *session = &g_user_sessions[slot];
    
    if (is_contract_mounted(ppn, name)) {
        *out_err = "REVERT: CONTRACT_ALREADY_MOUNTED";
        return false;
    }
    
    if (session->mount_count >= MAX_MOUNTS_PER_USER) {
        *out_err = "REVERT: MOUNT_LIMIT_EXCEEDED";
        return false;
    }
    
    strncpy(session->mounted_contract_names[session->mount_count++], name, 31);
    printf("   [Session] Mounted contract: %s for PPN [%u,%u]\n", name, ppn.project, ppn.programmer);
    return true;
}

// Unmount a contract from a user session
bool unmount_contract(PPN ppn, const char *name, const char **out_err) {
    uint32_t slot = get_ppn_slot(ppn);
    UserSession *session = &g_user_sessions[slot];
    
    for (size_t i = 0; i < session->mount_count; i++) {
        if (strcmp(session->mounted_contract_names[i], name) == 0) {
            // Remove by swapping with the last entry
            if (i < session->mount_count - 1) {
                strcpy(session->mounted_contract_names[i], session->mounted_contract_names[session->mount_count - 1]);
            }
            session->mount_count--;
            printf("   [Session] Unmounted contract: %s from PPN [%u,%u]\n", name, ppn.project, ppn.programmer);
            return true;
        }
    }
    
    *out_err = "REVERT: CONTRACT_NOT_FOUND_IN_MOUNTS";
    return false;
}

// Coaxial executor enforcing mount validation
bool process_coaxial_transaction(TwoChannelTx *tx, const char **out_err) {
    uint64_t balance = get_ppn_balance(tx->ppn);
    
    if (balance < UNIVERSAL_GAS_FEE) {
        *out_err = "REVERT: INSUFFICIENT_GAS_BALANCE";
        return false;
    }
    
    set_ppn_balance(tx->ppn, balance - UNIVERSAL_GAS_FEE);

    switch (tx->d_channel) {
        case YUL_OP_MOUNT_CONTRACT: {
            return mount_contract(tx->ppn, tx->contract_name, out_err);
        }
        case YUL_OP_UNMOUNT_CONTRACT: {
            return unmount_contract(tx->ppn, tx->contract_name, out_err);
        }
        case YUL_OP_QUERY_READ: {
            // Enforce that the target contract must be explicitly mounted to execute queries
            if (!is_contract_mounted(tx->ppn, tx->contract_name)) {
                *out_err = "REVERT: CONTRACT_NOT_MOUNTED_IN_SESSION";
                return false;
            }
            printf("   [READ QUERY] Successfully read dynamic balance of mounted contract '%s'\n", tx->contract_name);
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
    printf("AUNCIENT ZMM VM: DYNAMIC CONTRACT MOUNTING & VALIDATION\n");
    printf("=============================================================\n");

    // Initialize databases
    memset(g_balances, 0, sizeof(g_balances));
    memset(g_user_sessions, 0, sizeof(g_user_sessions));

    PPN user_alice = { .project = 10, .programmer = 5 };
    set_ppn_balance(user_alice, 100);

    const char *err = NULL;

    // 1. Test Case 1: Attempt to query TESTERC20 before mounting (Rejected)
    printf("1. Alice PPN [10,5] attempting read query on unmounted 'TESTERC20'...\n");
    TwoChannelTx tx_query_unmounted = {
        .ppn = user_alice,
        .key_id = 22,
        .d_channel = YUL_OP_QUERY_READ
    };
    strcpy(tx_query_unmounted.contract_name, "TESTERC20");
    
    bool res = process_coaxial_transaction(&tx_query_unmounted, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: CONTRACT_NOT_MOUNTED_IN_SESSION") == 0);
    assert(get_ppn_balance(user_alice) == 85); // Deducted 15 gas fee even on fail
    printf("   ✓ Read query rejected correctly (Contract not mounted). Balance: %lu Gas\n\n",
           get_ppn_balance(user_alice));

    // 2. Test Case 2: Mount TESTERC20 (Allowed)
    printf("2. Alice PPN [10,5] mounting contract 'TESTERC20'...\n");
    TwoChannelTx tx_mount = {
        .ppn = user_alice,
        .key_id = 22,
        .d_channel = YUL_OP_MOUNT_CONTRACT
    };
    strcpy(tx_mount.contract_name, "TESTERC20");

    res = process_coaxial_transaction(&tx_mount, &err);
    assert(res == true);
    assert(get_ppn_balance(user_alice) == 70); // 85 - 15 fee
    assert(is_contract_mounted(user_alice, "TESTERC20") == true);
    printf("   ✓ Mount succeeded. Balance: %lu Gas\n\n", get_ppn_balance(user_alice));

    // 3. Test Case 3: Query TESTERC20 after mounting (Allowed)
    printf("3. Alice PPN [10,5] executing read query on mounted 'TESTERC20'...\n");
    TwoChannelTx tx_query_mounted = {
        .ppn = user_alice,
        .key_id = 22,
        .d_channel = YUL_OP_QUERY_READ
    };
    strcpy(tx_query_mounted.contract_name, "TESTERC20");

    res = process_coaxial_transaction(&tx_query_mounted, &err);
    assert(res == true);
    assert(get_ppn_balance(user_alice) == 55); // 70 - 15 fee
    printf("   ✓ Query succeeded on mounted contract. Balance: %lu Gas\n\n", get_ppn_balance(user_alice));

    // 4. Test Case 4: Unmount TESTERC20 (Allowed)
    printf("4. Alice PPN [10,5] unmounting contract 'TESTERC20'...\n");
    TwoChannelTx tx_unmount = {
        .ppn = user_alice,
        .key_id = 22,
        .d_channel = YUL_OP_UNMOUNT_CONTRACT
    };
    strcpy(tx_unmount.contract_name, "TESTERC20");

    res = process_coaxial_transaction(&tx_unmount, &err);
    assert(res == true);
    assert(get_ppn_balance(user_alice) == 40); // 55 - 15 fee
    assert(is_contract_mounted(user_alice, "TESTERC20") == false);
    printf("   ✓ Unmount succeeded. Balance: %lu Gas\n\n", get_ppn_balance(user_alice));

    // 5. Test Case 5: Query TESTERC20 again after unmounting (Rejected again)
    printf("5. Alice PPN [10,5] querying 'TESTERC20' after unmounting...\n");
    res = process_coaxial_transaction(&tx_query_mounted, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: CONTRACT_NOT_MOUNTED_IN_SESSION") == 0);
    assert(get_ppn_balance(user_alice) == 25); // 40 - 15 fee
    printf("   ✓ Read query rejected correctly (Contract not mounted). Balance: %lu Gas\n\n",
           get_ppn_balance(user_alice));

    printf("=============================================================\n");
    printf("AUNCIENT DYNAMIC CONTRACT MOUNTING TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
