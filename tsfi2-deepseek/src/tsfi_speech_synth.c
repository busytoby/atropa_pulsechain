#include "tsfi_speech_synth.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Forward declarations of link, diagnostics, and models functions
int evaluate_ordinal_cloglog_rating(const teddy_bear_geometry_t *geom);
bool evaluate_keating_brow_dominance(const teddy_bear_geometry_t *geom, double brow_height, double *brow_dominance_out);
bool evaluate_kramer_ward_human_face_elongation(const teddy_bear_geometry_t *geom, double elongation_val, double *elongation_score_out);
bool evaluate_surrogate_residuals(const teddy_bear_geometry_t *geom, int observed_rating, double *residual_out);
bool evaluate_scarpi_hedonic_orientation(const teddy_bear_geometry_t *geom, double playfulness_scale, double *hedonic_out);
bool evaluate_cellarius_planetary_eccentricity(const teddy_bear_geometry_t *geom, double eccentricity_ratio, double *translation_offset_out);
bool evaluate_hyde_vocal_warmth_pitch(const teddy_bear_geometry_t *geom, double average_pitch_hz, double *warmth_offset_out);
bool evaluate_hyde_vocal_tremor_index(const teddy_bear_geometry_t *geom, double pitch_variance, double *tremor_uncanny_out);
bool evaluate_keating_mouth_curvature(const teddy_bear_geometry_t *geom, double upturn_curvature, double *warmth_rating_out);
bool evaluate_keating_babyfacedness_index(const teddy_bear_geometry_t *geom, double *babyfacedness_out);
bool evaluate_keating_posture_pitch(const teddy_bear_geometry_t *geom, double pitch_angle, double *submissiveness_out);
bool evaluate_keating_chin_asymmetry(const teddy_bear_geometry_t *geom, double left_jaw_width, double right_jaw_width, double *asymmetry_dominance_out);
bool evaluate_keating_sclera_size(const teddy_bear_geometry_t *geom, double sclera_ratio, double *submissiveness_out);
bool evaluate_keating_lip_trustworthiness(const teddy_bear_geometry_t *geom, double lip_thickness, double *lip_trustworthiness_out);
bool evaluate_keating_brow_gesture(const teddy_bear_geometry_t *geom, double brow_raise_amplitude, double *brow_submissiveness_out);
bool evaluate_keating_torso_head_ratio(const teddy_bear_geometry_t *geom, double torso_span, double *ratio_dominance_out);
bool evaluate_keating_width_asymmetry(const teddy_bear_geometry_t *geom, double left_width, double right_width, double *asymmetry_dominance_out);
bool evaluate_keating_brow_chin_proportion(const teddy_bear_geometry_t *geom, double brow_chin_distance, double *proportion_dominance_out);
bool evaluate_hyde_conversational_latency(const teddy_bear_geometry_t *geom, double lag_seconds, double *latency_uncanny_out);
bool evaluate_hyde_vocal_size_mismatch(const teddy_bear_geometry_t *geom, double voice_pitch_hz, double *size_mismatch_out);
bool evaluate_geniole_fwhr_dilation_map(const teddy_bear_geometry_t *geom, double base_dilation, double *mapped_dilation_out);
bool evaluate_keating_head_roundness_index(const teddy_bear_geometry_t *geom, double *roundness_out);
bool evaluate_keating_symmetry_trust(const teddy_bear_geometry_t *geom, double *trust_out);
bool evaluate_hyde_vocal_amplitude_mismatch(const teddy_bear_geometry_t *geom, double voice_amplitude_db, double *amplitude_mismatch_out);
bool evaluate_geniole_fwhr_jitter_mod(const teddy_bear_geometry_t *geom, double base_jitter, double *mapped_jitter_out);
bool evaluate_keating_gaze_dominance(const teddy_bear_geometry_t *geom, double gaze_duration_sec, double aversion_angle, double *gaze_dominance_out);
bool evaluate_hyde_turn_interruption(const teddy_bear_geometry_t *geom, double overlap_duration_sec, double *interruption_uncanny_out);
bool evaluate_keating_eye_dilation_sync(const teddy_bear_geometry_t *geom, double left_dilation, double right_dilation, double *babyface_sync_out);
bool evaluate_hyde_tremor_frequency_sync(const teddy_bear_geometry_t *geom, double chin_vibration_hz, double audio_tremor_hz, double *sync_rating_out);
bool evaluate_hyde_pitch_range_engagement(const teddy_bear_geometry_t *geom, double pitch_range_hz, double *engagement_rating_out);
bool evaluate_geniole_fwhr_boundary_map(const teddy_bear_geometry_t *geom, double threshold_scale, double *mapped_boundary_out);
bool evaluate_keating_chin_asymmetry_dilation(const teddy_bear_geometry_t *geom, double base_dilation, double *asymmetry_dilation_out);
bool evaluate_geniole_fwhr_retaliation_decay(const teddy_bear_geometry_t *geom, double exposure_duration_sec, double *decayed_retaliation_out);
bool evaluate_scarpi_aesthetic_trust(const teddy_bear_geometry_t *geom, double base_trust, double *aesthetic_trust_out);
bool evaluate_scarpi_utilitarian_orientation(const teddy_bear_geometry_t *geom, double efficiency_scale, double *utilitarian_out);
bool evaluate_cellarius_heliocentric_alignment(const teddy_bear_geometry_t *geom, double orbital_phase, double *alignment_offset_out);
bool evaluate_cellarius_constellation_boundary(const teddy_bear_geometry_t *geom, double celestial_longitude, double *boundary_limit_out);
bool evaluate_hyde_av_latency_jitter(const teddy_bear_geometry_t *geom, double latency_jitter_sec, double *naturalness_out);
bool evaluate_hyde_vocal_energy_variance(const teddy_bear_geometry_t *geom, double energy_variance, double *engagement_out);
bool evaluate_keating_fwhr_dominance(const teddy_bear_geometry_t *geom, double fwhr_value, double *dominance_out);
bool evaluate_keating_mouth_width_ratio(const teddy_bear_geometry_t *geom, double mouth_width, double jaw_width, double *submissiveness_out);
bool evaluate_keating_width_symmetry_trust(const teddy_bear_geometry_t *geom, double symmetry_ratio, double *trustworthiness_out);
bool evaluate_keating_lip_submissiveness(const teddy_bear_geometry_t *geom, double lip_height_ratio, double *submissiveness_out);

void tsfi_speech_synth_init(tsfi_speech_model_t *model, teddy_bear_personality_t personality) {
    teddy_bear_geometry_t geom;
    resolve_teddy_bear_geometry(personality, &geom);
    
    // Evaluate brow dominance gestural pitch shift
    double brow_dominance = 0.0;
    evaluate_keating_brow_dominance(&geom, 0.3, &brow_dominance);
    
    // Evaluate Keating brow chin proportion dominance pitch modulation
    double brow_chin_dominance = 0.0;
    evaluate_keating_brow_chin_proportion(&geom, 0.4, &brow_chin_dominance);
    
    // Evaluate Geniole fWHR dynamic threat dilation pitch modulation
    double fwhr_dilation = 0.0;
    evaluate_geniole_fwhr_dilation_map(&geom, 1.1, &fwhr_dilation);
    
    // Evaluate Keating gaze dominance pitch modulation
    double gaze_dominance = 0.0;
    evaluate_keating_gaze_dominance(&geom, 3.0, 10.0, &gaze_dominance);
    
    // Evaluate Keating fWHR dominance pitch modulation
    double fwhr_dominance = 0.0;
    evaluate_keating_fwhr_dominance(&geom, 1.9, &fwhr_dominance);
    
    // Lower pitch for higher dominance ratings (lower brow coordinates, brow-to-chin scaling, threat dilation mapping, gaze dominance, or fWHR dominance)
    double pitch_adjustment = (brow_dominance * 15.0) + (brow_chin_dominance * 15.0) + (fwhr_dilation * 10.0) + (gaze_dominance * 15.0) + (fwhr_dominance * 15.0);
    
    // Evaluate Hyde vocal warmth pitch offset (shifts fundamental pitch upward for warm profiles)
    double warmth_offset = 0.0;
    evaluate_hyde_vocal_warmth_pitch(&geom, geom.vocal_pitch, &warmth_offset);
    
    // Evaluate babyfacedness index to shift pitch upward (simulating juvenile voice characteristics)
    double babyfacedness = 0.0;
    evaluate_keating_babyfacedness_index(&geom, &babyfacedness);
    
    // Evaluate Keating head roundness babyface pitch modulation
    double head_roundness = 0.0;
    evaluate_keating_head_roundness_index(&geom, &head_roundness);
    
    // Evaluate Keating eye dilation synchronization babyface pitch modulation
    double eye_dilation_sync = 0.0;
    evaluate_keating_eye_dilation_sync(&geom, 0.5, 0.5, &eye_dilation_sync);
    
    // Evaluate Keating sclera size submissiveness pitch modulation
    double sclera_submissive = 0.0;
    evaluate_keating_sclera_size(&geom, 0.35, &sclera_submissive);
    
    // Evaluate Keating brow gesture submissiveness pitch modulation
    double brow_submissive = 0.0;
    evaluate_keating_brow_gesture(&geom, 0.5, &brow_submissive);
    
    // Evaluate Cellarius heliocentric alignment pitch modulation
    double helio_alignment = 0.0;
    evaluate_cellarius_heliocentric_alignment(&geom, 0.25, &helio_alignment);
    
    // Evaluate Keating mouth-to-jaw width ratio submissiveness pitch modulation
    double mouth_width_submissive = 0.0;
    evaluate_keating_mouth_width_ratio(&geom, 0.4, 0.8, &mouth_width_submissive);
    
    // Evaluate Keating lip-to-facial height ratio submissiveness pitch modulation
    double lip_submissive = 0.0;
    evaluate_keating_lip_submissiveness(&geom, 0.15, &lip_submissive);
    
    model->base_frequency = geom.vocal_pitch - pitch_adjustment + (warmth_offset * 10.0) + (babyfacedness * 20.0) + (head_roundness * 20.0) + (eye_dilation_sync * 15.0) + (sclera_submissive * 15.0) + (brow_submissive * 15.0) + (helio_alignment * 10.0) + (mouth_width_submissive * 15.0) + (lip_submissive * 15.0);
    
    // Modulate formant resonance via Christensen cloglog link ordinal rating (1 to 7)
    int cloglog_rating = evaluate_ordinal_cloglog_rating(&geom);
    
    // Query hedonic friendly parameters to apply a warm resonance boost
    double hedonic_warmth = 0.0;
    evaluate_scarpi_hedonic_orientation(&geom, 0.8, &hedonic_warmth);
    
    // Query mouth curvature to scale up resonance factor (smiling mouth shapes shorten vocal tract)
    double mouth_warmth = 0.0;
    evaluate_keating_mouth_curvature(&geom, 0.4, &mouth_warmth);
    
    // Evaluate lip trustworthiness boost
    double lip_warmth = 0.0;
    evaluate_keating_lip_trustworthiness(&geom, 0.6, &lip_warmth);
    
    // Evaluate bilateral symmetry trust boost
    double symmetry_trust = 0.0;
    evaluate_keating_symmetry_trust(&geom, &symmetry_trust);
    
    // Evaluate Hyde conversational pitch range engagement boost
    double pitch_engagement = 0.0;
    evaluate_hyde_pitch_range_engagement(&geom, 50.0, &pitch_engagement);
    
    // Evaluate Scarpi aesthetic trust boost
    double aesthetic_trust = 0.0;
    evaluate_scarpi_aesthetic_trust(&geom, 0.6, &aesthetic_trust);
    
    // Evaluate Hyde conversational vocal energy variance engagement boost
    double energy_engagement = 0.0;
    evaluate_hyde_vocal_energy_variance(&geom, 0.4, &energy_engagement);
    
    // Evaluate Keating facial width symmetry trust boost
    double width_symmetry_trust = 0.0;
    evaluate_keating_width_symmetry_trust(&geom, 0.95, &width_symmetry_trust);
    
    model->resonance_factor = (1.0 - (geom.jaw_scale * 0.2)) * (1.0 + (cloglog_rating - 4) * 0.05) + (hedonic_warmth * 0.15) + (mouth_warmth * 0.10) + (lip_warmth * 0.12) + (symmetry_trust * 0.15) + (pitch_engagement * 0.10) + (aesthetic_trust * 0.15) + (energy_engagement * 0.12) + (width_symmetry_trust * 0.14);
    
    // Evaluate surrogate residuals to scale up the frequency jitter factor
    double surrogate_residual = 0.0;
    evaluate_surrogate_residuals(&geom, 4, &surrogate_residual);
    
    // Evaluate Hyde vocal tremor index
    double tremor_uncanny = 0.0;
    evaluate_hyde_vocal_tremor_index(&geom, 0.2, &tremor_uncanny);
    
    // Evaluate Keating chin asymmetry
    double chin_asymmetry = 0.0;
    evaluate_keating_chin_asymmetry(&geom, 0.8, 0.9, &chin_asymmetry);
    
    // Evaluate Keating width asymmetry
    double width_asymmetry = 0.0;
    evaluate_keating_width_asymmetry(&geom, 1.1, 1.2, &width_asymmetry);
    
    // Evaluate Hyde vocal size mismatch
    double size_mismatch = 0.0;
    evaluate_hyde_vocal_size_mismatch(&geom, model->base_frequency, &size_mismatch);
    
    // Evaluate Geniole fWHR dominance threat micro-expression jitter modulation
    double fwhr_jitter = 0.0;
    evaluate_geniole_fwhr_jitter_mod(&geom, 0.05, &fwhr_jitter);
    
    // Evaluate Hyde conversational turn interruption to scale voice jitter
    double turn_interruption = 0.0;
    evaluate_hyde_turn_interruption(&geom, 0.8, &turn_interruption);
    
    // Evaluate Hyde tremor frequency sync to scale voice jitter under submissive tremors
    double tremor_sync = 0.0;
    evaluate_hyde_tremor_frequency_sync(&geom, 6.0, 6.2, &tremor_sync);
    
    // Evaluate Hyde AV latency jitter to scale voice jitter
    double av_naturalness = 1.0;
    evaluate_hyde_av_latency_jitter(&geom, 0.05, &av_naturalness);
    double av_jitter_mod = (1.0 - av_naturalness) * 0.03;
    
    // Evaluate jitter from behavioral mismatch and exposure characteristics, scaled by residuals, tremors, jaw asymmetry, width asymmetry, size mismatch, fWHR threat jitter, turn interruption, tremor sync, and AV latency mismatch
    double eccentric_vibrato = 0.0;
    evaluate_cellarius_planetary_eccentricity(&geom, 0.05, &eccentric_vibrato);
    model->jitter_factor = (geom.behavioral_mismatch * 0.05) + (fabs(surrogate_residual) * 0.02) + (fabs(eccentric_vibrato) * 0.01) + (tremor_uncanny * 0.03) + (fabs(chin_asymmetry) * 0.02) + (fabs(width_asymmetry) * 0.02) + (size_mismatch * 0.03) + (fwhr_jitter * 0.02) + (turn_interruption * 0.02) + (tremor_sync * 0.02) + av_jitter_mod;
    
    // Evaluate Kramer-Ward facial elongation to adjust vocal transition tempo envelope
    double face_elongation = 0.0;
    evaluate_kramer_ward_human_face_elongation(&geom, 1.0, &face_elongation);
    double envelope_scale = 1.0 + (face_elongation * 0.1);
    
    // Evaluate torso to head ratio dominance scaling parameter to scale response envelopes
    double torso_dominance = 0.0;
    evaluate_keating_torso_head_ratio(&geom, 1.2, &torso_dominance);
    double torso_scale = 1.0 / (1.0 + (torso_dominance * 0.3));
    
    // Evaluate Hyde turn-taking conversational latency to delay/sustain the attack curve
    double conversational_latency = 0.0;
    evaluate_hyde_conversational_latency(&geom, 1.5, &conversational_latency);
    double latency_attack_delay = conversational_latency * 0.05;
    
    // Evaluate Keating chin asymmetry dilation constraints to scale response envelopes
    double chin_dilation = 0.0;
    evaluate_keating_chin_asymmetry_dilation(&geom, 1.2, &chin_dilation);
    double dilation_scale = 1.0 + (chin_dilation * 0.15);
    
    // Evaluate Geniole fWHR provocation exposure retaliation decay to sustain decay curves
    double retaliation_decay = 0.0;
    evaluate_geniole_fwhr_retaliation_decay(&geom, 10.0, &retaliation_decay);
    double decay_sustain_scale = 1.0 + (retaliation_decay * 0.20);
    
    // Evaluate Scarpi utilitarian task-oriented orientation to scale response envelopes
    double utilitarian_efficiency = 0.0;
    evaluate_scarpi_utilitarian_orientation(&geom, 0.7, &utilitarian_efficiency);
    double utilitarian_scale = 1.0 / (1.0 + (utilitarian_efficiency * 0.25));
    
    // Set custom envelope curves based on physical dynamics
    model->envelope_attack = ((0.02 + (geom.stiffness * 0.05)) * envelope_scale * torso_scale + latency_attack_delay) * dilation_scale * utilitarian_scale;
    model->envelope_decay = (0.05 + (geom.damping * 0.1)) * envelope_scale * torso_scale * dilation_scale * decay_sustain_scale * utilitarian_scale;
    
    // Evaluate head posture pitch tilt submissiveness to damp amplitude baseline
    double submissive_rating = 0.0;
    evaluate_keating_posture_pitch(&geom, -15.0, &submissive_rating);
    
    // Evaluate Hyde conversational amplitude mismatch to damp base audio levels
    double amplitude_mismatch = 0.0;
    evaluate_hyde_vocal_amplitude_mismatch(&geom, 75.0, &amplitude_mismatch);
    
    // Set dynamic baseline amplitude factor (softer/damped for submissive or mismatched profiles)
    model->amplitude_factor = 12000.0 * (1.0 - (submissive_rating * 0.25)) * (1.0 - (amplitude_mismatch * 0.20));
    
    // Evaluate Geniole fWHR boundary mapping constraints
    double fwhr_boundary = 0.0;
    evaluate_geniole_fwhr_boundary_map(&geom, 1.5, &fwhr_boundary);
    
    // Evaluate Cellarius constellation boundary mapping constraints
    double constellation_boundary = 0.0;
    evaluate_cellarius_constellation_boundary(&geom, 120.0, &constellation_boundary);
    
    // Initialize congruent parameters for Wald-gate validation (non-significant p-value)
    model->wald_beta[0] = 0.1 + (fwhr_boundary * 0.05);
    model->wald_beta[1] = 0.1 + (constellation_boundary * 0.05);
    model->wald_beta[2] = 0.1;
    
    memset(model->wald_covariance, 0, sizeof(model->wald_covariance));
    model->wald_covariance[0] = 1.0;
    model->wald_covariance[4] = 1.0;
    model->wald_covariance[8] = 1.0;
}

// Forward declaration of the diagnostics function
bool evaluate_wald_nominal_test(const double *beta_vector, const double *covariance_matrix, int df, double *wald_stat_out, double *p_value_out);

// Forward declarations of link, diagnostics, and models functions
int evaluate_ordinal_cloglog_rating(const teddy_bear_geometry_t *geom);
bool evaluate_keating_brow_dominance(const teddy_bear_geometry_t *geom, double brow_height, double *brow_dominance_out);
bool evaluate_kramer_ward_human_face_elongation(const teddy_bear_geometry_t *geom, double elongation_val, double *elongation_score_out);
bool evaluate_surrogate_residuals(const teddy_bear_geometry_t *geom, int observed_rating, double *residual_out);
bool evaluate_scarpi_hedonic_orientation(const teddy_bear_geometry_t *geom, double playfulness_scale, double *hedonic_out);
bool evaluate_cellarius_planetary_eccentricity(const teddy_bear_geometry_t *geom, double eccentricity_ratio, double *translation_offset_out);
bool evaluate_hyde_vocal_warmth_pitch(const teddy_bear_geometry_t *geom, double average_pitch_hz, double *warmth_offset_out);
bool evaluate_hyde_vocal_tremor_index(const teddy_bear_geometry_t *geom, double pitch_variance, double *tremor_uncanny_out);
bool evaluate_keating_mouth_curvature(const teddy_bear_geometry_t *geom, double upturn_curvature, double *warmth_rating_out);
bool evaluate_keating_babyfacedness_index(const teddy_bear_geometry_t *geom, double *babyfacedness_out);
bool evaluate_hyde_mouth_speed_synchrony(const teddy_bear_geometry_t *geom, double mouth_speed, double pitch_acceleration, double *sync_mismatch_out);

// Forward declarations of system transaction controls
evaluation_tx_t begin_evaluation_transaction(teddy_bear_geometry_t *target);
bool commit_evaluation_transaction(evaluation_tx_t *tx);
void rollback_evaluation_transaction(evaluation_tx_t *tx);

bool tsfi_speech_synth_generate(const tsfi_speech_model_t *model, 
                                double duration, 
                                uint32_t sample_rate, 
                                int16_t *buffer, 
                                uint32_t buffer_size) {
    uint32_t total_samples = (uint32_t)(duration * sample_rate);
    
    // Cast away const to begin evaluation transaction safeguard
    teddy_bear_geometry_t dummy_geom;
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

teddy_bear_personality_t tsfi_speech_classify_pitch(double pitch_frequency) {
    // Classification logic mapping pitch boundaries to profile sets
    if (pitch_frequency > 220.0) {
        return PERSONALITY_TRUSTWORTHY;
    } else if (pitch_frequency > 150.0) {
        return PERSONALITY_SKEPTICAL;
    } else {
        // Distinguish between aggressive and eerie using known profile values
        // Both are set to 85Hz, so default to aggressive
        return PERSONALITY_AGGRESSIVE;
    }
}
