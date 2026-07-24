#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
    LABEL_LOW,
    LABEL_HIGH
} autodin_clearance_t;

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Simulated AUTODIN Ingestion Frame
typedef struct {
    autodin_clearance_t security_level;
    uint32_t payload_checksum;
    bool is_declassified_by_gatekeeper; // Authorized downgrade flag
} autodin_frame_t;

// Simulated AUTODIN Secure Router
typedef struct {
    gate_state_t routing_gate;
    autodin_clearance_t target_channel_clearance;
} autodin_router_t;

// -------------------------------------------------------------
// AUTODIN Information Flow Router (SIGPLAN security checks)
// -------------------------------------------------------------
bool route_autodin_frame(autodin_router_t *router, const autodin_frame_t *frame) {
    router->routing_gate = CUTOFF_STATE;

    // Safety rule: Frame clearance must be <= Target channel clearance,
    // unless the frame has been explicitly declassified by the authorized gatekeeper.
    if (frame->security_level > router->target_channel_clearance) {
        if (!frame->is_declassified_by_gatekeeper) {
            return false; // Trapped! Information flow leak prevented.
        }
    }

    router->routing_gate = CONDUC_STATE;
    return true; // Safe to route
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT AUTODIN SIGPLAN IFC ROUTER VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    autodin_router_t router = {
        .routing_gate = CUTOFF_STATE,
        .target_channel_clearance = LABEL_LOW // Public outbound channel
    };

    // 1. Compliant Case: Low clearance frame entering Low channel -> Should pass
    autodin_frame_t low_frame = {
        .security_level = LABEL_LOW,
        .payload_checksum = 0xAAAA,
        .is_declassified_by_gatekeeper = false
    };

    printf("[TEST] Routing low clearance frame to public channel...\n");
    fflush(stdout);
    bool ok = route_autodin_frame(&router, &low_frame);
    assert(ok == true);
    assert(router.routing_gate == CONDUC_STATE);
    printf("   ✓ Frame routed. Conduction open.\n");
    fflush(stdout);

    // 2. Unsafe Case: High clearance frame entering Low channel -> Should trap
    autodin_frame_t secret_frame = {
        .security_level = LABEL_HIGH,
        .payload_checksum = 0xBBBB,
        .is_declassified_by_gatekeeper = false
    };

    printf("[TEST] Routing high clearance frame to public channel...\n");
    fflush(stdout);
    ok = route_autodin_frame(&router, &secret_frame);
    assert(ok == false);
    assert(router.routing_gate == CUTOFF_STATE);
    printf("   ✓ High-to-low leakage violation trapped successfully.\n");
    fflush(stdout);

    // 3. Declassified Case: High clearance frame downgraded by Gatekeeper -> Should pass
    autodin_frame_t downgrade_frame = {
        .security_level = LABEL_HIGH,
        .payload_checksum = 0xCCCC,
        .is_declassified_by_gatekeeper = true // Explicitly downgraded
    };

    printf("[TEST] Routing declassified secret frame to public channel...\n");
    fflush(stdout);
    ok = route_autodin_frame(&router, &downgrade_frame);
    assert(ok == true);
    assert(router.routing_gate == CONDUC_STATE);
    printf("   ✓ Declassified frame routed successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("AUTODIN SIGPLAN IFC TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
