#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "auncient_sdk.h"

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TONAL & EMOTIONAL VOCABULARY SUITE\n");
    printf("=============================================================\n");

    auncient_transfluxor_registry_t registry;
    auncient_sdk_init_transfluxor_registry(&registry);

    // 1. Compile URGENT Lock SCSI Word (High Formants, Rapid Decay, ID = 1)
    auncient_transfluxor_word_t urgent_lock;
    bool ok = auncient_sdk_compile_transfluxor_word(
        &urgent_lock,
        "LOCK_SCSI_URGENT",
        1,       // Unique Word ID
        800.0,   // F1 Formant (Sharp)
        1600.0,  // F2 Formant (Sharp)
        0.1,     // Decay (Fast transient)
        0x10,    // WMQ command (Lock SCSI)
        0x00     // ABI op
    );
    assert(ok == true);

    // 2. Compile CAUTIOUS Lock SCSI Word (Low Formants, Slow Decay, ID = 2)
    auncient_transfluxor_word_t cautious_lock;
    ok = auncient_sdk_compile_transfluxor_word(
        &cautious_lock,
        "LOCK_SCSI_CAUTIOUS",
        2,       // Unique Word ID
        300.0,   // F1 Formant (Rounded/Low)
        600.0,   // F2 Formant (Rounded/Low)
        1.5,     // Decay (Slow sustain)
        0x10,    // WMQ command (Lock SCSI)
        0x00     // ABI op
    );
    assert(ok == true);

    // 3. Register both words in the shared transfluxor registry
    printf("[TEST] Registering both tonal variations in the registry...\n");
    ok = auncient_sdk_register_transfluxor_word(&registry, &urgent_lock);
    assert(ok == true);

    ok = auncient_sdk_register_transfluxor_word(&registry, &cautious_lock);
    assert(ok == true); // No collision because of unique word_id
    printf("   ✓ Both variations registered successfully. 0 collisions.\n");

    // 4. Dispatch both variations and check feedback loop
    double feedback_freq = 0.0;
    
    printf("[TEST] Dispatching URGENT Lock command wave...\n");
    bool dispatched = auncient_sdk_dispatch_transfluxor_word(&registry, &urgent_lock, &feedback_freq);
    assert(dispatched == true);
    assert(feedback_freq == 523.25); // Success tone
    printf("   ✓ Urgent variation resolved and executed successfully.\n");

    printf("[TEST] Dispatching CAUTIOUS Lock command wave...\n");
    dispatched = auncient_sdk_dispatch_transfluxor_word(&registry, &cautious_lock, &feedback_freq);
    assert(dispatched == true);
    assert(feedback_freq == 523.25); // Success tone
    printf("   ✓ Cautious variation resolved and executed successfully.\n");

    printf("=============================================================\n");
    printf("TONAL & EMOTIONAL VOCABULARY TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}
