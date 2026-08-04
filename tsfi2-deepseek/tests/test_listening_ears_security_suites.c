#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#define MAX_NODES 32

typedef struct {
    int id;
    double x;
    double y;
    double phase_offset;
    bool active;
} TestNode;

static TestNode g_network[MAX_NODES];
static int g_network_size = 0;

// Registers a node, enforcing a Sybil detector (reject duplicate coordinates)
static bool register_node_secure(int id, double x, double y, double phase) {
    for (int i = 0; i < g_network_size; i++) {
        if (g_network[i].active && fabs(g_network[i].x - x) < 1e-4 && fabs(g_network[i].y - y) < 1e-4) {
            // Sybil/Duplicate Coordinate detected
            return false;
        }
    }
    if (g_network_size < MAX_NODES) {
        g_network[g_network_size].id = id;
        g_network[g_network_size].x = x;
        g_network[g_network_size].y = y;
        g_network[g_network_size].phase_offset = phase;
        g_network[g_network_size].active = true;
        g_network_size++;
        return true;
    }
    return false;
}

// Runs validation of pilot phase offset to isolate spoofers
static int validate_and_isolate_pilot_spoofers() {
    int isolated_count = 0;
    for (int i = 0; i < g_network_size; i++) {
        if (g_network[i].active) {
            // Normal variance check: phase offsets should be small/calibrated
            // Spoofed offset exceeding 1.0 radian is isolated
            if (fabs(g_network[i].phase_offset) > 1.0) {
                g_network[i].active = false; // Isolate
                isolated_count++;
            }
        }
    }
    return isolated_count;
}

int main() {
    printf("=== STANAG Listening Ears Security Suites ===\n");

    // ==========================================
    // 1. ADVERSARIAL RED BOX TEST SUITE
    // ==========================================
    printf("\n[RED_BOX] Commencing Adversarial Suite...\n");
    g_network_size = 0;

    // Normal registration
    bool ok = register_node_secure(0, 0.0, 0.0, 0.0);
    assert(ok);
    ok = register_node_secure(1, 10.0, 10.0, 0.1);
    assert(ok);

    // Test Case A: Sybil Clone Registration Attack (Same location)
    printf("  [TEST] Injecting Sybil registration attempt...\n");
    bool sybil_ok = register_node_secure(99, 10.0, 10.0, 0.0); // Attacker tries to clone Node 1 coordinates
    assert(!sybil_ok); // Must be rejected
    printf("  [PASS] Sybil coordinate registration successfully blocked.\n");

    // Test Case B: Coordinated Pilot Phase Spoofing
    printf("  [TEST] Injecting pilot phase spoofing attack on Node 1...\n");
    g_network[1].phase_offset = 3.14159; // Large corrupted phase shift
    int isolated = validate_and_isolate_pilot_spoofers();
    assert(isolated == 1);
    assert(!g_network[1].active); // Node 1 must be isolated
    printf("  [PASS] Spoofed pilot calibration channel successfully isolated.\n");

    // ==========================================
    // 2. FRIENDLY YELLOW BOX TEST SUITE
    // ==========================================
    printf("\n[YELLOW_BOX] Commencing Friendly Suite...\n");
    g_network_size = 0;
    register_node_secure(0, 0.0, 0.0, 0.0);
    register_node_secure(1, 10.0, 0.0, 0.05);
    register_node_secure(2, 0.0, 10.0, -0.02);
    register_node_secure(3, 10.0, 10.0, 0.03);

    // Test Case A: Dynamic Sensor Node Dropout
    printf("  [TEST] Simulating dynamic battery dropout on Node 2...\n");
    g_network[2].active = false; // Spontaneous dropout
    int active_count = 0;
    for (int i = 0; i < g_network_size; i++) {
        if (g_network[i].active) active_count++;
    }
    assert(active_count == 3);
    printf("  [PASS] Network topology dynamically updated to %d active nodes.\n", active_count);

    // Test Case B: Thermal clock drift recovery threshold
    printf("  [TEST] Simulating thermal drift drift rate...\n");
    g_network[3].phase_offset = 0.5; // High but recoverable drift (within limits)
    int isolated_drift = validate_and_isolate_pilot_spoofers();
    assert(isolated_drift == 0); // Should not be isolated (recoverable)
    assert(g_network[3].active);
    printf("  [PASS] Recoverable clock drift handled without service interruption.\n");

    printf("\n=== ALL RED BOX & YELLOW BOX SECURITY SUITE TESTS PASSED ===\n");
    return 0;
}
