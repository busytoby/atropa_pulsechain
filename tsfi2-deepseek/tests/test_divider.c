#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "tsfi_divider.h"

int main() {
    printf("=== TSFi2 & Synapse Issue #3 7-Stage Frequency Divider Test ===\n");

    const float sample_rate = 44100.0f;
    const int duration_samples = 4410; // 0.1 second
    
    float *input = calloc(duration_samples, sizeof(float));
    float *output = calloc(duration_samples, sizeof(float));
    assert(input != NULL);
    assert(output != NULL);

    // Generate a 1000 Hz sine wave input
    printf("[DIVIDER] Generating 1000Hz sine input waveform...\n");
    for (int i = 0; i < duration_samples; i++) {
        float t = (float)i / sample_rate;
        input[i] = sinf(2.0f * 3.14159265f * 1000.0f * t);
    }

    // Initialize the processor
    TsfiDividerProcessor proc;
    tsfi_divider_init(&proc, sample_rate);

    // Process the signal
    printf("[DIVIDER] Processing signal through Schmitt Trigger comparator and CD4024 ripple stages...\n");
    tsfi_divider_process(&proc, input, output, duration_samples);

    // Basic assertions to verify output sanity
    float max_out = -99.0f;
    float min_out = 99.0f;
    for (int i = 0; i < duration_samples; i++) {
        if (output[i] > max_out) max_out = output[i];
        if (output[i] < min_out) min_out = output[i];
    }
    printf("[DIVIDER] Mixed output range: [%f, %f]\n", min_out, max_out);
    assert(max_out > 0.0f);
    assert(min_out < 0.0f);

    free(input);
    free(output);

    // Check for memory leaks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_divider", NULL);

    printf("[PASS] 7-Stage Frequency Divider emulation verification completed successfully.\n");
    return 0;
}
