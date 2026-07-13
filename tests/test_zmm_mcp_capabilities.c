#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// Yul Opcode definition
typedef enum {
    YUL_OP_GAS_TRANSFER = 0x20,
    YUL_OP_WRITE_STORAGE = 0x31,
    YUL_OP_MOUNT_DEVICE = 0x60
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

// Registered key entry with dynamic capability flags
#define MAX_KEYS 16
typedef struct {
    uint32_t key_id;
    PPN      ppn;
    AclLevel acl;
    bool     can_bypass_gas;
    bool     can_write_any_storage;
    bool     can_mount_devices;
    bool     is_active;
} KeyRegistryEntry;

KeyRegistryEntry g_key_registry[MAX_KEYS];
size_t g_key_count = 0;

// Global state variables simulating database
uint64_t g_balances[100]; 
char g_mounted_devices[4][16]; // Mock device mount paths e.g., DK0, LP0
size_t g_device_count = 0;

const uint64_t UNIVERSAL_GAS_FEE = 15;

uint32_t get_ppn_slot(PPN ppn) {
    return (ppn.project + ppn.programmer) % 100;
}

uint64_t get_ppn_balance(PPN ppn) {
    return g_balances[get_ppn_slot(ppn)];
}

void set_ppn_balance(PPN ppn, uint64_t balance) {
    g_balances[get_ppn_slot(ppn)] = balance;
}

void registry_init(void) {
    g_key_count = 0;
    g_device_count = 0;
    memset(g_key_registry, 0, sizeof(g_key_registry));
    memset(g_mounted_devices, 0, sizeof(g_mounted_devices));
}

bool register_key_capabilities(uint32_t key_id, uint16_t project, uint16_t programmer, AclLevel acl,
                               bool bypass_gas, bool write_any, bool mount_devices) {
    if (g_key_count >= MAX_KEYS) return false;
    
    KeyRegistryEntry *entry = &g_key_registry[g_key_count++];
    entry->key_id = key_id;
    entry->ppn.project = project;
    entry->ppn.programmer = programmer;
    entry->acl = acl;
    entry->can_bypass_gas = bypass_gas;
    entry->can_write_any_storage = write_any;
    entry->can_mount_devices = mount_devices;
    entry->is_active = true;
    
    return true;
}

// Find registry entry for caller key
KeyRegistryEntry* find_key_entry(uint32_t key_id) {
    for (size_t i = 0; i < g_key_count; i++) {
        if (g_key_registry[i].is_active && g_key_registry[i].key_id == key_id) {
            return &g_key_registry[i];
        }
    }
    return NULL;
}

// Coaxial transaction executor enforcing capabilities
bool process_coaxial_transaction(uint32_t key_id, PPN ppn, YulOpcode op, uint64_t target,
                                 uint64_t val, const char *dev_name, const char **out_err) {
    KeyRegistryEntry *key = find_key_entry(key_id);
    if (!key) {
        *out_err = "REVERT: UNREGISTERED_KEY";
        return false;
    }
    
    // Verify Key is authorized for the PPN
    if (key->ppn.project != ppn.project || key->ppn.programmer != ppn.programmer) {
        *out_err = "REVERT: KEY_NOT_AUTHORIZED_FOR_THIS_PPN";
        return false;
    }
    
    uint64_t balance = get_ppn_balance(ppn);
    uint64_t fee = key->can_bypass_gas ? 0 : UNIVERSAL_GAS_FEE;
    
    // Gas deduction check
    if (!key->can_bypass_gas && balance < fee) {
        *out_err = "REVERT: INSUFFICIENT_GAS_BALANCE";
        return false;
    }
    
    if (fee > 0) {
        set_ppn_balance(ppn, balance - fee);
    }

    switch (op) {
        case YUL_OP_GAS_TRANSFER: {
            PPN recipient = { .project = (uint16_t)(target >> 16), .programmer = (uint16_t)(target & 0xFFFF) };
            uint64_t usable_bal = get_ppn_balance(ppn);
            if (usable_bal < val) {
                *out_err = "REVERT: INSUFFICIENT_BALANCE_FOR_TRANSFER";
                return false;
            }
            set_ppn_balance(ppn, usable_bal - val);
            set_ppn_balance(recipient, get_ppn_balance(recipient) + val);
            printf("   [TRANSFER] Sent %lu to PPN [%u,%u] (Charged %lu Gas)\n",
                   val, recipient.project, recipient.programmer, fee);
            break;
        }
        case YUL_OP_WRITE_STORAGE: {
            PPN target_ppn = { .project = (uint16_t)(target >> 16), .programmer = (uint16_t)(target & 0xFFFF) };
            
            // Check storage override privilege
            if (!key->can_write_any_storage && (ppn.project != target_ppn.project || ppn.programmer != target_ppn.programmer)) {
                *out_err = "REVERT: STORAGE_WRITE_ACCESS_DENIED";
                return false;
            }
            
            set_ppn_balance(target_ppn, val);
            printf("   [STORAGE WRITE] Direct override of PPN [%u,%u] balance to %lu (Charged %lu Gas)\n",
                   target_ppn.project, target_ppn.programmer, val, fee);
            break;
        }
        case YUL_OP_MOUNT_DEVICE: {
            // Check mount device capability
            if (!key->can_mount_devices) {
                *out_err = "REVERT: CAPABILITY_MOUNT_DENIED";
                return false;
            }
            
            if (g_device_count >= 4) {
                *out_err = "REVERT: DEVICE_SLOTS_FULL";
                return false;
            }
            
            strncpy(g_mounted_devices[g_device_count++], dev_name, 15);
            printf("   [MOUNT] Device mounted successfully: %s (Charged %lu Gas)\n", dev_name, fee);
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
    printf("AUNCIENT ZMM VM: DYNAMIC CAPABILITY ACL VERIFICATION\n");
    printf("=============================================================\n");

    registry_init();
    memset(g_balances, 0, sizeof(g_balances));

    // Register PPN accounts
    PPN sys_admin = { .project = 1, .programmer = 2 }; // PPN [1,2]
    PPN user_bob  = { .project = 10, .programmer = 5 }; // PPN [10,5]
    
    set_ppn_balance(sys_admin, 100);
    set_ppn_balance(user_bob, 50);

    // Register Key 11 (Admin PPN [1,2]) with all capability flags enabled
    assert(register_key_capabilities(11, 1, 2, ACL_ADMIN, true, true, true) == true);
    // Register Key 22 (User PPN [10,5]) with standard default flags (no bypass, no override, no mount)
    assert(register_key_capabilities(22, 10, 5, ACL_USER, false, false, false) == true);

    const char *err = NULL;

    // 1. Test Case 1: Gas Bypass Verification (Key 11 is exempt)
    printf("1. Admin PPN [1,2] executing transfer (Bypass Gas: True)...\n");
    uint64_t target_encoded = (10LL << 16) | 5; // Bob PPN
    bool res = process_coaxial_transaction(11, sys_admin, YUL_OP_GAS_TRANSFER, target_encoded, 20, NULL, &err);
    assert(res == true);
    assert(get_ppn_balance(sys_admin) == 80); // 100 - 20 (no gas fee charged)
    assert(get_ppn_balance(user_bob) == 70);   // 50 + 20
    printf("   ✓ Gas bypass confirmed. Admin Balance: %lu Gas\n\n", get_ppn_balance(sys_admin));

    // 2. Test Case 2: Storage Write Override (Key 11 overrides Bob's balance directly)
    printf("2. Admin PPN [1,2] executing storage override on Bob's balance (Write Any: True)...\n");
    res = process_coaxial_transaction(11, sys_admin, YUL_OP_WRITE_STORAGE, target_encoded, 500, NULL, &err);
    assert(res == true);
    assert(get_ppn_balance(user_bob) == 500);
    printf("   ✓ Storage override confirmed. Bob Balance updated directly to: %lu Gas\n\n", get_ppn_balance(user_bob));

    // 3. Test Case 3: Storage Override Denied for Standard User
    printf("3. User Bob PPN [10,5] attempting storage override on Admin (Write Any: False)...\n");
    uint64_t admin_encoded = (1LL << 16) | 2;
    res = process_coaxial_transaction(22, user_bob, YUL_OP_WRITE_STORAGE, admin_encoded, 0, NULL, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: STORAGE_WRITE_ACCESS_DENIED") == 0);
    printf("   ✓ User write override rejected correctly: %s\n\n", err);

    // 4. Test Case 4: Device Mount Denied for Standard User
    printf("4. User Bob PPN [10,5] attempting to mount device 'DK0' (Mount Device: False)...\n");
    res = process_coaxial_transaction(22, user_bob, YUL_OP_MOUNT_DEVICE, 0, 0, "DK0", &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: CAPABILITY_MOUNT_DENIED") == 0);
    printf("   ✓ User device mount rejected correctly: %s\n\n", err);

    // 5. Test Case 5: Device Mount Allowed for Admin
    printf("5. Admin PPN [1,2] attempting to mount device 'DK0' (Mount Device: True)...\n");
    res = process_coaxial_transaction(11, sys_admin, YUL_OP_MOUNT_DEVICE, 0, 0, "DK0", &err);
    assert(res == true);
    assert(g_device_count == 1);
    assert(strcmp(g_mounted_devices[0], "DK0") == 0);
    printf("   ✓ Admin device mount succeeded.\n\n");

    printf("=============================================================\n");
    printf("AUNCIENT DYNAMIC CAPABILITY TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
