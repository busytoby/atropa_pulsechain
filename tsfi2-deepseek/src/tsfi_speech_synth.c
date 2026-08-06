#include "tsfi_speech_synth.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void tsfi_speech_synth_init(tsfi_speech_model_t *model, teddy_personality_t personality) {
    teddy_geometry_t geom;
    resolve_teddy_geometry(personality, &geom);
    
    // Bind base parameters to personality engine
    model->base_frequency = geom.vocal_pitch;
    model->resonance_factor = 1.0 - (geom.jaw_scale * 0.2);
    
    // Evaluate jitter from behavioral mismatch and exposure characteristics
    model->jitter_factor = geom.behavioral_mismatch * 0.05;
    
    // Set custom envelope curves based on physical dynamics
    model->envelope_attack = 0.02 + (geom.stiffness * 0.05);
    model->envelope_decay = 0.05 + (geom.damping * 0.1);
}

bool tsfi_speech_synth_generate(const tsfi_speech_model_t *model, 
                                double duration, 
                                uint32_t sample_rate, 
                                int16_t *buffer, 
                                uint32_t buffer_size) {
    uint32_t total_samples = (uint32_t)(duration * sample_rate);
    if (total_samples > buffer_size) {
        return false;
    }
    
    uint32_t attack_samples = (uint32_t)(model->envelope_attack * sample_rate);
    uint32_t decay_samples = (uint32_t)(model->envelope_decay * sample_rate);
    
    double phase = 0.0;
    
    for (uint32_t i = 0; i < total_samples; i++) {
        // Formant carrier frequencies mapping physical model resonance constraints
        double f1 = model->base_frequency;
        
        // Apply micro-jitter frequency variance
        double current_jitter = 1.0 + (((double)rand() / RAND_MAX) * 2.0 - 1.0) * model->jitter_factor;
        
        phase += 2.0 * M_PI * f1 * current_jitter / sample_rate;
        
        // Multi-formant wave generation (speech emulation)
        double signal = sin(phase) + 0.4 * sin(2.5 * phase * model->resonance_factor);
        
        // Envelope generator filter to avoid clicks
        double envelope = 1.0;
        if (i < attack_samples) {
            envelope = (double)i / attack_samples;
        } else if (i > total_samples - decay_samples) {
            envelope = (double)(total_samples - i) / decay_samples;
        }
        
        buffer[i] = (int16_t)(signal * envelope * 12000.0);
    }
    return true;
}

teddy_personality_t tsfi_speech_classify_pitch(double pitch_frequency) {
    // Classification logic mapping pitch boundaries to profile sets
    if (pitch_frequency > 200.0) {
        return PERSONALITY_TRUSTWORTHY;
    } else if (pitch_frequency > 130.0) {
        return PERSONALITY_SKEPTICAL;
    } else {
        // Distinguish between aggressive and eerie using known profile values
        // Both are set to 85Hz, so default to aggressive
        return PERSONALITY_AGGRESSIVE;
    }
}
