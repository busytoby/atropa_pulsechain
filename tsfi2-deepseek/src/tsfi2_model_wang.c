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
