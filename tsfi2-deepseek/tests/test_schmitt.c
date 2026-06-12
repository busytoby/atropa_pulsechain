#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_schmitt.h"

int main() {
    printf("=== TSFi2 & Computer Design Issue #18 Emitter-Coupled Schmitt Trigger Test ===\n");

    TsfiSchmittTrigger trigger;
    tsfi_schmitt_init(&trigger);

    // Initial state check
    printf("[SCHMITT] Initial state output: %fV, q1:%d, q2:%d\n", trigger.v_out, trigger.q1_state, trigger.q2_state);
    assert(trigger.q1_state == 0 && trigger.q2_state == 1);

    // Test 1: Verify UTP switching point (rising input)
    printf("[SCHMITT] Testing rising input threshold (UTP)...\n");
    float dt = 1.0f; // 1 ns steps
    int state = 0;
    
    // Ramp input from 0.0V to 2.5V
    float v_in = 0.0f;
    float v_utp_detected = 0.0f;
    for (int i = 0; i < 250; i++) {
        v_in = i * 0.01f; // 0.01V step
        state = tsfi_schmitt_step(&trigger, v_in, dt);
        if (state == 1 && v_utp_detected == 0.0f) {
            v_utp_detected = v_in;
        }
    }
    printf("[SCHMITT]   Switched HIGH at UTP: %f V\n", v_utp_detected);
    // Theoretical UTP should be around 1.92V
    assert(v_utp_detected >= 1.85f && v_utp_detected <= 2.0f);

    // Test 2: Verify LTP switching point (falling input)
    printf("[SCHMITT] Testing falling input threshold (LTP)...\n");
    float v_ltp_detected = 0.0f;
    for (int i = 250; i >= 0; i--) {
        v_in = i * 0.01f;
        state = tsfi_schmitt_step(&trigger, v_in, dt);
        if (state == 0 && v_ltp_detected == 0.0f) {
            v_ltp_detected = v_in;
        }
    }
    printf("[SCHMITT]   Switched LOW at LTP: %f V\n", v_ltp_detected);
    // Theoretical LTP should be around 1.3V
    assert(v_ltp_detected >= 1.25f && v_ltp_detected <= 1.45f);
    assert(v_utp_detected > v_ltp_detected); // Hysteresis exists!

    // Test 3: Noise rejection simulation
    printf("[SCHMITT] Testing noise rejection on slowly rising noisy signal...\n");
    tsfi_schmitt_init(&trigger);

    // We simulate a slow ramp rising from 1.0V to 2.5V over 300 steps
    // with a high frequency noise of amplitude 0.15V (0.3V peak-to-peak)
    int schmitt_transitions = 0;
    int comp_transitions = 0;
    
    int last_schmitt_state = 0;
    int last_comp_state = 0;

    for (int i = 0; i < 300; i++) {
        float ramp = 1.0f + (i / 300.0f) * 1.5f;
        // Superimpose high frequency noise
        float noise = 0.15f * sinf(i * 1.5f);
        float noisy_signal = ramp + noise;

        // 1. Simple Comparator logic (threshold = 1.6V, no hysteresis)
        int comp_state = (noisy_signal >= 1.6f) ? 1 : 0;
        if (comp_state != last_comp_state) {
            comp_transitions++;
            last_comp_state = comp_state;
        }

        // 2. Schmitt Trigger logic
        int schmitt_state = tsfi_schmitt_step(&trigger, noisy_signal, dt);
        if (schmitt_state != last_schmitt_state) {
            schmitt_transitions++;
            last_schmitt_state = schmitt_state;
        }
    }

    printf("[SCHMITT]   Comparator transitions: %d (Noisy glitches)\n", comp_transitions);
    printf("[SCHMITT]   Schmitt transitions:    %d (Clean transition)\n", schmitt_transitions);

    // Noisy comparator should double-trigger/glitch multiple times
    assert(comp_transitions > 2);
    // Schmitt trigger should only switch exactly once (State 0 -> State 1)
    assert(schmitt_transitions == 1);
    printf("[SCHMITT]   Verification passed: Schmitt trigger successfully rejected input noise.\n");

    // Memory sanitation checks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_schmitt", NULL);

    printf("[PASS] Schmitt Trigger simulation verified.\n");
    return 0;
}
