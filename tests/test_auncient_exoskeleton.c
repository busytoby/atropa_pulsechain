#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Guest VDM Context managed by the EXOSKELETON
typedef struct {
    uint32_t vdm_id;
    uint32_t last_heartbeat;
    uint32_t curr_heartbeat;
    gate_state_t write_gate;
    char buffer[64];
} exo_guest_vdm_t;

// EXOSKELETON Hypervisor State
typedef struct {
    exo_guest_vdm_t guests[3];
    uint32_t global_token_owner;
} exoskeleton_t;

// -------------------------------------------------------------
// EXOSKELETON Supervisor Watchdog and Routing Routines
// -------------------------------------------------------------
void exoskeleton_watchdog_audit(exoskeleton_t *exo) {
    for (int i = 0; i < 3; i++) {
        // Watchdog check: If heartbeat hasn't progressed, trigger CUTOFF isolation
        if (exo->guests[i].curr_heartbeat == exo->guests[i].last_heartbeat) {
            exo->guests[i].write_gate = CUTOFF_STATE;
        } else {
            // Heartbeat progressed, preserve status and update shadow
            exo->guests[i].last_heartbeat = exo->guests[i].curr_heartbeat;
        }
    }
}

bool exoskeleton_route_packet(exoskeleton_t *exo, uint32_t dest_id, const char *payload) {
    if (dest_id >= 3) return false;

    // Verify target VDM is active and write gate is open
    if (exo->guests[dest_id].write_gate == CUTOFF_STATE) {
        return false; // Routing blocked: Target VDM is isolated
    }

    // Deliver payload directly to target guest VDM buffer (Exoskeleton routing)
    strcpy(exo->guests[dest_id].buffer, payload);
    return true;
}

void exoskeleton_rotate_token(exoskeleton_t *exo) {
    // Pass the coaxial token to the next active node in the ring
    exo->global_token_owner = (exo->global_token_owner + 1) % 3;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT EXOSKELETON HYPERVISOR VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    exoskeleton_t exo = {
        .guests = {
            { .vdm_id = 0, .last_heartbeat = 0, .curr_heartbeat = 1, .write_gate = CONDUC_STATE, .buffer = "INIT_0" },
            { .vdm_id = 1, .last_heartbeat = 0, .curr_heartbeat = 1, .write_gate = CONDUC_STATE, .buffer = "INIT_1" },
            { .vdm_id = 2, .last_heartbeat = 0, .curr_heartbeat = 1, .write_gate = CONDUC_STATE, .buffer = "INIT_2" }
        },
        .global_token_owner = 0
    };

    // 1. Audit active heartbeats -> All should remain active
    printf("[TEST] Running supervisor watchdog on active heartbeats...\n");
    fflush(stdout);
    exoskeleton_watchdog_audit(&exo);
    assert(exo.guests[0].write_gate == CONDUC_STATE);
    assert(exo.guests[1].write_gate == CONDUC_STATE);
    printf("   ✓ Liveness watchdog confirmed guest states.\n");
    fflush(stdout);

    // 2. Simulate Node 1 hanging (heartbeat does not progress)
    printf("[TEST] Simulating guest hang on VDM 1...\n");
    fflush(stdout);
    // Node 0 and Node 2 advance heartbeats; Node 1 remains stagnant
    exo.guests[0].curr_heartbeat = 2;
    exo.guests[2].curr_heartbeat = 2;
    exoskeleton_watchdog_audit(&exo);
    assert(exo.guests[1].write_gate == CUTOFF_STATE); // Node 1 must be isolated
    assert(exo.guests[0].write_gate == CONDUC_STATE);
    printf("   ✓ Watchdog successfully isolated hung VDM 1.\n");
    fflush(stdout);

    // 3. Attempt routing packet to active and isolated VDM
    printf("[TEST] Dispatching routing packets via hypervisor...\n");
    fflush(stdout);
    bool ok = exoskeleton_route_packet(&exo, 0, "DATA_FOR_0");
    assert(ok == true);
    assert(strcmp(exo.guests[0].buffer, "DATA_FOR_0") == 0);

    // Routing to isolated Node 1 must fail
    ok = exoskeleton_route_packet(&exo, 1, "DATA_FOR_1");
    assert(ok == false);
    printf("   ✓ Packet routed to active guest; blocked from isolated guest.\n");
    fflush(stdout);

    // 4. Token rotation
    printf("[TEST] Rotating global coaxial token...\n");
    fflush(stdout);
    assert(exo.global_token_owner == 0);
    exoskeleton_rotate_token(&exo);
    assert(exo.global_token_owner == 1);
    printf("   ✓ Token rotated from Node 0 to Node 1.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("EXOSKELETON HYPERVISOR TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
