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
