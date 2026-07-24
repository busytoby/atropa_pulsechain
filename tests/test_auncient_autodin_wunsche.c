#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Secure Core System (Insulated Domain)
typedef struct {
    uint32_t active_register_val;
    bool alarm_flagged;
} secure_core_t;

// Incoming packet from external untrusted channel
typedef struct {
    double probabilities[4];
    uint32_t payload_val;
} external_packet_t;

// Helper to compute Shannon entropy
static double compute_shannon_entropy(const external_packet_t *pkt) {
    double entropy = 0.0;
    for (int i = 0; i < 4; i++) {
        double p = pkt->probabilities[i];
        if (p > 0.0) {
            entropy -= p * (log(p) / log(2.0));
        }
    }
    return entropy;
}

// -------------------------------------------------------------
// AUTODIN Insulation Gating
// -------------------------------------------------------------
bool process_autodin_insulated_input(secure_core_t *core, const external_packet_t *pkt, double max_entropy_limit) {
    double H = compute_shannon_entropy(pkt);

    // If signal entropy exceeds limit, it implies a Wünsche Byzantine attack.
    // Cut off conduction immediately to insulate the secure core.
    if (H > max_entropy_limit) {
        core->alarm_flagged = true;
        return false; // Connection cutoff, core insulated
    }

    // Normal safe conduction path: Update core values
    core->active_register_val = pkt->payload_val;
    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT AUTODIN WUNSCHE INSULATION VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    secure_core_t core = {
        .active_register_val = 500,
        .alarm_flagged = false
    };

    // 1. Compliant Case: Stable input (Low entropy: 0.811) -> Should update core
    external_packet_t pkt_stable = {
        .probabilities = { 0.8, 0.2, 0.0, 0.0 },
        .payload_val = 600
    };

    printf("[TEST] Routing stable low-entropy external packet (threshold = 1.0)...\n");
    fflush(stdout);
    bool ok = process_autodin_insulated_input(&core, &pkt_stable, 1.0);
    assert(ok == true);
    assert(core.active_register_val == 600);
    assert(core.alarm_flagged == false);
    printf("   ✓ Core updated successfully.\n");
    fflush(stdout);

    // 2. Attack Case: Noisy/Byzantine input (High entropy: 2.0) -> Should block and insulate core
    external_packet_t pkt_attack = {
        .probabilities = { 0.25, 0.25, 0.25, 0.25 }, // High entropy noise
        .payload_val = 9999 // Malicious payload attempt
    };

    printf("[TEST] Routing high-entropy attack packet (threshold = 1.0)...\n");
    fflush(stdout);
    ok = process_autodin_insulated_input(&core, &pkt_attack, 1.0);
    assert(ok == false);
    assert(core.active_register_val == 600); // Core remains insulated (unchanged)
    assert(core.alarm_flagged == true);     // Security alarm triggered
    printf("   ✓ Attack blocked. Secure core insulated successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("AUTODIN WUNSCHE INSULATION TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
