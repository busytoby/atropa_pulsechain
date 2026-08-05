#include "auncient_teddy_personality.h"
#include <stdio.h>
#include <math.h>

bool evaluate_kramer_ward_fwhr_dominance(const teddy_geometry_t *geom, double fwhr_val, double *dominance_out) {
    if (!geom || fwhr_val < 0.0 || !dominance_out) {
        return false;
    }
    *dominance_out = fwhr_val * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_kramer_ward_fwhr_trustworthiness(const teddy_geometry_t *geom, double fwhr_val, double *trustworthiness_out) {
    if (!geom || fwhr_val < 0.0 || !trustworthiness_out) {
        return false;
    }
    // High FWHR correlates with lower trust perception in visual bounds
    *trustworthiness_out = (1.0 / (1.0 + fwhr_val)) * (1.2 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_kramer_ward_fwhr_dominance_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (14.0 + geom->resilience_index * 4.0));
    return true;
}

bool evaluate_kramer_ward_fwhr_trustworthiness_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (14.0 + geom->resilience_index * 4.0));
    return true;
}

bool evaluate_kramer_ward_fwhr_threat(const teddy_geometry_t *geom, double fwhr_val, double *threat_out) {
    if (!geom || fwhr_val < 0.0 || !threat_out) {
        return false;
    }
    *threat_out = fwhr_val * (1.3 + geom->behavioral_mismatch * 0.3);
    return true;
}

bool evaluate_kramer_ward_fwhr_cooperation(const teddy_geometry_t *geom, double fwhr_val, double *cooperation_out) {
    if (!geom || fwhr_val < 0.0 || !cooperation_out) {
        return false;
    }
    // High fWHR is negatively associated with cooperative behavior
    *cooperation_out = (1.0 / (1.0 + fwhr_val)) * (1.4 + geom->cooperative_negotiation * 0.3);
    return true;
}

bool evaluate_kramer_ward_fwhr_threat_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (15.0 + geom->resilience_index * 5.0));
    return true;
}

bool evaluate_kramer_ward_fwhr_cooperation_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (15.0 + geom->resilience_index * 5.0));
    return true;
}

bool evaluate_kramer_ward_fwhr_3d_scan_discrepancy(const teddy_geometry_t *geom, double fwhr_val, double *discrepancy_out) {
    if (!geom || fwhr_val < 0.0 || !discrepancy_out) {
        return false;
    }
    *discrepancy_out = fwhr_val * 0.08 * (1.0 + geom->behavioral_mismatch * 0.1);
    return true;
}

bool evaluate_kramer_ward_fwhr_sexual_dimorphism_null_hypothesis(const teddy_geometry_t *geom, double male_fwhr, double female_fwhr, double *p_value_out) {
    if (!geom || male_fwhr < 0.0 || female_fwhr < 0.0 || !p_value_out) {
        return false;
    }
    double diff = fabs(male_fwhr - female_fwhr);
    *p_value_out = exp(-diff * 5.0) * (0.8 + geom->cooperative_negotiation * 0.1);
    return true;
}

bool evaluate_kramer_ward_fwhr_head_tilt_effect(const teddy_geometry_t *geom, double fwhr_val, double pitch_degrees, double *apparent_fwhr_out) {
    if (!geom || fwhr_val < 0.0 || !apparent_fwhr_out) {
        return false;
    }
    double rad = pitch_degrees * 3.141592653589793 / 180.0;
    *apparent_fwhr_out = fwhr_val / cos(rad * 0.5) * (1.0 + geom->head_tilt * 0.05);
    return true;
}

bool evaluate_kramer_ward_fwhr_actual_vs_perceived_aggression_mismatch(const teddy_geometry_t *geom, double perceived_dominance, double actual_aggression, double *mismatch_out) {
    if (!geom || perceived_dominance < 0.0 || actual_aggression < 0.0 || !mismatch_out) {
        return false;
    }
    *mismatch_out = fabs(perceived_dominance - actual_aggression) * (1.2 + geom->behavioral_mismatch * 0.4);
    return true;
}

bool evaluate_kramer_ward_fwhr_perceptual_noise_sensitivity(const teddy_geometry_t *geom, double noise_level, double *threshold_se_multiplier_out) {
    if (!geom || noise_level < 0.0 || !threshold_se_multiplier_out) {
        return false;
    }
    *threshold_se_multiplier_out = (1.0 + noise_level * 2.5) * (1.0 + geom->behavioral_mismatch * 0.2);
    return true;
}

bool evaluate_kramer_ward_fwhr_sequential_adaptation_bias(const teddy_geometry_t *geom, const double *recent_fwhr_history, int history_len, double *adaptation_offset_out) {
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

bool evaluate_kramer_ward_fwhr_hemiface_asymmetry_bias(const teddy_geometry_t *geom, double left_fwhr, double right_fwhr, double *weighted_fwhr_out) {
    if (!geom || left_fwhr < 0.0 || right_fwhr < 0.0 || !weighted_fwhr_out) {
        return false;
    }
    // Weighted towards the right hemiface due to left visual field bias
    *weighted_fwhr_out = (left_fwhr * 0.4 + right_fwhr * 0.6) * (1.0 + (1.0 - geom->symmetry) * 0.05);
    return true;
}

bool evaluate_kramer_ward_fwhr_ambient_shading_effect(const teddy_geometry_t *geom, double original_fwhr, double light_angle_degrees, double *apparent_fwhr_out) {
    if (!geom || original_fwhr < 0.0 || !apparent_fwhr_out) {
        return false;
    }
    double rad = light_angle_degrees * 3.141592653589793 / 180.0;
    *apparent_fwhr_out = original_fwhr * (1.0 + sin(rad) * 0.04 * (1.0 - geom->fur_roughness * 0.2));
    return true;
}

bool evaluate_kramer_ward_fwhr_dynamic_temporal_smoothing(const teddy_geometry_t *geom, const double *frame_fwhr_sequence, int sequence_len, double *smoothed_fwhr_out) {
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

bool evaluate_kramer_ward_fwhr_judgment_confidence_calibration(const teddy_geometry_t *geom, double fwhr_val, double *confidence_rating_out) {
    if (!geom || fwhr_val < 0.0 || !confidence_rating_out) {
        return false;
    }
    double deviation = fabs(fwhr_val - 1.85);
    *confidence_rating_out = (1.0 / (1.0 + deviation * 1.5)) * (0.9 + geom->intellect_index * 0.1);
    return true;
}

bool evaluate_kramer_ward_fwhr_perceived_vs_actual_health(const teddy_geometry_t *geom, double perceived_health, double actual_health, double *mismatch_out) {
    if (!geom || perceived_health < 0.0 || actual_health < 0.0 || !mismatch_out) {
        return false;
    }
    *mismatch_out = fabs(perceived_health - actual_health) * (1.1 + geom->behavioral_mismatch * 0.2);
    return true;
}

bool evaluate_kramer_ward_fwhr_cheekbone_prominence_modulation(const teddy_geometry_t *geom, double cheekbone_val, double *prominence_mod_out) {
    if (!geom || cheekbone_val < 0.0 || !prominence_mod_out) {
        return false;
    }
    *prominence_mod_out = cheekbone_val * 0.95 * (1.0 + (1.0 - geom->symmetry) * 0.15);
    return true;
}

bool evaluate_kramer_ward_fwhr_perspective_distortion(const teddy_geometry_t *geom, double fwhr_val, double focal_length_mm, double *distorted_fwhr_out) {
    if (!geom || fwhr_val < 0.0 || focal_length_mm <= 0.0 || !distorted_fwhr_out) {
        return false;
    }
    // Shorter focal lengths distort (widen) the apparent fWHR
    double distortion_factor = 1.0 + (35.0 / focal_length_mm) * 0.08;
    *distorted_fwhr_out = fwhr_val * distortion_factor * (1.0 + geom->head_fwhr * 0.02);
    return true;
}

bool evaluate_kramer_ward_fwhr_micro_exposure_resolution(const teddy_geometry_t *geom, double exposure_ms, double *resolution_accuracy_out) {
    if (!geom || exposure_ms < 0.0 || !resolution_accuracy_out) {
        return false;
    }
    // Trait resolution is sigmoid-gated by exposure duration up to ~500ms
    double k = 0.015; // sigmoid slope constant
    *resolution_accuracy_out = (1.0 / (1.0 + exp(-k * (exposure_ms - 100.0)))) * (0.85 + geom->intellect_index * 0.1);
    return true;
}

bool evaluate_kramer_king_ward_primate_extraversion_signaling(const teddy_geometry_t *geom, double face_proportion, double *extraversion_out) {
    if (!geom || face_proportion < 0.0 || !extraversion_out) {
        return false;
    }
    *extraversion_out = face_proportion * 1.15 * (0.9 + geom->social_extraversion * 0.2);
    return true;
}

bool evaluate_kramer_king_ward_cross_species_congruence(const teddy_geometry_t *geom, double human_score, double primate_score, double *congruence_out) {
    if (!geom || human_score < 0.0 || primate_score < 0.0 || !congruence_out) {
        return false;
    }
    double diff = fabs(human_score - primate_score);
    *congruence_out = (1.0 / (1.0 + diff)) * (0.95 + geom->cooperative_negotiation * 0.05);
    return true;
}

bool evaluate_kramer_king_ward_shared_evolutionary_threat(const teddy_geometry_t *geom, double fwhr_val, double brow_val, double *threat_out) {
    if (!geom || fwhr_val < 0.0 || brow_val < 0.0 || !threat_out) {
        return false;
    }
    *threat_out = (fwhr_val * 0.6 + brow_val * 0.4) * (1.1 + geom->behavioral_mismatch * 0.3);
    return true;
}

bool evaluate_kramer_king_ward_agreeableness_conscientiousness(const teddy_geometry_t *geom, double eye_orbit_val, double jaw_val, double *rating_out) {
    if (!geom || eye_orbit_val < 0.0 || jaw_val < 0.0 || !rating_out) {
        return false;
    }
    *rating_out = (eye_orbit_val * 0.5 + jaw_val * 0.5) * (0.95 + geom->parenting_capability * 0.1);
    return true;
}

bool evaluate_kramer_king_ward_species_general_cues(const teddy_geometry_t *geom, double general_cue_weight, double *marker_val_out) {
    if (!geom || general_cue_weight < 0.0 || !marker_val_out) {
        return false;
    }
    *marker_val_out = general_cue_weight * 1.05 * (0.9 + geom->social_status * 0.2);
    return true;
}

bool evaluate_kramer_king_ward_sex_modulated_accuracy(const teddy_geometry_t *geom, double base_accuracy, int biological_sex, double *accuracy_out) {
    if (!geom || base_accuracy < 0.0 || !accuracy_out) {
        return false;
    }
    double factor = (biological_sex == 1) ? 1.05 : 0.98;
    *accuracy_out = base_accuracy * factor * (1.0 + geom->emotional_stability * 0.05);
    return true;
}

bool evaluate_kramer_king_ward_inter_rater_consensus(const teddy_geometry_t *geom, const double *rater_judgments, int rater_count, double *consensus_out) {
    if (!geom || !rater_judgments || rater_count <= 1 || !consensus_out) {
        return false;
    }
    double sum = 0.0;
    for (int i = 0; i < rater_count; i++) {
        sum += rater_judgments[i];
    }
    double mean = sum / rater_count;
    double var_sum = 0.0;
    for (int i = 0; i < rater_count; i++) {
        double d = rater_judgments[i] - mean;
        var_sum += d * d;
    }
    double variance = var_sum / (rater_count - 1);
    *consensus_out = (1.0 / (1.0 + variance)) * (0.9 + geom->intellect_index * 0.1);
    return true;
}

bool evaluate_kramer_king_ward_intra_group_dominance(const teddy_geometry_t *geom, double group_rank, double *dominance_out) {
    if (!geom || group_rank <= 0.0 || !dominance_out) {
        return false;
    }
    *dominance_out = (1.0 / group_rank) * 1.5 * (0.9 + geom->command_authority * 0.2);
    return true;
}

bool evaluate_kramer_king_ward_species_distance_matrix(const teddy_geometry_t *geom, const double *human_coords, const double *primate_coords, int coords_len, double *distance_out) {
    if (!geom || !human_coords || !primate_coords || coords_len <= 0 || !distance_out) {
        return false;
    }
    double sq_sum = 0.0;
    for (int i = 0; i < coords_len; i++) {
        double diff = human_coords[i] - primate_coords[i];
        sq_sum += diff * diff;
    }
    *distance_out = sqrt(sq_sum) * (1.0 - geom->vocal_visual_harmony * 0.05);
    return true;
}

bool evaluate_kramer_king_ward_primate_openness(const teddy_geometry_t *geom, double eye_height_width_ratio, double face_elongation, double *openness_out) {
    if (!geom || eye_height_width_ratio < 0.0 || face_elongation < 0.0 || !openness_out) {
        return false;
    }
    *openness_out = (eye_height_width_ratio * 0.7 + (1.0 / (1.0 + face_elongation)) * 0.3) * (0.95 + geom->intellect_index * 0.1);
    return true;
}

bool evaluate_kramer_king_ward_primate_neuroticism(const teddy_geometry_t *geom, double aspect_ratio, double orbital_asymmetry, double *neuroticism_out) {
    if (!geom || aspect_ratio < 0.0 || orbital_asymmetry < 0.0 || !neuroticism_out) {
        return false;
    }
    *neuroticism_out = (aspect_ratio * 0.45 + orbital_asymmetry * 0.55) * (1.1 - geom->emotional_stability * 0.2);
    return true;
}

bool evaluate_kramer_king_ward_agreeableness_specificity(const teddy_geometry_t *geom, double species_factor, double *specificity_out) {
    if (!geom || species_factor < 0.0 || !specificity_out) {
        return false;
    }
    *specificity_out = species_factor * 1.08 * (0.9 + geom->parenting_capability * 0.15);
    return true;
}

bool evaluate_kramer_king_ward_sex_modulated_extraversion(const teddy_geometry_t *geom, double base_accuracy, int observer_sex, int primate_sex, double *accuracy_out) {
    if (!geom || base_accuracy < 0.0 || !accuracy_out) {
        return false;
    }
    double factor = 1.0;
    if (observer_sex == primate_sex) {
        factor = 1.08; // same-sex decoding advantage
    } else {
        factor = 0.95;
    }
    *accuracy_out = base_accuracy * factor * (0.95 + geom->social_extraversion * 0.1);
    return true;
}

bool evaluate_kramer_king_ward_morphological_pca_axes(const teddy_geometry_t *geom, const double *morpho_matrix, int size, double *pca1, double *pca2, double *pca3) {
    if (!geom || !morpho_matrix || size < 3 || !pca1 || !pca2 || !pca3) {
        return false;
    }
    // Speculative linear combination projection representing the three main dimensions
    *pca1 = morpho_matrix[0] * 0.5 + morpho_matrix[1] * 0.3 + morpho_matrix[2] * 0.2;
    *pca2 = morpho_matrix[0] * -0.2 + morpho_matrix[1] * 0.6 + morpho_matrix[2] * 0.2;
    *pca3 = morpho_matrix[0] * 0.1 + morpho_matrix[1] * -0.1 + morpho_matrix[2] * 0.8;
    
    // Scale by geometry metrics
    *pca1 *= (1.0 + geom->leadership_profile * 0.1);
    *pca2 *= (1.0 + geom->social_extraversion * 0.1);
    *pca3 *= (1.0 + geom->behavioral_mismatch * 0.1);
    return true;
}

bool evaluate_kramer_king_ward_dominance_assertiveness_decoupling(const teddy_geometry_t *geom, double dominance_cue, double assertiveness_cue, double *decoupled_val_out) {
    if (!geom || dominance_cue < 0.0 || assertiveness_cue < 0.0 || !decoupled_val_out) {
        return false;
    }
    *decoupled_val_out = fabs(dominance_cue - assertiveness_cue) * (0.95 + geom->confrontational_assertiveness * 0.1);
    return true;
}

bool evaluate_kramer_king_ward_neuroticism_specificity(const teddy_geometry_t *geom, double species_factor, double *specificity_out) {
    if (!geom || species_factor < 0.0 || !specificity_out) {
        return false;
    }
    *specificity_out = species_factor * 1.12 * (1.1 - geom->emotional_stability * 0.15);
    return true;
}

bool evaluate_kramer_king_ward_centroid_alignment(const teddy_geometry_t *geom, const double *human_centroid, const double *primate_centroid, int size, double *alignment_val_out) {
    if (!geom || !human_centroid || !primate_centroid || size <= 0 || !alignment_val_out) {
        return false;
    }
    double dot = 0.0, mag_h = 0.0, mag_p = 0.0;
    for (int i = 0; i < size; i++) {
        dot += human_centroid[i] * primate_centroid[i];
        mag_h += human_centroid[i] * human_centroid[i];
        mag_p += primate_centroid[i] * primate_centroid[i];
    }
    if (mag_h == 0.0 || mag_p == 0.0) {
        *alignment_val_out = 0.0;
    } else {
        *alignment_val_out = (dot / (sqrt(mag_h) * sqrt(mag_p))) * (0.95 + geom->vocal_visual_harmony * 0.05);
    }
    return true;
}
