#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// Yul Opcode definition
typedef enum {
    YUL_OP_EXEC_SYSCALL = 0x40,
    YUL_OP_MOUNT_LINUX = 0x63,
    YUL_OP_UNMOUNT_LINUX = 0x64
} YulOpcode;

// PPN Definition
typedef struct {
    uint16_t project;
    uint16_t programmer;
} PPN;

// Simulated Helmholtz Linux VM state
typedef struct {
    bool is_booted;
    bool is_mounted;
    char hostname[32];
} HelmholtzLinuxVm;

HelmholtzLinuxVm g_linux_vm;

// Registry tracking admin permissions
#define MAX_KEYS 4
typedef struct {
    uint32_t key_id;
    PPN      ppn;
    bool     can_mount_devices;
} KeyEntry;

KeyEntry g_keys[MAX_KEYS];

// Global gas tracking
uint64_t g_balances[100];
const uint64_t UNIVERSAL_GAS_FEE = 15;

uint32_t get_ppn_slot(PPN ppn) {
    return (ppn.project + ppn.programmer) % 100;
}

void init_system(void) {
    memset(g_balances, 0, sizeof(g_balances));
    memset(g_keys, 0, sizeof(g_keys));
    
    // Setup Virtual Linux VM
    g_linux_vm.is_booted = true;
    g_linux_vm.is_mounted = false;
    strcpy(g_linux_vm.hostname, "HelmholtzLinux");
    
    // Seed Keys
    g_keys[0] = (KeyEntry){ .key_id = 11, .ppn = {1, 2},  .can_mount_devices = true };  // Admin
    g_keys[1] = (KeyEntry){ .key_id = 22, .ppn = {10, 5}, .can_mount_devices = false }; // User
}

bool has_mount_capability(uint32_t key_id) {
    for (int i = 0; i < MAX_KEYS; i++) {
        if (g_keys[i].key_id == key_id && g_keys[i].can_mount_devices) {
            return true;
        }
    }
    return false;
}

// Coaxial execution pipeline
bool process_coaxial_transaction(uint32_t key_id, PPN ppn, YulOpcode op, const char **out_err) {
    uint32_t slot = get_ppn_slot(ppn);
    if (g_balances[slot] < UNIVERSAL_GAS_FEE) {
        *out_err = "REVERT: INSUFFICIENT_GAS_BALANCE";
        return false;
    }
    g_balances[slot] -= UNIVERSAL_GAS_FEE;

    switch (op) {
        case YUL_OP_MOUNT_LINUX: {
            if (!has_mount_capability(key_id)) {
                *out_err = "REVERT: CAPABILITY_MOUNT_DENIED";
                return false;
            }
            if (g_linux_vm.is_mounted) {
                *out_err = "REVERT: LINUX_VM_ALREADY_MOUNTED";
                return false;
            }
            g_linux_vm.is_mounted = true;
            printf("   [Admin] Successfully mounted booted ZMM Helmholtz Linux instance: %s\n", g_linux_vm.hostname);
            break;
        }
        case YUL_OP_UNMOUNT_LINUX: {
            if (!has_mount_capability(key_id)) {
                *out_err = "REVERT: CAPABILITY_MOUNT_DENIED";
                return false;
            }
            if (!g_linux_vm.is_mounted) {
                *out_err = "REVERT: LINUX_VM_NOT_MOUNTED";
                return false;
            }
            g_linux_vm.is_mounted = false;
            printf("   [Admin] Successfully unmounted ZMM Helmholtz Linux instance: %s\n", g_linux_vm.hostname);
            break;
        }
        case YUL_OP_EXEC_SYSCALL: {
            // Verify that the Linux VM is mounted to execute syscalls
            if (!g_linux_vm.is_mounted) {
                *out_err = "REVERT: LINUX_VM_NOT_MOUNTED_IN_SYSTEM";
                return false;
            }
            printf("   [Syscall] Processed syscall request on mounted instance '%s' successfully\n", g_linux_vm.hostname);
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
    printf("AUNCIENT ZMM VM: HELMHOLTZ LINUX INSTANCE MOUNT TESTS\n");
    printf("=============================================================\n");

    init_system();
    
    PPN admin_ppn = { .project = 1, .programmer = 2 };
    PPN user_ppn = { .project = 10, .programmer = 5 };
    
    g_balances[get_ppn_slot(admin_ppn)] = 100;
    g_balances[get_ppn_slot(user_ppn)] = 100;

    const char *err = NULL;

    // 1. Test Case 1: User attempts to execute syscall before mount (Rejected)
    printf("1. User PPN [10,5] executing Syscall on unmounted Linux VM...\n");
    bool res = process_coaxial_transaction(22, user_ppn, YUL_OP_EXEC_SYSCALL, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: LINUX_VM_NOT_MOUNTED_IN_SYSTEM") == 0);
    printf("   ✓ Syscall rejected correctly (Linux VM not mounted).\n\n");

    // 2. Test Case 2: User attempts to mount Linux VM (Denied by capability)
    printf("2. User PPN [10,5] attempting to mount Linux VM...\n");
    res = process_coaxial_transaction(22, user_ppn, YUL_OP_MOUNT_LINUX, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: CAPABILITY_MOUNT_DENIED") == 0);
    printf("   ✓ Mount rejected correctly for standard user: %s\n\n", err);

    // 3. Test Case 3: Admin mounts Linux VM (Allowed)
    printf("3. Admin PPN [1,2] mounting Linux VM...\n");
    res = process_coaxial_transaction(11, admin_ppn, YUL_OP_MOUNT_LINUX, &err);
    assert(res == true);
    assert(g_linux_vm.is_mounted == true);
    printf("   ✓ Mount succeeded.\n\n");

    // 4. Test Case 4: User executes syscall after mount (Allowed)
    printf("4. User PPN [10,5] executing Syscall on mounted Linux VM...\n");
    res = process_coaxial_transaction(22, user_ppn, YUL_OP_EXEC_SYSCALL, &err);
    assert(res == true);
    printf("   ✓ Syscall succeeded on mounted Linux VM.\n\n");

    // 5. Test Case 5: Admin unmounts Linux VM (Allowed)
    printf("5. Admin PPN [1,2] unmounting Linux VM...\n");
    res = process_coaxial_transaction(11, admin_ppn, YUL_OP_UNMOUNT_LINUX, &err);
    assert(res == true);
    assert(g_linux_vm.is_mounted == false);
    printf("   ✓ Unmount succeeded.\n\n");

    // 6. Test Case 6: Syscall rejected again after unmount
    printf("6. User PPN [10,5] executing Syscall after unmount...\n");
    res = process_coaxial_transaction(22, user_ppn, YUL_OP_EXEC_SYSCALL, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: LINUX_VM_NOT_MOUNTED_IN_SYSTEM") == 0);
    printf("   ✓ Syscall rejected correctly.\n\n");

    printf("=============================================================\n");
    printf("AUNCIENT HELMHOLTZ LINUX INSTANCE MOUNT TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
