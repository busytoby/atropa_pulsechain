#include "auncient_teddy_bear_personality.h"
#include <stdio.h>
#include <math.h>

bool evaluate_keating_babyfacedness_index(const teddy_bear_geometry_t *geom, double *babyfacedness_out) {
    if (!geom || !babyfacedness_out) {
        return false;
    }
    *babyfacedness_out = (geom->eye_scale * 0.4) + (-geom->feature_vertical_offset * 0.4) + ((1.0 / (1.0 + geom->jaw_scale)) * 0.2);
    return true;
}

bool evaluate_keating_dominance_cue_index(const teddy_bear_geometry_t *geom, double *dominance_out) {
    if (!geom || !dominance_out) {
        return false;
    }
    *dominance_out = (geom->jaw_scale * 0.4) + (geom->feature_vertical_offset * 0.4) + ((1.0 / (1.0 + geom->eye_scale)) * 0.2);
    return true;
}

bool evaluate_keating_brow_dominance(const teddy_bear_geometry_t *geom, double brow_height, double *brow_dominance_out) {
    if (!geom || brow_height < 0.0 || !brow_dominance_out) {
        return false;
    }
    *brow_dominance_out = (1.0 / (1.0 + brow_height)) * (1.2 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_lip_trustworthiness(const teddy_bear_geometry_t *geom, double lip_thickness, double *lip_trustworthiness_out) {
    if (!geom || lip_thickness < 0.0 || !lip_trustworthiness_out) {
        return false;
    }
    *lip_trustworthiness_out = lip_thickness * (0.8 + geom->empathy_index * 0.5);
    return true;
}

bool evaluate_keating_brow_eye_ratio(const teddy_bear_geometry_t *geom, double brow_eye_distance, double *ratio_dominance_out) {
    if (!geom || brow_eye_distance < 0.0 || !ratio_dominance_out) {
        return false;
    }
    *ratio_dominance_out = (1.0 / (1.0 + brow_eye_distance)) * (1.1 + geom->leadership_profile * 0.5);
    return true;
}

bool evaluate_keating_head_roundness_index(const teddy_bear_geometry_t *geom, double *roundness_out) {
    if (!geom || !roundness_out) {
        return false;
    }
    *roundness_out = geom->head_fwhr * (1.1 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_symmetry_trust(const teddy_bear_geometry_t *geom, double *trust_out) {
    if (!geom || !trust_out) {
        return false;
    }
    *trust_out = geom->symmetry * (1.0 + geom->honesty_index * 0.5);
    return true;
}

bool evaluate_keating_brow_chin_proportion(const teddy_bear_geometry_t *geom, double brow_chin_distance, double *proportion_dominance_out) {
    if (!geom || brow_chin_distance < 0.0 || !proportion_dominance_out) {
        return false;
    }
    *proportion_dominance_out = (1.0 / (1.0 + brow_chin_distance)) * (1.3 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_keating_gaze_dominance(const teddy_bear_geometry_t *geom, double gaze_duration_sec, double aversion_angle, double *gaze_dominance_out) {
    if (!geom || gaze_duration_sec < 0.0 || aversion_angle < 0.0 || !gaze_dominance_out) {
        return false;
    }
    *gaze_dominance_out = (gaze_duration_sec / (1.0 + aversion_angle)) * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_brow_gesture(const teddy_bear_geometry_t *geom, double brow_raise_amplitude, double *brow_submissiveness_out) {
    if (!geom || brow_raise_amplitude < 0.0 || !brow_submissiveness_out) {
        return false;
    }
    *brow_submissiveness_out = brow_raise_amplitude * (0.9 + geom->empathy_index * 0.5);
    return true;
}

bool evaluate_keating_torso_head_ratio(const teddy_bear_geometry_t *geom, double torso_span, double *ratio_dominance_out) {
    if (!geom || torso_span < 0.0 || !ratio_dominance_out) {
        return false;
    }
    *ratio_dominance_out = torso_span * (0.8 + geom->torso_ratio * 0.4 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_keating_eye_dilation_sync(const teddy_bear_geometry_t *geom, double left_dilation, double right_dilation, double *babyface_sync_out) {
    if (!geom || left_dilation < 0.0 || right_dilation < 0.0 || !babyface_sync_out) {
        return false;
    }
    *babyface_sync_out = (1.0 / (1.0 + fabs(left_dilation - right_dilation))) * (0.8 + geom->symmetry * 0.4);
    return true;
}

bool evaluate_keating_posture_pitch(const teddy_bear_geometry_t *geom, double pitch_angle, double *submissiveness_out) {
    if (!geom || !submissiveness_out) {
        return false;
    }
    *submissiveness_out = (pitch_angle + 1.0) * (0.9 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_chin_asymmetry(const teddy_bear_geometry_t *geom, double left_jaw_width, double right_jaw_width, double *asymmetry_dominance_out) {
    if (!geom || left_jaw_width < 0.0 || right_jaw_width < 0.0 || !asymmetry_dominance_out) {
        return false;
    }
    *asymmetry_dominance_out = fabs(left_jaw_width - right_jaw_width) * (1.2 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_mouth_curvature(const teddy_bear_geometry_t *geom, double upturn_curvature, double *warmth_rating_out) {
    if (!geom || upturn_curvature < 0.0 || !warmth_rating_out) {
        return false;
    }
    *warmth_rating_out = upturn_curvature * (0.8 + geom->empathy_index * 0.5);
    return true;
}

bool evaluate_keating_chin_asymmetry_dilation(const teddy_bear_geometry_t *geom, double base_dilation, double *asymmetry_dilation_out) {
    if (!geom || base_dilation < 0.0 || !asymmetry_dilation_out) {
        return false;
    }
    *asymmetry_dilation_out = base_dilation * (1.0 + (1.0 - geom->symmetry) * 0.4);
    return true;
}

bool evaluate_keating_sclera_size(const teddy_bear_geometry_t *geom, double sclera_ratio, double *submissiveness_out) {
    if (!geom || sclera_ratio < 0.0 || !submissiveness_out) {
        return false;
    }
    *submissiveness_out = sclera_ratio * (0.9 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_width_asymmetry(const teddy_bear_geometry_t *geom, double left_width, double right_width, double *asymmetry_dominance_out) {
    if (!geom || left_width < 0.0 || right_width < 0.0 || !asymmetry_dominance_out) {
        return false;
    }
    *asymmetry_dominance_out = fabs(left_width - right_width) * (1.1 + geom->leadership_profile * 0.5);
    return true;
}

bool evaluate_keating_fwhr_dominance(const teddy_bear_geometry_t *geom, double fwhr_value, double *dominance_out) {
    if (!geom || fwhr_value < 0.0 || !dominance_out) {
        return false;
    }
    *dominance_out = fwhr_value * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_mouth_width_ratio(const teddy_bear_geometry_t *geom, double mouth_width, double jaw_width, double *submissiveness_out) {
    if (!geom || mouth_width < 0.0 || jaw_width < 0.0 || !submissiveness_out) {
        return false;
    }
    *submissiveness_out = (mouth_width / (jaw_width > 0.0 ? jaw_width : 1.0)) * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_width_symmetry_trust(const teddy_bear_geometry_t *geom, double symmetry_ratio, double *trustworthiness_out) {
    if (!geom || symmetry_ratio < 0.0 || !trustworthiness_out) {
        return false;
    }
    *trustworthiness_out = symmetry_ratio * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_lip_submissiveness(const teddy_bear_geometry_t *geom, double lip_height_ratio, double *submissiveness_out) {
    if (!geom || lip_height_ratio < 0.0 || !submissiveness_out) {
        return false;
    }
    *submissiveness_out = lip_height_ratio * (0.9 + geom->symmetry * 0.3);
    return true;
}

bool evaluate_keating_gaze_shift_dominance(const teddy_bear_geometry_t *geom, double shift_frequency, double *dominance_out) {
    if (!geom || shift_frequency < 0.0 || !dominance_out) {
        return false;
    }
    *dominance_out = shift_frequency * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_lip_compression_trust(const teddy_bear_geometry_t *geom, double compression_ratio, double *trustworthiness_out) {
    if (!geom || compression_ratio < 0.0 || !trustworthiness_out) {
        return false;
    }
    *trustworthiness_out = (1.0 / (1.0 + compression_ratio)) * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_width_asymmetry_dominance(const teddy_bear_geometry_t *geom, double width_asymmetry_val, double *dominance_out) {
    if (!geom || width_asymmetry_val < 0.0 || !dominance_out) {
        return false;
    }
    *dominance_out = width_asymmetry_val * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_mouth_asymmetry_decay(const teddy_bear_geometry_t *geom, double mouth_asymmetry_val, double *trust_decay_out) {
    if (!geom || mouth_asymmetry_val < 0.0 || !trust_decay_out) {
        return false;
    }
    *trust_decay_out = mouth_asymmetry_val * (0.9 + geom->behavioral_mismatch * 0.3);
    return true;
}

bool evaluate_keating_width_asymmetry_trust(const teddy_bear_geometry_t *geom, double width_asymmetry_val, double *trustworthiness_out) {
    if (!geom || width_asymmetry_val < 0.0 || !trustworthiness_out) {
        return false;
    }
    *trustworthiness_out = (1.0 / (1.0 + width_asymmetry_val)) * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_mouth_asymmetry_dominance(const teddy_bear_geometry_t *geom, double mouth_asymmetry_val, double *dominance_out) {
    if (!geom || mouth_asymmetry_val < 0.0 || !dominance_out) {
        return false;
    }
    *dominance_out = mouth_asymmetry_val * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_eyebrow_asymmetry_dominance(const teddy_bear_geometry_t *geom, double eyebrow_asymmetry_val, double *dominance_out) {
    if (!geom || eyebrow_asymmetry_val < 0.0 || !dominance_out) {
        return false;
    }
    *dominance_out = eyebrow_asymmetry_val * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_sclera_submissiveness(const teddy_bear_geometry_t *geom, double sclera_size_ratio, double *submissiveness_out) {
    if (!geom || sclera_size_ratio < 0.0 || !submissiveness_out) {
        return false;
    }
    *submissiveness_out = sclera_size_ratio * (0.9 + geom->symmetry * 0.3);
    return true;
}

bool evaluate_keating_eyebrow_asymmetry_trust(const teddy_bear_geometry_t *geom, double eyebrow_asymmetry_val, double *trustworthiness_out) {
    if (!geom || eyebrow_asymmetry_val < 0.0 || !trustworthiness_out) {
        return false;
    }
    *trustworthiness_out = (1.0 / (1.0 + eyebrow_asymmetry_val)) * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_sclera_dominance(const teddy_bear_geometry_t *geom, double sclera_size_ratio, double *dominance_out) {
    if (!geom || sclera_size_ratio < 0.0 || !dominance_out) {
        return false;
    }
    *dominance_out = sclera_size_ratio * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_eyebrow_asymmetry_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_trust_out) {
    if (!geom || duration_sec < 0.0 || !decayed_trust_out) {
        return false;
    }
    *decayed_trust_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_sclera_submissiveness_mod(const teddy_bear_geometry_t *geom, double sclera_size_ratio, double *submissiveness_out) {
    if (!geom || sclera_size_ratio < 0.0 || !submissiveness_out) {
        return false;
    }
    *submissiveness_out = sclera_size_ratio * (0.9 + geom->symmetry * 0.3);
    return true;
}

bool evaluate_keating_eyebrow_asymmetry_dominance_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_dominance_out) {
    if (!geom || duration_sec < 0.0 || !decayed_dominance_out) {
        return false;
    }
    *decayed_dominance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_eyebrow_asymmetry_trust_decay_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_trust_out) {
    if (!geom || duration_sec < 0.0 || !decayed_trust_out) {
        return false;
    }
    *decayed_trust_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_sclera_submissiveness_variance(const teddy_bear_geometry_t *geom, double sclera_size_ratio, double *submissiveness_variance_out) {
    if (!geom || sclera_size_ratio < 0.0 || !submissiveness_variance_out) {
        return false;
    }
    *submissiveness_variance_out = sclera_size_ratio * (0.9 + geom->symmetry * 0.3);
    return true;
}

bool evaluate_keating_sclera_dominance_variance(const teddy_bear_geometry_t *geom, double sclera_size_ratio, double *dominance_variance_out) {
    if (!geom || sclera_size_ratio < 0.0 || !dominance_variance_out) {
        return false;
    }
    *dominance_variance_out = sclera_size_ratio * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_gaze_shift_dominance_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_dominance_out) {
    if (!geom || duration_sec < 0.0 || !decayed_dominance_out) {
        return false;
    }
    *decayed_dominance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_lip_compression_trust_variance(const teddy_bear_geometry_t *geom, double compression_ratio, double *trust_variance_out) {
    if (!geom || compression_ratio < 0.0 || !trust_variance_out) {
        return false;
    }
    *trust_variance_out = (1.0 / (1.0 + compression_ratio)) * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_sclera_submissiveness_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_submissiveness_out) {
    if (!geom || duration_sec < 0.0 || !decayed_submissiveness_out) {
        return false;
    }
    *decayed_submissiveness_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_width_asymmetry_dominance_variance(const teddy_bear_geometry_t *geom, double width_asymmetry_val, double *variance_out) {
    if (!geom || width_asymmetry_val < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = width_asymmetry_val * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_sclera_dominance_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_dominance_out) {
    if (!geom || duration_sec < 0.0 || !decayed_dominance_out) {
        return false;
    }
    *decayed_dominance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_mouth_asymmetry_dominance_variance(const teddy_bear_geometry_t *geom, double mouth_asymmetry_val, double *variance_out) {
    if (!geom || mouth_asymmetry_val < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = mouth_asymmetry_val * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_eyebrow_asymmetry_dominance_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_mouth_asymmetry_trust_variance(const teddy_bear_geometry_t *geom, double mouth_asymmetry_val, double *trust_variance_out) {
    if (!geom || mouth_asymmetry_val < 0.0 || !trust_variance_out) {
        return false;
    }
    *trust_variance_out = (1.0 / (1.0 + mouth_asymmetry_val)) * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_sclera_submissiveness_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_width_asymmetry_trust_variance(const teddy_bear_geometry_t *geom, double width_asymmetry_val, double *trust_variance_out) {
    if (!geom || width_asymmetry_val < 0.0 || !trust_variance_out) {
        return false;
    }
    *trust_variance_out = (1.0 / (1.0 + width_asymmetry_val)) * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_gaze_shift_dominance_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_lip_compression_trust_variance_mod(const teddy_bear_geometry_t *geom, double lip_compression_val, double *trust_variance_out) {
    if (!geom || lip_compression_val < 0.0 || !trust_variance_out) {
        return false;
    }
    *trust_variance_out = (1.0 / (1.0 + lip_compression_val)) * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_eyebrow_asymmetry_trust_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_sclera_dominance_variance_mod(const teddy_bear_geometry_t *geom, double sclera_size_ratio, double *variance_out) {
    if (!geom || sclera_size_ratio < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = sclera_size_ratio * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_sclera_dominance_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_mouth_asymmetry_trust_variance_mod(const teddy_bear_geometry_t *geom, double mouth_asymmetry_val, double *trust_variance_out) {
    if (!geom || mouth_asymmetry_val < 0.0 || !trust_variance_out) {
        return false;
    }
    *trust_variance_out = (1.0 / (1.0 + mouth_asymmetry_val)) * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_mouth_asymmetry_dominance_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_width_asymmetry_trust_variance_mod(const teddy_bear_geometry_t *geom, double width_asym_val, double *variance_out) {
    if (!geom || width_asym_val < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = width_asym_val * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_mouth_asymmetry_trust_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_width_asymmetry_dominance_variance_mod(const teddy_bear_geometry_t *geom, double width_asym_val, double *variance_out) {
    if (!geom || width_asym_val < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = width_asym_val * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_eyebrow_asymmetry_trust_decay_variance_single(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_sclera_submissiveness_variance_mod(const teddy_bear_geometry_t *geom, double sclera_val, double *variance_out) {
    if (!geom || sclera_val < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = sclera_val * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_sclera_dominance_variance_mod_single(const teddy_bear_geometry_t *geom, double sclera_val, double *variance_out) {
    if (!geom || sclera_val < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = sclera_val * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_eyebrow_asymmetry_dominance_variance_mod(const teddy_bear_geometry_t *geom, double eyebrow_asym_val, double *variance_out) {
    if (!geom || eyebrow_asym_val < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = eyebrow_asym_val * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_eyebrow_asymmetry_trust_variance_mod_single(const teddy_bear_geometry_t *geom, double eyebrow_asym_val, double *variance_out) {
    if (!geom || eyebrow_asym_val < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = eyebrow_asym_val * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_sclera_submissiveness_decay_variance_single(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_sclera_dominance_decay_variance_single(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_eyebrow_asymmetry_dominance_decay_variance_single(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_sclera_submissiveness_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_sclera_dominance_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_keating_cross_cultural_dominance_consensus(const teddy_bear_geometry_t *geom, double gesture_prominence, double *consensus_out) {
    if (!geom || gesture_prominence < 0.0 || !consensus_out) {
        return false;
    }
    *consensus_out = gesture_prominence * 0.95 * (0.9 + geom->social_status * 0.2);
    return true;
}

bool evaluate_keating_gender_status_interaction(const teddy_bear_geometry_t *geom, double status_cue, int observer_gender, double *dominance_out) {
    if (!geom || status_cue < 0.0 || !dominance_out) {
        return false;
    }
    double multiplier = (observer_gender == 1) ? 1.08 : 0.95;
    *dominance_out = status_cue * multiplier * (0.95 + geom->command_authority * 0.1);
    return true;
}

bool evaluate_keating_babyfacedness_attractiveness_decoupling(const teddy_bear_geometry_t *geom, double babyface_index, int target_gender, double *attractiveness_out) {
    if (!geom || babyface_index < 0.0 || !attractiveness_out) {
        return false;
    }
    double baseline = babyface_index * 1.15;
    if (target_gender == 1) { // females
        *attractiveness_out = baseline * (1.0 + geom->empathy_index * 0.15);
    } else { // males
        *attractiveness_out = baseline * (0.85 + geom->parenting_capability * 0.1);
    }
    return true;
}

bool evaluate_keating_smile_status_congruence(const teddy_bear_geometry_t *geom, double smile_intensity, double status_rank, double *trust_score_out) {
    if (!geom || smile_intensity < 0.0 || status_rank <= 0.0 || !trust_score_out) {
        return false;
    }
    // Trust score is modulated by congruence between status (lower rank = higher status) and smiling behavior
    double congruence = fabs(smile_intensity - (1.0 / status_rank));
    *trust_score_out = (1.0 / (1.0 + congruence)) * (0.9 + geom->cooperative_negotiation * 0.2);
    return true;
}

bool evaluate_keating_pitch_vocal_status_congruence(const teddy_bear_geometry_t *geom, double pitch_variation, double status_rank, double *congruence_score_out) {
    if (!geom || pitch_variation < 0.0 || status_rank <= 0.0 || !congruence_score_out) {
        return false;
    }
    double target_pitch = 1.0 / (status_rank + 0.1);
    double diff = fabs(pitch_variation - target_pitch);
    *congruence_score_out = (1.0 / (1.0 + diff)) * (0.95 + geom->social_status * 0.15);
    return true;
}

bool evaluate_keating_dynamic_pose_asymmetry_consensus(const teddy_bear_geometry_t *geom, double posture_asymmetry, double group_size, double *consensus_score_out) {
    if (!geom || posture_asymmetry < 0.0 || group_size <= 0.0 || !consensus_score_out) {
        return false;
    }
    double factor = posture_asymmetry * (1.0 + (1.0 / group_size));
    *consensus_score_out = factor * (0.85 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_keating_sclera_gaze_decoupling(const teddy_bear_geometry_t *geom, double sclera_exposure, double aversion_speed, double *dominance_score_out) {
    if (!geom || sclera_exposure < 0.0 || aversion_speed < 0.0 || !dominance_score_out) {
        return false;
    }
    *dominance_score_out = (sclera_exposure * (1.0 + aversion_speed)) * (0.9 + geom->command_authority * 0.25);
    return true;
}

bool evaluate_keating_smile_dominance_attenuation(const teddy_bear_geometry_t *geom, double smile_intensity, int gender, double *attenuated_dominance_out) {
    if (!geom || smile_intensity < 0.0 || !attenuated_dominance_out) {
        return false;
    }
    double factor = (gender == 1) ? 0.75 : 0.90; // Larger reduction in perceived dominance for males
    *attenuated_dominance_out = (1.0 - smile_intensity * factor) * (1.1 + geom->leadership_profile * 0.35);
    return true;
}

bool evaluate_keating_babyfacedness_leadership_decoupling(const teddy_bear_geometry_t *geom, double babyface_index, double *leadership_score_out) {
    if (!geom || babyface_index < 0.0 || !leadership_score_out) {
        return false;
    }
    *leadership_score_out = (1.0 / (1.0 + babyface_index)) * (1.2 + geom->leadership_profile * 0.45);
    return true;
}

bool evaluate_keating_torso_asymmetry_status(const teddy_bear_geometry_t *geom, double torso_asymmetry_val, double *status_score_out) {
    if (!geom || torso_asymmetry_val < 0.0 || !status_score_out) {
        return false;
    }
    *status_score_out = torso_asymmetry_val * (0.95 + geom->social_status * 0.3);
    return true;
}

bool evaluate_keating_smile_cooperation_modulator(const teddy_bear_geometry_t *geom, double smile_intensity, double *cooperation_score_out) {
    if (!geom || smile_intensity < 0.0 || !cooperation_score_out) {
        return false;
    }
    *cooperation_score_out = smile_intensity * 1.25 * (0.9 + geom->empathy_index * 0.35);
    return true;
}

bool evaluate_keating_eyebrow_dominance_decoupling(const teddy_bear_geometry_t *geom, double eyebrow_asymmetry, double fwhr_val, double *dominance_score_out) {
    if (!geom || eyebrow_asymmetry < 0.0 || fwhr_val < 0.0 || !dominance_score_out) {
        return false;
    }
    *dominance_score_out = (fwhr_val / (1.0 + eyebrow_asymmetry * 0.5)) * (0.95 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_keating_jaw_attractiveness_interaction(const teddy_bear_geometry_t *geom, double jaw_scale, int target_gender, double *attractiveness_score_out) {
    if (!geom || jaw_scale < 0.0 || !attractiveness_score_out) {
        return false;
    }
    double factor = (target_gender == 1) ? 0.85 : 1.25; // Smaller jaws preferred for female attractiveness
    *attractiveness_score_out = (factor / (1.0 + jaw_scale * 0.4)) * (0.9 + geom->empathy_index * 0.3);
    return true;
}




