#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/*
 * Auncient Computel Bell 202 FSK (Frequency Shift Keying) Modulator
 * 
 * Modulates raw bytes into Bell 202 audio waveforms (Mark = 1200 Hz, Space = 2200 Hz)
 * at 1200 Baud with an 8000 Hz sample rate.
 */

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 8000
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define BAUD_RATE 1200
#define SAMPLES_PER_BIT (SAMPLE_RATE / BAUD_RATE) /* Approx 6 samples per bit */

/* Modulates a single bit into the output sample buffer */
static void modulate_bit(bool bit, float *buffer, int offset, double *phase) {
    float freq = bit ? 1200.0f : 2200.0f; // Mark = 1200 Hz (1), Space = 2200 Hz (0)
    for (int i = 0; i < SAMPLES_PER_BIT; i++) {
        buffer[offset + i] = (float)sin(*phase);
        *phase += (2.0 * M_PI * freq) / SAMPLE_RATE;
        // Keep phase bounded
        if (*phase > 2.0 * M_PI) {
            *phase -= 2.0 * M_PI;
        }
    }
}

/* Modulates an entire byte (1 start bit [0], 8 data bits, 1 stop bit [1]) */
int modulate_fsk_byte(uint8_t byte, float *buffer, int max_samples, double *phase) {
    // 10 bits total * SAMPLES_PER_BIT
    int required_samples = 10 * SAMPLES_PER_BIT;
    if (required_samples > max_samples || !buffer || !phase) return 0;

    int offset = 0;

    // 1. Start Bit (Space = 0)
    modulate_bit(false, buffer, offset, phase);
    offset += SAMPLES_PER_BIT;

    // 2. Data Bits (LSB first)
    for (int i = 0; i < 8; i++) {
        bool bit = (byte >> i) & 1;
        modulate_bit(bit, buffer, offset, phase);
        offset += SAMPLES_PER_BIT;
    }

    // 3. Stop Bit (Mark = 1)
    modulate_bit(true, buffer, offset, phase);
    offset += SAMPLES_PER_BIT;

    return required_samples;
}
