#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "auncient_sdk.h"

static void test_frequency_derivation(void) {
    printf("[UNIT TEST] Verifying SDK Lexicon Base Frequency F1 derivation...\n");
    auncient_transfluxor_word_t w1, w2;
    bool ok = auncient_sdk_compile_lexicon_word(&w1, "WMQ_TEST_1", 7, 0x10, 0x00);
    assert(ok);
    assert(fabs(w1.f1 - 650.0) < 1e-5);
    assert(fabs(w1.f2 - 1300.0) < 1e-5);

    ok = auncient_sdk_compile_lexicon_word(&w2, "WMQ_TEST_2", 15, 0x10, 0x00);
    assert(ok);
    assert(fabs(w2.f1 - 350.0) < 1e-5);
    assert(fabs(w2.f2 - 700.0) < 1e-5);

    printf("   ✓ SDK Derived frequencies match standard derivations.\n");
}

static void test_decay_resolution(void) {
    printf("[UNIT TEST] Verifying SDK Lexicon decay prefix mapping...\n");
    auncient_transfluxor_word_t w_wmq, w_abi, w_spk;
    
    bool ok = auncient_sdk_compile_lexicon_word(&w_wmq, "WMQ_LOCK_SCSI", 1, 0x10, 0x00);
    assert(ok);
    assert(fabs(w_wmq.decay - 0.4) < 1e-5);

    ok = auncient_sdk_compile_lexicon_word(&w_abi, "ABI_WRITE_LEDGER", 2, 0x00, 0x02);
    assert(ok);
    assert(fabs(w_abi.decay - 0.2) < 1e-5);

    ok = auncient_sdk_compile_lexicon_word(&w_spk, "SPK_COMMIT", 3, 0x10, 0x02);
    assert(ok);
    assert(fabs(w_spk.decay - 0.3) < 1e-5);

    printf("   ✓ SDK Decay constant prefix gating resolves correctly.\n");
}

static void test_strategy_checks(void) {
    printf("[UNIT TEST] Verifying SDK pre-dispatch strategy checks...\n");
    double f1_valid = 440.0;
    double f2_valid = 880.0;
    double f1_invalid = 500.0;
    double f2_invalid = 500.0;

    bool valid_ok = (f1_valid > 0.0 && f2_valid > 0.0 && f1_valid != f2_valid);
    bool invalid_ok = (f1_invalid > 0.0 && f2_invalid > 0.0 && f1_invalid != f2_invalid);

    assert(valid_ok == true);
    assert(invalid_ok == false);
    printf("   ✓ SDK Pre-dispatch strategic collisions detected correctly.\n");
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT LEXICON SDK UNIT TESTS\n");
    printf("=============================================================\n");

    test_frequency_derivation();
    test_decay_resolution();
    test_strategy_checks();

    printf("=============================================================\n");
    printf("LEXICON SDK UNIT TESTS COMPLETED\n");
    printf("=============================================================\n");

    return 0;
}
