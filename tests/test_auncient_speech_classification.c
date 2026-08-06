#include "../tsfi2-deepseek/inc/auncient_teddy_personality.h"
#include "../tsfi2-deepseek/inc/tsfi_speech_synth.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

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
    assert(model.base_frequency == 250.0);
    assert(model.jitter_factor == 0.0); // Aligned alignment
    assert(tsfi_speech_synth_generate(&model, 0.5, 44100, buffer, buffer_size));
    
    // Verify classification
    teddy_personality_t class_trust = tsfi_speech_classify_pitch(model.base_frequency);
    assert(class_trust == PERSONALITY_TRUSTWORTHY);
    printf("   ✓ Trustworthy speech synthesis and classification verified.\n");

    // 2. Verify Aggressive speech setup (Deep pitch, high stiffness attack)
    printf("[TEST] Testing PERSONALITY_AGGRESSIVE speech profile...\n");
    tsfi_speech_synth_init(&model, PERSONALITY_AGGRESSIVE);
    assert(model.base_frequency == 85.0);
    assert(model.envelope_attack > 0.02);
    assert(tsfi_speech_synth_generate(&model, 0.5, 44100, buffer, buffer_size));
    
    // Verify classification
    teddy_personality_t class_aggr = tsfi_speech_classify_pitch(model.base_frequency);
    assert(class_aggr == PERSONALITY_AGGRESSIVE);
    printf("   ✓ Aggressive speech synthesis and classification verified.\n");

    // 3. Verify Skeptical speech setup (Medium pitch)
    printf("[TEST] Testing PERSONALITY_SKEPTICAL speech profile...\n");
    tsfi_speech_synth_init(&model, PERSONALITY_SKEPTICAL);
    assert(model.base_frequency == 180.0);
    assert(tsfi_speech_synth_generate(&model, 0.5, 44100, buffer, buffer_size));
    
    // Verify classification
    teddy_personality_t class_skept = tsfi_speech_classify_pitch(model.base_frequency);
    assert(class_skept == PERSONALITY_SKEPTICAL);
    printf("   ✓ Skeptical speech synthesis and classification verified.\n");

    printf("=============================================================\n");
    printf("ALL SPEECH SYNTHESIS & CLASSIFICATION TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}
