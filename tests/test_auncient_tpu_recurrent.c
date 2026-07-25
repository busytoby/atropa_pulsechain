#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "auncient_sdk.h"

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TPU RECURRENT STATE SUITE\n");
    printf("=============================================================\n");

    auncient_transfluxor_registry_t registry;
    auncient_sdk_init_transfluxor_registry(&registry);

    // 1. Initialize input word and initial state wave
    auncient_transfluxor_word_t input;
    bool ok = auncient_sdk_compile_transfluxor_word(&input, "INPUT_T1", 20, 400.0, 800.0, 0.4, 0x00, 0x00);
    assert(ok == true);

    auncient_transfluxor_word_t state;
    ok = auncient_sdk_compile_transfluxor_word(&state, "STATE_T0", 888, 100.0, 200.0, 0.2, 0x00, 0x00);
    assert(ok == true);

    // 2. Execute Step 1
    printf("[TEST] Running recurrent step 1...\n");
    ok = auncient_sdk_tpu_execute_recurrent_step(&registry, &input, &state);
    assert(ok == true);

    // Expected next F1: (400 * 0.7) + (100 * 0.3) = 310.0Hz
    // Expected next F2: (800 * 0.7) + (200 * 0.3) = 620.0Hz
    assert(fabs(state.f1 - 310.0) < 0.1);
    assert(fabs(state.f2 - 620.0) < 0.1);
    printf("   ✓ Step 1 output state verified: F1=%.1fHz, F2=%.1fHz\n", state.f1, state.f2);

    // 3. Execute Step 2 (feeding output back in with same input)
    printf("[TEST] Running recurrent step 2...\n");
    ok = auncient_sdk_tpu_execute_recurrent_step(&registry, &input, &state);
    assert(ok == true);

    // Expected next F1: (400 * 0.7) + (310 * 0.3) = 280 + 93 = 373.0Hz
    // Expected next F2: (800 * 0.7) + (620 * 0.3) = 560 + 186 = 746.0Hz
    assert(fabs(state.f1 - 373.0) < 0.1);
    assert(fabs(state.f2 - 746.0) < 0.1);
    printf("   ✓ Step 2 output state verified: F1=%.1fHz, F2=%.1fHz\n", state.f1, state.f2);

    printf("=============================================================\n");
    printf("TPU RECURRENT STATE TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}
