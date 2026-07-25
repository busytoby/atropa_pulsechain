#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    // 1. Gating parameter: flux state (0.0 = completely unvoiced/noise, 1.0 = completely voiced/sawtooth)
    double flux_state;
    
    // 2. Formant Tuning parameter: setting current (0.0V to 5.0V, modulating virtual inductance)
    double setting_current;
    double filter_history[2];
    
    // 3. Compression parameter: saturation threshold
    double saturation_limit;
} transfluxor_synth_t;

// Non-linear magnetic saturation compression curve
static double magnetic_compress(double signal, double limit) {
    // Use hyperbolic tangent approximation to simulate magnetic hysteresis saturation clamping
    double scaled = signal / limit;
    double saturated = tanh(scaled) * limit;
    return saturated;
}

// Process the transfluxor voice synthesis step
double process_transfluxor_synth(transfluxor_synth_t *synth, double voiced_in, double unvoiced_in) {
    // 1. Excitation Gating: Mix sources based on core magnetic flux state
    double excitation = (voiced_in * synth->flux_state) + (unvoiced_in * (1.0 - synth->flux_state));
    
    // 2. Dynamic Formant Tuning: Calculate cutoff based on setting current (variable inductance)
    // Cutoff frequency moves between 200Hz and 3000Hz based on setting current (0.0 to 5.0V)
    double inductance = 1.0 / (synth->setting_current + 0.1); // Inductance inversely proportional to current
    double cutoff_freq = 3000.0 * (1.0 - (inductance * 0.1));
    if (cutoff_freq < 200.0) cutoff_freq = 200.0;
    if (cutoff_freq > 3000.0) cutoff_freq = 3000.0;
    
    // RLC low-pass filter approximation using virtual inductance
    double rc = 1.0 / (2.0 * M_PI * cutoff_freq);
    double dt = 1.0 / 44100.0;
    double alpha = dt / (rc + dt);
    
    double filtered = alpha * excitation + (1.0 - alpha) * synth->filter_history[0];
    synth->filter_history[0] = filtered;
    
    // 3. Non-Linear Amplitude Compression
    double compressed_output = magnetic_compress(filtered, synth->saturation_limit);
    
    return compressed_output;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TRANSFLUXOR VOICE SYNTHESIS SUITE\n");
    printf("=============================================================\n");

    transfluxor_synth_t synth = {
        .flux_state = 1.0, // Fully voiced
        .setting_current = 5.0, // Maximum current (lowest inductance, highest filter cutoff)
        .filter_history = { 0.0, 0.0 },
        .saturation_limit = 0.5 // High compression limit
    };

    // 1. Verify Excitation Gating
    printf("[TEST] Testing voiced excitation routing (flux_state = 1.0)...\n");
    double out_voiced = process_transfluxor_synth(&synth, 1.0, 0.0);
    assert(out_voiced > 0.0);
    printf("   ✓ Voiced excitation routed successfully. Output: %f\n", out_voiced);

    printf("[TEST] Testing unvoiced excitation routing (flux_state = 0.0)...\n");
    synth.flux_state = 0.0; // Switch to unvoiced (noise)
    double out_unvoiced = process_transfluxor_synth(&synth, 1.0, 0.5);
    // Voiced input (1.0) should be gated off, only unvoiced input (0.5) is routed
    assert(out_unvoiced > 0.0);
    assert(out_unvoiced < 0.5); // Filtered and compressed
    printf("   ✓ Unvoiced excitation routed successfully. Output: %f\n", out_unvoiced);

    // 2. Verify Dynamic Formant Tuning
    printf("[TEST] Verifying formant tuning via setting current (inductance)...\n");
    synth.flux_state = 1.0;
    
    // High setting current (5.0V) -> High cutoff -> High output amplitude for high frequency inputs
    synth.setting_current = 5.0;
    double out_high_cutoff = process_transfluxor_synth(&synth, 1.0, 0.0);
    
    // Reset history and lower setting current (0.1V) -> Low cutoff -> Low output amplitude (filtered out)
    synth.setting_current = 0.1;
    synth.filter_history[0] = 0.0;
    double out_low_cutoff = process_transfluxor_synth(&synth, 1.0, 0.0);
    
    assert(out_high_cutoff > out_low_cutoff);
    printf("   ✓ Formant tuning verified. High cutoff (%.4f) > Low cutoff (%.4f).\n", 
           out_high_cutoff, out_low_cutoff);

    // 3. Verify Non-Linear Compression
    printf("[TEST] Verifying non-linear magnetic saturation compression...\n");
    synth.setting_current = 5.0;
    synth.saturation_limit = 0.2; // Heavy clamping
    double normal_input = 0.1;
    double high_input = 10.0;
    
    double out_normal = process_transfluxor_synth(&synth, normal_input, 0.0);
    double out_compressed = process_transfluxor_synth(&synth, high_input, 0.0);
    
    // Output should never exceed saturation limit (0.2)
    assert(out_compressed < 0.2);
    // Compression ratio: high input is 100x normal input, but output is heavily compressed
    assert(out_compressed < out_normal * 5.0);
    printf("   ✓ Non-linear compression verified. Normal: %f | Compressed: %f\n", 
           out_normal, out_compressed);

    printf("=============================================================\n");
    printf("TRANSFLUXOR VOICE SYNTHESIS TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}
