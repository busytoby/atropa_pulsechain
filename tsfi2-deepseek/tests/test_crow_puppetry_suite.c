#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_nand_trap_firmware.h"
#include "tsfi_wavelet_arena.h"
#include "tsfi_puppetry.h"
#include "lau_memory.h"
#include "lau_registry.h"
#include "tsfi_vision.h"

// Mock definitions for testing
#define PASS 0
#define FAIL 1

int test_genie_crow_classification() {
    printf("[TEST] Genie Semantic Proof (Classification)...\n");
    
    // Simulate a vision analysis that matches the crow profile
    TSFiResonanceAnalysis analysis = {0};
    analysis.captured_radial_profile[0] = 0.5f;
    analysis.captured_radial_profile[1] = 0.8f;
    analysis.captured_radial_profile[2] = 0.2f;
    analysis.captured_radial_profile[3] = 0.2f;
    analysis.captured_radial_profile[4] = 0.5f;
    analysis.captured_radial_profile[5] = 0.8f;
    analysis.captured_radial_profile[6] = 0.2f;
    analysis.captured_radial_profile[7] = 0.2f;
    
    // Crows are somewhat asymmetrical and highly complex
    analysis.glyph_symmetry = 0.4f;
    analysis.complexity = 0.8f;
    analysis.radial_score = 0.9f;

    LauClassificationAsset crow_class;
    if (!tsfi_classification_load("crow", &crow_class)) {
        printf("  [ERROR] Could not load crow.class\n");
        return FAIL;
    }

    TSFiClassification result = tsfi_vision_classify_asset(&analysis, &crow_class);
    
    if (result.class_id != TSFI_CLASS_CROW) {
        printf("  [FAIL] Expected class TSFI_CLASS_CROW, got %d\n", result.class_id);
        return FAIL;
    }
    if (result.confidence <= 0.5f) {
        printf("  [FAIL] Expected confidence > 0.5f, got %.3f\n", result.confidence);
        return FAIL;
    }

    printf("  [PASS] Genie successfully classified the Crow Apparition (Confidence: %.3f).\n", result.confidence);
    return PASS;
}

int test_spider_kinematic_injection() {
    printf("[TEST] Spider Kinematic Proof (XOR Injection)...\n");

    uint8_t *mem = malloc(1024 * 1024);
    TsfiWaveletArena arena;
    tsfi_wavelet_arena_init(&arena, mem, 1024 * 1024);
    TsfiWavelet *W = tsfi_STAT(&arena, 953473ULL);

    TsfiWavelet G, D;
    memset(&G, 0, sizeof(TsfiWavelet));
    memset(&D, 0, sizeof(TsfiWavelet));
    G.telemetry.Fa.identity = 0x11;
    D.telemetry.Fa.identity = 0x22;
    uint8_t *mock_manifold = calloc(1, 1024 * 1024);

    // Establish PUPPET trait
    tsfi_wavelet_xor_signal(mock_manifold, W, &G, &D, (const uint8_t *)"PUPPET:CROW_00");

    // Inject Kinematic Payload (Purple Wing Flap: 128, 0, 128)
    uint8_t crow_state[120];
    memset(crow_state, 0, 120);
    crow_state[0] = 128;
    crow_state[1] = 0;
    crow_state[2] = 128;
    tsfi_wavelet_xor_signal(mock_manifold, W, &G, &D, crow_state);

    int status = PASS;
    if (W->telemetry.trait != TSFI_TRAIT_PUPPET) {
        printf("  [FAIL] Trait not established as PUPPET.\n");
        status = FAIL;
    }
    if (W->payload[0] != 128 || W->payload[2] != 128) {
        printf("  [FAIL] Kinematic payload not properly injected.\n");
        status = FAIL;
    }

    if (status == PASS) {
        printf("  [PASS] Spider successfully injected kinematic wing joints into the payload.\n");
    }

    free(mock_manifold);
    free(mem);
    return status;
}

int test_acoustic_supercedence() {
    printf("[TEST] Acoustic-to-Visual Supercedence (Witness Consolidation)...\n");

    uint8_t *mem = malloc(1024 * 1024);
    TsfiWaveletArena arena;
    tsfi_wavelet_arena_init(&arena, mem, 1024 * 1024);
    TsfiWavelet *W = tsfi_STAT(&arena, 953473ULL);

    // Lock into acoustic state
    tsfi_Avail(W, 400, W->Prime);
    W->telemetry.Fa.monopole = 0xAC00571C;
    W->state = WAVELET_STATE_MAGNETIZE;
    W->telemetry.current_seal_level = 8;

    TsfiWavelet G, D;
    memset(&G, 0, sizeof(TsfiWavelet));
    memset(&D, 0, sizeof(TsfiWavelet));
    G.telemetry.Fa.identity = 0xAA;
    D.telemetry.Fa.identity = 0xBB;
    uint8_t *mock_manifold = calloc(1, 1024 * 1024);

    // Spider upgrades
    tsfi_wavelet_xor_signal(mock_manifold, W, &G, &D, (const uint8_t *)"PUPPET:CROW_01");
    uint8_t crow_state[120];
    memset(crow_state, 0xCC, 120);
    tsfi_wavelet_xor_signal(mock_manifold, W, &G, &D, crow_state);

    // Consolidate
    TSFiVisualWitness witness;
    tsfi_witness_establish(&witness, W, "A crow perches silently.", 0.85f, TSFI_CLASS_CROW);

    int status = PASS;
    if (witness.monopole_identity != 0xAC00571C) {
        printf("  [FAIL] Acoustic monopole identity lost during supercedence.\n");
        status = FAIL;
    }
    if (witness.classification_id != TSFI_CLASS_CROW) {
        printf("  [FAIL] Genie classification lost.\n");
        status = FAIL;
    }
    if (!witness.xor_solidified) {
        printf("  [FAIL] Spider state not solidified.\n");
        status = FAIL;
    }

    if (status == PASS) {
        printf("  [PASS] Apparition successfully witnessed upon the acoustic location.\n");
    }

    free(mock_manifold);
    free(mem);
    return status;
}

int main() {
    printf("=== TSFi Crow Apparition Unit Tests ===\n");
    int failures = 0;
    
    failures += test_genie_crow_classification();
    failures += test_spider_kinematic_injection();
    failures += test_acoustic_supercedence();

    printf("=======================================\n");
    if (failures == 0) {
        printf("STATUS: ALL TESTS PASSED. RIGID.\n");
        return 0;
    } else {
        printf("STATUS: %d TESTS FAILED. FRACTURE DETECTED.\n", failures);
        return 1;
    }
}
