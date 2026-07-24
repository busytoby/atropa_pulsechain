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

// Simulated Wünsche channel transmission packet
typedef struct {
    double probabilities[4]; // Probability distribution of signal states (must sum to 1.0)
    gate_state_t conduction_gate;
} wunsche_packet_t;

// -------------------------------------------------------------
// Wünsche Information Entropy Gating Rule
// -------------------------------------------------------------
double calculate_wunsche_entropy(const wunsche_packet_t *pkt) {
    double entropy = 0.0;
    for (int i = 0; i < 4; i++) {
        double p = pkt->probabilities[i];
        if (p > 0.0) {
            entropy -= p * (log(p) / log(2.0)); // Base-2 logarithm for shannon entropy
        }
    }
    return entropy;
}

bool evaluate_wunsche_gate(wunsche_packet_t *pkt, double max_entropy_threshold) {
    pkt->conduction_gate = CUTOFF_STATE;
    double H = calculate_wunsche_entropy(pkt);

    // If signal entropy is within stable limits, open the conduction gate
    if (H <= max_entropy_threshold) {
        pkt->conduction_gate = CONDUC_STATE;
        return true;
    }

    return false; // High signal degradation / noise cutoff active
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT WUNSCHE ENTROPY GATING VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Compliant Case: Highly ordered signal (Low entropy: 0.811) -> Should conduct
    wunsche_packet_t pkt_ordered = {
        .probabilities = { 0.8, 0.2, 0.0, 0.0 }, // Clear dominant state
        .conduction_gate = CUTOFF_STATE
    };

    printf("[TEST] Checking low-entropy ordered signal (threshold = 1.5)...\n");
    fflush(stdout);
    bool ok = evaluate_wunsche_gate(&pkt_ordered, 1.5);
    double H = calculate_wunsche_entropy(&pkt_ordered);
    printf("   Calculated Entropy H = %.4f\n", H);
    assert(ok == true);
    assert(pkt_ordered.conduction_gate == CONDUC_STATE);
    printf("   ✓ Stable signal approved. Conduction open.\n");
    fflush(stdout);

    // 2. Reject Case: Noisy/disordered signal (High entropy: 2.0) -> Should cutoff
    wunsche_packet_t pkt_noisy = {
        .probabilities = { 0.25, 0.25, 0.25, 0.25 }, // Uniform noise
        .conduction_gate = CUTOFF_STATE
    };

    printf("[TEST] Checking high-entropy noisy signal (threshold = 1.5)...\n");
    fflush(stdout);
    ok = evaluate_wunsche_gate(&pkt_noisy, 1.5);
    H = calculate_wunsche_entropy(&pkt_noisy);
    printf("   Calculated Entropy H = %.4f\n", H);
    assert(ok == false);
    assert(pkt_noisy.conduction_gate == CUTOFF_STATE);
    printf("   ✓ Degraded signal cutoff trapped successfully. Conduction blocked.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("WUNSCHE ENTROPY TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
