#ifndef TSFI_SPEECH_SYNTH_H
#define TSFI_SPEECH_SYNTH_H

#include "../tsfi2-deepseek/inc/auncient_teddy_bear_personality.h"
#include <stdint.h>
#include <stdbool.h>

// Speech synthesis model structure
typedef struct {
    double base_frequency;       // Base fundamental frequency (Hz)
    double resonance_factor;      // Throat formant resonance constraint factor
    double jitter_factor;         // Jitter/shimmer variance offset
    double envelope_attack;       // Attack time envelope (seconds)
    double envelope_decay;        // Decay time envelope (seconds)
    
    // Wald-gated validation parameters
    double wald_beta[3];          // Trait parameter beta vector
    double wald_covariance[9];    // Parameter covariance matrix
    
    // Submissiveness and synchrony volume factors
    double amplitude_factor;      // Dynamic baseline speech volume factor
} tsfi_speech_model_t;

// Initialize speech synthesis parameters based on personality traits
void tsfi_speech_synth_init(tsfi_speech_model_t *model, teddy_bear_personality_t personality);

// Generate synthesized speech buffer (gated by Wald nominal diagnostics)
bool tsfi_speech_synth_generate(const tsfi_speech_model_t *model, 
                                double duration, 
                                uint32_t sample_rate, 
                                int16_t *buffer, 
                                uint32_t buffer_size);

// Classify voice profile based on fundamental pitch parameters
teddy_bear_personality_t tsfi_speech_classify_pitch(double pitch_frequency);

#endif // TSFI_SPEECH_SYNTH_H
