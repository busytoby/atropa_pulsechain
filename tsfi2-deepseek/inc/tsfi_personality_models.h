#ifndef TSFI_PERSONALITY_MODELS_H
#define TSFI_PERSONALITY_MODELS_H

#include "tsfi_personality_core.h"

// 1. Geniole et al. Threat & Retaliation Models
bool evaluate_behavioral_threat_mismatch(const teddy_geometry_t *geom, double *mismatch_score);
double evaluate_reactive_mismatch_retaliation(const teddy_geometry_t *geom);
bool evaluate_exposure_threat_consistency(const teddy_geometry_t *geom, double exposure_ms, double *perceived_threat_out);
bool evaluate_cooperative_cheating_risk(const teddy_geometry_t *geom, double social_trust_factor, double *cheating_risk_out);
bool evaluate_rapid_threat_limit(const teddy_geometry_t *geom, double exposure_ms, double *detected_threat_out);
bool evaluate_reactive_retaliation_profile(const teddy_geometry_t *geom, double provocation_intensity, double *retaliation_out);
bool evaluate_provocation_aggression_threshold(const teddy_geometry_t *geom, double base_provocation, double *aggression_threshold_out);
bool evaluate_exposure_adjusted_threat_threshold(const teddy_geometry_t *geom, double exposure_ms, double *adjusted_threat_threshold_out);
bool evaluate_status_dominance_provocation(const teddy_geometry_t *geom, double provocation_scale, double *dominance_threat_out);
bool evaluate_threat_replication_consistency(const teddy_geometry_t *geom, const double *threat_observations, int count, double *consistency_out);
bool evaluate_reactive_retaliation_boundary(const teddy_geometry_t *geom, double provocation_scale, double *retaliation_boundary_out);
bool evaluate_retaliation_aggression_scaling(const teddy_geometry_t *geom, double baseline_aggression, double *scaled_aggression_out);
bool evaluate_retaliation_threshold_decay(const teddy_geometry_t *geom, double exposure_ms, double *decayed_threshold_out);
bool evaluate_provocation_exposure_decay(const teddy_geometry_t *geom, double provocation_scale, double exposure_ms, double *decayed_threat_out);
bool evaluate_retaliation_aggression_ceiling(const teddy_geometry_t *geom, double provocation_scale, double *aggression_ceiling_out);
bool evaluate_status_dominance_multiplier(const teddy_geometry_t *geom, double provocation_scale, double *multiplier_out);
bool evaluate_cheating_threat_index(const teddy_geometry_t *geom, double provocation_scale, double *cheating_threat_out);
bool evaluate_provocation_replication_consistency(const teddy_geometry_t *geom, double replication_scale, double *consistency_out);
bool evaluate_provocation_rapid_threat_limit(const teddy_geometry_t *geom, double provocation_scale, double *limit_out);
bool evaluate_provocation_retaliation_boundary(const teddy_geometry_t *geom, double exposure_ms, double *boundary_out);

// 2. Hyde et al. Uncanny Valley Models
bool evaluate_uncanny_mismatch_index(const teddy_geometry_t *geom, double *uncanny_score_out);
bool evaluate_motion_uncanny_index(const teddy_geometry_t *geom, double movement_stiffness, double *motion_uncanny_out);
bool evaluate_static_uncanny_index(const teddy_geometry_t *geom, double *static_uncanny_out);
bool evaluate_expression_uncanny_index(const teddy_geometry_t *geom, double expression_intensity, double *expression_uncanny_out);
bool evaluate_feature_vertical_uncanny_index(const teddy_geometry_t *geom, double feature_offset, double *vertical_uncanny_out);
bool evaluate_gaze_direction_uncanny_index(const teddy_geometry_t *geom, double gaze_deviation, double *gaze_uncanny_out);
bool evaluate_microexpression_uncanny_index(const teddy_geometry_t *geom, double rate_hz, double *micro_uncanny_out);
bool evaluate_voice_uncanny_index(const teddy_geometry_t *geom, double pitch_hz, double *voice_uncanny_out);
bool evaluate_skin_texture_uncanny_index(const teddy_geometry_t *geom, double roughness, double *skin_uncanny_out);
bool evaluate_hair_uncanny_index(const teddy_geometry_t *geom, double density, double *hair_uncanny_out);
bool evaluate_proportions_uncanny_index(const teddy_geometry_t *geom, double scale_ratio, double *proportions_uncanny_out);
bool evaluate_respiration_uncanny_index(const teddy_geometry_t *geom, double rate_pm, double *respiration_uncanny_out);
bool evaluate_asymmetry_uncanny_index(const teddy_geometry_t *geom, double asymmetry_val, double *asymmetry_uncanny_out);
bool evaluate_tactile_uncanny_index(const teddy_geometry_t *geom, double compliance, double *tactile_uncanny_out);
bool evaluate_posture_uncanny_index(const teddy_geometry_t *geom, double slouch_angle, double *posture_uncanny_out);
bool evaluate_thermal_uncanny_index(const teddy_geometry_t *geom, double temp_celsius, double *thermal_uncanny_out);
bool evaluate_scent_uncanny_index(const teddy_geometry_t *geom, double scent_intensity, double *scent_uncanny_out);
bool evaluate_gait_uncanny_index(const teddy_geometry_t *geom, double gait_velocity, double *gait_uncanny_out);
bool evaluate_shadow_uncanny_index(const teddy_geometry_t *geom, double shadow_angle, double *shadow_uncanny_out);
bool evaluate_pupil_uncanny_index(const teddy_geometry_t *geom, double pupil_dilation, double *pupil_uncanny_out);
bool evaluate_blink_uncanny_index(const teddy_geometry_t *geom, double blink_freq, double *blink_uncanny_out);
bool evaluate_voice_speech_uncanny_index(const teddy_geometry_t *geom, double speech_coherence, double *speech_uncanny_out);

// 3. Keating et al. Social Dominance & Facial Asymmetry Models
bool evaluate_keating_gaze_dominance(const teddy_geometry_t *geom, double gaze_duration_sec, double *dominance_out);
bool evaluate_keating_lip_compression_trust(const teddy_geometry_t *geom, double compression_ratio, double *trust_out);
bool evaluate_keating_width_asymmetry_dominance(const teddy_geometry_t *geom, double asymmetry_val, double *dominance_out);
bool evaluate_keating_mouth_asymmetry_trust_decay(const teddy_geometry_t *geom, double duration_sec, double *decay_out);
bool evaluate_keating_width_asymmetry_trust(const teddy_geometry_t *geom, double asymmetry_val, double *trust_out);
bool evaluate_keating_mouth_asymmetry_dominance(const teddy_geometry_t *geom, double asymmetry_val, double *dominance_out);
bool evaluate_keating_eyebrow_asymmetry_dominance(const teddy_geometry_t *geom, double asymmetry_val, double *dominance_out);
bool evaluate_keating_sclera_submissiveness(const teddy_geometry_t *geom, double exposure_ratio, double *submissiveness_out);
bool evaluate_keating_eyebrow_asymmetry_trust(const teddy_geometry_t *geom, double asymmetry_val, double *trust_out);
bool evaluate_keating_sclera_dominance(const teddy_geometry_t *geom, double exposure_ratio, double *dominance_out);
bool evaluate_keating_eyebrow_asymmetry_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_trust_out);
bool evaluate_keating_sclera_submissiveness_modulator(const teddy_geometry_t *geom, double baseline, double *modulated_submissiveness_out);
bool evaluate_keating_eyebrow_asymmetry_dominance_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_dominance_out);
bool evaluate_keating_sclera_dominance_variance(const teddy_geometry_t *geom, double exposure_ratio, double *variance_out);
bool evaluate_keating_eyebrow_asymmetry_trust_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_trust_out);
bool evaluate_keating_sclera_submissiveness_variance(const teddy_geometry_t *geom, double exposure_ratio, double *variance_out);
bool evaluate_keating_gaze_shift_dominance_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_dominance_out);
bool evaluate_keating_lip_compression_trust_variance(const teddy_geometry_t *geom, double compression_ratio, double *variance_out);
bool evaluate_keating_sclera_submissiveness_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_submissiveness_out);
bool evaluate_keating_width_asymmetry_dominance_variance(const teddy_geometry_t *geom, double asymmetry_val, double *variance_out);
bool evaluate_keating_sclera_dominance_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_dominance_out);
bool evaluate_keating_mouth_asymmetry_dominance_variance(const teddy_geometry_t *geom, double asymmetry_val, double *variance_out);
bool evaluate_keating_eyebrow_asymmetry_dominance_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_keating_mouth_asymmetry_trust_variance(const teddy_geometry_t *geom, double asymmetry_val, double *variance_out);
bool evaluate_keating_gaze_shift_dominance_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_keating_lip_compression_trust_variance_mod(const teddy_geometry_t *geom, double compression_ratio, double *variance_out);
bool evaluate_keating_eyebrow_asymmetry_trust_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_keating_sclera_dominance_variance_mod(const teddy_geometry_t *geom, double exposure_ratio, double *variance_out);
bool evaluate_keating_sclera_dominance_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_keating_eyebrow_asymmetry_dominance_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_keating_sclera_submissiveness_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_keating_sclera_dominance_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// 4. Scarpi Playful & Utilitarian Models
bool evaluate_scarpi_playfulness(const teddy_geometry_t *geom, double playfulness_score, double *rating_out);
bool evaluate_scarpi_utilitarian_trust(const teddy_geometry_t *geom, double utilitarian_score, double *rating_out);
bool evaluate_scarpi_hedonic_arousal_mod(const teddy_geometry_t *geom, double arousal_val, double *rating_out);
bool evaluate_scarpi_utilitarian_quality(const teddy_geometry_t *geom, double quality_val, double *rating_out);
bool evaluate_scarpi_aesthetic_quality(const teddy_geometry_t *geom, double quality_val, double *rating_out);
bool evaluate_scarpi_structural_consistency(const teddy_geometry_t *geom, double consistency_val, double *rating_out);
bool evaluate_scarpi_interface_trust(const teddy_geometry_t *geom, double trust_val, double *rating_out);
bool evaluate_scarpi_operational_trust_mod(const teddy_geometry_t *geom, double operational_val, double *rating_out);
bool evaluate_scarpi_playful_engagement(const teddy_geometry_t *geom, double engagement_val, double *rating_out);
bool evaluate_scarpi_structural_consistency_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_consistency_out);
bool evaluate_scarpi_playful_arousal_variance(const teddy_geometry_t *geom, double arousal_val, double *variance_out);
bool evaluate_scarpi_operational_trust_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_trust_out);
bool evaluate_scarpi_structural_consistency_variance(const teddy_geometry_t *geom, double consistency_val, double *variance_out);
bool evaluate_scarpi_interface_trust_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_trust_out);
bool evaluate_scarpi_interface_trust_efficiency(const teddy_geometry_t *geom, double efficiency_val, double *rating_out);
bool evaluate_scarpi_aesthetic_trust_variance(const teddy_geometry_t *geom, double aesthetic_val, double *variance_out);
bool evaluate_scarpi_quality_control_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_quality_out);
bool evaluate_scarpi_playful_engagement_variance(const teddy_geometry_t *geom, double engagement_val, double *variance_out);
bool evaluate_scarpi_consistency_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_scarpi_playful_arousal_mod_variance(const teddy_geometry_t *geom, double arousal_val, double *variance_out);
bool evaluate_scarpi_quality_control_variance(const teddy_geometry_t *geom, double quality_val, double *variance_out);
bool evaluate_scarpi_playful_arousal_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_scarpi_quality_control_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_scarpi_playful_engagement_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_scarpi_structural_consistency_variance_mod(const teddy_geometry_t *geom, double consistency_val, double *variance_out);
bool evaluate_scarpi_quality_control_variance_mod(const teddy_geometry_t *geom, double quality_val, double *variance_out);
bool evaluate_scarpi_quality_control_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_scarpi_playful_arousal_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_scarpi_operational_trust_variance_mod(const teddy_geometry_t *geom, double operational_val, double *variance_out);
bool evaluate_scarpi_playful_engagement_variance_mod(const teddy_geometry_t *geom, double engagement_val, double *variance_out);
bool evaluate_scarpi_operational_consistency_variance_mod(const teddy_geometry_t *geom, double consistency_val, double *variance_out);
bool evaluate_scarpi_playful_engagement_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_scarpi_consistency_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_scarpi_operational_trust_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_scarpi_interface_trust_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// 5. Cellarius Planetary & Astrological Geometry Models
bool evaluate_cellarius_planetary_velocity(const teddy_geometry_t *geom, double velocity_val, double *rating_out);
bool evaluate_cellarius_heliocentric_alignment(const teddy_geometry_t *geom, double alignment_val, double *rating_out);
bool evaluate_cellarius_constellation_scale_variance(const teddy_geometry_t *geom, double scale_val, double *variance_out);
bool evaluate_cellarius_epicycle_radius_decay(const teddy_geometry_t *geom, double duration_sec, double *decay_out);
bool evaluate_cellarius_epicycle_radius_variance(const teddy_geometry_t *geom, double radius_val, double *variance_out);
bool evaluate_cellarius_constellation_scale_decay(const teddy_geometry_t *geom, double duration_sec, double *decay_out);
bool evaluate_cellarius_planetary_velocity_decay(const teddy_geometry_t *geom, double duration_sec, double *decay_out);
bool evaluate_cellarius_heliocentric_alignment_decay(const teddy_geometry_t *geom, double duration_sec, double *decay_out);
bool evaluate_cellarius_planetary_velocity_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_cellarius_heliocentric_alignment_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_cellarius_planetary_velocity_variance_mod(const teddy_geometry_t *geom, double velocity_val, double *variance_out);
bool evaluate_cellarius_heliocentric_alignment_variance_mod(const teddy_geometry_t *geom, double alignment_val, double *variance_out);
bool evaluate_cellarius_constellation_scale_variance_mod_single(const teddy_geometry_t *geom, double scale_val, double *variance_out);
bool evaluate_cellarius_epicycle_radius_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// 6. Hyde et al. Vocal Signal Alignment Models
bool evaluate_hyde_dynamic_intonation(const teddy_geometry_t *geom, double intonation_range, double brow_movement, double *intonation_out);
bool evaluate_hyde_intonation_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_intonation_out);
bool evaluate_hyde_tempo_sync(const teddy_geometry_t *geom, double tempo_sync_val, double *rating_out);
bool evaluate_hyde_tempo_jitter_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_jitter_out);
bool evaluate_hyde_tempo_range(const teddy_geometry_t *geom, double tempo_range_val, double brow_movement, double *tempo_range_out);
bool evaluate_hyde_tempo_range_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_range_out);
bool evaluate_hyde_vocal_size_sync(const teddy_geometry_t *geom, double tempo_sync_val, double size_mismatch, double *sync_out);
bool evaluate_hyde_vocal_size_jitter_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_size_jitter_out);
bool evaluate_hyde_vocal_size_pitch(const teddy_geometry_t *geom, double size_variance, double pitch_range, double *pitch_out);
bool evaluate_hyde_vocal_size_pitch_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_pitch_out);
bool evaluate_hyde_vocal_size_sync_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_sync_out);
bool evaluate_hyde_vocal_warmth_variance_mod(const teddy_geometry_t *geom, double warmth_val, double *variance_out);
bool evaluate_hyde_vocal_tempo_range_variance_mod(const teddy_geometry_t *geom, double tempo_range_val, double *variance_out);
bool evaluate_hyde_vocal_size_jitter_variance_mod(const teddy_geometry_t *geom, double size_jitter_val, double *variance_out);
bool evaluate_hyde_vocal_warmth_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_hyde_vocal_tempo_range_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_hyde_vocal_size_jitter_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_hyde_vocal_tempo_sync_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_hyde_vocal_size_pitch_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_hyde_vocal_size_sync_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// 7. Castle et al. Diplomatic & Policy Alignment Models
bool evaluate_castle_diplomatic_alignment(const teddy_geometry_t *geom, double turn_taking_sync, double posture_alignment, double *alignment_out);
bool evaluate_castle_policy_consistency(const teddy_geometry_t *geom, double consistency_score, double *policy_consistency_out);
bool evaluate_castle_diplomatic_alignment_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_alignment_out);
bool evaluate_castle_policy_consistency_variance(const teddy_geometry_t *geom, double consistency_score, double *variance_out);
bool evaluate_castle_policy_target_engagement(const teddy_geometry_t *geom, double target_metric, double *engagement_out);
bool evaluate_castle_diplomatic_trust_bounds(const teddy_geometry_t *geom, double visual_symmetry, double vocal_symmetry, double *bounds_out);
bool evaluate_castle_diplomatic_trust_bounds_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_bounds_out);
bool evaluate_castle_policy_target_engagement_variance(const teddy_geometry_t *geom, double target_metric, double *variance_out);
bool evaluate_castle_diplomatic_trust_bounds_variance(const teddy_geometry_t *geom, double visual_symmetry, double vocal_symmetry, double *bounds_variance_out);
bool evaluate_castle_policy_target_engagement_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_engagement_out);
bool evaluate_castle_diplomatic_alignment_variance(const teddy_geometry_t *geom, double visual_symmetry, double vocal_symmetry, double *alignment_variance_out);
bool evaluate_castle_policy_consistency_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_consistency_out);
bool evaluate_castle_diplomatic_alignment_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_castle_policy_consistency_variance_mod(const teddy_geometry_t *geom, double consistency_score, double *variance_out);
bool evaluate_castle_diplomatic_trust_bounds_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_castle_policy_target_engagement_variance_mod(const teddy_geometry_t *geom, double engagement_score, double *variance_out);
bool evaluate_castle_diplomatic_alignment_variance_mod(const teddy_geometry_t *geom, double alignment_score, double *variance_out);
bool evaluate_castle_policy_consistency_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_castle_diplomatic_trust_bounds_variance_mod(const teddy_geometry_t *geom, double bounds_score, double *variance_out);
bool evaluate_castle_policy_target_engagement_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_castle_policy_consistency_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_castle_diplomatic_alignment_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_castle_diplomatic_trust_bounds_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_castle_policy_target_engagement_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// 8. Kramer & Ward fWHR Models
bool evaluate_kramer_ward_fwhr_dominance(const teddy_geometry_t *geom, double fwhr_val, double *dominance_out);
bool evaluate_kramer_ward_fwhr_trustworthiness(const teddy_geometry_t *geom, double fwhr_val, double *trustworthiness_out);
bool evaluate_kramer_ward_fwhr_dominance_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_kramer_ward_fwhr_trustworthiness_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_kramer_ward_fwhr_threat(const teddy_geometry_t *geom, double fwhr_val, double *threat_out);
bool evaluate_kramer_ward_fwhr_cooperation(const teddy_geometry_t *geom, double fwhr_val, double *cooperation_out);
bool evaluate_kramer_ward_fwhr_threat_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_kramer_ward_fwhr_cooperation_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);
bool evaluate_kramer_ward_fwhr_3d_scan_discrepancy(const teddy_geometry_t *geom, double fwhr_val, double *discrepancy_out);
bool evaluate_kramer_ward_fwhr_sexual_dimorphism_null_hypothesis(const teddy_geometry_t *geom, double male_fwhr, double female_fwhr, double *p_value_out);
bool evaluate_kramer_ward_fwhr_head_tilt_effect(const teddy_geometry_t *geom, double fwhr_val, double pitch_degrees, double *apparent_fwhr_out);
bool evaluate_kramer_ward_fwhr_actual_vs_perceived_aggression_mismatch(const teddy_geometry_t *geom, double perceived_dominance, double actual_aggression, double *mismatch_out);
bool evaluate_kramer_ward_fwhr_perceptual_noise_sensitivity(const teddy_geometry_t *geom, double noise_level, double *threshold_se_multiplier_out);
bool evaluate_kramer_ward_fwhr_sequential_adaptation_bias(const teddy_geometry_t *geom, const double *recent_fwhr_history, int history_len, double *adaptation_offset_out);
bool evaluate_kramer_ward_fwhr_hemiface_asymmetry_bias(const teddy_geometry_t *geom, double left_fwhr, double right_fwhr, double *weighted_fwhr_out);
bool evaluate_kramer_ward_fwhr_ambient_shading_effect(const teddy_geometry_t *geom, double original_fwhr, double light_angle_degrees, double *apparent_fwhr_out);

#endif // TSFI_PERSONALITY_MODELS_H
