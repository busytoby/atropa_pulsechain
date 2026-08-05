#include "auncient_teddy_personality.h"
#include <stdio.h>
#include <math.h>

bool evaluate_masuda_perceived_naturalness(const teddy_geometry_t *geom, double sync_delay_ms, double smile_intensity, double *naturalness_out) {
    if (!geom || sync_delay_ms < 0.0 || smile_intensity < 0.0 || !naturalness_out) {
        return false;
    }
    // High sync delay reduces perceived naturalness; optimal delay is small
    double factor = exp(-sync_delay_ms / 100.0);
    *naturalness_out = (smile_intensity * factor) * (0.95 + geom->reassurance_capability * 0.2);
    return true;
}

bool evaluate_masuda_conversational_familiarity(const teddy_geometry_t *geom, double reciprocal_gaze_sync, double symmetry_val, double *familiarity_out) {
    if (!geom || reciprocal_gaze_sync < 0.0 || symmetry_val < 0.0 || !familiarity_out) {
        return false;
    }
    *familiarity_out = (reciprocal_gaze_sync * 0.6 + symmetry_val * 0.4) * (0.9 + geom->empathy_index * 0.25);
    return true;
}

bool evaluate_masuda_empathic_synchronization(const teddy_geometry_t *geom, double blink_sync_rate, double pupil_dilation_sync, double *empathy_out) {
    if (!geom || blink_sync_rate < 0.0 || pupil_dilation_sync < 0.0 || !empathy_out) {
        return false;
    }
    *empathy_out = (blink_sync_rate * 0.5 + pupil_dilation_sync * 0.5) * (0.85 + geom->empathy_index * 0.35);
    return true;
}
