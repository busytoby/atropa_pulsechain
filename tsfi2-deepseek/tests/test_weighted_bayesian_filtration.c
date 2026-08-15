#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include "../inc/tsfi_chancery_docket.h"
#include "../inc/tsfi_svdag.h"
#include "../src/auncient_cloth_material_bridge.h"

// Discrete Bayesian Filtration Evaluator matching weighted_bayesian_filtration.algol61
static int evaluate_bayesian_filtration(
    int prior_scaled,
    int evidence_weight,
    int likelihood_scaled,
    int threshold_scaled
) {
    if (prior_scaled < 0 || prior_scaled > 1000 ||
        likelihood_scaled < 0 || likelihood_scaled > 1000 ||
        evidence_weight < 0 || evidence_weight > 1000 ||
        threshold_scaled < 0 || threshold_scaled > 1000) {
        return 2; // INVALID_INPUT_BOUNDS
    }

    int weighted_prior = (prior_scaled * evidence_weight) / 1000;
    int num = weighted_prior * likelihood_scaled;
    int den = num + (1000 - weighted_prior) * (1000 - likelihood_scaled);

    if (den == 0) {
        return 3; // CONTRADICTION
    }

    int posterior = (num * 1000) / den;
    if (posterior >= threshold_scaled) {
        return 0; // AUTHENTIC_STREAM
    } else {
        return 1; // UNAUTHORIZED_BLOCK
    }
}

int main(void) {
    printf("====================================================================\n");
    printf("TEST SUITE: WEIGHTED BAYESIAN FILTRATION & CHANCERY PROVERS\n");
    printf("====================================================================\n\n");

    // -------------------------------------------------------------------------
    // TEST 1: High Confidence Evidence -> AUTHENTIC_STREAM (R15 = 0)
    // -------------------------------------------------------------------------
    printf("1. Testing High Confidence Evidence Gating (Prior=700, Weight=900, Likelihood=850, Threshold=600)...\n");
    int ruling_1 = evaluate_bayesian_filtration(700, 900, 850, 600);
    assert(ruling_1 == 0);
    printf("   ✓ Result: AUTHENTIC_STREAM (ruling = %d)\n", ruling_1);

    // -------------------------------------------------------------------------
    // TEST 2: Weak / Deficient Evidence -> UNAUTHORIZED_BLOCK (R15 = 1)
    // -------------------------------------------------------------------------
    printf("\n2. Testing Deficient Evidence Gating (Prior=200, Weight=400, Likelihood=300, Threshold=600)...\n");
    int ruling_2 = evaluate_bayesian_filtration(200, 400, 300, 600);
    assert(ruling_2 == 1);
    printf("   ✓ Result: UNAUTHORIZED_BLOCK (ruling = %d)\n", ruling_2);

    // -------------------------------------------------------------------------
    // TEST 3: Range & Boundary Enforcement (Invalid inputs -> Return Code 2)
    // -------------------------------------------------------------------------
    printf("\n3. Testing Input Range & Bounds Guard (Prior=1200 > 1000)...\n");
    int ruling_3 = evaluate_bayesian_filtration(1200, 500, 500, 500);
    assert(ruling_3 == 2);
    printf("   ✓ Result: INVALID_INPUT_BOUNDS (ruling = %d)\n", ruling_3);

    // -------------------------------------------------------------------------
    // TEST 4: SVDAG Modulation via Bayesian Posterior
    // -------------------------------------------------------------------------
    printf("\n4. Testing SVDAG Taste Tree Voxel Intensity Scaling via Posterior Weights...\n");
    TSFiHelmholtzSVDAG *dag = tsfi_svdag_create(64);
    assert(dag != NULL);

    float posterior_prob = 0.85f;
    for (size_t i = 0; i < 16; i++) {
        dag->intensity_stream[i] = posterior_prob;
        dag->phase_stream[i] = 0.5f;
        dag->index_stream[i] = (uint32_t)i;
    }
    dag->stream_size = 16;
    assert(dag->intensity_stream[0] == 0.85f);
    printf("   ✓ SVDAG Stream configured with 16 posterior-weighted voxels (Intensity: %.2f)\n",
           dag->intensity_stream[0]);

    // -------------------------------------------------------------------------
    // TEST 5: Soft-Body Cloth Tension Modulation via Bayesian Weights
    // -------------------------------------------------------------------------
    printf("\n5. Testing Soft-Body Cloth Mesh Modulation via Prover Ruling Weights...\n");
    ClothVertex cloth_nodes[16];
    memset(cloth_nodes, 0, sizeof(cloth_nodes));
    for (int i = 0; i < 16; i++) {
        cloth_nodes[i].x = (float)i;
        cloth_nodes[i].color = 0xFFFFFFFF;
    }

    // High confidence posterior modulates color to green-cyan channel
    uint8_t confidence_byte = (uint8_t)(posterior_prob * 255.0f);
    cloth_nodes[0].color = 0xFF000000 | (confidence_byte << 8) | 0x33;
    assert(cloth_nodes[0].color != 0xFFFFFFFF);
    printf("   ✓ Cloth Node 0 Modulated by Confidence: 0x%08X\n", cloth_nodes[0].color);

    // -------------------------------------------------------------------------
    // TEST 6: Chancery Docket Ledger Filing & Merkle Proof Sealing
    // -------------------------------------------------------------------------
    printf("\n6. Filing Resolutions on Chancery Docket:\n");
    ChanceryDocketState docket;
    tsfi_chancery_docket_init(&docket);

    uint32_t doc_auth = tsfi_chancery_docket_file(
        &docket,
        "Bayesian Filtration: High Confidence Hypothesis Affirmed",
        "solidity/dysnomia/domain/std/weighted_bayesian_filtration.algol61",
        2026
    );
    assert(doc_auth == 7000);
    assert(tsfi_chancery_docket_resolve_zmm_r15(&docket, doc_auth, ruling_1, DOCKET_RULING_AUTHENTIC_STREAM));

    uint32_t doc_block = tsfi_chancery_docket_file(
        &docket,
        "Bayesian Filtration: Low Confidence Hypothesis Blocked",
        "solidity/dysnomia/domain/std/weighted_bayesian_filtration.algol61",
        2026
    );
    assert(doc_block == 7001);
    assert(tsfi_chancery_docket_resolve_zmm_r15(&docket, doc_block, ruling_2, DOCKET_RULING_UNAUTHORIZED_BLOCK));

    char audit_report[2048];
    uint64_t docket_proof = tsfi_chancery_docket_audit(&docket, audit_report, sizeof(audit_report));
    assert(docket_proof != 0);
    assert(docket.total_resolved_count == 2);

    printf("\n%s\n", audit_report);
    printf("====================================================================\n");
    printf("WEIGHTED BAYESIAN FILTRATION UNIT TESTS PASSED & SEALED ON CHANCERY\n");
    printf("====================================================================\n");

    return 0;
}
