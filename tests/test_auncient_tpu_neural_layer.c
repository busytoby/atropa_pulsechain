#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "auncient_sdk.h"

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TPU NEURAL LAYER SUITE\n");
    printf("=============================================================\n");

    auncient_transfluxor_registry_t registry;
    auncient_sdk_init_transfluxor_registry(&registry);

    // 1. Compile two input waves representing parallel activations
    auncient_transfluxor_word_t inputs[2];
    
    bool ok = auncient_sdk_compile_transfluxor_word(&inputs[0], "NODE_IN_A", 10, 300.0, 600.0, 0.4, 0x00, 0x00);
    assert(ok == true);
    
    ok = auncient_sdk_compile_transfluxor_word(&inputs[1], "NODE_IN_B", 11, 300.0, 600.0, 0.4, 0x00, 0x00);
    assert(ok == true);

    // 2. Execute TPU analog neural layer
    printf("[TEST] Running inputs through analog TPU neural layer...\n");
    auncient_transfluxor_word_t output_wave;
    
    ok = auncient_sdk_tpu_execute_layer(&registry, inputs, 2, &output_wave);
    assert(ok == true);

    // Verify non-linear activation threshold triggers correct activated state
    // Sum F1 = 600Hz (> 500Hz) -> activated to 900Hz
    // Sum F2 = 1200Hz (> 1000Hz) -> activated to 1800Hz
    assert(output_wave.f1 == 900.0);
    assert(output_wave.f2 == 1800.0);
    assert(output_wave.decay == 0.4);
    printf("   ✓ Non-linear activation triggered successfully: F1=%.1fHz, F2=%.1fHz.\n", 
           output_wave.f1, output_wave.f2);

    // 3. Register and dispatch the output word
    ok = auncient_sdk_register_transfluxor_word(&registry, &output_wave);
    assert(ok == true);

    double feedback_freq = 0.0;
    bool resolved = auncient_sdk_dispatch_transfluxor_word(&registry, &output_wave, &feedback_freq);
    assert(resolved == true);
    assert(feedback_freq == 523.25); // Success tone
    printf("   ✓ Output word successfully registered and dispatched.\n");

    printf("=============================================================\n");
    printf("TPU NEURAL LAYER TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}
