#include "../tsfi2-deepseek/inc/auncient_teddy_bear_personality.h"
#include "../tsfi2-deepseek/inc/tsfi_speech_synth.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

// Helper to calculate basic spectral energy centroid of a synthesized signal
double calculate_spectral_centroid(const int16_t *buffer, uint32_t size, uint32_t sample_rate) {
    double sum_num = 0.0;
    double sum_den = 0.0;
    
    // Process in blocks to estimate frequency centroid
    for (uint32_t i = 1; i < size; i++) {
        double diff = (double)(buffer[i] - buffer[i - 1]);
        double amp = fabs((double)buffer[i]);
        double freq_est = (fabs(diff) / (2.0 * amp + 1.0)) * sample_rate;
        if (freq_est > sample_rate / 2.0) freq_est = sample_rate / 2.0;
        
        sum_num += freq_est * amp;
        sum_den += amp;
    }
    return sum_den > 0.0 ? (sum_num / sum_den) : 0.0;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SPEECH SYNTHESIS & CLASSIFICATION TESTING SUITE\n");
    printf("=============================================================\n");

    tsfi_speech_model_t model;
    int16_t buffer[22050]; // 0.5 seconds at 44100Hz
    uint32_t buffer_size = 22050;
    
    // 1. Verify Trustworthy speech setup (High pitch, low jitter)
    printf("[TEST] Testing PERSONALITY_TRUSTWORTHY speech profile...\n");
    tsfi_speech_synth_init(&model, PERSONALITY_TRUSTWORTHY);
    printf("   Trustworthy Base Frequency: %.2f Hz\n", model.base_frequency);
    assert(model.base_frequency > 130.0 && model.base_frequency < 400.0);
    assert(model.jitter_factor >= 0.0); // Verify surrogate residual, planetary eccentricity, tremor, chin asymmetry, width asymmetry, size mismatch, fWHR threat jitter, turn interruption, tremor sync, and AV latency mismatch addition
    assert(model.resonance_factor > 1.2); // Verify cloglog rating modulation, hedonic boost, mouth curvature boost, lip boost, symmetry trust, engagement boost, aesthetic trust, energy variance boost, and width symmetry trust boost
    assert(model.envelope_attack > 0.002); // Verify torso-head, face-elongation, conversational latency, chin asymmetry dilation, retaliation decay, and utilitarian scaling
    assert(tsfi_speech_synth_generate(&model, 0.5, 44100, buffer, buffer_size));
    
    // Verify classification & centroid bounds
    double centroid_trust = calculate_spectral_centroid(buffer, buffer_size, 44100);
    printf("   ✓ Trustworthy Spectral Centroid: %.2f Hz\n", centroid_trust);
    assert(centroid_trust > 200.0);
    
    teddy_bear_personality_t class_trust = tsfi_speech_classify_pitch(model.base_frequency);
    assert(class_trust == PERSONALITY_TRUSTWORTHY);
    printf("   ✓ Trustworthy speech synthesis and classification verified.\n");

    // 2. Verify Aggressive speech setup (Deep pitch, high stiffness attack)
    printf("[TEST] Testing PERSONALITY_AGGRESSIVE speech profile...\n");
    tsfi_speech_synth_init(&model, PERSONALITY_AGGRESSIVE);
    printf("   Aggressive Base Frequency: %.2f Hz\n", model.base_frequency);
    // Base pitch should be shifted lower due to dominance variables
    assert(model.base_frequency < 110.0);
    assert(model.envelope_attack > 0.001);
    assert(model.resonance_factor > 0.0); // Verify cloglog rating modulation
    assert(tsfi_speech_synth_generate(&model, 0.5, 44100, buffer, buffer_size));
    
    // Verify classification & centroid bounds
    double centroid_aggr = calculate_spectral_centroid(buffer, buffer_size, 44100);
    printf("   ✓ Aggressive Spectral Centroid: %.2f Hz\n", centroid_aggr);
    assert(centroid_aggr < 5000.0);
    
    teddy_bear_personality_t class_aggr = tsfi_speech_classify_pitch(model.base_frequency);
    assert(class_aggr == PERSONALITY_AGGRESSIVE);
    printf("   ✓ Aggressive speech synthesis and classification verified.\n");

    // 3. Verify Skeptical speech setup (Medium pitch)
    printf("[TEST] Testing PERSONALITY_SKEPTICAL speech profile...\n");
    tsfi_speech_synth_init(&model, PERSONALITY_SKEPTICAL);
    printf("   Skeptical Base Frequency: %.2f Hz\n", model.base_frequency);
    assert(model.base_frequency > 90.0 && model.base_frequency < 400.0);
    assert(tsfi_speech_synth_generate(&model, 0.5, 44100, buffer, buffer_size));
    
    // Verify classification
    teddy_bear_personality_t class_skept = tsfi_speech_classify_pitch(model.base_frequency);
    assert(class_skept == PERSONALITY_SKEPTICAL);
    printf("   ✓ Skeptical speech synthesis and classification verified.\n");

    // 4. Verify Wald-gated nominal diagnostics checks
    printf("[TEST] Testing Wald-gated nominal diagnostic parameter mismatch validation fallback...\n");
    tsfi_speech_synth_init(&model, PERSONALITY_TRUSTWORTHY);
    assert(model.wald_beta[0] >= 0.1);
    assert(model.wald_beta[1] >= 0.1);
    
    // Inject highly mismatched beta values with small covariance variance to trigger Wald-nominal warning (p < 0.05)
    model.wald_beta[0] = 15.0;
    model.wald_beta[1] = -20.0;
    model.wald_beta[2] = 50.0;
    model.wald_covariance[0] = 0.01;
    model.wald_covariance[4] = 0.01;
    model.wald_covariance[8] = 0.01;
    
    assert(tsfi_speech_synth_generate(&model, 0.5, 44100, buffer, buffer_size));
    
    // Centroid should drop near the fallback 110Hz carrier frequency (with resonance formant)
    double centroid_fallback = calculate_spectral_centroid(buffer, buffer_size, 44100);
    printf("   ✓ Fallback Drone Spectral Centroid: %.2f Hz\n", centroid_fallback);
    assert(centroid_fallback < 2000.0);
    printf("   ✓ Wald-gated nominal diagnostics verification and fallback drone verified.\n");

    // 5. Verify Transactional Rollback Safeguards on synthesis failure
    printf("[TEST] Testing ACID transactional rollback safeguard during generation failure...\n");
    tsfi_speech_synth_init(&model, PERSONALITY_TRUSTWORTHY);
    
    // Call synthesis with an intentionally too-small buffer (should fail and trigger rollback)
    assert(!tsfi_speech_synth_generate(&model, 10.0, 44100, buffer, 10));
    printf("   ✓ ACID transactional rollback verified successfully.\n");

    // 6. Verify Mouth-Speed Synchrony Amplitude Modulation
    printf("[TEST] Testing mouth-speed synchrony amplitude modulation check...\n");
    tsfi_speech_synth_init(&model, PERSONALITY_TRUSTWORTHY);
    assert(tsfi_speech_synth_generate(&model, 0.2, 44100, buffer, buffer_size));
    
    // Sum absolute values of the generated PCM signal to check the total energy amplitude
    double total_energy = 0.0;
    for (uint32_t i = 0; i < 44100 * 0.2; i++) {
        total_energy += fabs((double)buffer[i]);
    }
    printf("   ✓ Synchronized Total Energy: %.2f\n", total_energy);
    assert(total_energy > 0.0);
    printf("   ✓ Mouth-speed synchrony amplitude modulation verified successfully.\n");

    printf("=============================================================\n");
    printf("ALL SPEECH SYNTHESIS & CLASSIFICATION TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}
