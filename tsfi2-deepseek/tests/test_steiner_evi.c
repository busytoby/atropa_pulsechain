#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_steiner_evi.h"

int main() {
    printf("=== Steiner EVI Breath Controller Test ===\n");

    SteinerEVIState state;
    state.base_pitch = 261.63f; // C4
    state.octave = 0;
    state.breath_pressure = 0.8f;
    state.valve1 = 0;
    state.valve2 = 0;
    state.valve3 = 0;

    float freq = 0.0f;
    float amp = 0.0f;

    // Test 1: Open fingering (C4)
    tsfi_steiner_evi_translate(&state, &freq, &amp);
    printf("Open (C4): Expected ~261.63 Hz, got %.2f Hz, amp: %.2f\n", freq, amp);
    assert(fabsf(freq - 261.63f) < 0.1f);
    assert(fabsf(amp - 0.8f) < 0.01f);

    // Test 2: Valve 2 pressed (-1 semitone -> B3)
    state.valve2 = 1;
    tsfi_steiner_evi_translate(&state, &freq, &amp);
    printf("Valve 2 (B3): Expected ~246.94 Hz, got %.2f Hz\n", freq);
    assert(fabsf(freq - 246.94f) < 0.1f);

    // Test 3: Valve 1 pressed (-2 semitones -> Bb3)
    state.valve2 = 0;
    state.valve1 = 1;
    tsfi_steiner_evi_translate(&state, &freq, &amp);
    printf("Valve 1 (Bb3): Expected ~233.08 Hz, got %.2f Hz\n", freq);
    assert(fabsf(freq - 233.08f) < 0.1f);

    // Test 4: Valve 1 + 2 pressed (-3 semitones -> A3)
    state.valve2 = 1;
    tsfi_steiner_evi_translate(&state, &freq, &amp);
    printf("Valves 1+2 (A3): Expected ~220.00 Hz, got %.2f Hz\n", freq);
    assert(fabsf(freq - 220.00f) < 0.1f);

    // Test 5: Octave shift +1
    state.octave = 1;
    state.valve1 = 0;
    state.valve2 = 0;
    tsfi_steiner_evi_translate(&state, &freq, &amp);
    printf("Octave +1 (C5): Expected ~523.25 Hz, got %.2f Hz\n", freq);
    assert(fabsf(freq - 523.25f) < 0.1f);

    // Test 6: Bidirectional Untranslate Verification
    printf("\n--- Starting Bidirectional Round-Trip Tests ---\n");
    SteinerEVIState recovered;
    
    // Round-trip C4 (Open)
    tsfi_steiner_evi_untranslate(261.63f, 0.8f, &recovered);
    assert(recovered.octave == 0);
    assert(recovered.valve1 == 0 && recovered.valve2 == 0 && recovered.valve3 == 0);
    
    // Round-trip B3 (Valve 2 pressed)
    tsfi_steiner_evi_untranslate(246.94f, 0.8f, &recovered);
    assert(recovered.octave == 0);
    assert(recovered.valve1 == 0 && recovered.valve2 == 1 && recovered.valve3 == 0);

    // Round-trip Bb3 (Valve 1 pressed)
    tsfi_steiner_evi_untranslate(233.08f, 0.8f, &recovered);
    assert(recovered.octave == 0);
    assert(recovered.valve1 == 1 && recovered.valve2 == 0 && recovered.valve3 == 0);

    // Round-trip A3 (Valves 1+2 pressed / semitone -3)
    tsfi_steiner_evi_untranslate(220.00f, 0.8f, &recovered);
    assert(recovered.octave == 0);
    // Since semitone -3 defaults to valve3 in untranslate:
    assert(recovered.valve3 == 1);

    printf("[PASS] Steiner EVI Bidirectional Round-Trip validated.\n");
    return 0;
}
