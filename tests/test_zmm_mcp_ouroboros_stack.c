#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// Opcodes
typedef enum {
    YUL_OP_OUROBOROS_PUSH = 0x88,
    YUL_OP_OUROBOROS_POP  = 0x89
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

// Ouroboros Stack: Self-referencing recursive B-level stack
#define OUROBOROS_STACK_SIZE 8
typedef struct {
    uint64_t stack_data[OUROBOROS_STACK_SIZE];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
} OuroborosStack;

OuroborosStack g_ouroboros_stack;

// Registry tracking ACL permissions
#define MAX_KEYS 4
typedef struct {
    uint32_t key_id;
    PPN      ppn;
    AclLevel acl;
} KeyEntry;

KeyEntry g_keys[MAX_KEYS];

// Global gas tracking
uint64_t g_balances[100];
const uint64_t UNIVERSAL_GAS_FEE = 15;

uint32_t get_ppn_slot(PPN ppn) {
    return (ppn.project + ppn.programmer) % 100;
}

void init_ouroboros_system(void) {
    memset(&g_ouroboros_stack, 0, sizeof(g_ouroboros_stack));
    memset(g_balances, 0, sizeof(g_balances));
    memset(g_keys, 0, sizeof(g_keys));
    
    // Seed Keys
    g_keys[0] = (KeyEntry){ .key_id = 11, .ppn = {1, 2},  .acl = ACL_ADMIN };
    g_keys[1] = (KeyEntry){ .key_id = 22, .ppn = {10, 5}, .acl = ACL_USER };
    g_keys[2] = (KeyEntry){ .key_id = 33, .ppn = {30, 1}, .acl = ACL_ANONYMOUS };
}

AclLevel get_key_acl(uint32_t key_id) {
    for (int i = 0; i < MAX_KEYS; i++) {
        if (g_keys[i].key_id == key_id) {
            return g_keys[i].acl;
        }
    }
    return ACL_ANONYMOUS;
}

// Push to recursive stack (Self-referencing loop)
bool ouroboros_push(uint64_t val, const char **out_err) {
    if (g_ouroboros_stack.count >= OUROBOROS_STACK_SIZE) {
        *out_err = "REVERT: OUROBOROS_STACK_OVERFLOW";
        return false;
    }
    g_ouroboros_stack.stack_data[g_ouroboros_stack.tail] = val;
    g_ouroboros_stack.tail = (g_ouroboros_stack.tail + 1) % OUROBOROS_STACK_SIZE;
    g_ouroboros_stack.count++;
    return true;
}

// Pop from recursive stack (Self-referencing loop)
bool ouroboros_pop(uint64_t *out_val, const char **out_err) {
    if (g_ouroboros_stack.count == 0) {
        *out_err = "REVERT: OUROBOROS_STACK_UNDERFLOW";
        return false;
    }
    *out_val = g_ouroboros_stack.stack_data[g_ouroboros_stack.head];
    g_ouroboros_stack.head = (g_ouroboros_stack.head + 1) % OUROBOROS_STACK_SIZE;
    g_ouroboros_stack.count--;
    return true;
}

// Helmholtz Coaxial Dispatcher with ACL protection
bool dispatch_ouroboros_transaction(uint32_t key_id, PPN ppn, YulOpcode op, uint64_t val_in,
                                    uint64_t *val_out, const char **out_err) {
    uint32_t slot = get_ppn_slot(ppn);
    if (g_balances[slot] < UNIVERSAL_GAS_FEE) {
        *out_err = "REVERT: INSUFFICIENT_GAS_BALANCE";
        return false;
    }
    g_balances[slot] -= UNIVERSAL_GAS_FEE;

    // Enforce ACL check: Anonymous keys are blocked from accessing the Ouroboros stack
    AclLevel acl = get_key_acl(key_id);
    if (acl < ACL_USER) {
        *out_err = "REVERT: ACL_PERMISSION_DENIED_FOR_OUROBOROS_ACCESS";
        return false;
    }

    switch (op) {
        case YUL_OP_OUROBOROS_PUSH: {
            printf("   [Ouroboros Push] PPN [%u,%u] pushed: 0x%lx\n", ppn.project, ppn.programmer, val_in);
            return ouroboros_push(val_in, out_err);
        }
        case YUL_OP_OUROBOROS_POP: {
            bool success = ouroboros_pop(val_out, out_err);
            if (success) {
                printf("   [Ouroboros Pop] PPN [%u,%u] popped: 0x%lx\n", ppn.project, ppn.programmer, *val_out);
            }
            return success;
        }
        default:
            *out_err = "REVERT: INVALID_OPCODE";
            return false;
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: OUROBOROS STACK PEERING TESTS\n");
    printf("=============================================================\n");

    init_ouroboros_system();
    
    PPN admin_ppn = { .project = 1, .programmer = 2 };
    PPN user_ppn = { .project = 10, .programmer = 5 };
    PPN anon_ppn = { .project = 30, .programmer = 1 };
    
    g_balances[get_ppn_slot(admin_ppn)] = 100;
    g_balances[get_ppn_slot(user_ppn)] = 100;
    g_balances[get_ppn_slot(anon_ppn)] = 100;

    const char *err = NULL;
    uint64_t val = 0;

    // 1. Test Case 1: Anonymous Peer attempts access (Blocked)
    printf("1. Anonymous peer 0x33 attempting to push to Ouroboros stack...\n");
    bool res = dispatch_ouroboros_transaction(33, anon_ppn, YUL_OP_OUROBOROS_PUSH, 0xABC123, NULL, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: ACL_PERMISSION_DENIED_FOR_OUROBOROS_ACCESS") == 0);
    printf("   ✓ Anonymous push access denied correctly.\n\n");

    // 2. Test Case 2: User Peer pushes data to stack (Allowed)
    printf("2. User Bob PPN [10,5] pushing data...\n");
    res = dispatch_ouroboros_transaction(22, user_ppn, YUL_OP_OUROBOROS_PUSH, 0x9999, NULL, &err);
    assert(res == true);
    printf("   ✓ Push succeeded.\n\n");

    // 3. Test Case 3: Admin Peer pops data from stack (Allowed)
    printf("3. Admin PPN [1,2] popping data...\n");
    res = dispatch_ouroboros_transaction(11, admin_ppn, YUL_OP_OUROBOROS_POP, 0, &val, &err);
    assert(res == true);
    assert(val == 0x9999);
    printf("   ✓ Pop succeeded. Popped value matches input in self-referencing stack.\n\n");

    // 4. Test Case 4: Pop from empty stack returns underflow
    printf("4. Attempting to pop from empty stack...\n");
    res = dispatch_ouroboros_transaction(11, admin_ppn, YUL_OP_OUROBOROS_POP, 0, &val, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: OUROBOROS_STACK_UNDERFLOW") == 0);
    printf("   ✓ Underflow caught successfully: %s\n\n", err);

    printf("=============================================================\n");
    printf("AUNCIENT OUROBOROS STACK PEERING TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
