#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/*
 * Auncient Computel Single-Frequency (SF) & Multi-Frequency (MF) Switch Controller
 * 
 * Simulates trunk seizure via 2600 Hz SF tones and routing via MF digits (Blue Box).
 */

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 8000
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// MF frequency pairs for trunk dialing
static const float mf_freqs_f1[12] = {700.0f, 700.0f, 900.0f, 700.0f, 900.0f, 1100.0f, 700.0f, 900.0f, 1100.0f, 1300.0f, 1100.0f, 1500.0f};
static const float mf_freqs_f2[12] = {900.0f, 1100.0f, 1100.0f, 1300.0f, 1300.0f, 1300.0f, 1500.0f, 1500.0f, 1500.0f, 1500.0f, 1700.0f, 1700.0f};
// Map characters: '1'-'9', '0', 'K' (KP), 'S' (ST)
static const char mf_char_map[12] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'K', 'S'};

/* Generates 2600 Hz SF tone to seize simulated trunk line */
void generate_sf_seizure(float *buffer, int num_samples) {
    if (!buffer || num_samples <= 0) return;
    for (int i = 0; i < num_samples; i++) {
        double t = (double)i / SAMPLE_RATE;
        buffer[i] = (float)sin(2.0 * M_PI * 2600.0 * t);
    }
}

/* Generates MF tones to route calls inside tandem trunk switch matrices */
bool generate_mf_tone(char digit, float *buffer, int num_samples) {
    if (!buffer || num_samples <= 0) return false;

    int idx = -1;
    for (int i = 0; i < 12; i++) {
        if (mf_char_map[i] == digit) {
            idx = i;
            break;
        }
    }

    if (idx == -1) return false;

    float f1 = mf_freqs_f1[idx];
    float f2 = mf_freqs_f2[idx];

    for (int i = 0; i < num_samples; i++) {
        double t = (double)i / SAMPLE_RATE;
        buffer[i] = (float)((sin(2.0 * M_PI * f1 * t) + sin(2.0 * M_PI * f2 * t)) * 0.5);
    }

    return true;
}
