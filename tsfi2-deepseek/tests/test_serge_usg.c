#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "tsfi_serge_usg.h"

int main() {
    printf("=== TSFi2 & Synapse Issue #4 Serge USG Simulator Test ===\n");

    const float sample_rate = 1000.0f; // 1 kHz sample rate to speed up test
    const int duration = 500;          // 500 ms / samples
    
    float *trigger = calloc(duration, sizeof(float));
    float *output = calloc(duration, sizeof(float));
    assert(trigger != NULL);
    assert(output != NULL);

    // 1. Envelope Generator mode test (Triggered envelope)
    printf("[USG] Testing trigger-input envelope generation...\n");
    // Send a single trigger pulse at t = 10 samples
    trigger[10] = 1.0f;

    TsfiSergeUsgProcessor proc;
    tsfi_serge_usg_init(&proc, sample_rate);
    proc.rise_time = 0.05f; // 50 ms
    proc.fall_time = 0.10f; // 100 ms

    tsfi_serge_usg_process(&proc, trigger, output, duration);

    // Peak should be reached around 10 + 50 = 60 samples
    printf("  Envelope at 60ms: %f (expected ~1.0)\n", output[60]);
    assert(output[60] > 0.95f);
    // Envelope should have decayed to zero after 10 + 50 + 100 = 160 samples
    printf("  Envelope at 200ms: %f (expected 0.0)\n", output[200]);
    assert(output[200] < 0.01f);

    // 2. LFO Oscillator mode test (Loop mode)
    printf("[USG] Testing self-oscillation LFO loop mode...\n");
    tsfi_serge_usg_init(&proc, sample_rate);
    proc.rise_time = 0.02f; // 20 ms
    proc.fall_time = 0.03f; // 30 ms
    proc.loop_enable = 1;

    // Process with zero inputs
    tsfi_serge_usg_process(&proc, NULL, output, duration);

    // Total period = 20ms rise + 30ms fall = 50ms (or 50 samples)
    // There should be multiple cycles. Check peaks at multiples of 50
    printf("  LFO at 20ms: %f (expected ~1.0)\n", output[20]);
    assert(output[20] > 0.95f);
    printf("  LFO at 50ms: %f (expected ~0.0)\n", output[50]);
    assert(output[50] <= 0.05f);
    printf("  LFO at 70ms: %f (expected ~1.0)\n", output[70]);
    assert(output[70] > 0.95f);

    free(trigger);
    free(output);

    // Check for memory leaks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_serge_usg", NULL);

    printf("[PASS] Serge USG emulator verification completed successfully.\n");
    return 0;
}
