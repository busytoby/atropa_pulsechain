#ifndef TSFI_PERSONALITY_MODELS_ADV_H
#define TSFI_PERSONALITY_MODELS_ADV_H
#include "tsfi_personality_core.h"
bool evaluate_keating_eyebrow_asymmetry_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_trust_out);

// Models submissiveness ratings based on exposed sclera size ratio variations (Keating, C. F.).
bool evaluate_keating_sclera_submissiveness_mod(const teddy_geometry_t *geom, double sclera_size_ratio, double *submissiveness_out);

// Models the decay rate of dominance ratings based on prolonged eyebrow gesture asymmetry durations (Keating, C. F.).
bool evaluate_keating_eyebrow_asymmetry_dominance_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_dominance_out);

// Models the decay rate of trustworthiness ratings based on eyebrow gesture asymmetry duration variations (Keating, C. F.).
bool evaluate_keating_eyebrow_asymmetry_trust_decay_mod(const teddy_geometry_t *geom, double duration_sec, double *decayed_trust_out);

// Models submissiveness ratings variance based on exposed sclera size ratio variations (Keating, C. F.).
bool evaluate_keating_sclera_submissiveness_variance(const teddy_geometry_t *geom, double sclera_size_ratio, double *submissiveness_variance_out);

// Models the decay rate of social dominance ratings based on prolonged gaze shift frequency variations (Keating, C. F.).
bool evaluate_keating_gaze_shift_dominance_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_dominance_out);

// Models the variance in trustworthiness ratings based on visual lip compression ratio variations (Keating, C. F.).
bool evaluate_keating_lip_compression_trust_variance(const teddy_geometry_t *geom, double compression_ratio, double *trust_variance_out);

// Models the decay rate of submissiveness ratings based on prolonged exposed sclera size ratio durations (Keating, C. F.).
bool evaluate_keating_sclera_submissiveness_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_submissiveness_out);

// Models the variance in social dominance ratings based on visual width asymmetry ratio variations (Keating, C. F.).
bool evaluate_keating_width_asymmetry_dominance_variance(const teddy_geometry_t *geom, double width_asymmetry_val, double *variance_out);

// Models the decay rate of dominance ratings based on prolonged exposed sclera size ratio durations (Keating, C. F.).
bool evaluate_keating_sclera_dominance_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_dominance_out);

// Models the variance in dominance ratings based on visual mouth asymmetry ratio variations (Keating, C. F.).
bool evaluate_keating_mouth_asymmetry_dominance_variance(const teddy_geometry_t *geom, double mouth_asymmetry_val, double *variance_out);

// Models the variance in eyebrow asymmetry dominance decay ratings based on prolonged tracking intervals (Keating).
bool evaluate_keating_eyebrow_asymmetry_dominance_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of mouth asymmetry dominance ratings based on prolonged tracking intervals (Keating).
bool evaluate_keating_mouth_asymmetry_dominance_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in width asymmetry trust modulator ratings under dynamic layout symmetry variations (Keating).
bool evaluate_keating_width_asymmetry_trust_variance_mod(const teddy_geometry_t *geom, double width_asym_val, double *variance_out);

// Models the variance in decay rates of mouth asymmetry trust ratings based on prolonged tracking intervals (Keating).
bool evaluate_keating_mouth_asymmetry_trust_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in width asymmetry dominance modulator ratings under dynamic layout symmetry variations (Keating).
bool evaluate_keating_width_asymmetry_dominance_variance_mod(const teddy_geometry_t *geom, double width_asym_val, double *variance_out);

// Models the variance in decay rates of eyebrow asymmetry trust ratings based on prolonged tracking intervals (Keating).
bool evaluate_keating_eyebrow_asymmetry_trust_decay_variance_single(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in sclera submissiveness modulator ratings under dynamic layout symmetry variations (Keating).
bool evaluate_keating_sclera_submissiveness_variance_mod(const teddy_geometry_t *geom, double sclera_val, double *variance_out);

// Models the variance in sclera dominance modulator ratings under dynamic layout symmetry variations (Keating).
bool evaluate_keating_sclera_dominance_variance_mod_single(const teddy_geometry_t *geom, double sclera_val, double *variance_out);

// Models the variance in eyebrow asymmetry dominance modulator ratings under dynamic layout symmetry variations (Keating).
bool evaluate_keating_eyebrow_asymmetry_dominance_variance_mod(const teddy_geometry_t *geom, double eyebrow_asym_val, double *variance_out);

// Models the variance in eyebrow asymmetry trust modulator ratings under dynamic layout symmetry variations (Keating).
bool evaluate_keating_eyebrow_asymmetry_trust_variance_mod_single(const teddy_geometry_t *geom, double eyebrow_asym_val, double *variance_out);

// Models the variance in decay rates of sclera submissiveness ratings based on prolonged tracking intervals (Keating).
bool evaluate_keating_sclera_submissiveness_decay_variance_single(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of sclera dominance ratings based on prolonged tracking intervals (Keating).
bool evaluate_keating_sclera_dominance_decay_variance_single(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of eyebrow asymmetry dominance ratings based on prolonged tracking intervals (Keating).
bool evaluate_keating_eyebrow_asymmetry_dominance_decay_variance_single(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of sclera submissiveness ratings under dynamic layout symmetry variations (Keating).
bool evaluate_keating_sclera_submissiveness_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of sclera dominance ratings under dynamic layout symmetry variations (Keating).
bool evaluate_keating_sclera_dominance_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in trustworthiness ratings based on visual mouth asymmetry ratio variations (Keating, C. F.).
bool evaluate_keating_mouth_asymmetry_trust_variance(const teddy_geometry_t *geom, double mouth_asymmetry_val, double *trust_variance_out);

// Models the variance in submissiveness decay rates based on prolonged exposed sclera size ratio durations (Keating, C. F.).
bool evaluate_keating_sclera_submissiveness_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in dominance decay rates based on prolonged eyebrow asymmetry duration variations (Keating, C. F.).
bool evaluate_keating_eyebrow_asymmetry_dominance_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in trustworthiness ratings based on visual mouth asymmetry ratio variations (Keating, C. F.).
bool evaluate_keating_mouth_asymmetry_trust_variance(const teddy_geometry_t *geom, double mouth_asymmetry_val, double *trust_variance_out);

// Models the variance in submissiveness decay rates based on prolonged exposed sclera size ratio durations (Keating, C. F.).
bool evaluate_keating_sclera_submissiveness_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in trustworthiness ratings based on visual width asymmetry ratio variations (Keating, C. F.).
bool evaluate_keating_width_asymmetry_trust_variance(const teddy_geometry_t *geom, double width_asymmetry_val, double *trust_variance_out);

// Models the variance in social dominance decay rates based on prolonged gaze shift duration variations (Keating, C. F.).
bool evaluate_keating_gaze_shift_dominance_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in trustworthiness ratings based on visual lip compression ratio variations (Keating, C. F.).
bool evaluate_keating_lip_compression_trust_variance_mod(const teddy_geometry_t *geom, double lip_compression_val, double *trust_variance_out);

// Models the variance in trustworthiness decay rates based on prolonged eyebrow asymmetry duration variations (Keating, C. F.).
bool evaluate_keating_eyebrow_asymmetry_trust_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in social dominance ratings based on exposed sclera size ratio variations (Keating, C. F.).
bool evaluate_keating_sclera_dominance_variance_mod(const teddy_geometry_t *geom, double sclera_size_ratio, double *variance_out);

// Models the variance in social dominance decay rates based on prolonged exposed sclera size ratio duration variations (Keating, C. F.).
bool evaluate_keating_sclera_dominance_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in trustworthiness ratings based on visual mouth asymmetry ratio variations (Keating, C. F.).
bool evaluate_keating_mouth_asymmetry_trust_variance_mod(const teddy_geometry_t *geom, double mouth_asymmetry_val, double *trust_variance_out);

// Models perceived social dominance variance based on exposed sclera size ratio variations (Keating, C. F.).
bool evaluate_keating_sclera_dominance_variance(const teddy_geometry_t *geom, double sclera_size_ratio, double *dominance_variance_out);

// Evaluates friendly visual parameters scaling on playfulness ratings (Scarpi).
bool evaluate_scarpi_hedonic_playfulness(const teddy_geometry_t *geom, double base_playfulness, double *playfulness_out);

// Models transaction trust based on H-bridge switching stability and safety margins (Scarpi).
bool evaluate_scarpi_utilitarian_trust(const teddy_geometry_t *geom, double safety_margin, double *operational_trust_out);

// Evaluates playfulness arousal levels based on dynamic novelty shifts and eyebrow gestures (Scarpi).
bool evaluate_scarpi_hedonic_arousal_mod(const teddy_geometry_t *geom, double novelty_shift, double brow_gesture_val, double *arousal_out);

// Models interaction performance indices based on spatial alignment boundary metrics (Scarpi).
bool evaluate_scarpi_utilitarian_quality_control(const teddy_geometry_t *geom, double spatial_alignment, double *quality_control_out);

// Evaluates playfulness ratings based on dynamic aesthetic alignment parameters (Scarpi).
bool evaluate_scarpi_hedonic_aesthetic_quality(const teddy_geometry_t *geom, double aesthetic_alignment, double *quality_out);

// Models user interaction ratings based on layout consistency checks (Scarpi).
bool evaluate_scarpi_utilitarian_consistency(const teddy_geometry_t *geom, double consistency_score, double *consistency_mod_out);

// Evaluates playfulness ratings based on dynamic aesthetic trust parameters (Scarpi).
bool evaluate_scarpi_hedonic_interface_trust(const teddy_geometry_t *geom, double aesthetic_trust_val, double *interface_trust_out);

// Models transaction trust ratings based on operational efficiency bounds (Scarpi).
bool evaluate_scarpi_utilitarian_operational_trust_mod(const teddy_geometry_t *geom, double efficiency_val, double *operational_trust_out);

// Evaluates playfulness ratings based on dynamic novelty scales and aesthetic alignment (Scarpi).
bool evaluate_scarpi_hedonic_playful_engagement(const teddy_geometry_t *geom, double novelty_scale, double aesthetic_alignment, double *engagement_out);

// Models user interaction consistency decay rates over prolonged tracking cycles (Scarpi).
bool evaluate_scarpi_utilitarian_consistency_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_consistency_out);

// Models the variance in playfulness ratings based on dynamic novelty shift variations and eyebrow gestures (Scarpi).
bool evaluate_scarpi_hedonic_playful_arousal_variance(const teddy_geometry_t *geom, double novelty_shift, double brow_gesture_val, double *arousal_variance_out);

// Models the decay rate of operational trust ratings based on prolonged transaction latency intervals (Scarpi).
bool evaluate_scarpi_utilitarian_operational_trust_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_operational_trust_out);

// Models the variance in user interaction consistency ratings under dynamic boundary conditions (Scarpi).
bool evaluate_scarpi_utilitarian_consistency_variance(const teddy_geometry_t *geom, double consistency_score, double *variance_out);

// Models the decay rate of interface trust ratings based on prolonged visual interaction durations (Scarpi).
bool evaluate_scarpi_hedonic_interface_trust_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_trust_out);

// Models the variance in interface trust ratings based on visual design symmetry and dynamic novelty (Scarpi).
bool evaluate_scarpi_hedonic_trust_variance(const teddy_geometry_t *geom, double aesthetic_rating, double novelty_scale, double *trust_variance_out);

// Models the decay rate of quality ratings based on prolonged testing durations (Scarpi).
bool evaluate_scarpi_utilitarian_quality_control_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_quality_out);

// Models the variance in playfulness engagement ratings based on dynamic novelty and aesthetic alignment (Scarpi).
bool evaluate_scarpi_hedonic_playful_engagement_variance(const teddy_geometry_t *geom, double novelty_scale, double aesthetic_alignment, double *engagement_variance_out);

// Models the variance in consistency decay rates based on tracking duration variations (Scarpi).
bool evaluate_scarpi_utilitarian_consistency_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in playful arousal ratings under dynamic novelty shifts and design layout symmetry variations (Scarpi).
bool evaluate_scarpi_hedonic_playful_arousal_mod_variance(const teddy_geometry_t *geom, double novelty_scale, double design_symmetry, double *variance_out);

// Models the variance in quality control ratings based on task complexity levels (Scarpi).
bool evaluate_scarpi_utilitarian_quality_control_variance(const teddy_geometry_t *geom, double complexity_level, double *variance_out);

// Models the variance in playful arousal decay rates based on dynamic novelty shifts (Scarpi).
bool evaluate_scarpi_hedonic_playful_arousal_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in quality control decay rates over prolonged testing durations (Scarpi).
bool evaluate_scarpi_utilitarian_quality_control_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in playful engagement decay rates based on prolonged novelty exposure durations (Scarpi).
bool evaluate_scarpi_hedonic_playful_engagement_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in utilitarian structural consistency ratings based on task complexity levels (Scarpi).
bool evaluate_scarpi_utilitarian_consistency_variance_mod(const teddy_geometry_t *geom, double complexity_level, double *variance_out);

// Models the variance in quality control modulator ratings based on task complexity levels (Scarpi).
bool evaluate_scarpi_utilitarian_quality_control_variance_mod(const teddy_geometry_t *geom, double complexity_level, double *variance_out);

// Models the variance in playful arousal decay modulator ratings under dynamic layout symmetry variations (Scarpi).
bool evaluate_scarpi_hedonic_playful_arousal_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in utilitarian operational trust modulator ratings under dynamic layout symmetry variations (Scarpi).
bool evaluate_scarpi_utilitarian_operational_trust_variance_mod(const teddy_geometry_t *geom, double trust_score, double *variance_out);

// Models the variance in playful engagement modulator ratings under dynamic layout symmetry variations (Scarpi).
bool evaluate_scarpi_hedonic_playful_engagement_variance_mod(const teddy_geometry_t *geom, double engagement_score, double *variance_out);

// Models the variance in utilitarian operational consistency modulator ratings under dynamic layout symmetry variations (Scarpi).
bool evaluate_scarpi_utilitarian_consistency_variance_mod_single(const teddy_geometry_t *geom, double consistency_score, double *variance_out);

// Models the variance in decay rates of quality control modulator ratings based on prolonged tracking intervals (Scarpi).
bool evaluate_scarpi_utilitarian_quality_control_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of playful engagement modulator ratings based on prolonged tracking intervals (Scarpi).
bool evaluate_scarpi_hedonic_playful_engagement_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of operational consistency modulator ratings based on prolonged tracking intervals (Scarpi).
bool evaluate_scarpi_utilitarian_consistency_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of operational trust modulator ratings based on prolonged tracking intervals (Scarpi).
bool evaluate_scarpi_utilitarian_operational_trust_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of interface trust modulator ratings based on prolonged tracking intervals (Scarpi).
bool evaluate_scarpi_hedonic_interface_trust_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Evaluates interaction trust based on visual aesthetics and novelty parameters (Scarpi).
bool evaluate_scarpi_hedonic_trust(const teddy_geometry_t *geom, double aesthetic_rating, double novelty_scale, double *trust_out);

// Models user interaction ratings based on latency speeds and coordinate consistency checking times (Scarpi).
bool evaluate_scarpi_utilitarian_efficiency(const teddy_geometry_t *geom, double latency_sec, double *efficiency_out);

// Evaluates perceived conversational naturalness based on vocal pitch frequency jitter (Hyde et al.).
bool evaluate_hyde_vocal_jitter_naturalness(const teddy_geometry_t *geom, double pitch_jitter, double *naturalness_out);

// Models engagement rates based on vocal intonation amplitude variance (Hyde et al.).
bool evaluate_hyde_intonation_amplitude(const teddy_geometry_t *geom, double intonation_variance, double *engagement_out);

// Models the variance in conversational vocal naturalness ratings based on vocal jitter and intonation amplitude mismatch levels (Hyde et al.).
bool evaluate_hyde_vocal_naturalness_variance(const teddy_geometry_t *geom, double vocal_jitter, double amplitude_mismatch, double *naturalness_variance_out);

// Models the decay rate of intonation amplitude variance over dynamic interaction durations (Hyde et al.).
bool evaluate_hyde_intonation_amplitude_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_amplitude_out);

// Models the variance in conversational synchronization ratings based on vocal tempo matching and size mismatch levels (Hyde et al.).
bool evaluate_hyde_vocal_tempo_sync_variance(const teddy_geometry_t *geom, double tempo_sync_val, double size_mismatch, double *sync_variance_out);

// Models the decay rate of vocal size jitter variation over dynamic interaction durations (Hyde et al.).
bool evaluate_hyde_vocal_size_jitter_decay_mod(const teddy_geometry_t *geom, double duration_sec, double *decayed_size_jitter_out);

// Models the variance in vocal warmth ratings based on conversational interruption frequency variations and vocal pitch (Hyde et al.).
bool evaluate_hyde_vocal_warmth_variance(const teddy_geometry_t *geom, double interruption_freq, double pitch_hz, double *warmth_variance_out);

// Models the decay rate of vocal size pitch variation over dynamic interaction durations (Hyde et al.).
bool evaluate_hyde_vocal_size_pitch_decay_mod(const teddy_geometry_t *geom, double duration_sec, double *decayed_size_pitch_out);

// Models the variance in conversational synchronization ratings based on vocal tempo range variations (Hyde et al.).
bool evaluate_hyde_vocal_tempo_range_variance(const teddy_geometry_t *geom, double tempo_range_val, double size_mismatch, double *range_variance_out);

// Models the variance in decay rates of vocal size jitter variation over dynamic interaction durations (Hyde et al.).
bool evaluate_hyde_vocal_size_jitter_decay_mod_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in vocal warmth ratings based on conversational turn interruption frequency variations (Hyde et al.).
bool evaluate_hyde_vocal_turn_interruption_variance(const teddy_geometry_t *geom, double interruption_freq, double *variance_out);

// Models the variance in vocal size pitch values under dynamic pitch range variations (Hyde et al.).
bool evaluate_hyde_vocal_size_pitch_variance(const teddy_geometry_t *geom, double size_pitch_val, double pitch_range, double *variance_out);

// Models the variance in decay rates of vocal tempo range variations over dynamic interaction durations (Hyde et al.).
bool evaluate_hyde_vocal_tempo_range_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in conversational synchronization ratings based on vocal size sync variations (Hyde et al.).
bool evaluate_hyde_vocal_size_sync_variance(const teddy_geometry_t *geom, double size_sync_val, double size_mismatch, double *variance_out);

// Models the variance in decay rates of vocal warmth ratings based on dynamic interaction durations (Hyde et al.).
bool evaluate_hyde_vocal_warmth_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in conversational synchronization ratings based on speech tempo sync variations (Hyde et al.).
bool evaluate_hyde_vocal_tempo_sync_variance_mod(const teddy_geometry_t *geom, double tempo_sync_val, double *variance_out);

// Models the variance in decay rates of vocal warmth range ratings based on prolonged tracking intervals (Hyde et al.).
bool evaluate_hyde_vocal_warmth_range_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in conversational synchronization ratings based on speech warmth modulation variations (Hyde et al.).
bool evaluate_hyde_vocal_warmth_modulation_variance_mod(const teddy_geometry_t *geom, double warmth_mod_val, double *variance_out);

// Models the variance in vocal warmth range ratings under dynamic layout symmetry variations (Hyde et al.).
bool evaluate_hyde_vocal_warmth_range_variance_mod(const teddy_geometry_t *geom, double warmth_range_val, double *variance_out);

// Models the variance in conversational synchronization ratings based on speech size pitch variations (Hyde et al.).
bool evaluate_hyde_vocal_size_pitch_variance_mod(const teddy_geometry_t *geom, double size_pitch_val, double *variance_out);

// Models the variance in decay rates of vocal size jitter variations based on prolonged tracking intervals (Hyde et al.).
bool evaluate_hyde_vocal_size_jitter_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in conversational synchronization ratings based on speech size sync variations (Hyde et al.).
bool evaluate_hyde_vocal_size_sync_variance_mod(const teddy_geometry_t *geom, double size_sync_val, double *variance_out);

// Models the variance in vocal size jitter modulator ratings under dynamic layout symmetry variations (Hyde et al.).
bool evaluate_hyde_vocal_size_jitter_variance_mod(const teddy_geometry_t *geom, double size_jitter_val, double *variance_out);

// Models the variance in vocal warmth modulator ratings under dynamic layout symmetry variations (Hyde et al.).
bool evaluate_hyde_vocal_warmth_variance_mod(const teddy_geometry_t *geom, double warmth_val, double *variance_out);

// Models the variance in conversational synchronization ratings based on speech tempo range variations (Hyde et al.).
bool evaluate_hyde_vocal_tempo_range_variance_mod(const teddy_geometry_t *geom, double tempo_range_val, double *variance_out);

// Models the variance in decay rates of vocal warmth modulator ratings based on prolonged tracking intervals (Hyde et al.).
bool evaluate_hyde_vocal_warmth_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of conversational synchronization ratings based on speech tempo range decay variations (Hyde et al.).
bool evaluate_hyde_vocal_tempo_range_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of vocal size jitter modulator ratings based on prolonged tracking intervals (Hyde et al.).
bool evaluate_hyde_vocal_size_jitter_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of conversational synchronization ratings based on speech tempo sync decay variations (Hyde et al.).
bool evaluate_hyde_vocal_tempo_sync_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of vocal size pitch modulator ratings based on prolonged tracking intervals (Hyde et al.).
bool evaluate_hyde_vocal_size_pitch_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of conversational synchronization ratings based on speech size sync decay variations (Hyde et al.).
bool evaluate_hyde_vocal_size_sync_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Simulates the low-level diode reflex and relative capacitor charge decay loop.
bool simulate_diode_capacitor_loop(double input_voltage, double resistance, double capacitance, double time_step, double *charge_state);

// Evaluates the H-bridge switching flyback transient mismatch for izotope filtering.
bool evaluate_hbridge_izotope_mismatch(const teddy_geometry_t *geom, double switching_frequency, double *flyback_mismatch_out);

// Simulates the snubber-clamped inductive flyback transient voltage decay.
bool simulate_snubber_clamped_flyback(double peak_voltage, double inductance, double snubber_resistance, double time_step, double *clamped_voltage_out);

// Simulates the RCD snubber resonant capacitor decay stage.
bool simulate_rcd_snubber_decay(double peak_voltage, double resistance, double capacitance, double inductance, double time_step, double *voltage_state);

// Calculates the nerve-like Gumbel transient diyat tax on top of base gas costs.
bool calculate_diyat_tax(const teddy_geometry_t *geom, double switching_frequency, double base_gas_cost, double *total_cost_out);

// Calculates the nerve-like Gumbel transient diyat tax with biological refractory recovery periods.
bool calculate_diyat_tax_with_refractory(const teddy_geometry_t *geom, double switching_frequency, double base_gas_cost, double time_since_last_event, double *total_cost_out);

// Calculates the nerve-like Gumbel transient diyat tax modulated by tremolo and sustain envelope dynamics.
bool calculate_diyat_tax_with_envelope(const teddy_geometry_t *geom, double switching_frequency, double base_gas_cost, double tremolo_freq, double sustain_time, double *total_cost_out);

// Simulates the phase-modulated flyback noise distortion index for izotope filtering.
bool simulate_phase_flyback_noise(const teddy_geometry_t *geom, double phase_angle, double *noise_out);

// Evaluates symmetric threshold (tremolo) and group scale (sustain) constraints in the izotope system.
bool evaluate_izotope_constrained_parameters(const teddy_geometry_t *geom, int group_id, double *tremolo_spacing_out, double *sustain_decay_out);

// Evaluates the izotope/ozone-adjusted habituation decay logic (Christensen).
bool evaluate_izotope_ozone_habituation_decay(const teddy_geometry_t *geom, double base_decay, double *decayed_out);

// Evaluates relationship warmth ratings based on conversational turn-taking sync and posture pitch alignment metrics (Castle).
bool evaluate_castle_diplomatic_alignment(const teddy_geometry_t *geom, double turn_taking_sync, double posture_alignment, double *alignment_out);

// Models communication consistency ratings based on message repetition and alignment bounds (Castle).
bool evaluate_castle_policy_consistency(const teddy_geometry_t *geom, double consistency_score, double *policy_consistency_out);

// Models the decay rate of relationship warmth ratings based on prolonged lack of interaction (Castle).
bool evaluate_castle_diplomatic_alignment_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_alignment_out);

// Models the variance in communication consistency ratings under dynamic boundary conditions (Castle).
bool evaluate_castle_policy_consistency_variance(const teddy_geometry_t *geom, double consistency_score, double *variance_out);

// Evaluates policy implementation progress ratings based on dynamic target metrics (Castle).
bool evaluate_castle_policy_target_engagement(const teddy_geometry_t *geom, double target_metric, double *engagement_out);

// Models trust boundary configurations based on visual and vocal symmetry matching (Castle).
bool evaluate_castle_diplomatic_trust_bounds(const teddy_geometry_t *geom, double visual_symmetry, double vocal_symmetry, double *bounds_out);

// Models the decay rate of diplomatic trust boundary configurations over prolonged lack of verification (Castle).
bool evaluate_castle_diplomatic_trust_bounds_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_bounds_out);

// Models the variance in progress ratings based on dynamic target metrics (Castle).
bool evaluate_castle_policy_target_engagement_variance(const teddy_geometry_t *geom, double target_metric, double *variance_out);

// Models the variance in trust boundary configurations under dynamic context variations (Castle).
bool evaluate_castle_diplomatic_trust_bounds_variance(const teddy_geometry_t *geom, double visual_symmetry, double vocal_symmetry, double *bounds_variance_out);

// Models the decay rate of target engagement ratings over prolonged tracking durations (Castle).
bool evaluate_castle_policy_target_engagement_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_engagement_out);

// Models the variance in diplomatic alignment ratings under dynamic boundary variations (Castle).
bool evaluate_castle_diplomatic_alignment_variance(const teddy_geometry_t *geom, double visual_symmetry, double vocal_symmetry, double *alignment_variance_out);

// Models the decay rate of policy consistency ratings over prolonged tracking intervals (Castle).
bool evaluate_castle_policy_consistency_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_consistency_out);

// Models the variance in diplomatic alignment decay rates under dynamic interaction durations (Castle).
bool evaluate_castle_diplomatic_alignment_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in policy consistency ratings based on visual design asymmetry (Castle).
bool evaluate_castle_policy_consistency_variance_mod(const teddy_geometry_t *geom, double consistency_score, double *variance_out);

// Models the variance in diplomatic trust bounds decay rates under dynamic interaction durations (Castle).
bool evaluate_castle_diplomatic_trust_bounds_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in policy target engagement ratings based on visual design asymmetry (Castle).
bool evaluate_castle_policy_target_engagement_variance_mod(const teddy_geometry_t *geom, double engagement_score, double *variance_out);

// Models the variance in diplomatic alignment ratings under dynamic boundary variations based on design layout symmetry (Castle).
bool evaluate_castle_diplomatic_alignment_variance_mod(const teddy_geometry_t *geom, double alignment_score, double *variance_out);

// Models the variance in decay rates of policy consistency ratings based on prolonged tracking intervals (Castle).
bool evaluate_castle_policy_consistency_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in diplomatic trust bounds ratings under dynamic boundary variations based on design layout symmetry (Castle).
bool evaluate_castle_diplomatic_trust_bounds_variance_mod(const teddy_geometry_t *geom, double bounds_score, double *variance_out);

// Models the variance in decay rates of policy target engagement ratings based on prolonged tracking intervals (Castle).
bool evaluate_castle_policy_target_engagement_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in policy consistency decay modulator ratings over prolonged tracking intervals (Castle).
bool evaluate_castle_policy_consistency_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of diplomatic alignment modulator ratings under dynamic layout symmetry variations (Castle).
bool evaluate_castle_diplomatic_alignment_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of diplomatic trust bounds modulator ratings under dynamic layout symmetry variations (Castle).
bool evaluate_castle_diplomatic_trust_bounds_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of policy target engagement modulator ratings under dynamic layout symmetry variations (Castle).
bool evaluate_castle_policy_target_engagement_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);


// Models the relationship between facial width-to-height ratio variations and social dominance ratings (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_dominance(const teddy_geometry_t *geom, double fwhr_val, double *dominance_out);

// Models the relationship between facial width-to-height ratio variations and trustworthiness ratings (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_trustworthiness(const teddy_geometry_t *geom, double fwhr_val, double *trustworthiness_out);

// Models the variance in decay rates of FWHR dominance modulator ratings under dynamic layout symmetry variations (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_dominance_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of FWHR trustworthiness modulator ratings under dynamic layout symmetry variations (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_trustworthiness_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the relationship between facial width-to-height ratio variations and perceived threat ratings (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_threat(const teddy_geometry_t *geom, double fwhr_val, double *threat_out);

// Models the relationship between facial width-to-height ratio variations and expected cooperation ratings (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_cooperation(const teddy_geometry_t *geom, double fwhr_val, double *cooperation_out);

// Models the variance in decay rates of FWHR threat modulator ratings under dynamic layout symmetry variations (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_threat_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of FWHR cooperation modulator ratings under dynamic layout symmetry variations (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_cooperation_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);


// Models the discrepancy/distortion between 2D photographic fWHR and direct 3D scan geometry (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_3d_scan_discrepancy(const teddy_geometry_t *geom, double fwhr_val, double *discrepancy_out);

// Models the null-hypothesis scale factor (lack of significant sexual dimorphism) between male/female fWHR values (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_sexual_dimorphism_null_hypothesis(const teddy_geometry_t *geom, double male_fwhr, double female_fwhr, double *p_value_out);

// Models how pitch-angle head tilt (upward or downward) distorts the apparent fWHR projected from 3D scan to 2D photograph (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_head_tilt_effect(const teddy_geometry_t *geom, double fwhr_val, double pitch_degrees, double *apparent_fwhr_out);

// Models the mismatch between perceived dominance/threat ratings and actual cooperative/aggressive outputs (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_actual_vs_perceived_aggression_mismatch(const teddy_geometry_t *geom, double perceived_dominance, double actual_aggression, double *mismatch_out);

// Models the perceptual sensitivity decay under noise, blurring, or features occlusion (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_perceptual_noise_sensitivity(const teddy_geometry_t *geom, double noise_level, double *threshold_se_multiplier_out);

// Models contrast-effect threshold shifts based on sequential history of processed faces (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_sequential_adaptation_bias(const teddy_geometry_t *geom, const double *recent_fwhr_history, int history_len, double *adaptation_offset_out);

// Models the weighting bias prioritizing right hemiface fWHR cues over left hemiface cues in dominance judgments (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_hemiface_asymmetry_bias(const teddy_geometry_t *geom, double left_fwhr, double right_fwhr, double *weighted_fwhr_out);

// Models apparent fWHR cheekbone edge warping under directional lighting and fur roughness highlights (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_ambient_shading_effect(const teddy_geometry_t *geom, double original_fwhr, double light_angle_degrees, double *apparent_fwhr_out);

// Models dynamic temporal rating integration over sequential video frame captures (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_dynamic_temporal_smoothing(const teddy_geometry_t *geom, const double *frame_fwhr_sequence, int sequence_len, double *smoothed_fwhr_out);

// Models observer judgment confidence distributions relative to fWHR structural deviation from average (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_judgment_confidence_calibration(const teddy_geometry_t *geom, double fwhr_val, double *confidence_rating_out);

// Models the discrepancy between perceived health and actual physiological health (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_perceived_vs_actual_health(const teddy_geometry_t *geom, double perceived_health, double actual_health, double *mismatch_out);

// Models the geometric modulation of cheekbone prominence and cheek height (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_cheekbone_prominence_modulation(const teddy_geometry_t *geom, double cheekbone_val, double *prominence_mod_out);

// Models perspective distortion of apparent fWHR as a function of lens focal length (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_perspective_distortion(const teddy_geometry_t *geom, double fwhr_val, double focal_length_mm, double *distorted_fwhr_out);

// Models the accuracy limit of fWHR trait resolution under rapid micro-exposure (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_micro_exposure_resolution(const teddy_geometry_t *geom, double exposure_ms, double *resolution_accuracy_out);

// Models primate Extraversion traits based on facial proportions (Kramer, King & Ward).
bool evaluate_kramer_king_ward_primate_extraversion_signaling(const teddy_geometry_t *geom, double face_proportion, double *extraversion_out);

// Models cross-species classifier congruence between humans and primates (Kramer, King & Ward).
bool evaluate_kramer_king_ward_cross_species_congruence(const teddy_geometry_t *geom, double human_score, double primate_score, double *congruence_out);

// Models shared evolutionary threat cues (fWHR + brow prominence) across primates (Kramer, King & Ward).
bool evaluate_kramer_king_ward_shared_evolutionary_threat(const teddy_geometry_t *geom, double fwhr_val, double brow_val, double *threat_out);

// Models Agreeableness and Conscientiousness based on eye-orbit and jaw/muzzle geometry (Kramer, King & Ward).
bool evaluate_kramer_king_ward_agreeableness_conscientiousness(const teddy_geometry_t *geom, double eye_orbit_val, double jaw_val, double *rating_out);

// Evaluates species-specific vs. species-general cue markers in personality decoding (Kramer, King & Ward).
bool evaluate_kramer_king_ward_species_general_cues(const teddy_geometry_t *geom, double general_cue_weight, double *marker_val_out);

// Models sex-modulated accuracy of static face personality decoding (Kramer, King & Ward).
bool evaluate_kramer_king_ward_sex_modulated_accuracy(const teddy_geometry_t *geom, double base_accuracy, int biological_sex, double *accuracy_out);

// Calculates inter-rater consensus among observers decoding primate traits (Kramer, King & Ward).
bool evaluate_kramer_king_ward_inter_rater_consensus(const teddy_geometry_t *geom, const double *rater_judgments, int rater_count, double *consensus_out);

// Models group dominance rank convergence using facial features (Kramer, King & Ward).
bool evaluate_kramer_king_ward_intra_group_dominance(const teddy_geometry_t *geom, double group_rank, double *dominance_out);

// Evaluates morphological species distance based on trait coordinates (Kramer, King & Ward).
bool evaluate_kramer_king_ward_species_distance_matrix(const teddy_geometry_t *geom, const double *human_coords, const double *primate_coords, int coords_len, double *distance_out);

// Models primate Openness/Curiosity index from eye and face elongation geometry (Kramer, King & Ward).
bool evaluate_kramer_king_ward_primate_openness(const teddy_geometry_t *geom, double eye_height_width_ratio, double face_elongation, double *openness_out);

#endif // TSFI_PERSONALITY_MODELS_ADV_H
