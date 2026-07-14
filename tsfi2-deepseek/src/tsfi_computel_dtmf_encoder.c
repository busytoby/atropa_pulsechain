#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/*
 * Auncient Computel DTMF (Dual-Tone Multi-Frequency) Tone Generator / Encoder
 * 
 * Generates raw PCM sine wave buffers corresponding to standard DTMF keypad characters.
 */

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 8000
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const float row_freqs[4] = {697.0f, 770.0f, 852.0f, 941.0f};
static const float col_freqs[4] = {1209.0f, 1336.0f, 1477.0f, 1633.0f};

/* Fills buffer with generated DTMF tone for a given key character */
bool generate_dtmf_tone(char key, float *buffer, int num_samples) {
    if (!buffer || num_samples <= 0) return false;

    int row = -1;
    int col = -1;

    // Find row/col indices for the keypad character
    switch (key) {
        case '1': row = 0; col = 0; break;
        case '2': row = 0; col = 1; break;
        case '3': row = 0; col = 2; break;
        case 'A': row = 0; col = 3; break;

        case '4': row = 1; col = 0; break;
        case '5': row = 1; col = 1; break;
        case '6': row = 1; col = 2; break;
        case 'B': row = 1; col = 3; break;

        case '7': row = 2; col = 0; break;
        case '8': row = 2; col = 1; break;
        case '9': row = 2; col = 2; break;
        case 'C': row = 2; col = 3; break;

        case '*': row = 3; col = 0; break;
        case '0': row = 3; col = 1; break;
        case '#': row = 3; col = 2; break;
        case 'D': row = 3; col = 3; break;

        default:
            return false;
    }

    float f_row = row_freqs[row];
    float f_col = col_freqs[col];

    for (int i = 0; i < num_samples; i++) {
        double t = (double)i / SAMPLE_RATE;
        // Normalize amplitudes to peak at 1.0 combined
        buffer[i] = (float)((sin(2.0 * M_PI * f_row * t) + sin(2.0 * M_PI * f_col * t)) * 0.5);
    }

    return true;
}
