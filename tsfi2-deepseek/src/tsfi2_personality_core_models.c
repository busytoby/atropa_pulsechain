#include "auncient_teddy_bear_personality.h"
#include <stdio.h>
#include <math.h>

double evaluate_fw_threat_level(const teddy_bear_geometry_t *geom) {
    if (!geom) return 0.0;
    double threat = (geom->head_fwhr * 1.5) + (geom->jaw_scale * 0.8) - (geom->symmetry * 0.3);
    if (threat < 0.0) threat = 0.0;
    return threat;
}

bool evaluate_behavioral_threat_mismatch(const teddy_bear_geometry_t *geom, double *mismatch_score) {
    if (!geom || !mismatch_score) {
        return false;
    }
    double physical_threat = evaluate_fw_threat_level(geom);
    double vocal_factor = (geom->vocal_pitch > 200.0) ? (geom->vocal_pitch / 250.0) : 0.5;
    *mismatch_score = physical_threat * vocal_factor + (geom->behavioral_mismatch * 1.2);
    return true;
}

double evaluate_reactive_mismatch_retaliation(const teddy_bear_geometry_t *geom) {
    if (!geom) return 0.0;
    double physical_threat = evaluate_fw_threat_level(geom);
    double score = physical_threat * (1.0 + geom->behavioral_mismatch) * (0.5 + geom->status_aggression);
    if (score < 0.0) score = 0.0;
    return score;
}

bool evaluate_exposure_threat_consistency(const teddy_bear_geometry_t *geom, double exposure_ms, double *perceived_threat_out) {
    if (!geom || exposure_ms < 1.0 || !perceived_threat_out) {
        return false;
    }
    double stable_threat = evaluate_fw_threat_level(geom);
    double exposure_factor = 1.0;
    if (exposure_ms < 39.0) {
        exposure_factor = 0.7 + (exposure_ms / 39.0) * 0.3;
    }
    *perceived_threat_out = stable_threat * exposure_factor;
    return true;
}

bool evaluate_cooperative_cheating_risk(const teddy_bear_geometry_t *geom, double social_trust_factor, double *cheating_risk_out) {
    if (!geom || !cheating_risk_out) {
        return false;
    }
    double physical_threat = evaluate_fw_threat_level(geom);
    double risk = (physical_threat * (0.5 + geom->status_aggression)) - (social_trust_factor * 0.4);
    if (risk < 0.0) risk = 0.0;
    *cheating_risk_out = risk;
    return true;
}

bool evaluate_rapid_threat_limit(const teddy_bear_geometry_t *geom, double exposure_ms, double *detected_threat_out) {
    if (!geom || exposure_ms < 0.0 || !detected_threat_out) {
        return false;
    }
    double base_threat = evaluate_fw_threat_level(geom);
    double scale = 1.0;
    if (exposure_ms < 100.0) {
        scale = exposure_ms / 100.0;
    }
    *detected_threat_out = base_threat * scale;
    return true;
}

bool evaluate_reactive_retaliation_profile(const teddy_bear_geometry_t *geom, double provocation_intensity, double *retaliation_out) {
    if (!geom || provocation_intensity < 0.0 || !retaliation_out) {
        return false;
    }
    double formidability = geom->jaw_scale * 1.5;
    *retaliation_out = provocation_intensity * formidability * (1.0 + geom->status_aggression);
    return true;
}

bool evaluate_provocation_aggression_threshold(const teddy_bear_geometry_t *geom, double base_provocation, double *aggression_threshold_out) {
    if (!geom || base_provocation < 0.0 || !aggression_threshold_out) {
        return false;
    }
    double sensitivity = (geom->head_fwhr * 1.5) + (geom->status_aggression * 2.0);
    double threshold = base_provocation / (1.0 + sensitivity);
    if (threshold < 0.0) threshold = 0.0;
    *aggression_threshold_out = threshold;
    return true;
}

bool evaluate_exposure_adjusted_threat_threshold(const teddy_bear_geometry_t *geom, double exposure_ms, double *adjusted_threat_threshold_out) {
    if (!geom || exposure_ms < 0.0 || !adjusted_threat_threshold_out) {
        return false;
    }
    double base_threshold = evaluate_fw_threat_level(geom);
    double duration_scale = (exposure_ms > 300.0) ? 1.0 : (exposure_ms / 300.0);
    *adjusted_threat_threshold_out = base_threshold * duration_scale;
    return true;
}

bool evaluate_status_dominance_provocation(const teddy_bear_geometry_t *geom, double provocation_scale, double *dominance_threat_out) {
    if (!geom || provocation_scale < 0.0 || !dominance_threat_out) {
        return false;
    }
    double formidability = evaluate_fw_threat_level(geom);
    *dominance_threat_out = formidability * provocation_scale * (1.0 + geom->status_aggression);
    return true;
}

bool evaluate_threat_replication_consistency(const teddy_bear_geometry_t *geom, const double *threat_observations, int count, double *consistency_out) {
    if (!geom || !threat_observations || count < 1 || !consistency_out) {
        return false;
    }
    double expected_threat = evaluate_fw_threat_level(geom);
    double error_sum = 0.0;
    for (int i = 0; i < count; ++i) {
        double diff = threat_observations[i] - expected_threat;
        error_sum += diff * diff;
    }
    double variance = error_sum / count;
    *consistency_out = 1.0 / (1.0 + variance);
    return true;
}

bool evaluate_reactive_retaliation_boundary(const teddy_bear_geometry_t *geom, double provocation_scale, double *retaliation_boundary_out) {
    if (!geom || provocation_scale < 0.0 || !retaliation_boundary_out) {
        return false;
    }
    double base_retaliation = 0.0;
    evaluate_reactive_retaliation_profile(geom, provocation_scale, &base_retaliation);
    *retaliation_boundary_out = base_retaliation * (1.0 + (geom->head_fwhr * 0.5));
    return true;
}

bool evaluate_retaliation_aggression_scaling(const teddy_bear_geometry_t *geom, double baseline_aggression, double *scaled_aggression_out) {
    if (!geom || baseline_aggression < 0.0 || !scaled_aggression_out) {
        return false;
    }
    double threat_level = evaluate_fw_threat_level(geom);
    *scaled_aggression_out = baseline_aggression * (1.0 + (threat_level * 0.4));
    return true;
}

bool evaluate_retaliation_threshold_decay(const teddy_bear_geometry_t *geom, double exposure_ms, double *decayed_threshold_out) {
    if (!geom || exposure_ms < 0.0 || !decayed_threshold_out) {
        return false;
    }
    double base_threshold = 2.0 / (1.0 + (geom->head_fwhr * 0.8));
    double decay_factor = exp(-exposure_ms * 0.005);
    *decayed_threshold_out = base_threshold * decay_factor;
    return true;
}

bool evaluate_provocation_exposure_decay(const teddy_bear_geometry_t *geom, double provocation_scale, double exposure_ms, double *decayed_threat_out) {
    if (!geom || provocation_scale < 0.0 || exposure_ms < 0.0 || !decayed_threat_out) {
        return false;
    }
    double base_threat = 0.0;
    evaluate_status_dominance_provocation(geom, provocation_scale, &base_threat);
    double decay_factor = exp(-exposure_ms * 0.003);
    *decayed_threat_out = base_threat * decay_factor;
    return true;
}

bool evaluate_retaliation_aggression_ceiling(const teddy_bear_geometry_t *geom, double provocation_scale, double *aggression_ceiling_out) {
    if (!geom || provocation_scale < 0.0 || !aggression_ceiling_out) {
        return false;
    }
    double threat_level = evaluate_fw_threat_level(geom);
    *aggression_ceiling_out = (threat_level * 2.0) + (provocation_scale * 1.5 * geom->status_aggression);
    return true;
}

bool evaluate_status_dominance_multiplier(const teddy_bear_geometry_t *geom, double provocation_scale, double *multiplier_out) {
    if (!geom || provocation_scale < 0.0 || !multiplier_out) {
        return false;
    }
    double base_multiplier = 1.0 + (geom->status_aggression * 0.5);
    *multiplier_out = base_multiplier * (1.0 + (provocation_scale * geom->head_fwhr * 0.4));
    return true;
}

bool evaluate_cheating_threat_index(const teddy_bear_geometry_t *geom, double provocation_scale, double *cheating_threat_out) {
    if (!geom || provocation_scale < 0.0 || !cheating_threat_out) {
        return false;
    }
    double base_cheat = 0.0;
    if (!evaluate_cooperative_cheating_risk(geom, 0.5, &base_cheat)) {
        return false;
    }
    *cheating_threat_out = base_cheat + (provocation_scale * 0.35 * geom->head_fwhr);
    return true;
}

bool evaluate_provocation_replication_consistency(const teddy_bear_geometry_t *geom, double replication_scale, double *consistency_out) {
    if (!geom || replication_scale < 0.0 || !consistency_out) {
        return false;
    }
    double threat = evaluate_fw_threat_level(geom);
    *consistency_out = threat * (1.0 + (replication_scale * geom->status_aggression * 0.15));
    return true;
}

bool evaluate_provocation_rapid_threat_limit(const teddy_bear_geometry_t *geom, double provocation_scale, double *limit_out) {
    if (!geom || provocation_scale < 0.0 || !limit_out) {
        return false;
    }
    double threat = evaluate_fw_threat_level(geom);
    *limit_out = threat * (1.5 + (provocation_scale * geom->status_aggression * 0.25));
    return true;
}

bool evaluate_provocation_retaliation_boundary(const teddy_bear_geometry_t *geom, double exposure_ms, double *boundary_out) {
    if (!geom || exposure_ms < 0.0 || !boundary_out) {
        return false;
    }
    double base_boundary = 0.0;
    if (!evaluate_reactive_retaliation_boundary(geom, exposure_ms, &base_boundary)) {
        return false;
    }
    *boundary_out = base_boundary * (1.1 + geom->status_aggression * 0.1);
    return true;
}

bool evaluate_uncanny_mismatch_index(const teddy_bear_geometry_t *geom, double *uncanny_score_out) {
    if (!geom || !uncanny_score_out) {
        return false;
    }
    double asymmetry_score = 1.0 - geom->symmetry;
    double pitch_mismatch = 0.0;
    if (geom->head_fwhr > 1.0 && geom->vocal_pitch > 220.0) {
        pitch_mismatch = (geom->head_fwhr - 1.0) * (geom->vocal_pitch - 220.0) / 100.0;
    }
    *uncanny_score_out = (asymmetry_score * 2.0) + pitch_mismatch + (geom->behavioral_mismatch * 1.5);
    return true;
}

bool evaluate_motion_uncanny_index(const teddy_bear_geometry_t *geom, double movement_stiffness, double *motion_uncanny_out) {
    if (!geom || !motion_uncanny_out) {
        return false;
    }
    double uncanny_base = 0.0;
    evaluate_uncanny_mismatch_index(geom, &uncanny_base);
    double motion_factor = (movement_stiffness > 0.6) ? (movement_stiffness * geom->symmetry) : 0.2;
    *motion_uncanny_out = uncanny_base + (motion_factor * 2.5);
    return true;
}

bool evaluate_expression_jitter_uncanny(const teddy_bear_geometry_t *geom, double jitter_frequency, double *uncanny_score_out) {
    if (!geom || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double jitter_factor = (jitter_frequency > 10.0) ? (jitter_frequency * 0.15) : 0.05;
    *uncanny_score_out = base_uncanny + (jitter_factor * (1.0 + geom->behavioral_mismatch));
    return true;
}

bool evaluate_exposure_decay_uncanny(const teddy_bear_geometry_t *geom, double exposure_duration_sec, double *decayed_uncanny_out) {
    if (!geom || exposure_duration_sec < 0.0 || !decayed_uncanny_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double tau = 10.0;
    *decayed_uncanny_out = base_uncanny * exp(-exposure_duration_sec / tau);
    return true;
}

bool evaluate_freeze_habituation_decay(const teddy_bear_geometry_t *geom, double exposure_duration_sec, double *decayed_uncanny_out) {
    if (!geom || exposure_duration_sec < 0.0 || !decayed_uncanny_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double tau = 15.0;
    *decayed_uncanny_out = base_uncanny * exp(-exposure_duration_sec / tau);
    return true;
}

bool evaluate_velocity_jitter_uncanny(const teddy_bear_geometry_t *geom, double velocity_variance, double *uncanny_score_out) {
    if (!geom || velocity_variance < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    *uncanny_score_out = base_uncanny + (velocity_variance * 3.5 * geom->behavioral_mismatch);
    return true;
}

bool evaluate_expression_freeze_uncanny(const teddy_bear_geometry_t *geom, double freeze_duration_sec, double *uncanny_score_out) {
    if (!geom || freeze_duration_sec < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double freeze_factor = (freeze_duration_sec > 1.5) ? (freeze_duration_sec * 2.0) : 0.2;
    *uncanny_score_out = base_uncanny + (freeze_factor * (1.0 + geom->behavioral_mismatch));
    return true;
}

bool evaluate_expression_freeze_frequency(const teddy_bear_geometry_t *geom, double freeze_frequency_hz, double *uncanny_score_out) {
    if (!geom || freeze_frequency_hz < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    *uncanny_score_out = base_uncanny + (freeze_frequency_hz * 1.5 * (1.0 + geom->behavioral_mismatch));
    return true;
}

bool evaluate_expression_asymmetry_uncanny(const teddy_bear_geometry_t *geom, double asymmetry_deviation, double *uncanny_score_out) {
    if (!geom || asymmetry_deviation < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    *uncanny_score_out = base_uncanny + (asymmetry_deviation * 4.5 * (1.0 - geom->symmetry));
    return true;
}

bool evaluate_expression_asymmetry_duration(const teddy_bear_geometry_t *geom, double asymmetry_duration_ms, double *uncanny_score_out) {
    if (!geom || asymmetry_duration_ms < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double duration_factor = (asymmetry_duration_ms > 200.0) ? (asymmetry_duration_ms * 0.02) : 0.01;
    *uncanny_score_out = base_uncanny + (duration_factor * (1.0 - geom->symmetry));
    return true;
}

bool evaluate_expression_sync_uncanny(const teddy_bear_geometry_t *geom, double sync_delay_ms, double *uncanny_score_out) {
    if (!geom || sync_delay_ms < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double sync_factor = (sync_delay_ms > 50.0) ? (sync_delay_ms * 0.08) : 0.05;
    *uncanny_score_out = base_uncanny + (sync_factor * (1.0 + geom->behavioral_mismatch));
    return true;
}

bool evaluate_sync_habituation_decay(const teddy_bear_geometry_t *geom, double exposure_duration_sec, double *decayed_uncanny_out) {
    if (!geom || exposure_duration_sec < 0.0 || !decayed_uncanny_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double tau = 12.0;
    *decayed_uncanny_out = base_uncanny * exp(-exposure_duration_sec / tau);
    return true;
}

bool evaluate_acceleration_jitter_uncanny(const teddy_bear_geometry_t *geom, double acceleration_variance, double *uncanny_score_out) {
    if (!geom || acceleration_variance < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    *uncanny_score_out = base_uncanny + (acceleration_variance * 5.0 * geom->behavioral_mismatch);
    return true;
}

bool evaluate_acceleration_habituation_decay(const teddy_bear_geometry_t *geom, double exposure_duration_sec, double *decayed_uncanny_out) {
    if (!geom || exposure_duration_sec < 0.0 || !decayed_uncanny_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double tau = 14.0;
    *decayed_uncanny_out = base_uncanny * exp(-exposure_duration_sec / tau);
    return true;
}

bool evaluate_animation_acceleration_jitter(const teddy_bear_geometry_t *geom, double acceleration_jitter, double *uncanny_score_out) {
    if (!geom || acceleration_jitter < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    *uncanny_score_out = base_uncanny + (acceleration_jitter * 6.0 * geom->behavioral_mismatch);
    return true;
}

bool evaluate_amplitude_jitter_uncanny(const teddy_bear_geometry_t *geom, double amplitude_variance, double *uncanny_score_out) {
    if (!geom || amplitude_variance < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    *uncanny_score_out = base_uncanny + (amplitude_variance * 4.0 * geom->behavioral_mismatch);
    return true;
}

bool evaluate_expression_amplitude_jitter(const teddy_bear_geometry_t *geom, double amplitude_variance, double *uncanny_score_out) {
    return evaluate_amplitude_jitter_uncanny(geom, amplitude_variance, uncanny_score_out);
}

bool evaluate_amplitude_habituation_decay(const teddy_bear_geometry_t *geom, double exposure_duration_sec, double *decayed_uncanny_out) {
    if (!geom || exposure_duration_sec < 0.0 || !decayed_uncanny_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double tau = 16.0;
    *decayed_uncanny_out = base_uncanny * exp(-exposure_duration_sec / tau);
    return true;
}

bool evaluate_vocal_visual_sync_uncanny(const teddy_bear_geometry_t *geom, double audio_lag_ms, double *uncanny_score_out) {
    if (!geom || audio_lag_ms < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double lag_factor = (audio_lag_ms > 80.0) ? (audio_lag_ms * 0.05) : 0.05;
    *uncanny_score_out = base_uncanny + (lag_factor * (1.0 + geom->behavioral_mismatch));
    return true;
}

bool evaluate_vocal_visual_acceleration_sync(const teddy_bear_geometry_t *geom, double acceleration_delay_ms, double *uncanny_score_out) {
    if (!geom || acceleration_delay_ms < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double delay_factor = (acceleration_delay_ms > 40.0) ? (acceleration_delay_ms * 0.1) : 0.05;
    *uncanny_score_out = base_uncanny + (delay_factor * (1.0 + geom->behavioral_mismatch));
    return true;
}

bool evaluate_pitch_velocity_mismatch(const teddy_bear_geometry_t *geom, double pitch_velocity_mismatch, double *uncanny_score_out) {
    if (!geom || pitch_velocity_mismatch < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    *uncanny_score_out = base_uncanny + (pitch_velocity_mismatch * 3.5 * geom->behavioral_mismatch);
    return true;
}

bool evaluate_vocal_visual_pitch_mismatch(const teddy_bear_geometry_t *geom, double pitch_velocity_mismatch, double *uncanny_score_out) {
    return evaluate_pitch_velocity_mismatch(geom, pitch_velocity_mismatch, uncanny_score_out);
}

bool evaluate_pitch_freeze_uncanny(const teddy_bear_geometry_t *geom, double pitch_velocity_mismatch, double freeze_duration_sec, double *uncanny_score_out) {
    if (!geom || pitch_velocity_mismatch < 0.0 || freeze_duration_sec < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_mismatch = 0.0;
    if (!evaluate_vocal_visual_pitch_mismatch(geom, pitch_velocity_mismatch, &base_mismatch)) {
        return false;
    }
    double freeze_factor = (freeze_duration_sec > 1.5) ? (freeze_duration_sec * 0.4) : 0.1;
    *uncanny_score_out = base_mismatch + (freeze_factor * (1.0 + geom->behavioral_mismatch));
    return true;
}

bool evaluate_pitch_jitter_uncanny(const teddy_bear_geometry_t *geom, double pitch_velocity_mismatch, double jitter_deviation, double *uncanny_score_out) {
    if (!geom || pitch_velocity_mismatch < 0.0 || jitter_deviation < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_mismatch = 0.0;
    if (!evaluate_vocal_visual_pitch_mismatch(geom, pitch_velocity_mismatch, &base_mismatch)) {
        return false;
    }
    double jitter_factor = (jitter_deviation > 0.05) ? (jitter_deviation * 2.5) : 0.05;
    *uncanny_score_out = base_mismatch + (jitter_factor * (1.0 + geom->behavioral_mismatch));
    return true;
}

bool evaluate_motion_acceleration_mismatch(const teddy_bear_geometry_t *geom, double acceleration_mismatch, double *uncanny_score_out) {
    if (!geom || acceleration_mismatch < 0.0 || !uncanny_score_out) {
        return false;
    }
    *uncanny_score_out = acceleration_mismatch * (1.2 + geom->behavioral_mismatch * 0.4);
    return true;
}

bool evaluate_motion_freeze_uncanny(const teddy_bear_geometry_t *geom, double motion_mismatch, double freeze_duration_sec, double *uncanny_score_out) {
    if (!geom || motion_mismatch < 0.0 || freeze_duration_sec < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_mismatch = 0.0;
    if (!evaluate_motion_uncanny_index(geom, motion_mismatch, &base_mismatch)) {
        return false;
    }
    double freeze_factor = (freeze_duration_sec > 1.5) ? (freeze_duration_sec * 0.45) : 0.12;
    *uncanny_score_out = base_mismatch + (freeze_factor * (1.0 + geom->behavioral_mismatch));
    return true;
}

bool evaluate_pitch_mismatch_habituation_decay(const teddy_bear_geometry_t *geom, double exposure_duration_sec, double *decayed_uncanny_out) {
    if (!geom || exposure_duration_sec < 0.0 || !decayed_uncanny_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double tau = 18.0;
    *decayed_uncanny_out = base_uncanny * exp(-exposure_duration_sec / tau);
    return true;
}

bool evaluate_spatial_interaction_distance(const teddy_bear_geometry_t *geom, double physical_distance_meters, double *interaction_uncanny_out) {
    if (!geom || physical_distance_meters < 0.0 || !interaction_uncanny_out) {
        return false;
    }
    double proximity_factor = 0.0;
    if (physical_distance_meters < 1.0) {
        proximity_factor = (1.0 - physical_distance_meters) * 4.0;
    }
    *interaction_uncanny_out = proximity_factor * (1.2 + geom->eye_scale * 0.5);
    return true;
}

bool evaluate_dynamic_auditory_offset(const teddy_bear_geometry_t *geom, double ambient_noise_db, double vocal_gain_db, double *auditory_offset_out) {
    if (!geom || ambient_noise_db < 0.0 || vocal_gain_db < 0.0 || !auditory_offset_out) {
        return false;
    }
    double diff = fabs(vocal_gain_db - ambient_noise_db);
    *auditory_offset_out = diff * (0.8 + geom->head_fwhr * 0.3);
    return true;
}

bool evaluate_geniole_provocation_aggression_limit(const teddy_bear_geometry_t *geom, double provocation_scale, double *aggression_limit_out) {
    if (!geom || provocation_scale < 0.0 || !aggression_limit_out) {
        return false;
    }
    *aggression_limit_out = provocation_scale * (0.5 + geom->head_fwhr * 1.5 + geom->leadership_profile * 0.5);
    return true;
}

bool evaluate_geniole_testosterone_modulator(const teddy_bear_geometry_t *geom, double baseline_testosterone, double *modulator_out) {
    if (!geom || baseline_testosterone < 0.0 || !modulator_out) {
        return false;
    }
    *modulator_out = baseline_testosterone * (0.8 + geom->head_fwhr * 0.4 + geom->jaw_scale * 0.2);
    return true;
}

bool evaluate_geniole_fwhr_dilation_map(const teddy_bear_geometry_t *geom, double base_dilation, double *mapped_dilation_out) {
    if (!geom || base_dilation < 0.0 || !mapped_dilation_out) {
        return false;
    }
    *mapped_dilation_out = base_dilation * (1.0 + geom->head_fwhr * 0.6);
    return true;
}

bool evaluate_geniole_fwhr_jitter_mod(const teddy_bear_geometry_t *geom, double base_jitter, double *mapped_jitter_out) {
    if (!geom || base_jitter < 0.0 || !mapped_jitter_out) {
        return false;
    }
    *mapped_jitter_out = base_jitter * (0.9 + geom->head_fwhr * 0.5);
    return true;
}

bool evaluate_geniole_fwhr_boundary_map(const teddy_bear_geometry_t *geom, double threshold_scale, double *mapped_boundary_out) {
    if (!geom || threshold_scale < 0.0 || !mapped_boundary_out) {
        return false;
    }
    *mapped_boundary_out = threshold_scale * (1.1 + geom->head_fwhr * 0.5);
    return true;
}

bool evaluate_geniole_fwhr_retaliation_decay(const teddy_bear_geometry_t *geom, double exposure_duration_sec, double *decayed_retaliation_out) {
    if (!geom || exposure_duration_sec < 0.0 || !decayed_retaliation_out) {
        return false;
    }
    *decayed_retaliation_out = exp(-exposure_duration_sec / (10.0 + geom->head_fwhr * 5.0));
    return true;
}

