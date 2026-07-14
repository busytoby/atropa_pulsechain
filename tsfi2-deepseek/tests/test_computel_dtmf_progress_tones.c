#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include "../src/tsfi_computel_dtmf_progress_tones.c"

int main(void) {
    printf("[TEST] Initializing Auncient Computel DTMF Progress Tones test suite...\n");

    // Test Dial Tone (continuous)
    float dial_sample = generate_progress_sample(TONE_DIAL, 100);
    assert(dial_sample != 0.0f);

    // Test Busy Signal (0.5s on / 0.5s off)
    // At index 1000 (0.125s) it should be active
    float busy_on = generate_progress_sample(TONE_BUSY, 1000);
    assert(busy_on != 0.0f);

    // At index 6000 (0.75s) it should be silent
    float busy_off = generate_progress_sample(TONE_BUSY, 6000);
    assert(busy_off == 0.0f);

    // Test Ringback Tone (2.0s on / 4.0s off)
    // At index 8000 (1.0s) it should be active
    float ring_on = generate_progress_sample(TONE_RINGBACK, 8000);
    assert(ring_on != 0.0f);

    // At index 32000 (4.0s) it should be silent
    float ring_off = generate_progress_sample(TONE_RINGBACK, 32000);
    assert(ring_off == 0.0f);

    printf("[SUCCESS] All Computel progress tone generator tests passed successfully.\n");
    return 0;
}
