#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// Yul Opcode definition
typedef enum {
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

// Mock virtual card (process) slot in the VM scheduler
#define MAX_CARDS 8
typedef struct {
    uint32_t card_id;
    bool     is_active;
    uint32_t epoch;
    uint64_t gas_balance;
    char     current_directive[64];
} VirtualCard;

// Represents our VM State
typedef struct {
    VirtualCard cards[MAX_CARDS];
    char        system_log[4096];
} ZmmVmState;

ZmmVmState g_vm;

// Dynamic LUT entry mapping Key -> PPN/ACL
typedef struct {
    uint32_t key_id;
    PPN      ppn;
    AclLevel acl;
} KeyEntry;

#define MAX_KEYS 4
KeyEntry g_keys[MAX_KEYS];

// Initialize system
void init_vm(void) {
    memset(&g_vm, 0, sizeof(g_vm));
    memset(g_keys, 0, sizeof(g_keys));
    
    // Seed some card processes
    for (uint32_t i = 0; i < MAX_CARDS; i++) {
        g_vm.cards[i].card_id = i;
        g_vm.cards[i].is_active = (i < 3); // 3 active cards initially
        g_vm.cards[i].epoch = i + 1;
        g_vm.cards[i].gas_balance = 50 + (i * 20);
        strcpy(g_vm.cards[i].current_directive, "WAIT");
    }
    
    // Key 11 is admin PPN [1,1]
    g_keys[0] = (KeyEntry){ .key_id = 11, .ppn = {1, 1}, .acl = ACL_ADMIN };
    // Key 22 is user PPN [10,5]
    g_keys[1] = (KeyEntry){ .key_id = 22, .ppn = {10, 5}, .acl = ACL_USER };
}

// Check if a caller key has admin credentials
bool is_admin(uint32_t key_id) {
    for (int i = 0; i < MAX_KEYS; i++) {
        if (g_keys[i].key_id == key_id && g_keys[i].acl == ACL_ADMIN) {
            return true;
        }
    }
    return false;
}

// Executes Helmholtz-compatible KBM commands in-memory
bool execute_kbm_directive(uint32_t caller_key, const char *kbm_cmd, const char **out_err) {
    char cmd_copy[128];
    strncpy(cmd_copy, kbm_cmd, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';

    char *token = strtok(cmd_copy, " ");
    if (!token) return false;

    // 1. SYSTAT Command: Helmholtz-compatible status logger
    if (strcmp(token, "SYSTAT") == 0) {
        printf("   [Helmholtz KBM] Executing SYSTAT...\n");
        char temp[512];
        strcpy(g_vm.system_log, "=== RSTS/E SYSTAT ===\n");
        for (int i = 0; i < MAX_CARDS; i++) {
            if (g_vm.cards[i].is_active) {
                snprintf(temp, sizeof(temp), "Job %d | Epoch: %d | Bal: %lu Gas | Dir: %s\n",
                         g_vm.cards[i].card_id, g_vm.cards[i].epoch,
                         g_vm.cards[i].gas_balance, g_vm.cards[i].current_directive);
                strcat(g_vm.system_log, temp);
            }
        }
        return true;
    }

    // 2. FORCE Command: Forces execution on a target card process slot
    if (strcmp(token, "FORCE") == 0) {
        // Enforce Admin ACL check
        if (!is_admin(caller_key)) {
            *out_err = "REVERT: KBM_FORCE_PERMISSION_DENIED";
            return false;
        }

        char *target_str = strtok(NULL, " ");
        char *directive = strtok(NULL, " ");
        if (!target_str || !directive) {
            *out_err = "REVERT: KBM_FORCE_INVALID_SYNTAX";
            return false;
        }

        uint32_t target_card = atoi(target_str);
        if (target_card >= MAX_CARDS || !g_vm.cards[target_card].is_active) {
            *out_err = "REVERT: KBM_FORCE_TARGET_NOT_FOUND";
            return false;
        }

        // Force transition directive into the target card's buffer
        strncpy(g_vm.cards[target_card].current_directive, directive, 63);
        g_vm.cards[target_card].current_directive[63] = '\0';
        
        // Mock state execution update triggered on target card
        if (strcmp(directive, "EPOCH_INIT") == 0) {
            g_vm.cards[target_card].epoch = 0;
        } else if (strcmp(directive, "HELMHOLTZ_RESONATE") == 0) {
            g_vm.cards[target_card].epoch = 4; // Map directly to Resonance epoch
        }

        printf("   [Helmholtz KBM] Forced directive '%s' onto target Job %d\n", directive, target_card);
        return true;
    }

    *out_err = "REVERT: KBM_UNKNOWN_COMMAND";
    return false;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: HELMHOLTZ-COMPATIBLE KBM COMMAND VERIFICATION\n");
    printf("=============================================================\n");

    init_vm();
    const char *err = NULL;

    // 1. Test Case 1: Execute SYSTAT (Allowed for anyone)
    printf("1. Triggering Helmholtz SYSTAT...\n");
    bool res = execute_kbm_directive(22, "SYSTAT", &err); // User key 22
    assert(res == true);
    printf("%s\n", g_vm.system_log);
    printf("   ✓ SYSTAT executed and logged successfully.\n\n");

    // 2. Test Case 2: User attempts FORCE command (Denied by ACL)
    printf("2. User key 22 attempting to FORCE directive on Job 2...\n");
    res = execute_kbm_directive(22, "FORCE 2 HELMHOLTZ_RESONATE", &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: KBM_FORCE_PERMISSION_DENIED") == 0);
    printf("   ✓ Force command rejected correctly: %s\n\n", err);

    // 3. Test Case 3: Admin attempts FORCE command (Allowed)
    printf("3. Admin key 11 attempting to FORCE directive on Job 2...\n");
    // Verify target card state before force
    assert(g_vm.cards[2].epoch == 3);
    assert(strcmp(g_vm.cards[2].current_directive, "WAIT") == 0);

    res = execute_kbm_directive(11, "FORCE 2 HELMHOLTZ_RESONATE", &err);
    assert(res == true);
    
    // Verify target card state after force (transitioned to resonance epoch)
    assert(g_vm.cards[2].epoch == 4);
    assert(strcmp(g_vm.cards[2].current_directive, "HELMHOLTZ_RESONATE") == 0);
    printf("   ✓ Force command succeeded. Job 2 transitioned to: %s (Epoch %d)\n\n",
           g_vm.cards[2].current_directive, g_vm.cards[2].epoch);

    // 4. Test Case 4: Run final SYSTAT to show updated state
    printf("4. Running final SYSTAT validation...\n");
    execute_kbm_directive(11, "SYSTAT", &err);
    printf("%s\n", g_vm.system_log);

    printf("=============================================================\n");
    printf("AUNCIENT HELMHOLTZ KBM TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
