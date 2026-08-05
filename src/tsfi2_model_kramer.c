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
