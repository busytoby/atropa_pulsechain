#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// Universal Opcodes
typedef enum {
    YUL_OP_GAS_TRANSFER     = 0x20,
    YUL_OP_WRITE_STORAGE    = 0x31,
    YUL_OP_QUERY_READ       = 0x30,
    YUL_OP_EXEC_SYSCALL     = 0x40,
    YUL_OP_MOUNT_CONTRACT   = 0x61,
    YUL_OP_UNMOUNT_CONTRACT = 0x62,
    YUL_OP_MOUNT_LINUX      = 0x63,
    YUL_OP_UNMOUNT_LINUX    = 0x64,
    YUL_OP_KBM_CMD          = 0x80
} YulOpcode;

// ACL Level definitions
typedef enum {
    ACL_ANONYMOUS = 0,
    ACL_USER = 1,
    ACL_ADMIN = 2
} AclLevel;

// PPN Account Structure
typedef struct {
    uint16_t project;
    uint16_t programmer;
} PPN;

// Key registry entry mapping Key -> PPN and specific capabilities
typedef struct {
    uint32_t key_id;
    PPN      ppn;
    AclLevel acl;
    bool     can_bypass_gas;
    bool     can_write_any_storage;
    bool     can_mount_devices;
    bool     is_active;
} KeyEntry;

#define MAX_KEYS 16
KeyEntry g_key_registry[MAX_KEYS];
size_t g_key_count = 0;

// Dynamic contract mounts per session
#define MAX_MOUNTS 4
typedef struct {
    char mounted_contracts[MAX_MOUNTS][32];
    size_t mount_count;
} UserSession;

// ZMM Helmholtz Linux VM State
typedef struct {
    bool is_booted;
    bool is_mounted;
    char hostname[32];
} HelmholtzLinuxVm;

// Virtual card (process) slot in the VM scheduler
#define MAX_JOBS 8
typedef struct {
    uint32_t job_id;
    PPN      ppn;
    bool     is_active;
    bool     is_sleeping;
    char     current_directive[64];
} JobSlot;

// Telemetry Log Format
typedef struct {
    uint64_t timestamp;
    PPN      caller;
    uint32_t key_id;
    YulOpcode opcode;
    bool     success;
    char     status_msg[32];
} TelemLog;

#define TELEM_SIZE 16
typedef struct {
    TelemLog logs[TELEM_SIZE];
    uint32_t head;
    uint32_t tail;
} TelemBuffer;

// Global System State
uint64_t g_balances[100];
bool g_seen_clients[100];
uint32_t g_consecutive_failures[100];
UserSession g_user_sessions[100];
JobSlot g_jobs[MAX_JOBS];
HelmholtzLinuxVm g_linux_vm;
TelemBuffer g_telem;
uint32_t g_reschedule_count = 0;

const uint64_t UNIVERSAL_GAS_FEE = 15;
const uint64_t DEFAULT_BOOTSTRAP_ALLOWANCE = 50;
const uint32_t MAX_ALLOWED_FAILURES = 3;

uint32_t get_ppn_slot(PPN ppn) {
    return (ppn.project + ppn.programmer) % 100;
}

uint64_t get_balance(PPN ppn) {
    return g_balances[get_ppn_slot(ppn)];
}

void set_balance(PPN ppn, uint64_t balance) {
    g_balances[get_ppn_slot(ppn)] = balance;
}

// Telemetry log push
void telem_push(PPN caller, uint32_t key_id, YulOpcode op, bool success, const char *status) {
    uint32_t next_tail = (g_telem.tail + 1) % TELEM_SIZE;
    if (next_tail == g_telem.head) return; // Full, discard
    
    TelemLog *log = &g_telem.logs[g_telem.tail];
    log->timestamp = 1000;
    log->caller = caller;
    log->key_id = key_id;
    log->opcode = op;
    log->success = success;
    strncpy(log->status_msg, status, 31);
    
    g_telem.tail = next_tail;
}

// Registry helpers
void init_unified_system(void) {
    g_key_count = 0;
    g_reschedule_count = 0;
    memset(g_balances, 0, sizeof(g_balances));
    memset(g_seen_clients, 0, sizeof(g_seen_clients));
    memset(g_consecutive_failures, 0, sizeof(g_consecutive_failures));
    memset(g_user_sessions, 0, sizeof(g_user_sessions));
    memset(g_jobs, 0, sizeof(g_jobs));
    memset(&g_telem, 0, sizeof(g_telem));
    
    // Setup Linux VM
    g_linux_vm.is_booted = true;
    g_linux_vm.is_mounted = false;
    strcpy(g_linux_vm.hostname, "HelmholtzLinux");

    // Seed Active Jobs
    for (uint32_t i = 0; i < MAX_JOBS; i++) {
        g_jobs[i].job_id = i;
        g_jobs[i].is_active = (i < 3);
        g_jobs[i].is_sleeping = false;
        strcpy(g_jobs[i].current_directive, "WAIT");
    }
}

bool register_key(uint32_t key_id, uint16_t proj, uint16_t prog, AclLevel acl, bool bypass, bool write, bool mount) {
    if (g_key_count >= MAX_KEYS) return false;
    g_key_registry[g_key_count++] = (KeyEntry){
        .key_id = key_id, .ppn = {proj, prog}, .acl = acl,
        .can_bypass_gas = bypass, .can_write_any_storage = write, .can_mount_devices = mount, .is_active = true
    };
    return true;
}

KeyEntry* get_key(uint32_t key_id) {
    for (size_t i = 0; i < g_key_count; i++) {
        if (g_key_registry[i].is_active && g_key_registry[i].key_id == key_id) {
            return &g_key_registry[i];
        }
    }
    return NULL;
}

// Pre-flight check with dynamic allowance bootstrap
bool preflight_check(PPN caller, uint32_t key_id, const char **out_err) {
    (void)out_err;
    uint32_t slot = get_ppn_slot(caller);
    
    // Telemetry rate limiter: check consecutive failures
    if (g_consecutive_failures[slot] >= MAX_ALLOWED_FAILURES) {
        KeyEntry *key = get_key(key_id);
        if (key && key->acl > ACL_ANONYMOUS) {
            printf("   [Telemetry Limiter] Downgrading Key %u to ACL_ANONYMOUS due to %u consecutive failures.\n",
                   key_id, g_consecutive_failures[slot]);
            key->acl = ACL_ANONYMOUS;
        }
    }
    
    // Bootstrap unknown client
    if (!g_seen_clients[slot]) {
        g_seen_clients[slot] = true;
        set_balance(caller, DEFAULT_BOOTSTRAP_ALLOWANCE);
        register_key(key_id, caller.project, caller.programmer, ACL_ANONYMOUS, false, false, false);
        printf("   [Bootstrap] Registered anonymous caller PPN [%u,%u] with %lu Gas allowance.\n",
               caller.project, caller.programmer, DEFAULT_BOOTSTRAP_ALLOWANCE);
    }
    
    return true;
}

// Route KBM commands
bool process_kbm_command(KeyEntry *key, const char *cmd, const char **out_err) {
    char cmd_copy[128];
    strncpy(cmd_copy, cmd, 127);
    cmd_copy[127] = '\0';
    
    char *token = strtok(cmd_copy, " ");
    if (!token) return false;
    
    if (strcmp(token, "SYSTAT") == 0) {
        printf("\n======= UNIFIED SYSTEM SYSTAT =======\n");
        printf("Linux VM: %s | Mounted: %s\n", g_linux_vm.hostname, g_linux_vm.is_mounted ? "YES" : "NO");
        printf("Reschedule Interrupts Triggered: %u\n", g_reschedule_count);
        for (int i = 0; i < MAX_JOBS; i++) {
            if (g_jobs[i].is_active) {
                printf("Job %d | PPN [%u,%u] | Sleeping: %s | Directive: %s\n",
                       g_jobs[i].job_id, g_jobs[i].ppn.project, g_jobs[i].ppn.programmer,
                       g_jobs[i].is_sleeping ? "YES" : "NO", g_jobs[i].current_directive);
            }
        }
        printf("=====================================\n\n");
        return true;
    }
    
    if (strcmp(token, "FORCE") == 0) {
        if (key->acl < ACL_ADMIN) {
            *out_err = "REVERT: KBM_FORCE_PERMISSION_DENIED";
            return false;
        }
        char *target_str = strtok(NULL, " ");
        char *directive = strtok(NULL, " ");
        if (!target_str || !directive) return false;
        
        uint32_t target_job = atoi(target_str);
        if (target_job >= MAX_JOBS || !g_jobs[target_job].is_active) return false;
        
        strcpy(g_jobs[target_job].current_directive, directive);
        if (g_jobs[target_job].is_sleeping) {
            g_jobs[target_job].is_sleeping = false;
            g_reschedule_count++;
        }
        printf("   [KBM FORCE] Forced directive '%s' onto Job %d\n", directive, target_job);
        return true;
    }
    
    *out_err = "REVERT: KBM_UNKNOWN_COMMAND";
    return false;
}

// Unified dispatcher
bool dispatch_transaction(PPN caller, uint32_t key_id, YulOpcode op, uint64_t target,
                          uint64_t val, const char *name, const char **out_err) {
    if (!preflight_check(caller, key_id, out_err)) {
        return false;
    }
    
    KeyEntry *key = get_key(key_id);
    if (!key || key->ppn.project != caller.project || key->ppn.programmer != caller.programmer) {
        *out_err = "REVERT: KEY_NOT_AUTHORIZED_FOR_THIS_PPN";
        telem_push(caller, key_id, op, false, "PPN_AUTH_FAILED");
        return false;
    }
    
    uint64_t balance = get_balance(caller);
    uint64_t fee = key->can_bypass_gas ? 0 : UNIVERSAL_GAS_FEE;
    
    if (balance < fee) {
        *out_err = "REVERT: INSUFFICIENT_GAS_BALANCE";
        telem_push(caller, key_id, op, false, "OUT_OF_GAS");
        return false;
    }
    
    // Enforce ACL
    bool acl_ok = false;
    switch (op) {
        case YUL_OP_QUERY_READ:
        case YUL_OP_MOUNT_CONTRACT:
        case YUL_OP_UNMOUNT_CONTRACT:
        case YUL_OP_EXEC_SYSCALL:
        case YUL_OP_KBM_CMD:
            acl_ok = true; // Anyone can attempt
            break;
        case YUL_OP_GAS_TRANSFER:
            acl_ok = (key->acl >= ACL_USER);
            break;
        case YUL_OP_WRITE_STORAGE:
        case YUL_OP_MOUNT_LINUX:
        case YUL_OP_UNMOUNT_LINUX:
            acl_ok = (key->acl >= ACL_ADMIN);
            break;
    }
    
    if (!acl_ok) {
        *out_err = "REVERT: ACL_PERMISSION_DENIED";
        g_consecutive_failures[get_ppn_slot(caller)]++;
        telem_push(caller, key_id, op, false, "ACL_DENIED");
        return false;
    }

    // Deduct gas
    if (fee > 0) {
        set_balance(caller, balance - fee);
    }
    
    bool execution_success = false;
    switch (op) {
        case YUL_OP_MOUNT_CONTRACT: {
            uint32_t slot = get_ppn_slot(caller);
            UserSession *session = &g_user_sessions[slot];
            if (session->mount_count >= MAX_MOUNTS) {
                *out_err = "REVERT: MOUNT_LIMIT_EXCEEDED";
            } else {
                strncpy(session->mounted_contracts[session->mount_count++], name, 31);
                printf("   [Session] Mounted contract '%s' for PPN [%u,%u]\n", name, caller.project, caller.programmer);
                execution_success = true;
            }
            break;
        }
        case YUL_OP_QUERY_READ: {
            uint32_t slot = get_ppn_slot(caller);
            UserSession *session = &g_user_sessions[slot];
            bool mounted = false;
            for (size_t i = 0; i < session->mount_count; i++) {
                if (strcmp(session->mounted_contracts[i], name) == 0) {
                    mounted = true; break;
                }
            }
            if (!mounted) {
                *out_err = "REVERT: TARGET_CONTRACT_NOT_MOUNTED";
            } else {
                printf("   [Read Query] Successfully queried mounted contract '%s'\n", name);
                execution_success = true;
            }
            break;
        }
        case YUL_OP_MOUNT_LINUX: {
            if (!key->can_mount_devices) {
                *out_err = "REVERT: CAPABILITY_MOUNT_DENIED";
            } else if (g_linux_vm.is_mounted) {
                *out_err = "REVERT: LINUX_VM_ALREADY_MOUNTED";
            } else {
                g_linux_vm.is_mounted = true;
                printf("   [Admin] Mounted Helmholtz Linux VM instance.\n");
                execution_success = true;
            }
            break;
        }
        case YUL_OP_EXEC_SYSCALL: {
            if (!g_linux_vm.is_mounted) {
                *out_err = "REVERT: LINUX_VM_NOT_MOUNTED_IN_SYSTEM";
            } else {
                printf("   [Syscall] Processed syscall request on mounted Linux instance.\n");
                execution_success = true;
            }
            break;
        }
        case YUL_OP_KBM_CMD: {
            execution_success = process_kbm_command(key, name, out_err);
            break;
        }
        case YUL_OP_WRITE_STORAGE: {
            if (!key->can_write_any_storage) {
                *out_err = "REVERT: STORAGE_WRITE_ACCESS_DENIED";
            } else {
                PPN target_ppn = { .project = (uint16_t)(target >> 16), .programmer = (uint16_t)(target & 0xFFFF) };
                set_balance(target_ppn, val);
                printf("   [Override] Storage balance of PPN [%u,%u] forced to %lu\n",
                       target_ppn.project, target_ppn.programmer, val);
                execution_success = true;
            }
            break;
        }
        default:
            *out_err = "REVERT: INVALID_OPCODE";
            break;
    }
    
    if (execution_success) {
        g_consecutive_failures[get_ppn_slot(caller)] = 0; // Reset
        telem_push(caller, key_id, op, true, "SUCCESS");
    } else {
        g_consecutive_failures[get_ppn_slot(caller)]++;
        telem_push(caller, key_id, op, false, "EXEC_FAILED");
    }
    
    return execution_success;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: UNIFIED MCI MCP TAC SYSTEM TESTS\n");
    printf("=============================================================\n");

    init_unified_system();
    
    PPN bob   = {10, 5};
    PPN admin = {1, 2};
    const char *err = NULL;

    // Register primary keys
    assert(register_key(22, 10, 5, ACL_USER, false, false, false) == true);
    assert(register_key(11, 1, 2, ACL_ADMIN, true, true, true) == true); // Admin Key 11 (Bypass gas, write any, mount devices)
    
    set_balance(bob, 100);
    set_balance(admin, 100);

    // 1. Test Step 1: Anonymous peer bootstraps, mounts contract, and queries it
    printf("1. Running bootstrap and mounting validation...\n");
    PPN anon_user = {30, 1}; // Unknown client
    assert(dispatch_transaction(anon_user, 99, YUL_OP_MOUNT_CONTRACT, 0, 0, "TESTERC20", &err) == true);
    assert(get_balance(anon_user) == 35); // 50 (bootstrap) - 15 fee
    
    // Query contract
    assert(dispatch_transaction(anon_user, 99, YUL_OP_QUERY_READ, 0, 0, "TESTERC20", &err) == true);
    assert(get_balance(anon_user) == 20); // 35 - 15 fee
    printf("   ✓ Anonymous bootstrapping and contract mounting validated.\n\n");

    // 2. Test Step 2: Bob (User) triggers consecutive failures to provoke telemetry downgrade
    printf("2. Bob triggering consecutive transaction failures...\n");
    
    // Failure 1: Bob tries to execute storage override (Requires Admin)
    bool res = dispatch_transaction(bob, 22, YUL_OP_WRITE_STORAGE, 0, 0, NULL, &err);
    assert(res == false);
    
    // Failure 2: Bob tries to mount the Linux VM (Requires Admin)
    res = dispatch_transaction(bob, 22, YUL_OP_MOUNT_LINUX, 0, 0, NULL, &err);
    assert(res == false);
    
    // Failure 3: Bob tries an invalid opcode
    res = dispatch_transaction(bob, 22, 0x99, 0, 0, NULL, &err);
    assert(res == false);
    
    // Verify Bob is downgraded to ANONYMOUS
    assert(get_key(22)->acl == ACL_ANONYMOUS);
    printf("   ✓ Bob dynamically downgraded to ACL_ANONYMOUS due to 3 consecutive failures.\n\n");

    // 3. Test Step 3: Admin mounts Helmholtz Linux VM using gas bypass
    printf("3. Admin PPN [1,2] mounting Linux VM (Bypass Gas: True)...\n");
    res = dispatch_transaction(admin, 11, YUL_OP_MOUNT_LINUX, 0, 0, NULL, &err);
    assert(res == true);
    assert(get_balance(admin) == 100); // Balance unchanged due to gas bypass
    assert(g_linux_vm.is_mounted == true);
    printf("   ✓ Linux VM mounted successfully. Admin Balance: %lu Gas\n\n", get_balance(admin));

    // 4. Test Step 4: Admin runs KBM SYSTAT to print system logs
    printf("4. Admin running SYSTAT command...\n");
    res = dispatch_transaction(admin, 11, YUL_OP_KBM_CMD, 0, 0, "SYSTAT", &err);
    assert(res == true);
    
    // 5. Test Step 5: Read and print telemetry logs
    printf("5. Validating telemetry ring buffer...\n");
    uint32_t head = g_telem.head;
    uint32_t tail = g_telem.tail;
    for (uint32_t i = head; i < tail; i++) {
        TelemLog *log = &g_telem.logs[i];
        printf("   [Telemetry %d] Caller: [%u,%u] | Key: %u | Opcode: 0x%02X | Success: %s | Msg: %s\n",
               i, log->caller.project, log->caller.programmer, log->key_id, log->opcode,
               log->success ? "YES" : "NO", log->status_msg);
    }

    printf("\n=============================================================\n");
    printf("AUNCIENT UNIFIED MCI MCP TAC SYSTEM TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
