#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

typedef struct {
    char header[5]; // Must be "ADIN" for valid structure
    uint32_t payload_id;
    bool pre_txn_schema_ok;  // Pre-transaction schema validation status
    bool per_txn_format_ok;  // Per-transaction frame layout validation status
    bool post_txn_hash_ok;   // Post-transaction commitment validation status
} autodin_profile_t;

// Simulated Wünsche channel transmission packet
typedef struct {
    double probabilities[4]; // Probability distribution of signal states (must sum to 1.0)
    autodin_profile_t profile;
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

    // Better Defined AUTODIN Gating (Pre, Per, and Post Transaction Basis):
    // If the message contains a structured, valid AUTODIN information profile at all three phases,
    // its layout is fully deterministic, meaning its entropy is resolved to 0.0,
    // preventing Wünsche entropy violations entirely.
    if (pkt->profile.pre_txn_schema_ok &&
        pkt->profile.per_txn_format_ok &&
        pkt->profile.post_txn_hash_ok &&
        strcmp(pkt->profile.header, "ADIN") == 0) {
        pkt->conduction_gate = CONDUC_STATE;
        return true;
    }

    // Fallback: If unstructured, calculate raw signal entropy
    double H = calculate_wunsche_entropy(pkt);
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
        .profile = { .pre_txn_schema_ok = false, .per_txn_format_ok = false, .post_txn_hash_ok = false },
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
        .profile = { .pre_txn_schema_ok = false, .per_txn_format_ok = false, .post_txn_hash_ok = false },
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

    // 3. AUTODIN Gating Case: Noisy probabilities but valid pre, per, and post transaction phases -> Should bypass raw entropy check
    wunsche_packet_t pkt_adin = {
        .probabilities = { 0.25, 0.25, 0.25, 0.25 }, // Noisy probabilities
        .profile = {
            .header = "ADIN",
            .payload_id = 42,
            .pre_txn_schema_ok = true,
            .per_txn_format_ok = true,
            .post_txn_hash_ok = true
        },
        .conduction_gate = CUTOFF_STATE
    };

    printf("[TEST] Checking structured three-phase AUTODIN packet (bypasses raw entropy check)...\n");
    fflush(stdout);
    ok = evaluate_wunsche_gate(&pkt_adin, 1.5);
    assert(ok == true);
    assert(pkt_adin.conduction_gate == CONDUC_STATE);
    printf("   ✓ Structured AUTODIN packet bypasses raw entropy checks. Conduction open.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("WUNSCHE ENTROPY TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
