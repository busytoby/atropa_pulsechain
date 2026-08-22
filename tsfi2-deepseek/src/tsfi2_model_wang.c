#include "auncient_teddy_bear_personality.h"
#include <stdio.h>
#include <math.h>

bool evaluate_wang_geigel_character_warmth(const teddy_bear_geometry_t *geom, double gaze_shift_freq, double head_tilt_val, double *warmth_score_out) {
    if (!geom || gaze_shift_freq < 0.0 || head_tilt_val < 0.0 || !warmth_score_out) {
        return false;
    }
    *warmth_score_out = (gaze_shift_freq * 0.4 + head_tilt_val * 0.6) * (0.9 + geom->empathy_index * 0.25);
    return true;
}

bool evaluate_wang_geigel_competence_projection(const teddy_bear_geometry_t *geom, double posture_pitch, double dominance_cue_val, double *competence_score_out) {
    if (!geom || posture_pitch < 0.0 || dominance_cue_val < 0.0 || !competence_score_out) {
        return false;
    }
    *competence_score_out = (posture_pitch * 0.5 + dominance_cue_val * 0.5) * (0.95 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_wang_geigel_emotional_contagion(const teddy_bear_geometry_t *geom, double expression_intensity, double pupil_dilation_sync, double *contagion_score_out) {
    if (!geom || expression_intensity < 0.0 || pupil_dilation_sync < 0.0 || !contagion_score_out) {
        return false;
    }
    *contagion_score_out = (expression_intensity * 0.6 + pupil_dilation_sync * 0.4) * (0.85 + geom->empathy_index * 0.35);
    return true;
}

bool evaluate_wang_geigel_warmth_attenuation(const teddy_bear_geometry_t *geom, double gaze_erraticness, double *attenuated_warmth_out) {
    if (!geom || gaze_erraticness < 0.0 || !attenuated_warmth_out) {
        return false;
    }
    *attenuated_warmth_out = (1.0 / (1.0 + gaze_erraticness * 0.8)) * (1.15 + geom->empathy_index * 0.3);
    return true;
}

bool evaluate_wang_geigel_gaze_dominance_modulator(const teddy_bear_geometry_t *geom, double direct_gaze_ratio, double *dominance_score_out) {
    if (!geom || direct_gaze_ratio < 0.0 || !dominance_score_out) {
        return false;
    }
    *dominance_score_out = direct_gaze_ratio * (1.1 + geom->leadership_profile * 0.45);
    return true;
}

bool evaluate_wang_geigel_realism_attraction_congruence(const teddy_bear_geometry_t *geom, double realism_index, double expression_intensity, double *attraction_score_out) {
    if (!geom || realism_index < 0.0 || expression_intensity < 0.0 || !attraction_score_out) {
        return false;
    }
    double uncanny_factor = fabs(realism_index - 0.8) * expression_intensity;
    *attraction_score_out = (1.0 / (1.0 + uncanny_factor)) * (0.95 + geom->social_extraversion * 0.3);
    return true;
}

bool evaluate_wang_geigel_gaze_competence_modulator(const teddy_bear_geometry_t *geom, double direct_gaze_ratio, double aversion_speed, double *competence_score_out) {
    if (!geom || direct_gaze_ratio < 0.0 || aversion_speed < 0.0 || !competence_score_out) {
        return false;
    }
    *competence_score_out = (direct_gaze_ratio * 0.7 + (1.0 / (1.0 + aversion_speed)) * 0.3) * (0.95 + geom->leadership_profile * 0.25);
    return true;
}

bool evaluate_wang_geigel_contagion_arousal(const teddy_bear_geometry_t *geom, double pupil_dilation_sync, double head_tilt_val, double *arousal_score_out) {
    if (!geom || pupil_dilation_sync < 0.0 || head_tilt_val < 0.0 || !arousal_score_out) {
        return false;
    }
    *arousal_score_out = (pupil_dilation_sync * 0.5 + head_tilt_val * 0.5) * (0.9 + geom->social_extraversion * 0.35);
    return true;
}

bool evaluate_wang_geigel_realism_trust_congruence(const teddy_bear_geometry_t *geom, double realism_index, double smile_intensity, double *trustworthiness_out) {
    if (!geom || realism_index < 0.0 || smile_intensity < 0.0 || !trustworthiness_out) {
        return false;
    }
    double diff = fabs(realism_index - smile_intensity);
    *trustworthiness_out = (1.0 / (1.0 + diff)) * (0.95 + geom->cooperative_negotiation * 0.2);
    return true;
}

bool evaluate_wang_geigel_emotional_contagion_decay(const teddy_bear_geometry_t *geom, double initial_contagion, double duration_sec, double *decayed_contagion_out) {
    if (!geom || initial_contagion < 0.0 || duration_sec < 0.0 || !decayed_contagion_out) {
        return false;
    }
    *decayed_contagion_out = initial_contagion * exp(-duration_sec / (18.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_wang_geigel_competence_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (16.0 + geom->resilience_index * 5.0));
    return true;
}

bool evaluate_wang_geigel_gaze_realism_mismatch(const teddy_bear_geometry_t *geom, double gaze_shift_freq, double realism_index, double *mismatch_score_out) {
    if (!geom || gaze_shift_freq < 0.0 || realism_index < 0.0 || !mismatch_score_out) {
        return false;
    }
    *mismatch_score_out = fabs(gaze_shift_freq - realism_index) * (1.1 + geom->behavioral_mismatch * 0.3);
    return true;
}

bool evaluate_wang_geigel_avatar_blink_trust(const teddy_bear_geometry_t *geom, double blink_rate, double *trust_score_out) {
    if (!geom || blink_rate < 0.0 || !trust_score_out) {
        return false;
    }
    // Moderate blinks are perceived as trustful; high blink rates reduce trust
    double optimal_diff = fabs(blink_rate - 0.25);
    *trust_score_out = (1.0 / (1.0 + optimal_diff * 1.5)) * (0.95 + geom->cooperative_negotiation * 0.2);
    return true;
}

bool evaluate_wang_geigel_avatar_head_shake(const teddy_bear_geometry_t *geom, double head_shake_intensity, double *negativity_score_out) {
    if (!geom || head_shake_intensity < 0.0 || !negativity_score_out) {
        return false;
    }
    *negativity_score_out = head_shake_intensity * 1.35 * (1.0 + geom->behavioral_mismatch * 0.25);
    return true;
}

bool evaluate_wang_geigel_avatar_gaze_submissiveness(const teddy_bear_geometry_t *geom, double averted_gaze_ratio, double posture_pitch, double *submissiveness_score_out) {
    if (!geom || averted_gaze_ratio < 0.0 || posture_pitch < 0.0 || !submissiveness_score_out) {
        return false;
    }
    *submissiveness_score_out = (averted_gaze_ratio * 0.6 + (1.0 / (1.0 + posture_pitch)) * 0.4) * (0.9 + geom->confrontational_assertiveness * -0.2);
    return true;
}

bool evaluate_wang_geigel_avatar_smile_attractiveness(const teddy_bear_geometry_t *geom, double smile_intensity, double head_tilt_val, double *attractiveness_score_out) {
    if (!geom || smile_intensity < 0.0 || head_tilt_val < 0.0 || !attractiveness_score_out) {
        return false;
    }
    *attractiveness_score_out = (smile_intensity * 0.65 + head_tilt_val * 0.35) * (0.9 + geom->empathy_index * 0.3);
    return true;
}

bool evaluate_wang_geigel_avatar_eyebrow_furrow_threat(const teddy_bear_geometry_t *geom, double furrow_intensity, double direct_gaze_ratio, double *threat_score_out) {
    if (!geom || furrow_intensity < 0.0 || direct_gaze_ratio < 0.0 || !threat_score_out) {
        return false;
    }
    *threat_score_out = (furrow_intensity * 0.7 + direct_gaze_ratio * 0.3) * (1.1 + geom->behavioral_mismatch * 0.25);
    return true;
}

bool evaluate_wang_geigel_avatar_realism_warmth_interaction(const teddy_bear_geometry_t *geom, double realism_index, double smile_intensity, double *warmth_score_out) {
    if (!geom || realism_index < 0.0 || smile_intensity < 0.0 || !warmth_score_out) {
        return false;
    }
    // High realism interacts positively with smile to create high warmth; low realism creates Uncanny valley dampening
    double multiplier = (realism_index >= 0.7) ? 1.25 : 0.65;
    *warmth_score_out = (smile_intensity * multiplier) * (0.9 + geom->reassurance_capability * 0.35);
    return true;
}





