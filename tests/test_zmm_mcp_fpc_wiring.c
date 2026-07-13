#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// Standard Operation Types matching RSTS/E file access
typedef enum {
    OP_READ    = 1 << 0, // Read storage or wire state
    OP_WRITE   = 1 << 1, // Write to storage or configure wire
    OP_EXECUTE = 1 << 2, // Execute thunk code
    OP_ROUTE   = 1 << 3  // Route signal via coaxial connections / synthesizer
} FpcOp;

// PPN Definition
typedef struct {
    uint16_t project;
    uint16_t programmer;
} PPN;

// File Protection Code structure applied to nodes/wires/thunks
typedef struct {
    PPN      owner;
    uint16_t user_bits;  // Owner rights (Read, Write, Execute, Route)
    uint16_t group_bits; // Group rights (Same Project)
    uint16_t world_bits; // World rights (Anyone/Anonymous)
} FPC;

// 1. Synthesizer/Bionika Wire Node
typedef struct {
    char     wire_name[32];
    FPC      fpc;
    double   frequency;
} BionikaWire;

// 2. Coaxial Connection / Thunk Peer
typedef struct {
    char     peer_uri[64];
    FPC      fpc;
    bool     is_active;
} CoaxialPeer;

// 3. Merkle 2-3 Tree Storage Node
typedef struct {
    uint32_t register_id;
    FPC      fpc;
    uint64_t value;
} StorageNode;

// Helper to evaluate FPC permissions
bool verify_fpc_access(PPN caller, FPC target_fpc, FpcOp op) {
    // Admin override (PPN [1,2] bypasses all FPCs)
    if (caller.project == 1 && caller.programmer == 2) {
        return true;
    }
    
    uint16_t requested = (uint16_t)op;
    
    // Check Owner match
    if (caller.project == target_fpc.owner.project && caller.programmer == target_fpc.owner.programmer) {
        return (target_fpc.user_bits & requested) == requested;
    }
    
    // Check Group match (Same Project)
    if (caller.project == target_fpc.owner.project) {
        return (target_fpc.group_bits & requested) == requested;
    }
    
    // Check World / Anonymous rights
    return (target_fpc.world_bits & requested) == requested;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: UNIVERSAL FILE PROTECTION CODE TESTS\n");
    printf("=============================================================\n");

    PPN sys_admin = { .project = 1, .programmer = 2 };
    PPN user_alice = { .project = 10, .programmer = 5 };
    PPN user_bob = { .project = 10, .programmer = 6 };
    PPN user_charlie = { .project = 20, .programmer = 1 }; // Different project (World category)

    // Setup FPCs:
    // Owner: Full rights. Group: Read/Route only. World: No rights.
    FPC secure_fpc = {
        .owner = user_alice,
        .user_bits = OP_READ | OP_WRITE | OP_EXECUTE | OP_ROUTE,
        .group_bits = OP_READ | OP_ROUTE,
        .world_bits = 0
    };

    // 1. Verify Bionika Synthesizer Wire access
    printf("1. Verifying Bionika Synthesizer Wire Access...\n");
    BionikaWire wire = { .wire_name = "Synthesizer_Out_0", .fpc = secure_fpc, .frequency = 440.0 };

    // Alice (Owner) tries to write new frequency (Allowed)
    assert(verify_fpc_access(user_alice, wire.fpc, OP_WRITE) == true);
    // Bob (Group member) tries to write (Denied)
    assert(verify_fpc_access(user_bob, wire.fpc, OP_WRITE) == false);
    // Bob (Group member) tries to route signal (Allowed)
    assert(verify_fpc_access(user_bob, wire.fpc, OP_ROUTE) == true);
    // Charlie (World) tries to read (Denied)
    assert(verify_fpc_access(user_charlie, wire.fpc, OP_READ) == false);
    // Admin bypasses checks (Allowed)
    assert(verify_fpc_access(sys_admin, wire.fpc, OP_WRITE) == true);
    printf("   ✓ Bionika wire protection codes validated successfully.\n\n");

    // 2. Verify Coaxial Peer/Thunk access
    printf("2. Verifying Coaxial Connection / Thunk Peer Access...\n");
    CoaxialPeer peer = { .peer_uri = "peer_thunk://ALSA_AUDIO", .fpc = secure_fpc, .is_active = true };

    // Bob (Group) tries to execute thunk code (Denied)
    assert(verify_fpc_access(user_bob, peer.fpc, OP_EXECUTE) == false);
    // Alice (Owner) executes thunk (Allowed)
    assert(verify_fpc_access(user_alice, peer.fpc, OP_EXECUTE) == true);
    // Charlie (World) routes signal (Denied)
    assert(verify_fpc_access(user_charlie, peer.fpc, OP_ROUTE) == false);
    printf("   ✓ Coaxial peer execution rights validated successfully.\n\n");

    // 3. Verify 2-3 Tree Storage node access
    printf("3. Verifying Merkle 2-3 Tree Storage Node Access...\n");
    StorageNode node = { .register_id = 45, .fpc = secure_fpc, .value = 9999 };

    // Bob (Group) reads register value (Allowed)
    assert(verify_fpc_access(user_bob, node.fpc, OP_READ) == true);
    // Charlie (World) reads register value (Denied)
    assert(verify_fpc_access(user_charlie, node.fpc, OP_READ) == false);
    // Bob (Group) writes register value (Denied)
    assert(verify_fpc_access(user_bob, node.fpc, OP_WRITE) == false);
    printf("   ✓ 2-3 Tree storage node security checks validated successfully.\n\n");

    printf("=============================================================\n");
    printf("AUNCIENT UNIVERSAL FPC TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
