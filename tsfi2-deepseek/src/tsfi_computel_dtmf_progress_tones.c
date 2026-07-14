#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/*
 * Auncient Computel Telephony Progress Tone Generator
 * 
 * Generates standard call progress signals (Dial Tone, Ringback, Busy Signal)
 * using superimposed dual frequencies.
 */

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 8000
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef enum {
    TONE_DIAL,
    TONE_BUSY,
    TONE_RINGBACK
} ProgressToneType;

/* Generates PCM progress tone samples based on type and sample index offset */
float generate_progress_sample(ProgressToneType type, uint32_t sample_index) {
    double t = (double)sample_index / SAMPLE_RATE;
    float f1 = 0.0f;
    float f2 = 0.0f;
    bool is_on = true;

    switch (type) {
        case TONE_DIAL:
            // Dial Tone: 350 Hz + 440 Hz, continuous
            f1 = 350.0f;
            f2 = 440.0f;
            break;

        case TONE_BUSY:
            // Busy Signal: 480 Hz + 620 Hz, 0.5s on / 0.5s off
            f1 = 480.0f;
            f2 = 620.0f;
            is_on = ((sample_index % SAMPLE_RATE) < (SAMPLE_RATE / 2));
            break;

        case TONE_RINGBACK:
            // Ringback: 440 Hz + 480 Hz, 2.0s on / 4.0s off
            f1 = 440.0f;
            f2 = 480.0f;
            is_on = ((sample_index % (SAMPLE_RATE * 6)) < (SAMPLE_RATE * 2));
            break;
    }

    if (!is_on) return 0.0f;
    return (float)((sin(2.0 * M_PI * f1 * t) + sin(2.0 * M_PI * f2 * t)) * 0.5);
}
