#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// Opcodes
typedef enum {
    YUL_OP_OUROBOROS_PUSH = 0x88,
    YUL_OP_OUROBOROS_POP  = 0x89,
    YUL_OP_PEER_MOUNT     = 0x8A
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

// Peer Discovery System
#define MAX_PEERS 4
typedef struct {
    PPN ppn;
    bool is_connected;
    bool is_mounted;
} DiscoveryPeer;

DiscoveryPeer g_discovery_peers[MAX_PEERS];

uint32_t get_ppn_slot(PPN ppn) {
    return (ppn.project + ppn.programmer) % 100;
}

void init_ouroboros_system(void) {
    memset(&g_ouroboros_stack, 0, sizeof(g_ouroboros_stack));
    memset(g_balances, 0, sizeof(g_balances));
    memset(g_keys, 0, sizeof(g_keys));
    memset(g_discovery_peers, 0, sizeof(g_discovery_peers));
    
    // Seed Keys
    g_keys[0] = (KeyEntry){ .key_id = 11, .ppn = {1, 2},  .acl = ACL_ADMIN };
    g_keys[1] = (KeyEntry){ .key_id = 22, .ppn = {10, 5}, .acl = ACL_USER };
    g_keys[2] = (KeyEntry){ .key_id = 33, .ppn = {30, 1}, .acl = ACL_ANONYMOUS };

    // Seed Discovery Peers
    g_discovery_peers[0] = (DiscoveryPeer){ .ppn = {10, 5}, .is_connected = true, .is_mounted = false };
    g_discovery_peers[1] = (DiscoveryPeer){ .ppn = {30, 1}, .is_connected = false, .is_mounted = false };
}

AclLevel get_key_acl(uint32_t key_id) {
    for (int i = 0; i < MAX_KEYS; i++) {
        if (g_keys[i].key_id == key_id) {
            return g_keys[i].acl;
        }
    }
    return ACL_ANONYMOUS;
}

// Check peer discovery list
bool verify_peer_connected(PPN peer_ppn) {
    for (int i = 0; i < MAX_PEERS; i++) {
        if (g_discovery_peers[i].ppn.project == peer_ppn.project &&
            g_discovery_peers[i].ppn.programmer == peer_ppn.programmer) {
            return g_discovery_peers[i].is_connected;
        }
    }
    return false;
}

// Mount target peer
bool mount_target_peer(PPN peer_ppn, const char **out_err) {
    for (int i = 0; i < MAX_PEERS; i++) {
        if (g_discovery_peers[i].ppn.project == peer_ppn.project &&
            g_discovery_peers[i].ppn.programmer == peer_ppn.programmer) {
            if (!g_discovery_peers[i].is_connected) {
                *out_err = "REVERT: PEER_NOT_CONNECTED_IN_DISCOVERY";
                return false;
            }
            g_discovery_peers[i].is_mounted = true;
            return true;
        }
    }
    *out_err = "REVERT: PEER_NOT_FOUND_IN_DISCOVERY";
    return false;
}

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

// Helmholtz Coaxial Dispatcher with ACL protection and Peer-to-Peer mounting
bool dispatch_ouroboros_transaction(uint32_t key_id, PPN ppn, YulOpcode op, uint64_t val_in,
                                    uint64_t *val_out, const char **out_err) {
    uint32_t slot = get_ppn_slot(ppn);
    if (g_balances[slot] < UNIVERSAL_GAS_FEE) {
        *out_err = "REVERT: INSUFFICIENT_GAS_BALANCE";
        return false;
    }
    g_balances[slot] -= UNIVERSAL_GAS_FEE;

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
        case YUL_OP_PEER_MOUNT: {
            PPN target_peer = {
                .project = (uint16_t)(val_in >> 16),
                .programmer = (uint16_t)(val_in & 0xFFFF)
            };
            printf("   [Peer Mount] PPN [%u,%u] requesting mount for Peer [%u,%u]...\n",
                   ppn.project, ppn.programmer, target_peer.project, target_peer.programmer);
            return mount_target_peer(target_peer, out_err);
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

    // 4. Test Case 4: Peer Mount (Mount active connected peer Bob [10,5])
    printf("4. Admin PPN [1,2] attempting to mount connected Peer [10,5]...\n");
    uint64_t peer_val = ((uint64_t)10 << 16) | 5;
    res = dispatch_ouroboros_transaction(11, admin_ppn, YUL_OP_PEER_MOUNT, peer_val, NULL, &err);
    assert(res == true);
    printf("   ✓ Peer mount succeeded.\n\n");

    // 5. Test Case 5: Peer Mount Failure (Mount disconnected Peer [30,1])
    printf("5. Admin PPN [1,2] attempting to mount disconnected Peer [30,1]...\n");
    uint64_t offline_peer_val = ((uint64_t)30 << 16) | 1;
    res = dispatch_ouroboros_transaction(11, admin_ppn, YUL_OP_PEER_MOUNT, offline_peer_val, NULL, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: PEER_NOT_CONNECTED_IN_DISCOVERY") == 0);
    printf("   ✓ Caught expected failure: %s\n\n", err);

    printf("=============================================================\n");
    printf("AUNCIENT OUROBOROS STACK PEERING TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
