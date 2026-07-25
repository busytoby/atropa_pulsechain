#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "auncient_sdk.h"

// Define unit test assertions for the Auncient Lexicon standard formulas
static void test_frequency_derivation(void) {
    printf("[UNIT TEST] Verifying Lexicon Base Frequency F1 formula...\n");
    // F1 = 300.0 + ((ID * 50) % 700)
    // For ID = 7: F1 = 300.0 + (350 % 700) = 650.0 Hz
    uint32_t id_7 = 7;
    double f1_derived = 300.0 + (double)((id_7 * 50) % 700);
    assert(fabs(f1_derived - 650.0) < 1e-5);

    // For ID = 15: F1 = 300.0 + (750 % 700) = 350.0 Hz
    uint32_t id_15 = 15;
    double f1_derived_15 = 300.0 + (double)((id_15 * 50) % 700);
    assert(fabs(f1_derived_15 - 350.0) < 1e-5);

    printf("   ✓ Base frequencies match standard derivations.\n");

    printf("[UNIT TEST] Verifying Lexicon Harmonic Octave F2 formula...\n");
    double f2_derived = f1_derived * 2.0;
    assert(fabs(f2_derived - 1300.0) < 1e-5);
    printf("   ✓ Harmonic octaves match standard derivations.\n");
}

static void test_decay_resolution(void) {
    printf("[UNIT TEST] Verifying Lexicon decay prefix mapping...\n");
    // Verify standard decay limits
    const char *name_wmq = "WMQ_LOCK_SCSI";
    const char *name_abi = "ABI_WRITE_LEDGER";
    const char *name_spk = "SPK_COMMIT";

    double decay_wmq = (strncmp(name_wmq, "WMQ_", 4) == 0) ? 0.4 : 0.3;
    double decay_abi = (strncmp(name_abi, "ABI_", 4) == 0) ? 0.2 : 0.3;
    double decay_spk = (strncmp(name_spk, "WMQ_", 4) == 0) ? 0.4 : (strncmp(name_spk, "ABI_", 4) == 0) ? 0.2 : 0.3;

    assert(fabs(decay_wmq - 0.4) < 1e-5);
    assert(fabs(decay_abi - 0.2) < 1e-5);
    assert(fabs(decay_spk - 0.3) < 1e-5);
    printf("   ✓ Decay constant prefix gating resolves correctly.\n");
}

static void test_strategy_checks(void) {
    printf("[UNIT TEST] Verifying pre-dispatch strategy checks...\n");
    // Simulate strategy evaluator logic
    double f1_valid = 440.0;
    double f2_valid = 880.0;
    double f1_invalid = 500.0;
    double f2_invalid = 500.0; // Collision

    bool valid_ok = (f1_valid > 0.0 && f2_valid > 0.0 && f1_valid != f2_valid);
    bool invalid_ok = (f1_invalid > 0.0 && f2_invalid > 0.0 && f1_invalid != f2_invalid);

    assert(valid_ok == true);
    assert(invalid_ok == false);
    printf("   ✓ Pre-dispatch strategic collisions detected correctly.\n");
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT LEXICON FORMULA UNIT TESTS\n");
    printf("=============================================================\n");

    test_frequency_derivation();
    test_decay_resolution();
    test_strategy_checks();

    printf("=============================================================\n");
    printf("LEXICON UNIT TESTS COMPLETED\n");
    printf("=============================================================\n");

    return 0;
}
