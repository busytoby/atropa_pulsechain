#include "auncient_teddy_personality.h"
#include <stdio.h>
#include <math.h>

bool evaluate_keating_babyfacedness_index(const teddy_geometry_t *geom, double *babyfacedness_out) {
    if (!geom || !babyfacedness_out) {
        return false;
    }
    *babyfacedness_out = (geom->eye_scale * 0.4) + (-geom->feature_vertical_offset * 0.4) + ((1.0 / (1.0 + geom->jaw_scale)) * 0.2);
    return true;
}

bool evaluate_keating_dominance_cue_index(const teddy_geometry_t *geom, double *dominance_out) {
    if (!geom || !dominance_out) {
        return false;
    }
    *dominance_out = (geom->jaw_scale * 0.4) + (geom->feature_vertical_offset * 0.4) + ((1.0 / (1.0 + geom->eye_scale)) * 0.2);
    return true;
}

bool evaluate_keating_brow_dominance(const teddy_geometry_t *geom, double brow_height, double *brow_dominance_out) {
    if (!geom || brow_height < 0.0 || !brow_dominance_out) {
        return false;
    }
    *brow_dominance_out = (1.0 / (1.0 + brow_height)) * (1.2 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_lip_trustworthiness(const teddy_geometry_t *geom, double lip_thickness, double *lip_trustworthiness_out) {
    if (!geom || lip_thickness < 0.0 || !lip_trustworthiness_out) {
        return false;
    }
    *lip_trustworthiness_out = lip_thickness * (0.8 + geom->empathy_index * 0.5);
    return true;
}

bool evaluate_keating_brow_eye_ratio(const teddy_geometry_t *geom, double brow_eye_distance, double *ratio_dominance_out) {
    if (!geom || brow_eye_distance < 0.0 || !ratio_dominance_out) {
        return false;
    }
    *ratio_dominance_out = (1.0 / (1.0 + brow_eye_distance)) * (1.1 + geom->leadership_profile * 0.5);
    return true;
}

bool evaluate_keating_head_roundness_index(const teddy_geometry_t *geom, double *roundness_out) {
    if (!geom || !roundness_out) {
        return false;
    }
    *roundness_out = geom->head_fwhr * (1.1 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_symmetry_trust(const teddy_geometry_t *geom, double *trust_out) {
    if (!geom || !trust_out) {
        return false;
    }
    *trust_out = geom->symmetry * (1.0 + geom->honesty_index * 0.5);
    return true;
}

bool evaluate_keating_brow_chin_proportion(const teddy_geometry_t *geom, double brow_chin_distance, double *proportion_dominance_out) {
    if (!geom || brow_chin_distance < 0.0 || !proportion_dominance_out) {
        return false;
    }
    *proportion_dominance_out = (1.0 / (1.0 + brow_chin_distance)) * (1.3 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_keating_gaze_dominance(const teddy_geometry_t *geom, double gaze_duration_sec, double aversion_angle, double *gaze_dominance_out) {
    if (!geom || gaze_duration_sec < 0.0 || aversion_angle < 0.0 || !gaze_dominance_out) {
        return false;
    }
    *gaze_dominance_out = (gaze_duration_sec / (1.0 + aversion_angle)) * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_brow_gesture(const teddy_geometry_t *geom, double brow_raise_amplitude, double *brow_submissiveness_out) {
    if (!geom || brow_raise_amplitude < 0.0 || !brow_submissiveness_out) {
        return false;
    }
    *brow_submissiveness_out = brow_raise_amplitude * (0.9 + geom->empathy_index * 0.5);
    return true;
}

bool evaluate_keating_torso_head_ratio(const teddy_geometry_t *geom, double torso_span, double *ratio_dominance_out) {
    if (!geom || torso_span < 0.0 || !ratio_dominance_out) {
        return false;
    }
    *ratio_dominance_out = torso_span * (0.8 + geom->torso_ratio * 0.4 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_keating_eye_dilation_sync(const teddy_geometry_t *geom, double left_dilation, double right_dilation, double *babyface_sync_out) {
    if (!geom || left_dilation < 0.0 || right_dilation < 0.0 || !babyface_sync_out) {
        return false;
    }
    *babyface_sync_out = (1.0 / (1.0 + fabs(left_dilation - right_dilation))) * (0.8 + geom->symmetry * 0.4);
    return true;
}

bool evaluate_keating_posture_pitch(const teddy_geometry_t *geom, double pitch_angle, double *submissiveness_out) {
    if (!geom || !submissiveness_out) {
        return false;
    }
    *submissiveness_out = (pitch_angle + 1.0) * (0.9 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_chin_asymmetry(const teddy_geometry_t *geom, double left_jaw_width, double right_jaw_width, double *asymmetry_dominance_out) {
    if (!geom || left_jaw_width < 0.0 || right_jaw_width < 0.0 || !asymmetry_dominance_out) {
        return false;
    }
    *asymmetry_dominance_out = fabs(left_jaw_width - right_jaw_width) * (1.2 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_mouth_curvature(const teddy_geometry_t *geom, double upturn_curvature, double *warmth_rating_out) {
    if (!geom || upturn_curvature < 0.0 || !warmth_rating_out) {
        return false;
    }
    *warmth_rating_out = upturn_curvature * (0.8 + geom->empathy_index * 0.5);
    return true;
}

bool evaluate_keating_chin_asymmetry_dilation(const teddy_geometry_t *geom, double base_dilation, double *asymmetry_dilation_out) {
    if (!geom || base_dilation < 0.0 || !asymmetry_dilation_out) {
        return false;
    }
    *asymmetry_dilation_out = base_dilation * (1.0 + (1.0 - geom->symmetry) * 0.4);
    return true;
}

bool evaluate_keating_sclera_size(const teddy_geometry_t *geom, double sclera_ratio, double *submissiveness_out) {
    if (!geom || sclera_ratio < 0.0 || !submissiveness_out) {
        return false;
    }
    *submissiveness_out = sclera_ratio * (0.9 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_width_asymmetry(const teddy_geometry_t *geom, double left_width, double right_width, double *asymmetry_dominance_out) {
    if (!geom || left_width < 0.0 || right_width < 0.0 || !asymmetry_dominance_out) {
        return false;
    }
    *asymmetry_dominance_out = fabs(left_width - right_width) * (1.1 + geom->leadership_profile * 0.5);
    return true;
}

bool evaluate_keating_fwhr_dominance(const teddy_geometry_t *geom, double fwhr_value, double *dominance_out) {
    if (!geom || fwhr_value < 0.0 || !dominance_out) {
        return false;
    }
    *dominance_out = fwhr_value * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_mouth_width_ratio(const teddy_geometry_t *geom, double mouth_width, double jaw_width, double *submissiveness_out) {
    if (!geom || mouth_width < 0.0 || jaw_width < 0.0 || !submissiveness_out) {
        return false;
    }
    *submissiveness_out = (mouth_width / (jaw_width > 0.0 ? jaw_width : 1.0)) * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_width_symmetry_trust(const teddy_geometry_t *geom, double symmetry_ratio, double *trustworthiness_out) {
    if (!geom || symmetry_ratio < 0.0 || !trustworthiness_out) {
        return false;
    }
    *trustworthiness_out = symmetry_ratio * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_lip_submissiveness(const teddy_geometry_t *geom, double lip_height_ratio, double *submissiveness_out) {
    if (!geom || lip_height_ratio < 0.0 || !submissiveness_out) {
        return false;
    }
    *submissiveness_out = lip_height_ratio * (0.9 + geom->symmetry * 0.3);
    return true;
}

bool evaluate_keating_gaze_shift_dominance(const teddy_geometry_t *geom, double shift_frequency, double *dominance_out) {
    if (!geom || shift_frequency < 0.0 || !dominance_out) {
        return false;
    }
    *dominance_out = shift_frequency * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_lip_compression_trust(const teddy_geometry_t *geom, double compression_ratio, double *trustworthiness_out) {
    if (!geom || compression_ratio < 0.0 || !trustworthiness_out) {
        return false;
    }
    *trustworthiness_out = (1.0 / (1.0 + compression_ratio)) * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_width_asymmetry_dominance(const teddy_geometry_t *geom, double width_asymmetry_val, double *dominance_out) {
    if (!geom || width_asymmetry_val < 0.0 || !dominance_out) {
        return false;
    }
    *dominance_out = width_asymmetry_val * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_mouth_asymmetry_decay(const teddy_geometry_t *geom, double mouth_asymmetry_val, double *trust_decay_out) {
    if (!geom || mouth_asymmetry_val < 0.0 || !trust_decay_out) {
        return false;
    }
    *trust_decay_out = mouth_asymmetry_val * (0.9 + geom->behavioral_mismatch * 0.3);
    return true;
}

bool evaluate_keating_width_asymmetry_trust(const teddy_geometry_t *geom, double width_asymmetry_val, double *trustworthiness_out) {
    if (!geom || width_asymmetry_val < 0.0 || !trustworthiness_out) {
        return false;
    }
    *trustworthiness_out = (1.0 / (1.0 + width_asymmetry_val)) * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_mouth_asymmetry_dominance(const teddy_geometry_t *geom, double mouth_asymmetry_val, double *dominance_out) {
    if (!geom || mouth_asymmetry_val < 0.0 || !dominance_out) {
        return false;
    }
    *dominance_out = mouth_asymmetry_val * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_eyebrow_asymmetry_dominance(const teddy_geometry_t *geom, double eyebrow_asymmetry_val, double *dominance_out) {
    if (!geom || eyebrow_asymmetry_val < 0.0 || !dominance_out) {
        return false;
    }
    *dominance_out = eyebrow_asymmetry_val * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_sclera_submissiveness(const teddy_geometry_t *geom, double sclera_size_ratio, double *submissiveness_out) {
    if (!geom || sclera_size_ratio < 0.0 || !submissiveness_out) {
        return false;
    }
    *submissiveness_out = sclera_size_ratio * (0.9 + geom->symmetry * 0.3);
    return true;
}

bool evaluate_keating_eyebrow_asymmetry_trust(const teddy_geometry_t *geom, double eyebrow_asymmetry_val, double *trustworthiness_out) {
    if (!geom || eyebrow_asymmetry_val < 0.0 || !trustworthiness_out) {
        return false;
    }
    *trustworthiness_out = (1.0 / (1.0 + eyebrow_asymmetry_val)) * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_sclera_dominance(const teddy_geometry_t *geom, double sclera_size_ratio, double *dominance_out) {
    if (!geom || sclera_size_ratio < 0.0 || !dominance_out) {
        return false;
    }
    *dominance_out = sclera_size_ratio * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_eyebrow_asymmetry_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_trust_out) {
    if (!geom || duration_sec < 0.0 || !decayed_trust_out) {
        return false;
    }
    *decayed_trust_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_sclera_submissiveness_mod(const teddy_geometry_t *geom, double sclera_size_ratio, double *submissiveness_out) {
    if (!geom || sclera_size_ratio < 0.0 || !submissiveness_out) {
        return false;
    }
    *submissiveness_out = sclera_size_ratio * (0.9 + geom->symmetry * 0.3);
    return true;
}

bool evaluate_keating_eyebrow_asymmetry_dominance_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_dominance_out) {
    if (!geom || duration_sec < 0.0 || !decayed_dominance_out) {
        return false;
    }
    *decayed_dominance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_eyebrow_asymmetry_trust_decay_mod(const teddy_geometry_t *geom, double duration_sec, double *decayed_trust_out) {
    if (!geom || duration_sec < 0.0 || !decayed_trust_out) {
        return false;
    }
    *decayed_trust_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_sclera_submissiveness_variance(const teddy_geometry_t *geom, double sclera_size_ratio, double *submissiveness_variance_out) {
    if (!geom || sclera_size_ratio < 0.0 || !submissiveness_variance_out) {
        return false;
    }
    *submissiveness_variance_out = sclera_size_ratio * (0.9 + geom->symmetry * 0.3);
    return true;
}

bool evaluate_keating_sclera_dominance_variance(const teddy_geometry_t *geom, double sclera_size_ratio, double *dominance_variance_out) {
    if (!geom || sclera_size_ratio < 0.0 || !dominance_variance_out) {
        return false;
    }
    *dominance_variance_out = sclera_size_ratio * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_gaze_shift_dominance_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_dominance_out) {
    if (!geom || duration_sec < 0.0 || !decayed_dominance_out) {
        return false;
    }
    *decayed_dominance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_lip_compression_trust_variance(const teddy_geometry_t *geom, double compression_ratio, double *trust_variance_out) {
    if (!geom || compression_ratio < 0.0 || !trust_variance_out) {
        return false;
    }
    *trust_variance_out = (1.0 / (1.0 + compression_ratio)) * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_sclera_submissiveness_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_submissiveness_out) {
    if (!geom || duration_sec < 0.0 || !decayed_submissiveness_out) {
        return false;
    }
    *decayed_submissiveness_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_width_asymmetry_dominance_variance(const teddy_geometry_t *geom, double width_asymmetry_val, double *variance_out) {
    if (!geom || width_asymmetry_val < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = width_asymmetry_val * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_sclera_dominance_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_dominance_out) {
    if (!geom || duration_sec < 0.0 || !decayed_dominance_out) {
        return false;
    }
    *decayed_dominance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_mouth_asymmetry_dominance_variance(const teddy_geometry_t *geom, double mouth_asymmetry_val, double *variance_out) {
    if (!geom || mouth_asymmetry_val < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = mouth_asymmetry_val * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_eyebrow_asymmetry_dominance_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_mouth_asymmetry_trust_variance(const teddy_geometry_t *geom, double mouth_asymmetry_val, double *trust_variance_out) {
    if (!geom || mouth_asymmetry_val < 0.0 || !trust_variance_out) {
        return false;
    }
    *trust_variance_out = (1.0 / (1.0 + mouth_asymmetry_val)) * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_sclera_submissiveness_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_width_asymmetry_trust_variance(const teddy_geometry_t *geom, double width_asymmetry_val, double *trust_variance_out) {
    if (!geom || width_asymmetry_val < 0.0 || !trust_variance_out) {
        return false;
    }
    *trust_variance_out = (1.0 / (1.0 + width_asymmetry_val)) * (0.8 + geom->empathy_index * 0.4);
    return true;
}

