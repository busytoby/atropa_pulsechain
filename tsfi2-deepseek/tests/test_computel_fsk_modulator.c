#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include "../src/tsfi_computel_fsk_modulator.c"

int main(void) {
    printf("[TEST] Initializing Auncient Computel Bell 202 FSK Modulator test suite...\n");

    float samples[128];
    double phase = 0.0;

    // Modulate byte 0xAA (binary 10101010)
    int samples_generated = modulate_fsk_byte(0xAA, samples, 128, &phase);
    
    // 10 bits * 6 samples/bit = 60 samples
    printf("[TEST] FSK Samples generated: %d\n", samples_generated);
    assert(samples_generated == 60);

    // Verify start bit (Space = 2200 Hz) has higher frequency than stop bit (Mark = 1200 Hz)
    // We can count zero crossings in the first 6 samples (start bit) vs the last 6 samples (stop bit)
    int start_crossings = 0;
    for (int i = 1; i < 6; i++) {
        if ((samples[i-1] >= 0 && samples[i] < 0) || (samples[i-1] < 0 && samples[i] >= 0)) {
            start_crossings++;
        }
    }

    int stop_crossings = 0;
    for (int i = 55; i < 60; i++) {
        if ((samples[i-1] >= 0 && samples[i] < 0) || (samples[i-1] < 0 && samples[i] >= 0)) {
            stop_crossings++;
        }
    }

    printf("[TEST] Start crossings: %d | Stop crossings: %d\n", start_crossings, stop_crossings);
    // Since 2200 Hz is almost double 1200 Hz, zero crossings should be greater or equal
    assert(start_crossings >= stop_crossings);

    printf("[SUCCESS] Bell 202 FSK Modulator tests passed successfully.\n");
    return 0;
}
