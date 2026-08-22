#include "auncient_teddy_bear_personality.h"
#include <stdio.h>
#include <math.h>

bool evaluate_kramer_ward_fwhr_dominance(const teddy_bear_geometry_t *geom, double fwhr_val, double *dominance_out) {
    if (!geom || fwhr_val < 0.0 || !dominance_out) {
        return false;
    }
    *dominance_out = fwhr_val * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_kramer_ward_fwhr_trustworthiness(const teddy_bear_geometry_t *geom, double fwhr_val, double *trustworthiness_out) {
    if (!geom || fwhr_val < 0.0 || !trustworthiness_out) {
        return false;
    }
    // High FWHR correlates with lower trust perception in visual bounds
    *trustworthiness_out = (1.0 / (1.0 + fwhr_val)) * (1.2 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_kramer_ward_fwhr_dominance_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (14.0 + geom->resilience_index * 4.0));
    return true;
}

bool evaluate_kramer_ward_fwhr_trustworthiness_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (14.0 + geom->resilience_index * 4.0));
    return true;
}

bool evaluate_kramer_ward_fwhr_threat(const teddy_bear_geometry_t *geom, double fwhr_val, double *threat_out) {
    if (!geom || fwhr_val < 0.0 || !threat_out) {
        return false;
    }
    *threat_out = fwhr_val * (1.3 + geom->behavioral_mismatch * 0.3);
    return true;
}

bool evaluate_kramer_ward_fwhr_cooperation(const teddy_bear_geometry_t *geom, double fwhr_val, double *cooperation_out) {
    if (!geom || fwhr_val < 0.0 || !cooperation_out) {
        return false;
    }
    // High fWHR is negatively associated with cooperative behavior
    *cooperation_out = (1.0 / (1.0 + fwhr_val)) * (1.4 + geom->cooperative_negotiation * 0.3);
    return true;
}

bool evaluate_kramer_ward_fwhr_threat_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (15.0 + geom->resilience_index * 5.0));
    return true;
}

bool evaluate_kramer_ward_fwhr_cooperation_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (15.0 + geom->resilience_index * 5.0));
    return true;
}

bool evaluate_kramer_ward_fwhr_3d_scan_discrepancy(const teddy_bear_geometry_t *geom, double fwhr_val, double *discrepancy_out) {
    if (!geom || fwhr_val < 0.0 || !discrepancy_out) {
        return false;
    }
    *discrepancy_out = fwhr_val * 0.08 * (1.0 + geom->behavioral_mismatch * 0.1);
    return true;
}

bool evaluate_kramer_ward_fwhr_sexual_dimorphism_null_hypothesis(const teddy_bear_geometry_t *geom, double male_fwhr, double female_fwhr, double *p_value_out) {
    if (!geom || male_fwhr < 0.0 || female_fwhr < 0.0 || !p_value_out) {
        return false;
    }
    double diff = fabs(male_fwhr - female_fwhr);
    *p_value_out = exp(-diff * 5.0) * (0.8 + geom->cooperative_negotiation * 0.1);
    return true;
}

bool evaluate_kramer_ward_fwhr_head_tilt_effect(const teddy_bear_geometry_t *geom, double fwhr_val, double pitch_degrees, double *apparent_fwhr_out) {
    if (!geom || fwhr_val < 0.0 || !apparent_fwhr_out) {
        return false;
    }
    double rad = pitch_degrees * 3.141592653589793 / 180.0;
    *apparent_fwhr_out = fwhr_val / cos(rad * 0.5) * (1.0 + geom->head_tilt * 0.05);
    return true;
}

bool evaluate_kramer_ward_fwhr_actual_vs_perceived_aggression_mismatch(const teddy_bear_geometry_t *geom, double perceived_dominance, double actual_aggression, double *mismatch_out) {
    if (!geom || perceived_dominance < 0.0 || actual_aggression < 0.0 || !mismatch_out) {
        return false;
    }
    *mismatch_out = fabs(perceived_dominance - actual_aggression) * (1.2 + geom->behavioral_mismatch * 0.4);
    return true;
}

bool evaluate_kramer_ward_fwhr_perceptual_noise_sensitivity(const teddy_bear_geometry_t *geom, double noise_level, double *threshold_se_multiplier_out) {
    if (!geom || noise_level < 0.0 || !threshold_se_multiplier_out) {
        return false;
    }
    *threshold_se_multiplier_out = (1.0 + noise_level * 2.5) * (1.0 + geom->behavioral_mismatch * 0.2);
    return true;
}

bool evaluate_kramer_ward_fwhr_sequential_adaptation_bias(const teddy_bear_geometry_t *geom, const double *recent_fwhr_history, int history_len, double *adaptation_offset_out) {
    if (!geom || !recent_fwhr_history || history_len <= 0 || !adaptation_offset_out) {
        return false;
    }
    double sum = 0.0;
    for (int i = 0; i < history_len; i++) {
        sum += recent_fwhr_history[i];
    }
    double avg = sum / (double)history_len;
    *adaptation_offset_out = (avg - 1.8) * 0.15 * (1.0 + geom->head_fwhr * 0.1);
    return true;
}

bool evaluate_kramer_ward_fwhr_hemiface_asymmetry_bias(const teddy_bear_geometry_t *geom, double left_fwhr, double right_fwhr, double *weighted_fwhr_out) {
    if (!geom || left_fwhr < 0.0 || right_fwhr < 0.0 || !weighted_fwhr_out) {
        return false;
    }
    // Weighted towards the right hemiface due to left visual field bias
    *weighted_fwhr_out = (left_fwhr * 0.4 + right_fwhr * 0.6) * (1.0 + (1.0 - geom->symmetry) * 0.05);
    return true;
}

bool evaluate_kramer_ward_fwhr_ambient_shading_effect(const teddy_bear_geometry_t *geom, double original_fwhr, double light_angle_degrees, double *apparent_fwhr_out) {
    if (!geom || original_fwhr < 0.0 || !apparent_fwhr_out) {
        return false;
    }
    double rad = light_angle_degrees * 3.141592653589793 / 180.0;
    *apparent_fwhr_out = original_fwhr * (1.0 + sin(rad) * 0.04 * (1.0 - geom->fur_roughness * 0.2));
    return true;
}

bool evaluate_kramer_ward_fwhr_dynamic_temporal_smoothing(const teddy_bear_geometry_t *geom, const double *frame_fwhr_sequence, int sequence_len, double *smoothed_fwhr_out) {
    if (!geom || !frame_fwhr_sequence || sequence_len <= 0 || !smoothed_fwhr_out) {
        return false;
    }
    double sum = 0.0;
    for (int i = 0; i < sequence_len; i++) {
        sum += frame_fwhr_sequence[i];
    }
    *smoothed_fwhr_out = (sum / (double)sequence_len) * (1.0 + geom->behavioral_mismatch * 0.05);
    return true;
}

bool evaluate_kramer_ward_fwhr_judgment_confidence_calibration(const teddy_bear_geometry_t *geom, double fwhr_val, double *confidence_rating_out) {
    if (!geom || fwhr_val < 0.0 || !confidence_rating_out) {
        return false;
    }
    double deviation = fabs(fwhr_val - 1.85);
    *confidence_rating_out = (1.0 / (1.0 + deviation * 1.5)) * (0.9 + geom->intellect_index * 0.1);
    return true;
}

bool evaluate_kramer_ward_fwhr_perceived_vs_actual_health(const teddy_bear_geometry_t *geom, double perceived_health, double actual_health, double *mismatch_out) {
    if (!geom || perceived_health < 0.0 || actual_health < 0.0 || !mismatch_out) {
        return false;
    }
    *mismatch_out = fabs(perceived_health - actual_health) * (1.1 + geom->behavioral_mismatch * 0.2);
    return true;
}

bool evaluate_kramer_ward_fwhr_cheekbone_prominence_modulation(const teddy_bear_geometry_t *geom, double cheekbone_val, double *prominence_mod_out) {
    if (!geom || cheekbone_val < 0.0 || !prominence_mod_out) {
        return false;
    }
    *prominence_mod_out = cheekbone_val * 0.95 * (1.0 + (1.0 - geom->symmetry) * 0.15);
    return true;
}

bool evaluate_kramer_ward_fwhr_perspective_distortion(const teddy_bear_geometry_t *geom, double fwhr_val, double focal_length_mm, double *distorted_fwhr_out) {
    if (!geom || fwhr_val < 0.0 || focal_length_mm <= 0.0 || !distorted_fwhr_out) {
        return false;
    }
    // Shorter focal lengths distort (widen) the apparent fWHR
    double distortion_factor = 1.0 + (35.0 / focal_length_mm) * 0.08;
    *distorted_fwhr_out = fwhr_val * distortion_factor * (1.0 + geom->head_fwhr * 0.02);
    return true;
}

bool evaluate_kramer_ward_fwhr_micro_exposure_resolution(const teddy_bear_geometry_t *geom, double exposure_ms, double *resolution_accuracy_out) {
    if (!geom || exposure_ms < 0.0 || !resolution_accuracy_out) {
        return false;
    }
    // Trait resolution is sigmoid-gated by exposure duration up to ~500ms
    double k = 0.015; // sigmoid slope constant
    *resolution_accuracy_out = (1.0 / (1.0 + exp(-k * (exposure_ms - 100.0)))) * (0.85 + geom->intellect_index * 0.1);
    return true;
}

bool evaluate_kramer_ward_human_fwhr_trustworthiness(const teddy_bear_geometry_t *geom, double fwhr_val, double *trustworthiness_out) {
    if (!geom || fwhr_val < 0.0 || !trustworthiness_out) {
        return false;
    }
    // Higher fWHR (wider face) tends to be associated with lower perceived trustworthiness in certain scenarios
    *trustworthiness_out = (1.0 / (1.0 + fwhr_val * 0.2)) * (0.95 + geom->cooperative_negotiation * 0.1);
    return true;
}

bool evaluate_kramer_ward_human_face_asymmetry_aggression(const teddy_bear_geometry_t *geom, double asymmetry_val, double *aggression_rating_out) {
    if (!geom || asymmetry_val < 0.0 || !aggression_rating_out) {
        return false;
    }
    *aggression_rating_out = asymmetry_val * 1.12 * (1.0 + geom->confrontational_assertiveness * 0.15);
    return true;
}

bool evaluate_kramer_ward_human_eye_to_face_proportion(const teddy_bear_geometry_t *geom, double eye_size_val, double pupil_dilation_val, double *social_trait_rating_out) {
    if (!geom || eye_size_val < 0.0 || pupil_dilation_val < 0.0 || !social_trait_rating_out) {
        return false;
    }
    *social_trait_rating_out = (eye_size_val * 0.65 + pupil_dilation_val * 0.35) * (0.9 + geom->social_extraversion * 0.1);
    return true;
}

bool evaluate_kramer_ward_human_fwhr_dominance(const teddy_bear_geometry_t *geom, double fwhr_val, double *dominance_out) {
    if (!geom || fwhr_val < 0.0 || !dominance_out) {
        return false;
    }
    *dominance_out = fwhr_val * 1.05 * (0.95 + geom->command_authority * 0.1);
    return true;
}

bool evaluate_kramer_ward_human_face_elongation(const teddy_bear_geometry_t *geom, double elongation_val, double *elongation_score_out) {
    if (!geom || elongation_val < 0.0 || !elongation_score_out) {
        return false;
    }
    *elongation_score_out = (1.0 / (1.0 + elongation_val)) * 1.4 * (0.9 + geom->intellect_index * 0.15);
    return true;
}

bool evaluate_kramer_ward_human_eye_to_brow_trust(const teddy_bear_geometry_t *geom, double eye_to_brow_distance, double *trust_out) {
    if (!geom || eye_to_brow_distance < 0.0 || !trust_out) {
        return false;
    }
    *trust_out = eye_to_brow_distance * 1.15 * (0.95 + geom->cooperative_negotiation * 0.1);
    return true;
}

bool evaluate_kramer_ward_human_fwhr_trustworthiness_decay(const teddy_bear_geometry_t *geom, double initial_trust, double duration_sec, double *decayed_trust_out) {
    if (!geom || initial_trust < 0.0 || duration_sec < 0.0 || !decayed_trust_out) {
        return false;
    }
    *decayed_trust_out = initial_trust * exp(-duration_sec / (20.0 + geom->resilience_index * 5.0));
    return true;
}

bool evaluate_kramer_ward_human_face_symmetry_trust(const teddy_bear_geometry_t *geom, double symmetry_val, double *trustworthiness_out) {
    if (!geom || symmetry_val < 0.0 || !trustworthiness_out) {
        return false;
    }
    *trustworthiness_out = symmetry_val * 1.10 * (0.95 + geom->cooperative_negotiation * 0.1);
    return true;
}

bool evaluate_kramer_ward_human_eye_dilation_arousal(const teddy_bear_geometry_t *geom, double eye_size_val, double pupil_dilation_val, double *arousal_out) {
    if (!geom || eye_size_val < 0.0 || pupil_dilation_val < 0.0 || !arousal_out) {
        return false;
    }
    *arousal_out = (eye_size_val * 0.5 + pupil_dilation_val * 0.5) * (0.95 + geom->social_extraversion * 0.1);
    return true;
}

bool evaluate_kramer_king_ward_perceived_agreeableness_consensus(const teddy_bear_geometry_t *geom, double mouth_curvature, double eye_size, double *consensus_out) {
    if (!geom || mouth_curvature < 0.0 || eye_size < 0.0 || !consensus_out) {
        return false;
    }
    *consensus_out = (mouth_curvature * 0.6 + eye_size * 0.4) * (0.9 + geom->empathy_index * 0.2);
    return true;
}

bool evaluate_kramer_king_ward_actual_extraversion_modulator(const teddy_bear_geometry_t *geom, double elongation_val, double cheekbone_prominence, double *extraversion_mod_out) {
    if (!geom || elongation_val < 0.0 || cheekbone_prominence < 0.0 || !extraversion_mod_out) {
        return false;
    }
    *extraversion_mod_out = (cheekbone_prominence / (elongation_val > 0.0 ? elongation_val : 1.0)) * (0.8 + geom->social_extraversion * 0.35);
    return true;
}

bool evaluate_kramer_king_ward_neuroticism_threat_mapping(const teddy_bear_geometry_t *geom, double asymmetry_val, double vertical_offset, double *threat_out) {
    if (!geom || asymmetry_val < 0.0 || vertical_offset < 0.0 || !threat_out) {
        return false;
    }
    *threat_out = (asymmetry_val * 0.7 + vertical_offset * 0.3) * (1.1 + geom->behavioral_mismatch * 0.25);
    return true;
}

bool evaluate_kramer_king_ward_perceived_conscientiousness_consensus(const teddy_bear_geometry_t *geom, double symmetry_val, double forehead_proportion, double *consensus_out) {
    if (!geom || symmetry_val < 0.0 || forehead_proportion < 0.0 || !consensus_out) {
        return false;
    }
    *consensus_out = (symmetry_val * 0.5 + forehead_proportion * 0.5) * (0.9 + geom->honesty_index * 0.25);
    return true;
}

bool evaluate_kramer_king_ward_perceived_openness_morphological_mapping(const teddy_bear_geometry_t *geom, double eye_size, double elongation_val, double *openness_score_out) {
    if (!geom || eye_size < 0.0 || elongation_val < 0.0 || !openness_score_out) {
        return false;
    }
    *openness_score_out = (eye_size * 0.7 + (1.0 / (1.0 + elongation_val)) * 0.3) * (0.85 + geom->intellect_index * 0.3);
    return true;
}

bool evaluate_kramer_king_ward_actual_agreeableness_correlation(const teddy_bear_geometry_t *geom, double mouth_upturn, double fwhr_val, double *agreeableness_score_out) {
    if (!geom || mouth_upturn < 0.0 || fwhr_val < 0.0 || !agreeableness_score_out) {
        return false;
    }
    *agreeableness_score_out = (mouth_upturn * 0.8 + (1.0 / (1.0 + fwhr_val)) * 0.2) * (0.9 + geom->empathy_index * 0.25);
    return true;
}

bool evaluate_kramer_king_ward_perceived_extraversion_consensus(const teddy_bear_geometry_t *geom, double forehead_proportion, double eye_size, double *consensus_out) {
    if (!geom || forehead_proportion < 0.0 || eye_size < 0.0 || !consensus_out) {
        return false;
    }
    *consensus_out = (forehead_proportion * 0.4 + eye_size * 0.6) * (0.8 + geom->social_extraversion * 0.3);
    return true;
}

bool evaluate_kramer_king_ward_perceived_neuroticism_consensus(const teddy_bear_geometry_t *geom, double asymmetry_val, double eyebrow_height, double *consensus_out) {
    if (!geom || asymmetry_val < 0.0 || eyebrow_height < 0.0 || !consensus_out) {
        return false;
    }
    *consensus_out = (asymmetry_val * 0.5 + (1.0 / (1.0 + eyebrow_height)) * 0.5) * (1.1 + geom->behavioral_mismatch * 0.2);
    return true;
}

bool evaluate_kramer_king_ward_actual_conscientiousness_correlation(const teddy_bear_geometry_t *geom, double elongation_val, double symmetry_val, double *conscientiousness_score_out) {
    if (!geom || elongation_val < 0.0 || symmetry_val < 0.0 || !conscientiousness_score_out) {
        return false;
    }
    *conscientiousness_score_out = (symmetry_val * 0.7 + (1.0 / (1.0 + elongation_val)) * 0.3) * (0.9 + geom->honesty_index * 0.2);
    return true;
}



