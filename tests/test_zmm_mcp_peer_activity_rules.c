#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_PEERS 4
#define DAMAGE_THRESHOLD 5

typedef enum {
    ACTIVITY_READ_TELEMETRY = 1 << 0,
    ACTIVITY_WRITE_COMMAND  = 1 << 1,
    ACTIVITY_PASSIVE_AUDIT  = 1 << 2
} PeerActivity;

typedef struct {
    uint32_t peer_id;
    uint32_t allowed_activities_mask; // Bitmask of activities this peer can perform on others
    uint32_t high_impedance_taps;     // Accumulator for passive audit overhead ("damage")
    uint64_t pmk;                     // Pairwise Master Key (PMK)
    bool is_quarantined;
} PeerNode;

// Checks activity rules and processes interactions between Peer A (actor) and Peer B (target)
bool execute_peer_action(PeerNode *actor, PeerNode *target, PeerActivity action) {
    // 1. Rule Check: Is the actor quarantined?
    if (actor->is_quarantined) {
        printf("   [Rule Deny] Actor Peer %u is quarantined.\n", actor->peer_id);
        return false;
    }

    // 2. Rule Check: Is the action allowed by the actor's register mapping?
    if (!(actor->allowed_activities_mask & action)) {
        printf("   [Rule Deny] Actor Peer %u lacks permission for action 0x%X\n", actor->peer_id, action);
        return false;
    }

    // 3. Process Activity
    if (action == ACTIVITY_PASSIVE_AUDIT) {
        // High-impedance taps act as accumulators for audit "damage"
        target->high_impedance_taps++;
        printf("   [Activity Audit] Peer %u audited Peer %u. High-impedance tap count: %u\n", 
               actor->peer_id, target->peer_id, target->high_impedance_taps);

        // Preempt and quarantine if target tap "damage" exceeds threshold limits
        if (target->high_impedance_taps >= DAMAGE_THRESHOLD) {
            target->is_quarantined = true;
            printf("   [Rule Lockout] Peer %u high-impedance tap damage exceeded threshold. Quarantined.\n", 
                   target->peer_id);
        }
    } else if (action == ACTIVITY_WRITE_COMMAND) {
        printf("   [Activity Write] Peer %u wrote command to Peer %u.\n", actor->peer_id, target->peer_id);
    } else if (action == ACTIVITY_READ_TELEMETRY) {
        printf("   [Activity Read] Peer %u read telemetry from Peer %u.\n", actor->peer_id, target->peer_id);
    }

    return true;
}

// Establishes a Pairwise Master Key (PMK) session between two valid peers
void establish_pmk_handshake(PeerNode *p1, PeerNode *p2) {
    // Simple custom hash combination for PMK generation
    p1->pmk = (p1->peer_id * 31ULL) ^ (p2->peer_id * 17ULL);
    p2->pmk = p1->pmk;
    printf("   [PMK Handshake] PMK established between Peer %u and Peer %u: 0x%lX\n", 
           p1->peer_id, p2->peer_id, p1->pmk);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PEER ACTIVITY RULES & HIGH-IMPEDANCE TAP ACCUMULATORS\n");
    printf("=============================================================\n");

    // Initialize 3 peers
    // Peer 500: Full access (read, write, audit)
    // Peer 501: Read only
    // Peer 502: Target peer
    PeerNode p500 = {.peer_id = 500, .allowed_activities_mask = ACTIVITY_READ_TELEMETRY | ACTIVITY_WRITE_COMMAND | ACTIVITY_PASSIVE_AUDIT, .high_impedance_taps = 0, .pmk = 0, .is_quarantined = false};
    PeerNode p501 = {.peer_id = 501, .allowed_activities_mask = ACTIVITY_READ_TELEMETRY, .high_impedance_taps = 0, .pmk = 0, .is_quarantined = false};
    PeerNode p502 = {.peer_id = 502, .allowed_activities_mask = ACTIVITY_READ_TELEMETRY, .high_impedance_taps = 0, .pmk = 0, .is_quarantined = false};

    // 1. Establish PMK Session
    printf("1. Establishing PMK Session...\n");
    establish_pmk_handshake(&p500, &p502);
    assert(p500.pmk != 0 && p500.pmk == p502.pmk);
    printf("   ✓ PMK successfully synced.\n\n");

    // 2. Validate disallowed activity permission checks
    printf("2. Peer 501 attempting to audit Peer 502 (Expected Deny)...\n");
    bool act1 = execute_peer_action(&p501, &p502, ACTIVITY_PASSIVE_AUDIT);
    assert(act1 == false);
    printf("   ✓ Write permissions blocked successfully.\n\n");

    // 3. Validate audit damage accumulation and threshold quarantines
    printf("3. Simulating repeated passive auditing on Peer 502 to accumulate damage...\n");
    for (int i = 0; i < DAMAGE_THRESHOLD; i++) {
        bool status = execute_peer_action(&p500, &p502, ACTIVITY_PASSIVE_AUDIT);
        assert(status == true);
    }
    // Peer 502 must now be quarantined
    assert(p502.is_quarantined == true);
    printf("   ✓ Accumulator threshold lockout verified.\n\n");

    // 4. Validate post-quarantine action blocks
    printf("4. Quarantined Peer 502 attempting to read telemetry from Peer 500 (Expected Deny)...\n");
    bool act2 = execute_peer_action(&p502, &p500, ACTIVITY_READ_TELEMETRY);
    assert(act2 == false);
    printf("   ✓ Post-quarantine action blocked successfully.\n\n");

    printf("=============================================================\n");
    printf("PEER ACTIVITY RULES AND TAP ACCUMULATOR TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
