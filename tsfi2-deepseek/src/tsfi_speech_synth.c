#include "tsfi_speech_synth.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Forward declaration of link function
int evaluate_ordinal_cloglog_rating(const teddy_geometry_t *geom);

void tsfi_speech_synth_init(tsfi_speech_model_t *model, teddy_personality_t personality) {
    teddy_geometry_t geom;
    resolve_teddy_geometry(personality, &geom);
    
    // Bind base parameters to personality engine
    model->base_frequency = geom.vocal_pitch;
    
    // Modulate formant resonance via Christensen cloglog link ordinal rating (1 to 7)
    int cloglog_rating = evaluate_ordinal_cloglog_rating(&geom);
    model->resonance_factor = (1.0 - (geom.jaw_scale * 0.2)) * (1.0 + (cloglog_rating - 4) * 0.05);
    
    // Evaluate jitter from behavioral mismatch and exposure characteristics
    model->jitter_factor = geom.behavioral_mismatch * 0.05;
    
    // Set custom envelope curves based on physical dynamics
    model->envelope_attack = 0.02 + (geom.stiffness * 0.05);
    model->envelope_decay = 0.05 + (geom.damping * 0.1);
    
    // Initialize congruent parameters for Wald-gate validation (non-significant p-value)
    model->wald_beta[0] = 0.1;
    model->wald_beta[1] = 0.1;
    model->wald_beta[2] = 0.1;
    
    memset(model->wald_covariance, 0, sizeof(model->wald_covariance));
    model->wald_covariance[0] = 1.0;
    model->wald_covariance[4] = 1.0;
    model->wald_covariance[8] = 1.0;
}

// Forward declaration of the diagnostics function
bool evaluate_wald_nominal_test(const double *beta_vector, const double *covariance_matrix, int df, double *wald_stat_out, double *p_value_out);

bool tsfi_speech_synth_generate(const tsfi_speech_model_t *model, 
                                double duration, 
                                uint32_t sample_rate, 
                                int16_t *buffer, 
                                uint32_t buffer_size) {
    uint32_t total_samples = (uint32_t)(duration * sample_rate);
    if (total_samples > buffer_size) {
        return false;
    }
    
    // Run Wald-gate nominal verification check on input parameters
    double wald_stat = 0.0;
    double p_val = 1.0;
    evaluate_wald_nominal_test(model->wald_beta, model->wald_covariance, 3, &wald_stat, &p_val);
    
    bool wald_failed = (p_val < 0.05);
    
    uint32_t attack_samples = (uint32_t)(model->envelope_attack * sample_rate);
    uint32_t decay_samples = (uint32_t)(model->envelope_decay * sample_rate);
    
    double phase = 0.0;
    
    for (uint32_t i = 0; i < total_samples; i++) {
        // Fallback to safe 110Hz drone if parameter mismatch triggers the Wald gate
        double f1 = wald_failed ? 110.0 : model->base_frequency;
        double current_res = wald_failed ? 1.0 : model->resonance_factor;
        double current_jit = wald_failed ? 0.0 : model->jitter_factor;
        
        // Apply micro-jitter frequency variance
        double current_jitter = 1.0 + (((double)rand() / RAND_MAX) * 2.0 - 1.0) * current_jit;
        
        phase += 2.0 * M_PI * f1 * current_jitter / sample_rate;
        
        // Multi-formant wave generation (speech emulation)
        double signal = sin(phase) + 0.4 * sin(2.5 * phase * current_res);
        
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
