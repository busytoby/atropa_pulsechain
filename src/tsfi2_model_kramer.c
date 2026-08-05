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
