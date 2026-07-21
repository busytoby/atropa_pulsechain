#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "tsfi_logan_synth.h"
#include "tsfi_dys_math.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int tsfi_logan_synth_init(tsfi_logan_synth_t *synth, float frequency_hz, float sample_rate) {
    if (!synth || sample_rate <= 0.0f) return -1;
    memset(synth, 0, sizeof(tsfi_logan_synth_t));

    // Populate William A. Logan's 512-sample logarithmic sine lookup table
    for (int i = 0; i < LOGAN_TABLE_SIZE; i++) {
        float angle = (float)i * (2.0f * (float)M_PI / (float)LOGAN_TABLE_SIZE);
        float raw_sine = sinf(angle);
        // Non-linear logarithmic curve shaping
        synth->log_sine_table[i] = (raw_sine >= 0.0f) ?
            log1pf(raw_sine * 1.7182818f) :
            -log1pf(-raw_sine * 1.7182818f);
    }

    synth->phase_increment = (uint32_t)((frequency_hz / sample_rate) * 4294967296.0f);
    synth->resonance_q = 0.707f;

    printf("[LOGAN SYNTH] Initialized 512-Sample Logarithmic Sine Wavetable (Freq: %.1f Hz)\n", frequency_hz);
    return 0;
}

float tsfi_logan_synth_step(tsfi_logan_synth_t *synth) {
    if (!synth) return 0.0f;
    synth->phase_accumulator += synth->phase_increment;
    uint32_t idx = (synth->phase_accumulator >> 23) & (LOGAN_TABLE_SIZE - 1);
    return synth->log_sine_table[idx];
}

int tsfi_logan_synth_map_lissajous(const tsfi_logan_synth_t *synth, float *out_phi, float *out_fx) {
    if (!synth || !out_phi || !out_fx) return -1;
    uint32_t idx = (synth->phase_accumulator >> 23) & (LOGAN_TABLE_SIZE - 1);
    *out_phi = synth->log_sine_table[idx] * (float)M_PI;
    *out_fx = 1.0f + fabsf(synth->log_sine_table[(idx + 128) & (LOGAN_TABLE_SIZE - 1)]) * 4.0f;
    return 0;
}
