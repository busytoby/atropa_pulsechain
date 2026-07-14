#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/*
 * Auncient Computel DTMF (Dual-Tone Multi-Frequency) Goertzel Decoder
 * 
 * Decodes standard DTMF keypad tones (0-9, A-D, *, #) using the Goertzel algorithm.
 */

#define SAMPLE_RATE 8000
#define GOERTZEL_N 205

static const float dtmf_row_freqs[4] = {697.0f, 770.0f, 852.0f, 941.0f};
static const float dtmf_col_freqs[4] = {1209.0f, 1336.0f, 1477.0f, 1633.0f};

static const char dtmf_char_map[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

/* Evaluates a single frequency power using the Goertzel algorithm */
float goertzel_magnitude(const float *samples, int num_samples, float target_freq) {
    float k = (float)((num_samples * target_freq) / SAMPLE_RATE);
    float omega = (float)((2.0f * M_PI * k) / num_samples);
    float cosine = cosf(omega);
    float coeff = 2.0f * cosine;

    float q0 = 0.0f;
    float q1 = 0.0f;
    float q2 = 0.0f;

    for (int i = 0; i < num_samples; i++) {
        q0 = coeff * q1 - q2 + samples[i];
        q2 = q1;
        q1 = q0;
    }

    return (q1 * q1) + (q2 * q2) - (q1 * q2 * coeff);
}

/* Decodes the dominant DTMF tone character from an audio sample block */
char decode_dtmf_block(const float *samples, int num_samples) {
    if (!samples || num_samples < GOERTZEL_N) return '\0';

    int row_match = -1;
    int col_match = -1;
    float max_row_pwr = 0.0f;
    float max_col_pwr = 0.0f;

    for (int r = 0; r < 4; r++) {
        float pwr = goertzel_magnitude(samples, GOERTZEL_N, dtmf_row_freqs[r]);
        if (pwr > max_row_pwr && pwr > 15.0f) { /* detection power threshold */
            max_row_pwr = pwr;
            row_match = r;
        }
    }

    for (int c = 0; c < 4; c++) {
        float pwr = goertzel_magnitude(samples, GOERTZEL_N, dtmf_col_freqs[c]);
        if (pwr > max_col_pwr && pwr > 15.0f) {
            max_col_pwr = pwr;
            col_match = c;
        }
    }

    if (row_match != -1 && col_match != -1) {
        return dtmf_char_map[row_match][col_match];
    }

    return '\0';
}
