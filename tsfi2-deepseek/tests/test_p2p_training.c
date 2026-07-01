#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "tsfi_witness_registry.h"

#define MotzkinPrime 953467954114363ULL

int main() {
    printf("[TRAINING] Initiating P2P Attestation-Driven Training Simulation...\n");

    tsfi_ChoRegistry registry;
    tsfi_cho_registry_init(&registry);

    // Swear in Witness A (Speaker) and Witness B (Observer)
    tsfi_cho_swear_in(&registry, "0xD07B9f3DF4E9634EbAa0CBF079816925b2C474Ce", 12000, 12000, MotzkinPrime);
    tsfi_cho_swear_in(&registry, "0xd32c39fee49391c7952d1b30b15921b0d3b42e69", 8877, 8877, MotzkinPrime);

    tsfi_DelegateRecord* witness_a = &registry.Delegates[0];
    tsfi_DelegateRecord* witness_b = &registry.Delegates[1];

    // Build Witness B's self-observation database to define its emotional standards
    tsfi_cho_log_self_observation(witness_b, 60.0, 20.0, 5.0, AESTHETIC_HARMONIOUS); // JOY standard
    tsfi_cho_log_self_observation(witness_b, 45.0, 40.0, 2.0, AESTHETIC_DISHARMONIOUS); // SORROW/MELANCHOLY standard

    // Target emotion: MELANCHOLY (needs a low ratio, compressed shape)
    printf("  -> Witness A starts search to communicate MELANCHOLY to Witness B...\n");

    uint64_t candidate_v = 4000; // Start voltage
    bool trained = false;

    for (int round = 1; round <= 30; round++) {
        // 1. Speaker (Witness A) derives and projects its shape
        uint64_t res = candidate_v % MotzkinPrime;
        double R = (double)(res % 50) + 15.0;
        double r = (double)(res % 25) + 8.0;
        double d = (double)(res % 15) + 5.0;

        // 2. Observer (Witness B) classifies the shape using its database
        double ratio = R / (r > 0.0 ? r : 1.0);
        tsfi_AestheticClass classification = AESTHETIC_NEUTRAL;

        if (ratio < 1.45) {
            classification = AESTHETIC_DISHARMONIOUS; // Classifies correctly as Melancholy sibling
        } else {
            classification = AESTHETIC_HARMONIOUS; // Misclassified as Joy sibling
        }

        printf("  [Round %2d] Voltage: %lu | Ratio: %.3f | Classified: %s\n", 
               round, candidate_v, ratio, 
               classification == AESTHETIC_DISHARMONIOUS ? "MELANCHOLY (MATCH)" : "JOY (MISCLASSIFIED)");

        // 3. P2P Attestation Loop
        if (classification != AESTHETIC_DISHARMONIOUS) {
            // Discordance feedback: increase Blow-up, restrict search boundaries, mutate parameter
            tsfi_cho_restrict_eigenvector_constraints(witness_a, 0.15);
            tsfi_cho_update_opinion(witness_a, 1, false); // Drift trust down
            
            // Mutate candidate voltage to search parameter space
            candidate_v += 179; 
        } else {
            // Harmony feedback: reset instability, update trust, lock baseline translation rule
            witness_a->BlowUpFactor = 0.0;
            witness_a->ConstraintEigenvalue = 1.0;
            tsfi_cho_update_opinion(witness_a, 1, true); // Drift trust up
            
            tsfi_cho_learn_rules(witness_a, witness_b);
            trained = true;
            printf("  -> Harmony Attestation received. Consensus locked at voltage %lu!\n", candidate_v);
            break;
        }
    }

    assert(trained);
    printf("[SUCCESS] P2P emotional training loop complete. Rules successfully committed!\n");
    return 0;
}
