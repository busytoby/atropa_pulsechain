#include "auncient_teddy_personality.h"
#include <stdio.h>
#include <math.h>

bool evaluate_castle_diplomatic_alignment(const teddy_geometry_t *geom, double turn_taking_sync, double posture_alignment, double *alignment_out) {
    if (!geom || turn_taking_sync < 0.0 || posture_alignment < 0.0 || !alignment_out) {
        return false;
    }
    *alignment_out = turn_taking_sync * posture_alignment * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_castle_policy_consistency(const teddy_geometry_t *geom, double consistency_score, double *policy_consistency_out) {
    if (!geom || consistency_score < 0.0 || !policy_consistency_out) {
        return false;
    }
    *policy_consistency_out = consistency_score * (0.9 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_castle_diplomatic_alignment_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_alignment_out) {
    if (!geom || duration_sec < 0.0 || !decayed_alignment_out) {
        return false;
    }
    *decayed_alignment_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_castle_policy_consistency_variance(const teddy_geometry_t *geom, double consistency_score, double *variance_out) {
    if (!geom || consistency_score < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = consistency_score * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

