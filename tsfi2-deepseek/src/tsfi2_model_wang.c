#include "auncient_teddy_personality.h"
#include <stdio.h>
#include <math.h>

bool evaluate_wang_geigel_character_warmth(const teddy_geometry_t *geom, double gaze_shift_freq, double head_tilt_val, double *warmth_score_out) {
    if (!geom || gaze_shift_freq < 0.0 || head_tilt_val < 0.0 || !warmth_score_out) {
        return false;
    }
    *warmth_score_out = (gaze_shift_freq * 0.4 + head_tilt_val * 0.6) * (0.9 + geom->empathy_index * 0.25);
    return true;
}

bool evaluate_wang_geigel_competence_projection(const teddy_geometry_t *geom, double posture_pitch, double dominance_cue_val, double *competence_score_out) {
    if (!geom || posture_pitch < 0.0 || dominance_cue_val < 0.0 || !competence_score_out) {
        return false;
    }
    *competence_score_out = (posture_pitch * 0.5 + dominance_cue_val * 0.5) * (0.95 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_wang_geigel_emotional_contagion(const teddy_geometry_t *geom, double expression_intensity, double pupil_dilation_sync, double *contagion_score_out) {
    if (!geom || expression_intensity < 0.0 || pupil_dilation_sync < 0.0 || !contagion_score_out) {
        return false;
    }
    *contagion_score_out = (expression_intensity * 0.6 + pupil_dilation_sync * 0.4) * (0.85 + geom->empathy_index * 0.35);
    return true;
}

bool evaluate_wang_geigel_warmth_attenuation(const teddy_geometry_t *geom, double gaze_erraticness, double *attenuated_warmth_out) {
    if (!geom || gaze_erraticness < 0.0 || !attenuated_warmth_out) {
        return false;
    }
    *attenuated_warmth_out = (1.0 / (1.0 + gaze_erraticness * 0.8)) * (1.15 + geom->empathy_index * 0.3);
    return true;
}

bool evaluate_wang_geigel_gaze_dominance_modulator(const teddy_geometry_t *geom, double direct_gaze_ratio, double *dominance_score_out) {
    if (!geom || direct_gaze_ratio < 0.0 || !dominance_score_out) {
        return false;
    }
    *dominance_score_out = direct_gaze_ratio * (1.1 + geom->leadership_profile * 0.45);
    return true;
}

bool evaluate_wang_geigel_realism_attraction_congruence(const teddy_geometry_t *geom, double realism_index, double expression_intensity, double *attraction_score_out) {
    if (!geom || realism_index < 0.0 || expression_intensity < 0.0 || !attraction_score_out) {
        return false;
    }
    double uncanny_factor = fabs(realism_index - 0.8) * expression_intensity;
    *attraction_score_out = (1.0 / (1.0 + uncanny_factor)) * (0.95 + geom->social_extraversion * 0.3);
    return true;
}

