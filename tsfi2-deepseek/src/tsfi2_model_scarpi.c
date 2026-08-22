#include "auncient_teddy_bear_personality.h"
#include <stdio.h>
#include <math.h>

bool evaluate_scarpi_hedonic_orientation(const teddy_bear_geometry_t *geom, double playfulness_scale, double *hedonic_out) {
    if (!geom || playfulness_scale < 0.0 || !hedonic_out) {
        return false;
    }
    *hedonic_out = playfulness_scale * (0.8 + geom->empathy_index * 0.4 + (1.0 - geom->stiffness) * 0.2);
    return true;
}

bool evaluate_scarpi_utilitarian_orientation(const teddy_bear_geometry_t *geom, double efficiency_scale, double *utilitarian_out) {
    if (!geom || efficiency_scale < 0.0 || !utilitarian_out) {
        return false;
    }
    *utilitarian_out = efficiency_scale * (0.7 + geom->leadership_profile * 0.5 + geom->head_fwhr * 0.2);
    return true;
}

bool evaluate_scarpi_aesthetic_trust(const teddy_bear_geometry_t *geom, double base_trust, double *aesthetic_trust_out) {
    if (!geom || base_trust < 0.0 || !aesthetic_trust_out) {
        return false;
    }
    *aesthetic_trust_out = base_trust * (0.9 + geom->symmetry * 0.3 + geom->empathy_index * 0.2);
    return true;
}

bool evaluate_scarpi_utilitarian_decay(const teddy_bear_geometry_t *geom, double interaction_duration_sec, double *decayed_efficiency_out) {
    if (!geom || interaction_duration_sec < 0.0 || !decayed_efficiency_out) {
        return false;
    }
    *decayed_efficiency_out = exp(-interaction_duration_sec / (20.0 + geom->resilience_index * 10.0));
    return true;
}

bool evaluate_scarpi_hedonic_arousal(const teddy_bear_geometry_t *geom, double playfulness_scale, double novelty_index, double *arousal_out) {
    if (!geom || playfulness_scale < 0.0 || novelty_index < 0.0 || !arousal_out) {
        return false;
    }
    *arousal_out = playfulness_scale * novelty_index * (0.9 + geom->symmetry * 0.3);
    return true;
}

bool evaluate_scarpi_utilitarian_quality(const teddy_bear_geometry_t *geom, double stability_index, double *quality_rating_out) {
    if (!geom || stability_index < 0.0 || !quality_rating_out) {
        return false;
    }
    *quality_rating_out = stability_index * (0.8 + geom->leadership_profile * 0.4 + geom->symmetry * 0.2);
    return true;
}

bool evaluate_scarpi_hedonic_arousal_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_arousal_out) {
    if (!geom || duration_sec < 0.0 || !decayed_arousal_out) {
        return false;
    }
    *decayed_arousal_out = exp(-duration_sec / (15.0 + (1.0 - geom->stiffness) * 10.0));
    return true;
}

bool evaluate_scarpi_utilitarian_loyalty(const teddy_bear_geometry_t *geom, double success_rate, double *loyalty_out) {
    if (!geom || success_rate < 0.0 || !loyalty_out) {
        return false;
    }
    *loyalty_out = success_rate * (0.8 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_scarpi_hedonic_playfulness(const teddy_bear_geometry_t *geom, double base_playfulness, double *playfulness_out) {
    if (!geom || base_playfulness < 0.0 || !playfulness_out) {
        return false;
    }
    *playfulness_out = base_playfulness * (0.8 + geom->empathy_index * 0.4 + (1.0 - geom->stiffness) * 0.2);
    return true;
}

bool evaluate_scarpi_utilitarian_trust(const teddy_bear_geometry_t *geom, double safety_margin, double *operational_trust_out) {
    if (!geom || safety_margin < 0.0 || !operational_trust_out) {
        return false;
    }
    *operational_trust_out = safety_margin * (0.9 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_scarpi_hedonic_arousal_mod(const teddy_bear_geometry_t *geom, double novelty_shift, double brow_gesture_val, double *arousal_out) {
    if (!geom || novelty_shift < 0.0 || brow_gesture_val < 0.0 || !arousal_out) {
        return false;
    }
    *arousal_out = novelty_shift * brow_gesture_val * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_scarpi_utilitarian_quality_control(const teddy_bear_geometry_t *geom, double spatial_alignment, double *quality_control_out) {
    if (!geom || spatial_alignment < 0.0 || !quality_control_out) {
        return false;
    }
    *quality_control_out = spatial_alignment * (0.9 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_scarpi_hedonic_aesthetic_quality(const teddy_bear_geometry_t *geom, double aesthetic_alignment, double *quality_out) {
    if (!geom || aesthetic_alignment < 0.0 || !quality_out) {
        return false;
    }
    *quality_out = aesthetic_alignment * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_scarpi_utilitarian_consistency(const teddy_bear_geometry_t *geom, double consistency_score, double *consistency_mod_out) {
    if (!geom || consistency_score < 0.0 || !consistency_mod_out) {
        return false;
    }
    *consistency_mod_out = consistency_score * (0.9 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_scarpi_hedonic_interface_trust(const teddy_bear_geometry_t *geom, double aesthetic_trust_val, double *interface_trust_out) {
    if (!geom || aesthetic_trust_val < 0.0 || !interface_trust_out) {
        return false;
    }
    *interface_trust_out = aesthetic_trust_val * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_scarpi_utilitarian_operational_trust_mod(const teddy_bear_geometry_t *geom, double efficiency_val, double *operational_trust_out) {
    if (!geom || efficiency_val < 0.0 || !operational_trust_out) {
        return false;
    }
    *operational_trust_out = efficiency_val * (0.9 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_scarpi_hedonic_playful_engagement(const teddy_bear_geometry_t *geom, double novelty_scale, double aesthetic_alignment, double *engagement_out) {
    if (!geom || novelty_scale < 0.0 || aesthetic_alignment < 0.0 || !engagement_out) {
        return false;
    }
    *engagement_out = novelty_scale * aesthetic_alignment * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_scarpi_utilitarian_consistency_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_consistency_out) {
    if (!geom || duration_sec < 0.0 || !decayed_consistency_out) {
        return false;
    }
    *decayed_consistency_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_scarpi_hedonic_playful_arousal_variance(const teddy_bear_geometry_t *geom, double novelty_shift, double brow_gesture_val, double *arousal_variance_out) {
    if (!geom || novelty_shift < 0.0 || brow_gesture_val < 0.0 || !arousal_variance_out) {
        return false;
    }
    *arousal_variance_out = novelty_shift * brow_gesture_val * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_scarpi_utilitarian_operational_trust_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_operational_trust_out) {
    if (!geom || duration_sec < 0.0 || !decayed_operational_trust_out) {
        return false;
    }
    *decayed_operational_trust_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_scarpi_utilitarian_consistency_variance(const teddy_bear_geometry_t *geom, double consistency_score, double *variance_out) {
    if (!geom || consistency_score < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = consistency_score * (0.9 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_scarpi_hedonic_interface_trust_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_trust_out) {
    if (!geom || duration_sec < 0.0 || !decayed_trust_out) {
        return false;
    }
    *decayed_trust_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_scarpi_hedonic_trust_variance(const teddy_bear_geometry_t *geom, double aesthetic_rating, double novelty_scale, double *trust_variance_out) {
    if (!geom || aesthetic_rating < 0.0 || novelty_scale < 0.0 || !trust_variance_out) {
        return false;
    }
    *trust_variance_out = aesthetic_rating * novelty_scale * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_scarpi_utilitarian_quality_control_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_quality_out) {
    if (!geom || duration_sec < 0.0 || !decayed_quality_out) {
        return false;
    }
    *decayed_quality_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_scarpi_hedonic_playful_engagement_variance(const teddy_bear_geometry_t *geom, double novelty_scale, double aesthetic_alignment, double *engagement_variance_out) {
    if (!geom || novelty_scale < 0.0 || aesthetic_alignment < 0.0 || !engagement_variance_out) {
        return false;
    }
    *engagement_variance_out = novelty_scale * aesthetic_alignment * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_scarpi_utilitarian_consistency_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_scarpi_hedonic_playful_arousal_mod_variance(const teddy_bear_geometry_t *geom, double novelty_scale, double design_symmetry, double *variance_out) {
    if (!geom || novelty_scale < 0.0 || design_symmetry < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = novelty_scale * design_symmetry * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_scarpi_utilitarian_quality_control_variance(const teddy_bear_geometry_t *geom, double complexity_level, double *variance_out) {
    if (!geom || complexity_level < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = complexity_level * (0.9 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_scarpi_hedonic_playful_arousal_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_scarpi_utilitarian_quality_control_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_scarpi_hedonic_playful_engagement_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_scarpi_utilitarian_consistency_variance_mod(const teddy_bear_geometry_t *geom, double complexity_level, double *variance_out) {
    if (!geom || complexity_level < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = complexity_level * (0.9 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_scarpi_utilitarian_quality_control_variance_mod(const teddy_bear_geometry_t *geom, double complexity_level, double *variance_out) {
    if (!geom || complexity_level < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = complexity_level * (0.9 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_scarpi_hedonic_trust(const teddy_bear_geometry_t *geom, double aesthetic_rating, double novelty_scale, double *trust_out) {
    if (!geom || aesthetic_rating < 0.0 || novelty_scale < 0.0 || !trust_out) {
        return false;
    }
    *trust_out = aesthetic_rating * novelty_scale * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_scarpi_utilitarian_efficiency(const teddy_bear_geometry_t *geom, double latency_sec, double *efficiency_out) {
    if (!geom || latency_sec < 0.0 || !efficiency_out) {
        return false;
    }
    *efficiency_out = (1.0 / (1.0 + latency_sec)) * (0.9 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_scarpi_hedonic_playful_arousal_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (15.0 + geom->empathy_index * 5.0));
    return true;
}

bool evaluate_scarpi_utilitarian_operational_trust_variance_mod(const teddy_bear_geometry_t *geom, double trust_score, double *variance_out) {
    if (!geom || trust_score < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = trust_score * (0.8 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_scarpi_hedonic_playful_engagement_variance_mod(const teddy_bear_geometry_t *geom, double engagement_score, double *variance_out) {
    if (!geom || engagement_score < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = engagement_score * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_scarpi_utilitarian_consistency_variance_mod_single(const teddy_bear_geometry_t *geom, double consistency_score, double *variance_out) {
    if (!geom || consistency_score < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = consistency_score * (0.9 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_scarpi_utilitarian_quality_control_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (15.0 + geom->empathy_index * 5.0));
    return true;
}

bool evaluate_scarpi_hedonic_playful_engagement_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (15.0 + geom->empathy_index * 5.0));
    return true;
}

bool evaluate_scarpi_utilitarian_consistency_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (15.0 + geom->empathy_index * 5.0));
    return true;
}

bool evaluate_scarpi_utilitarian_operational_trust_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (15.0 + geom->empathy_index * 5.0));
    return true;
}

bool evaluate_scarpi_hedonic_interface_trust_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (15.0 + geom->empathy_index * 5.0));
    return true;
}

