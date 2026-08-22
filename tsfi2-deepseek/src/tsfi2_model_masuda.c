#include "auncient_teddy_bear_personality.h"
#include <stdio.h>
#include <math.h>

bool evaluate_masuda_perceived_naturalness(const teddy_bear_geometry_t *geom, double sync_delay_ms, double smile_intensity, double *naturalness_out) {
    if (!geom || sync_delay_ms < 0.0 || smile_intensity < 0.0 || !naturalness_out) {
        return false;
    }
    // High sync delay reduces perceived naturalness; optimal delay is small
    double factor = exp(-sync_delay_ms / 100.0);
    *naturalness_out = (smile_intensity * factor) * (0.95 + geom->reassurance_capability * 0.2);
    return true;
}

bool evaluate_masuda_conversational_familiarity(const teddy_bear_geometry_t *geom, double reciprocal_gaze_sync, double symmetry_val, double *familiarity_out) {
    if (!geom || reciprocal_gaze_sync < 0.0 || symmetry_val < 0.0 || !familiarity_out) {
        return false;
    }
    *familiarity_out = (reciprocal_gaze_sync * 0.6 + symmetry_val * 0.4) * (0.9 + geom->empathy_index * 0.25);
    return true;
}

bool evaluate_masuda_empathic_synchronization(const teddy_bear_geometry_t *geom, double blink_sync_rate, double pupil_dilation_sync, double *empathy_out) {
    if (!geom || blink_sync_rate < 0.0 || pupil_dilation_sync < 0.0 || !empathy_out) {
        return false;
    }
    *empathy_out = (blink_sync_rate * 0.5 + pupil_dilation_sync * 0.5) * (0.85 + geom->empathy_index * 0.35);
    return true;
}

bool evaluate_masuda_naturalness_attenuation(const teddy_bear_geometry_t *geom, double sync_delay_ms, double *attenuated_naturalness_out) {
    if (!geom || sync_delay_ms < 0.0 || !attenuated_naturalness_out) {
        return false;
    }
    *attenuated_naturalness_out = (1.0 / (1.0 + (sync_delay_ms / 150.0))) * (1.10 + geom->reassurance_capability * 0.25);
    return true;
}

bool evaluate_masuda_conversational_familiarity_decay(const teddy_bear_geometry_t *geom, double initial_familiarity, double duration_sec, double *decayed_familiarity_out) {
    if (!geom || initial_familiarity < 0.0 || duration_sec < 0.0 || !decayed_familiarity_out) {
        return false;
    }
    *decayed_familiarity_out = initial_familiarity * exp(-duration_sec / (24.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_masuda_empathic_sync_variance(const teddy_bear_geometry_t *geom, double blink_sync_rate, double pupil_dilation_sync, double *sync_variance_out) {
    if (!geom || blink_sync_rate < 0.0 || pupil_dilation_sync < 0.0 || !sync_variance_out) {
        return false;
    }
    double diff = fabs(blink_sync_rate - pupil_dilation_sync);
    *sync_variance_out = (1.0 - diff * 0.85) * (0.95 + geom->empathy_index * 0.25);
    return true;
}

bool evaluate_masuda_naturalness_variance(const teddy_bear_geometry_t *geom, double reciprocal_gaze_sync, double *naturalness_variance_out) {
    if (!geom || reciprocal_gaze_sync < 0.0 || !naturalness_variance_out) {
        return false;
    }
    *naturalness_variance_out = reciprocal_gaze_sync * (0.9 + geom->reassurance_capability * 0.3);
    return true;
}

bool evaluate_masuda_reciprocal_gaze_attenuation(const teddy_bear_geometry_t *geom, double gaze_return_delay_ms, double *attenuated_gaze_score_out) {
    if (!geom || gaze_return_delay_ms < 0.0 || !attenuated_gaze_score_out) {
        return false;
    }
    *attenuated_gaze_score_out = (1.0 / (1.0 + (gaze_return_delay_ms / 200.0))) * (0.95 + geom->empathy_index * 0.2);
    return true;
}

bool evaluate_masuda_empathic_contagion_interaction(const teddy_bear_geometry_t *geom, double sync_level, double contagion_rate, double *interaction_score_out) {
    if (!geom || sync_level < 0.0 || contagion_rate < 0.0 || !interaction_score_out) {
        return false;
    }
    *interaction_score_out = (sync_level * 0.5 + contagion_rate * 0.5) * (0.95 + geom->cooperative_negotiation * 0.35);
    return true;
}


