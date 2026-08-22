#include "auncient_teddy_bear_personality.h"
#include <stdio.h>
#include <math.h>

bool evaluate_hyde_gaze_direction_index(const teddy_bear_geometry_t *geom, double target_angle, double *gaze_alignment_out) {
    if (!geom || !gaze_alignment_out) {
        return false;
    }
    *gaze_alignment_out = fabs(sin(target_angle)) * (1.0 + (1.0 - geom->symmetry) * 2.0);
    return true;
}

bool evaluate_hyde_vocal_tempo_variance(const teddy_bear_geometry_t *geom, double vocal_tempo_bpm, double *vocal_uncanny_out) {
    if (!geom || vocal_tempo_bpm < 0.0 || !vocal_uncanny_out) {
        return false;
    }
    *vocal_uncanny_out = fabs(vocal_tempo_bpm - 120.0) * (0.05 + geom->behavioral_mismatch * 0.2);
    return true;
}

bool evaluate_hyde_conversational_latency(const teddy_bear_geometry_t *geom, double lag_seconds, double *latency_uncanny_out) {
    if (!geom || lag_seconds < 0.0 || !latency_uncanny_out) {
        return false;
    }
    *latency_uncanny_out = lag_seconds * (0.6 + geom->behavioral_mismatch * 0.5);
    return true;
}

bool evaluate_hyde_vocal_size_mismatch(const teddy_bear_geometry_t *geom, double voice_pitch_hz, double *size_mismatch_out) {
    if (!geom || voice_pitch_hz < 0.0 || !size_mismatch_out) {
        return false;
    }
    *size_mismatch_out = fabs(voice_pitch_hz - (300.0 - geom->torso_ratio * 100.0)) * 0.05;
    return true;
}

bool evaluate_hyde_vocal_amplitude_mismatch(const teddy_bear_geometry_t *geom, double voice_amplitude_db, double *amplitude_mismatch_out) {
    if (!geom || voice_amplitude_db < 0.0 || !amplitude_mismatch_out) {
        return false;
    }
    *amplitude_mismatch_out = fabs(voice_amplitude_db - 60.0) * (0.4 + geom->behavioral_mismatch * 0.4);
    return true;
}

bool evaluate_hyde_mouth_speed_synchrony(const teddy_bear_geometry_t *geom, double mouth_speed, double pitch_acceleration, double *sync_mismatch_out) {
    if (!geom || mouth_speed < 0.0 || !sync_mismatch_out) {
        return false;
    }
    *sync_mismatch_out = fabs(mouth_speed - fabs(pitch_acceleration)) * (0.8 + geom->stiffness * 0.5);
    return true;
}

bool evaluate_hyde_turn_interruption(const teddy_bear_geometry_t *geom, double overlap_duration_sec, double *interruption_uncanny_out) {
    if (!geom || overlap_duration_sec < 0.0 || !interruption_uncanny_out) {
        return false;
    }
    *interruption_uncanny_out = overlap_duration_sec * (0.5 + geom->behavioral_mismatch * 0.4);
    return true;
}

bool evaluate_hyde_vocal_tremor_index(const teddy_bear_geometry_t *geom, double pitch_variance, double *tremor_uncanny_out) {
    if (!geom || pitch_variance < 0.0 || !tremor_uncanny_out) {
        return false;
    }
    *tremor_uncanny_out = pitch_variance * (0.8 + (1.0 - geom->resilience_index) * 0.5);
    return true;
}

bool evaluate_hyde_tremor_frequency_sync(const teddy_bear_geometry_t *geom, double chin_vibration_hz, double audio_tremor_hz, double *sync_rating_out) {
    if (!geom || chin_vibration_hz < 0.0 || audio_tremor_hz < 0.0 || !sync_rating_out) {
        return false;
    }
    *sync_rating_out = (1.0 / (1.0 + fabs(chin_vibration_hz - audio_tremor_hz))) * (0.8 + geom->symmetry * 0.4);
    return true;
}

bool evaluate_hyde_pitch_range_engagement(const teddy_bear_geometry_t *geom, double pitch_range_hz, double *engagement_rating_out) {
    if (!geom || pitch_range_hz < 0.0 || !engagement_rating_out) {
        return false;
    }
    *engagement_rating_out = pitch_range_hz * (0.5 + geom->behavioral_mismatch * 0.5);
    return true;
}

bool evaluate_hyde_vocal_warmth_pitch(const teddy_bear_geometry_t *geom, double average_pitch_hz, double *warmth_offset_out) {
    if (!geom || average_pitch_hz < 0.0 || !warmth_offset_out) {
        return false;
    }
    *warmth_offset_out = average_pitch_hz * (0.6 + geom->empathy_index * 0.4) * 0.01;
    return true;
}

bool evaluate_hyde_interruption_frequency(const teddy_bear_geometry_t *geom, double collision_rate, double *aversion_rating_out) {
    if (!geom || collision_rate < 0.0 || !aversion_rating_out) {
        return false;
    }
    *aversion_rating_out = collision_rate * (1.1 + geom->behavioral_mismatch * 0.5);
    return true;
}

bool evaluate_hyde_av_latency_jitter(const teddy_bear_geometry_t *geom, double latency_jitter_sec, double *naturalness_out) {
    if (!geom || latency_jitter_sec < 0.0 || !naturalness_out) {
        return false;
    }
    *naturalness_out = (1.0 / (1.0 + latency_jitter_sec)) * (0.8 + geom->symmetry * 0.4);
    return true;
}

bool evaluate_hyde_vocal_energy_variance(const teddy_bear_geometry_t *geom, double energy_variance, double *engagement_out) {
    if (!geom || energy_variance < 0.0 || !engagement_out) {
        return false;
    }
    *engagement_out = energy_variance * (0.7 + geom->behavioral_mismatch * 0.5);
    return true;
}

bool evaluate_hyde_vocal_jitter_naturalness(const teddy_bear_geometry_t *geom, double pitch_jitter, double *naturalness_out) {
    if (!geom || pitch_jitter < 0.0 || !naturalness_out) {
        return false;
    }
    *naturalness_out = (1.0 / (1.0 + pitch_jitter)) * (0.9 + geom->symmetry * 0.3);
    return true;
}

bool evaluate_hyde_intonation_amplitude(const teddy_bear_geometry_t *geom, double intonation_variance, double *engagement_out) {
    if (!geom || intonation_variance < 0.0 || !engagement_out) {
        return false;
    }
    *engagement_out = intonation_variance * (0.8 + geom->behavioral_mismatch * 0.4);
    return true;
}

bool evaluate_hyde_dynamic_intonation(const teddy_bear_geometry_t *geom, double intonation_range, double brow_movement, double *intonation_out) {
    if (!geom || intonation_range < 0.0 || brow_movement < 0.0 || !intonation_out) {
        return false;
    }
    *intonation_out = intonation_range * brow_movement * (0.8 + geom->symmetry * 0.4);
    return true;
}

bool evaluate_hyde_intonation_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_intonation_out) {
    if (!geom || duration_sec < 0.0 || !decayed_intonation_out) {
        return false;
    }
    *decayed_intonation_out = exp(-duration_sec / (18.0 + geom->resilience_index * 8.0));
    return true;
}

bool evaluate_hyde_tempo_sync(const teddy_bear_geometry_t *geom, double tempo_sync_val, double *rating_out) {
    if (!geom || tempo_sync_val < 0.0 || !rating_out) {
        return false;
    }
    *rating_out = tempo_sync_val * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_hyde_tempo_jitter_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_jitter_out) {
    if (!geom || duration_sec < 0.0 || !decayed_jitter_out) {
        return false;
    }
    *decayed_jitter_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_hyde_tempo_range(const teddy_bear_geometry_t *geom, double tempo_range_val, double brow_movement, double *tempo_range_out) {
    if (!geom || tempo_range_val < 0.0 || brow_movement < 0.0 || !tempo_range_out) {
        return false;
    }
    *tempo_range_out = tempo_range_val * brow_movement * (0.8 + geom->symmetry * 0.4);
    return true;
}

bool evaluate_hyde_tempo_range_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_range_out) {
    if (!geom || duration_sec < 0.0 || !decayed_range_out) {
        return false;
    }
    *decayed_range_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_hyde_vocal_size_sync(const teddy_bear_geometry_t *geom, double tempo_sync_val, double size_mismatch, double *sync_out) {
    if (!geom || tempo_sync_val < 0.0 || size_mismatch < 0.0 || !sync_out) {
        return false;
    }
    *sync_out = tempo_sync_val * size_mismatch * (0.8 + geom->symmetry * 0.4);
    return true;
}

bool evaluate_hyde_vocal_size_jitter_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_size_jitter_out) {
    if (!geom || duration_sec < 0.0 || !decayed_size_jitter_out) {
        return false;
    }
    *decayed_size_jitter_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_hyde_vocal_size_pitch(const teddy_bear_geometry_t *geom, double size_variance, double pitch_range, double *pitch_out) {
    if (!geom || size_variance < 0.0 || pitch_range < 0.0 || !pitch_out) {
        return false;
    }
    *pitch_out = size_variance * pitch_range * (0.8 + geom->symmetry * 0.4);
    return true;
}

bool evaluate_hyde_vocal_size_pitch_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_size_pitch_out) {
    if (!geom || duration_sec < 0.0 || !decayed_size_pitch_out) {
        return false;
    }
    *decayed_size_pitch_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_hyde_vocal_warmth_range(const teddy_bear_geometry_t *geom, double pitch_range, double size_mismatch, double *warmth_range_out) {
    if (!geom || pitch_range < 0.0 || size_mismatch < 0.0 || !warmth_range_out) {
        return false;
    }
    *warmth_range_out = pitch_range * size_mismatch * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_hyde_vocal_warmth_range_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_warmth_range_out) {
    if (!geom || duration_sec < 0.0 || !decayed_warmth_range_out) {
        return false;
    }
    *decayed_warmth_range_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_hyde_vocal_warmth_modulation(const teddy_bear_geometry_t *geom, double pitch_hz, double chin_curvature, double *warmth_out) {
    if (!geom || pitch_hz < 0.0 || chin_curvature < 0.0 || !warmth_out) {
        return false;
    }
    *warmth_out = pitch_hz * chin_curvature * (0.8 + geom->empathy_index * 0.4) * 0.01;
    return true;
}

bool evaluate_hyde_interruption_recovery(const teddy_bear_geometry_t *geom, double recovery_time_sec, double *recovery_rating_out) {
    if (!geom || recovery_time_sec < 0.0 || !recovery_rating_out) {
        return false;
    }
    *recovery_rating_out = (1.0 / (1.0 + recovery_time_sec)) * (0.9 + geom->resilience_index * 0.3);
    return true;
}

bool evaluate_hyde_vocal_naturalness_variance(const teddy_bear_geometry_t *geom, double vocal_jitter, double amplitude_mismatch, double *naturalness_variance_out) {
    if (!geom || vocal_jitter < 0.0 || amplitude_mismatch < 0.0 || !naturalness_variance_out) {
        return false;
    }
    *naturalness_variance_out = vocal_jitter * amplitude_mismatch * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_hyde_intonation_amplitude_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_amplitude_out) {
    if (!geom || duration_sec < 0.0 || !decayed_amplitude_out) {
        return false;
    }
    *decayed_amplitude_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_hyde_vocal_tempo_sync_variance(const teddy_bear_geometry_t *geom, double tempo_sync_val, double size_mismatch, double *sync_variance_out) {
    if (!geom || tempo_sync_val < 0.0 || size_mismatch < 0.0 || !sync_variance_out) {
        return false;
    }
    *sync_variance_out = tempo_sync_val * size_mismatch * (0.8 + geom->symmetry * 0.4);
    return true;
}

bool evaluate_hyde_vocal_size_jitter_decay_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_size_jitter_out) {
    if (!geom || duration_sec < 0.0 || !decayed_size_jitter_out) {
        return false;
    }
    *decayed_size_jitter_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_hyde_vocal_warmth_variance(const teddy_bear_geometry_t *geom, double interruption_freq, double pitch_hz, double *warmth_variance_out) {
    if (!geom || interruption_freq < 0.0 || pitch_hz < 0.0 || !warmth_variance_out) {
        return false;
    }
    *warmth_variance_out = interruption_freq * pitch_hz * (0.8 + geom->empathy_index * 0.4) * 0.01;
    return true;
}

bool evaluate_hyde_vocal_size_pitch_decay_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_size_pitch_out) {
    if (!geom || duration_sec < 0.0 || !decayed_size_pitch_out) {
        return false;
    }
    *decayed_size_pitch_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_hyde_vocal_tempo_range_variance(const teddy_bear_geometry_t *geom, double tempo_range_val, double size_mismatch, double *range_variance_out) {
    if (!geom || tempo_range_val < 0.0 || size_mismatch < 0.0 || !range_variance_out) {
        return false;
    }
    *range_variance_out = tempo_range_val * size_mismatch * (0.8 + geom->symmetry * 0.4);
    return true;
}

bool evaluate_hyde_vocal_size_jitter_decay_mod_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_hyde_vocal_turn_interruption_variance(const teddy_bear_geometry_t *geom, double interruption_freq, double *variance_out) {
    if (!geom || interruption_freq < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = interruption_freq * (0.8 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_hyde_vocal_size_pitch_variance(const teddy_bear_geometry_t *geom, double size_pitch_val, double pitch_range, double *variance_out) {
    if (!geom || size_pitch_val < 0.0 || pitch_range < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = size_pitch_val * pitch_range * (0.8 + geom->symmetry * 0.4);
    return true;
}

bool evaluate_hyde_vocal_tempo_range_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_hyde_vocal_size_sync_variance(const teddy_bear_geometry_t *geom, double size_sync_val, double size_mismatch, double *variance_out) {
    if (!geom || size_sync_val < 0.0 || size_mismatch < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = size_sync_val * size_mismatch * (0.8 + geom->symmetry * 0.4);
    return true;
}

bool evaluate_hyde_vocal_warmth_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_hyde_vocal_tempo_sync_variance_mod(const teddy_bear_geometry_t *geom, double tempo_sync_val, double *variance_out) {
    if (!geom || tempo_sync_val < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = tempo_sync_val * (0.8 + geom->symmetry * 0.4);
    return true;
}

bool evaluate_hyde_vocal_warmth_range_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_hyde_vocal_warmth_modulation_variance_mod(const teddy_bear_geometry_t *geom, double warmth_mod_val, double *variance_out) {
    if (!geom || warmth_mod_val < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = warmth_mod_val * (0.8 + geom->symmetry * 0.4);
    return true;
}

bool evaluate_hyde_vocal_warmth_range_variance_mod(const teddy_bear_geometry_t *geom, double warmth_range_val, double *variance_out) {
    if (!geom || warmth_range_val < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = warmth_range_val * (0.8 + geom->symmetry * 0.4);
    return true;
}

bool evaluate_hyde_vocal_size_pitch_variance_mod(const teddy_bear_geometry_t *geom, double size_pitch_val, double *variance_out) {
    if (!geom || size_pitch_val < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = size_pitch_val * (0.8 + geom->symmetry * 0.4);
    return true;
}

bool evaluate_hyde_vocal_size_jitter_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_hyde_vocal_size_sync_variance_mod(const teddy_bear_geometry_t *geom, double size_sync_val, double *variance_out) {
    if (!geom || size_sync_val < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = size_sync_val * (0.8 + geom->symmetry * 0.4);
    return true;
}

bool evaluate_hyde_vocal_warmth_variance_mod(const teddy_bear_geometry_t *geom, double warmth_val, double *variance_out) {
    if (!geom || warmth_val < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = warmth_val * (0.8 + geom->symmetry * 0.4);
    return true;
}

bool evaluate_hyde_vocal_tempo_range_variance_mod(const teddy_bear_geometry_t *geom, double tempo_range_val, double *variance_out) {
    if (!geom || tempo_range_val < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = tempo_range_val * (0.8 + geom->symmetry * 0.4);
    return true;
}

bool evaluate_hyde_vocal_size_jitter_variance_mod(const teddy_bear_geometry_t *geom, double size_jitter_val, double *variance_out) {
    if (!geom || size_jitter_val < 0.0 || !variance_out) {
        return false;
    }
    *variance_out = size_jitter_val * (0.8 + geom->symmetry * 0.4);
    return true;
}

bool evaluate_hyde_vocal_warmth_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_hyde_vocal_tempo_range_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_hyde_vocal_size_jitter_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_hyde_vocal_tempo_sync_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_hyde_vocal_size_pitch_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

bool evaluate_hyde_vocal_size_sync_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out) {
    if (!geom || duration_sec < 0.0 || !decay_variance_out) {
        return false;
    }
    *decay_variance_out = exp(-duration_sec / (16.0 + geom->resilience_index * 6.0));
    return true;
}

