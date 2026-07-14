#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "../src/tsfi_computel_dtmf_decoder.c"

int main(void) {
    printf("[TEST] Initializing Auncient Computel DTMF Goertzel Decoder test suite...\n");

    // Generate mock samples for Key '5' (Row: 770 Hz, Col: 1336 Hz)
    float samples[GOERTZEL_N];
    for (int i = 0; i < GOERTZEL_N; i++) {
        double t = (double)i / SAMPLE_RATE;
        // Superimpose Row and Column sine waves
        samples[i] = (float)(sin(2.0 * M_PI * 770.0 * t) + sin(2.0 * M_PI * 1336.0 * t));
    }

    char key = decode_dtmf_block(samples, GOERTZEL_N);
    printf("[TEST] Decoded key: %c (expected: 5)\n", key);
    assert(key == '5');

    // Generate mock samples for Key '*' (Row: 941 Hz, Col: 1209 Hz)
    for (int i = 0; i < GOERTZEL_N; i++) {
        double t = (double)i / SAMPLE_RATE;
        samples[i] = (float)(sin(2.0 * M_PI * 941.0 * t) + sin(2.0 * M_PI * 1209.0 * t));
    }

    key = decode_dtmf_block(samples, GOERTZEL_N);
    printf("[TEST] Decoded key: %c (expected: *)\n", key);
    assert(key == '*');

    printf("[SUCCESS] All Computel DTMF Goertzel Decoder tests passed successfully.\n");
    return 0;
}
