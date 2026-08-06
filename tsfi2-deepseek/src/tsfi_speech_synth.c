#include "tsfi_speech_synth.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Forward declarations of link, diagnostics, and models functions
int evaluate_ordinal_cloglog_rating(const teddy_geometry_t *geom);
bool evaluate_keating_brow_dominance(const teddy_geometry_t *geom, double brow_height, double *brow_dominance_out);
bool evaluate_kramer_ward_human_face_elongation(const teddy_geometry_t *geom, double elongation_val, double *elongation_score_out);
bool evaluate_surrogate_residuals(const teddy_geometry_t *geom, int observed_rating, double *residual_out);
bool evaluate_scarpi_hedonic_orientation(const teddy_geometry_t *geom, double playfulness_scale, double *hedonic_out);
bool evaluate_cellarius_planetary_eccentricity(const teddy_geometry_t *geom, double eccentricity_ratio, double *translation_offset_out);
bool evaluate_hyde_vocal_warmth_pitch(const teddy_geometry_t *geom, double average_pitch_hz, double *warmth_offset_out);
bool evaluate_hyde_vocal_tremor_index(const teddy_geometry_t *geom, double pitch_variance, double *tremor_uncanny_out);
bool evaluate_keating_mouth_curvature(const teddy_geometry_t *geom, double upturn_curvature, double *warmth_rating_out);
bool evaluate_keating_babyfacedness_index(const teddy_geometry_t *geom, double *babyfacedness_out);
bool evaluate_keating_posture_pitch(const teddy_geometry_t *geom, double pitch_angle, double *submissiveness_out);

void tsfi_speech_synth_init(tsfi_speech_model_t *model, teddy_personality_t personality) {
    teddy_geometry_t geom;
    resolve_teddy_geometry(personality, &geom);
    
    // Evaluate brow dominance gestural pitch shift
    double brow_dominance = 0.0;
    evaluate_keating_brow_dominance(&geom, 0.3, &brow_dominance);
    
    // Lower pitch for higher dominance ratings (lower brow coordinates)
    double pitch_adjustment = brow_dominance * 15.0;
    
    // Evaluate Hyde vocal warmth pitch offset (shifts fundamental pitch upward for warm profiles)
    double warmth_offset = 0.0;
    evaluate_hyde_vocal_warmth_pitch(&geom, geom.vocal_pitch, &warmth_offset);
    
    // Evaluate babyfacedness index to shift pitch upward (simulating juvenile voice characteristics)
    double babyfacedness = 0.0;
    evaluate_keating_babyfacedness_index(&geom, &babyfacedness);
    
    model->base_frequency = geom.vocal_pitch - pitch_adjustment + (warmth_offset * 10.0) + (babyfacedness * 20.0);
    
    // Modulate formant resonance via Christensen cloglog link ordinal rating (1 to 7)
    int cloglog_rating = evaluate_ordinal_cloglog_rating(&geom);
    
    // Query hedonic friendly parameters to apply a warm resonance boost
    double hedonic_warmth = 0.0;
    evaluate_scarpi_hedonic_orientation(&geom, 0.8, &hedonic_warmth);
    
    // Query mouth curvature to scale up resonance factor (smiling mouth shapes shorten vocal tract)
    double mouth_warmth = 0.0;
    evaluate_keating_mouth_curvature(&geom, 0.4, &mouth_warmth);
    
    model->resonance_factor = (1.0 - (geom.jaw_scale * 0.2)) * (1.0 + (cloglog_rating - 4) * 0.05) + (hedonic_warmth * 0.15) + (mouth_warmth * 0.10);
    
    // Evaluate surrogate residuals to scale up the frequency jitter factor
    double surrogate_residual = 0.0;
    evaluate_surrogate_residuals(&geom, 4, &surrogate_residual);
    
    // Evaluate Hyde vocal tremor index
    double tremor_uncanny = 0.0;
    evaluate_hyde_vocal_tremor_index(&geom, 0.2, &tremor_uncanny);
    
    // Evaluate jitter from behavioral mismatch and exposure characteristics, scaled by residuals and tremor indices
    double eccentric_vibrato = 0.0;
    evaluate_cellarius_planetary_eccentricity(&geom, 0.05, &eccentric_vibrato);
    model->jitter_factor = (geom.behavioral_mismatch * 0.05) + (fabs(surrogate_residual) * 0.02) + (fabs(eccentric_vibrato) * 0.01) + (tremor_uncanny * 0.03);
    
    // Evaluate Kramer-Ward facial elongation to adjust vocal transition tempo envelope
    double face_elongation = 0.0;
    evaluate_kramer_ward_human_face_elongation(&geom, 1.0, &face_elongation);
    double envelope_scale = 1.0 + (face_elongation * 0.1);
    
    // Set custom envelope curves based on physical dynamics
    model->envelope_attack = (0.02 + (geom.stiffness * 0.05)) * envelope_scale;
    model->envelope_decay = (0.05 + (geom.damping * 0.1)) * envelope_scale;
    
    // Evaluate head posture pitch tilt submissiveness to damp amplitude baseline
    double submissive_rating = 0.0;
    evaluate_keating_posture_pitch(&geom, -15.0, &submissive_rating);
    
    // Set dynamic baseline amplitude factor (softer/damped for submissive profiles)
    model->amplitude_factor = 12000.0 * (1.0 - (submissive_rating * 0.25));
    
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

// Forward declarations of link, diagnostics, and models functions
int evaluate_ordinal_cloglog_rating(const teddy_geometry_t *geom);
bool evaluate_keating_brow_dominance(const teddy_geometry_t *geom, double brow_height, double *brow_dominance_out);
bool evaluate_kramer_ward_human_face_elongation(const teddy_geometry_t *geom, double elongation_val, double *elongation_score_out);
bool evaluate_surrogate_residuals(const teddy_geometry_t *geom, int observed_rating, double *residual_out);
bool evaluate_scarpi_hedonic_orientation(const teddy_geometry_t *geom, double playfulness_scale, double *hedonic_out);
bool evaluate_cellarius_planetary_eccentricity(const teddy_geometry_t *geom, double eccentricity_ratio, double *translation_offset_out);
bool evaluate_hyde_vocal_warmth_pitch(const teddy_geometry_t *geom, double average_pitch_hz, double *warmth_offset_out);
bool evaluate_hyde_vocal_tremor_index(const teddy_geometry_t *geom, double pitch_variance, double *tremor_uncanny_out);
bool evaluate_keating_mouth_curvature(const teddy_geometry_t *geom, double upturn_curvature, double *warmth_rating_out);
bool evaluate_keating_babyfacedness_index(const teddy_geometry_t *geom, double *babyfacedness_out);
bool evaluate_hyde_mouth_speed_synchrony(const teddy_geometry_t *geom, double mouth_speed, double pitch_acceleration, double *sync_mismatch_out);

// Forward declarations of system transaction controls
evaluation_tx_t begin_evaluation_transaction(teddy_geometry_t *target);
bool commit_evaluation_transaction(evaluation_tx_t *tx);
void rollback_evaluation_transaction(evaluation_tx_t *tx);

bool tsfi_speech_synth_generate(const tsfi_speech_model_t *model, 
                                double duration, 
                                uint32_t sample_rate, 
                                int16_t *buffer, 
                                uint32_t buffer_size) {
    uint32_t total_samples = (uint32_t)(duration * sample_rate);
    
    // Cast away const to begin evaluation transaction safeguard
    teddy_geometry_t dummy_geom;
    dummy_geom.vocal_pitch = model->base_frequency;
    
    // Aligned settings for simulated synchrony evaluation
    dummy_geom.leadership_profile = 0.5;
    dummy_geom.stiffness = model->envelope_attack;
    
    evaluation_tx_t tx = begin_evaluation_transaction(&dummy_geom);
    
    if (total_samples > buffer_size) {
        rollback_evaluation_transaction(&tx);
        return false;
    }
    
    // Run Wald-gate nominal verification check on input parameters
    double wald_stat = 0.0;
    double p_val = 1.0;
    evaluate_wald_nominal_test(model->wald_beta, model->wald_covariance, 3, &wald_stat, &p_val);
    
    bool wald_failed = (p_val < 0.05);
    
    // Evaluate mouth speed synchrony (damps output volume if mismatch is high)
    double sync_mismatch = 0.0;
    evaluate_hyde_mouth_speed_synchrony(&dummy_geom, 2.0, 1.5, &sync_mismatch);
    double amplitude_scale = (sync_mismatch > 0.5) ? (model->amplitude_factor * 0.67) : model->amplitude_factor;
    
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
        
        buffer[i] = (int16_t)(signal * envelope * amplitude_scale);
    }
    
    commit_evaluation_transaction(&tx);
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
