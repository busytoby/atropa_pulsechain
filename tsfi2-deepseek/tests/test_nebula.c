#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "tsfi_nebula.h"

int main() {
    printf("=== TSFi2 & Synapse Issue #2 BCD Nebula Processor Test ===\n");

    const float sample_rate = 44100.0f;
    const int duration_samples = 44100; // 1 second
    
    float *input = calloc(duration_samples, sizeof(float));
    float *output = calloc(duration_samples, sizeof(float));
    assert(input != NULL);
    assert(output != NULL);

    // Generate a decaying sine wave sweep representing a guitar string pluck
    printf("[NEBULA] Generating decaying 440Hz string pluck input waveform...\n");
    for (int i = 0; i < duration_samples; i++) {
        float t = (float)i / sample_rate;
        float freq = 440.0f - 100.0f * t; // Frequency sweep
        float envelope = expf(-6.0f * t); // Decaying amplitude envelope
        input[i] = sinf(2.0f * 3.14159265f * freq * t) * envelope;
    }

    // Initialize the processor
    TsfiNebulaProcessor proc;
    tsfi_nebula_init(&proc, sample_rate);
    proc.ext_vcf_cv = 0.5f; // Apply external VCF CV offset
    proc.lfo_rate = 8.0f;   // Set LFO rate to 8 Hz

    // Process the signal
    printf("[NEBULA] Processing through compandor, octave divider, fuzz, LFO, and SSM VCF...\n");
    tsfi_nebula_process(&proc, input, output, duration_samples);

    // Basic assertions to verify output sanity
    float max_out = 0.0f;
    for (int i = 0; i < duration_samples; i++) {
        if (fabsf(output[i]) > max_out) {
            max_out = fabsf(output[i]);
        }
    }
    printf("[NEBULA] Max output amplitude: %f\n", max_out);
    assert(max_out > 0.01f); // Output must have signal

    // Verify control processes
    printf("[NEBULA] Verifying control signals: LFO Phase: %f, Gate Output: %f\n", proc.lfo_phase, proc.gate_out);
    assert(proc.lfo_phase >= 0.0f);
    // After processing decaying pluck, gate should eventually decay to 0
    assert(proc.gate_out == 0.0f);

    free(input);
    free(output);

    // Check for memory leaks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_nebula", NULL);

    printf("[PASS] BCD Nebula processor emulation verification completed successfully.\n");
    return 0;
}
