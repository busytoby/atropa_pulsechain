#ifndef TSFI_PERSONALITY_MODELS_ADV_H
#define TSFI_PERSONALITY_MODELS_ADV_H
#include "tsfi_personality_core.h"
bool evaluate_keating_eyebrow_asymmetry_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_trust_out);

// Models submissiveness ratings based on exposed sclera size ratio variations (Keating, C. F.).
bool evaluate_keating_sclera_submissiveness_mod(const teddy_bear_geometry_t *geom, double sclera_size_ratio, double *submissiveness_out);

// Models the decay rate of dominance ratings based on prolonged eyebrow gesture asymmetry durations (Keating, C. F.).
bool evaluate_keating_eyebrow_asymmetry_dominance_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_dominance_out);

// Models the decay rate of trustworthiness ratings based on eyebrow gesture asymmetry duration variations (Keating, C. F.).
bool evaluate_keating_eyebrow_asymmetry_trust_decay_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_trust_out);

// Models submissiveness ratings variance based on exposed sclera size ratio variations (Keating, C. F.).
bool evaluate_keating_sclera_submissiveness_variance(const teddy_bear_geometry_t *geom, double sclera_size_ratio, double *submissiveness_variance_out);

// Models the decay rate of social dominance ratings based on prolonged gaze shift frequency variations (Keating, C. F.).
bool evaluate_keating_gaze_shift_dominance_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_dominance_out);

// Models the variance in trustworthiness ratings based on visual lip compression ratio variations (Keating, C. F.).
bool evaluate_keating_lip_compression_trust_variance(const teddy_bear_geometry_t *geom, double compression_ratio, double *trust_variance_out);

// Models the decay rate of submissiveness ratings based on prolonged exposed sclera size ratio durations (Keating, C. F.).
bool evaluate_keating_sclera_submissiveness_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_submissiveness_out);

// Models the variance in social dominance ratings based on visual width asymmetry ratio variations (Keating, C. F.).
bool evaluate_keating_width_asymmetry_dominance_variance(const teddy_bear_geometry_t *geom, double width_asymmetry_val, double *variance_out);

// Models the decay rate of dominance ratings based on prolonged exposed sclera size ratio durations (Keating, C. F.).
bool evaluate_keating_sclera_dominance_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_dominance_out);

// Models the variance in dominance ratings based on visual mouth asymmetry ratio variations (Keating, C. F.).
bool evaluate_keating_mouth_asymmetry_dominance_variance(const teddy_bear_geometry_t *geom, double mouth_asymmetry_val, double *variance_out);

// Models the variance in eyebrow asymmetry dominance decay ratings based on prolonged tracking intervals (Keating).
bool evaluate_keating_eyebrow_asymmetry_dominance_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of mouth asymmetry dominance ratings based on prolonged tracking intervals (Keating).
bool evaluate_keating_mouth_asymmetry_dominance_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in width asymmetry trust modulator ratings under dynamic layout symmetry variations (Keating).
bool evaluate_keating_width_asymmetry_trust_variance_mod(const teddy_bear_geometry_t *geom, double width_asym_val, double *variance_out);

// Models the variance in decay rates of mouth asymmetry trust ratings based on prolonged tracking intervals (Keating).
bool evaluate_keating_mouth_asymmetry_trust_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in width asymmetry dominance modulator ratings under dynamic layout symmetry variations (Keating).
bool evaluate_keating_width_asymmetry_dominance_variance_mod(const teddy_bear_geometry_t *geom, double width_asym_val, double *variance_out);

// Models the variance in decay rates of eyebrow asymmetry trust ratings based on prolonged tracking intervals (Keating).
bool evaluate_keating_eyebrow_asymmetry_trust_decay_variance_single(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in sclera submissiveness modulator ratings under dynamic layout symmetry variations (Keating).
bool evaluate_keating_sclera_submissiveness_variance_mod(const teddy_bear_geometry_t *geom, double sclera_val, double *variance_out);

// Models the variance in sclera dominance modulator ratings under dynamic layout symmetry variations (Keating).
bool evaluate_keating_sclera_dominance_variance_mod_single(const teddy_bear_geometry_t *geom, double sclera_val, double *variance_out);

// Models the variance in eyebrow asymmetry dominance modulator ratings under dynamic layout symmetry variations (Keating).
bool evaluate_keating_eyebrow_asymmetry_dominance_variance_mod(const teddy_bear_geometry_t *geom, double eyebrow_asym_val, double *variance_out);

// Models the variance in eyebrow asymmetry trust modulator ratings under dynamic layout symmetry variations (Keating).
bool evaluate_keating_eyebrow_asymmetry_trust_variance_mod_single(const teddy_bear_geometry_t *geom, double eyebrow_asym_val, double *variance_out);

// Models the variance in decay rates of sclera submissiveness ratings based on prolonged tracking intervals (Keating).
bool evaluate_keating_sclera_submissiveness_decay_variance_single(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of sclera dominance ratings based on prolonged tracking intervals (Keating).
bool evaluate_keating_sclera_dominance_decay_variance_single(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of eyebrow asymmetry dominance ratings based on prolonged tracking intervals (Keating).
bool evaluate_keating_eyebrow_asymmetry_dominance_decay_variance_single(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of sclera submissiveness ratings under dynamic layout symmetry variations (Keating).
bool evaluate_keating_sclera_submissiveness_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of sclera dominance ratings under dynamic layout symmetry variations (Keating).
bool evaluate_keating_sclera_dominance_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in trustworthiness ratings based on visual mouth asymmetry ratio variations (Keating, C. F.).
bool evaluate_keating_mouth_asymmetry_trust_variance(const teddy_bear_geometry_t *geom, double mouth_asymmetry_val, double *trust_variance_out);

// Models the variance in submissiveness decay rates based on prolonged exposed sclera size ratio durations (Keating, C. F.).
bool evaluate_keating_sclera_submissiveness_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in dominance decay rates based on prolonged eyebrow asymmetry duration variations (Keating, C. F.).
bool evaluate_keating_eyebrow_asymmetry_dominance_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in trustworthiness ratings based on visual mouth asymmetry ratio variations (Keating, C. F.).
bool evaluate_keating_mouth_asymmetry_trust_variance(const teddy_bear_geometry_t *geom, double mouth_asymmetry_val, double *trust_variance_out);

// Models the variance in submissiveness decay rates based on prolonged exposed sclera size ratio durations (Keating, C. F.).
bool evaluate_keating_sclera_submissiveness_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in trustworthiness ratings based on visual width asymmetry ratio variations (Keating, C. F.).
bool evaluate_keating_width_asymmetry_trust_variance(const teddy_bear_geometry_t *geom, double width_asymmetry_val, double *trust_variance_out);

// Models the variance in social dominance decay rates based on prolonged gaze shift duration variations (Keating, C. F.).
bool evaluate_keating_gaze_shift_dominance_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in trustworthiness ratings based on visual lip compression ratio variations (Keating, C. F.).
bool evaluate_keating_lip_compression_trust_variance_mod(const teddy_bear_geometry_t *geom, double lip_compression_val, double *trust_variance_out);

// Models the variance in trustworthiness decay rates based on prolonged eyebrow asymmetry duration variations (Keating, C. F.).
bool evaluate_keating_eyebrow_asymmetry_trust_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in social dominance ratings based on exposed sclera size ratio variations (Keating, C. F.).
bool evaluate_keating_sclera_dominance_variance_mod(const teddy_bear_geometry_t *geom, double sclera_size_ratio, double *variance_out);

// Models the variance in social dominance decay rates based on prolonged exposed sclera size ratio duration variations (Keating, C. F.).
bool evaluate_keating_sclera_dominance_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in trustworthiness ratings based on visual mouth asymmetry ratio variations (Keating, C. F.).
bool evaluate_keating_mouth_asymmetry_trust_variance_mod(const teddy_bear_geometry_t *geom, double mouth_asymmetry_val, double *trust_variance_out);

// Models perceived social dominance variance based on exposed sclera size ratio variations (Keating, C. F.).
bool evaluate_keating_sclera_dominance_variance(const teddy_bear_geometry_t *geom, double sclera_size_ratio, double *dominance_variance_out);

// Evaluates friendly visual parameters scaling on playfulness ratings (Scarpi).
bool evaluate_scarpi_hedonic_playfulness(const teddy_bear_geometry_t *geom, double base_playfulness, double *playfulness_out);

// Models transaction trust based on H-bridge switching stability and safety margins (Scarpi).
bool evaluate_scarpi_utilitarian_trust(const teddy_bear_geometry_t *geom, double safety_margin, double *operational_trust_out);

// Evaluates playfulness arousal levels based on dynamic novelty shifts and eyebrow gestures (Scarpi).
bool evaluate_scarpi_hedonic_arousal_mod(const teddy_bear_geometry_t *geom, double novelty_shift, double brow_gesture_val, double *arousal_out);

// Models interaction performance indices based on spatial alignment boundary metrics (Scarpi).
bool evaluate_scarpi_utilitarian_quality_control(const teddy_bear_geometry_t *geom, double spatial_alignment, double *quality_control_out);

// Evaluates playfulness ratings based on dynamic aesthetic alignment parameters (Scarpi).
bool evaluate_scarpi_hedonic_aesthetic_quality(const teddy_bear_geometry_t *geom, double aesthetic_alignment, double *quality_out);

// Models user interaction ratings based on layout consistency checks (Scarpi).
bool evaluate_scarpi_utilitarian_consistency(const teddy_bear_geometry_t *geom, double consistency_score, double *consistency_mod_out);

// Evaluates playfulness ratings based on dynamic aesthetic trust parameters (Scarpi).
bool evaluate_scarpi_hedonic_interface_trust(const teddy_bear_geometry_t *geom, double aesthetic_trust_val, double *interface_trust_out);

// Models transaction trust ratings based on operational efficiency bounds (Scarpi).
bool evaluate_scarpi_utilitarian_operational_trust_mod(const teddy_bear_geometry_t *geom, double efficiency_val, double *operational_trust_out);

// Evaluates playfulness ratings based on dynamic novelty scales and aesthetic alignment (Scarpi).
bool evaluate_scarpi_hedonic_playful_engagement(const teddy_bear_geometry_t *geom, double novelty_scale, double aesthetic_alignment, double *engagement_out);

// Models user interaction consistency decay rates over prolonged tracking cycles (Scarpi).
bool evaluate_scarpi_utilitarian_consistency_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_consistency_out);

// Models the variance in playfulness ratings based on dynamic novelty shift variations and eyebrow gestures (Scarpi).
bool evaluate_scarpi_hedonic_playful_arousal_variance(const teddy_bear_geometry_t *geom, double novelty_shift, double brow_gesture_val, double *arousal_variance_out);

// Models the decay rate of operational trust ratings based on prolonged transaction latency intervals (Scarpi).
bool evaluate_scarpi_utilitarian_operational_trust_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_operational_trust_out);

// Models the variance in user interaction consistency ratings under dynamic boundary conditions (Scarpi).
bool evaluate_scarpi_utilitarian_consistency_variance(const teddy_bear_geometry_t *geom, double consistency_score, double *variance_out);

// Models the decay rate of interface trust ratings based on prolonged visual interaction durations (Scarpi).
bool evaluate_scarpi_hedonic_interface_trust_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_trust_out);

// Models the variance in interface trust ratings based on visual design symmetry and dynamic novelty (Scarpi).
bool evaluate_scarpi_hedonic_trust_variance(const teddy_bear_geometry_t *geom, double aesthetic_rating, double novelty_scale, double *trust_variance_out);

// Models the decay rate of quality ratings based on prolonged testing durations (Scarpi).
bool evaluate_scarpi_utilitarian_quality_control_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_quality_out);

// Models the variance in playfulness engagement ratings based on dynamic novelty and aesthetic alignment (Scarpi).
bool evaluate_scarpi_hedonic_playful_engagement_variance(const teddy_bear_geometry_t *geom, double novelty_scale, double aesthetic_alignment, double *engagement_variance_out);

// Models the variance in consistency decay rates based on tracking duration variations (Scarpi).
bool evaluate_scarpi_utilitarian_consistency_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in playful arousal ratings under dynamic novelty shifts and design layout symmetry variations (Scarpi).
bool evaluate_scarpi_hedonic_playful_arousal_mod_variance(const teddy_bear_geometry_t *geom, double novelty_scale, double design_symmetry, double *variance_out);

// Models the variance in quality control ratings based on task complexity levels (Scarpi).
bool evaluate_scarpi_utilitarian_quality_control_variance(const teddy_bear_geometry_t *geom, double complexity_level, double *variance_out);

// Models the variance in playful arousal decay rates based on dynamic novelty shifts (Scarpi).
bool evaluate_scarpi_hedonic_playful_arousal_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in quality control decay rates over prolonged testing durations (Scarpi).
bool evaluate_scarpi_utilitarian_quality_control_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in playful engagement decay rates based on prolonged novelty exposure durations (Scarpi).
bool evaluate_scarpi_hedonic_playful_engagement_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in utilitarian structural consistency ratings based on task complexity levels (Scarpi).
bool evaluate_scarpi_utilitarian_consistency_variance_mod(const teddy_bear_geometry_t *geom, double complexity_level, double *variance_out);

// Models the variance in quality control modulator ratings based on task complexity levels (Scarpi).
bool evaluate_scarpi_utilitarian_quality_control_variance_mod(const teddy_bear_geometry_t *geom, double complexity_level, double *variance_out);

// Models the variance in playful arousal decay modulator ratings under dynamic layout symmetry variations (Scarpi).
bool evaluate_scarpi_hedonic_playful_arousal_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in utilitarian operational trust modulator ratings under dynamic layout symmetry variations (Scarpi).
bool evaluate_scarpi_utilitarian_operational_trust_variance_mod(const teddy_bear_geometry_t *geom, double trust_score, double *variance_out);

// Models the variance in playful engagement modulator ratings under dynamic layout symmetry variations (Scarpi).
bool evaluate_scarpi_hedonic_playful_engagement_variance_mod(const teddy_bear_geometry_t *geom, double engagement_score, double *variance_out);

// Models the variance in utilitarian operational consistency modulator ratings under dynamic layout symmetry variations (Scarpi).
bool evaluate_scarpi_utilitarian_consistency_variance_mod_single(const teddy_bear_geometry_t *geom, double consistency_score, double *variance_out);

// Models the variance in decay rates of quality control modulator ratings based on prolonged tracking intervals (Scarpi).
bool evaluate_scarpi_utilitarian_quality_control_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of playful engagement modulator ratings based on prolonged tracking intervals (Scarpi).
bool evaluate_scarpi_hedonic_playful_engagement_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of operational consistency modulator ratings based on prolonged tracking intervals (Scarpi).
bool evaluate_scarpi_utilitarian_consistency_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of operational trust modulator ratings based on prolonged tracking intervals (Scarpi).
bool evaluate_scarpi_utilitarian_operational_trust_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of interface trust modulator ratings based on prolonged tracking intervals (Scarpi).
bool evaluate_scarpi_hedonic_interface_trust_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Evaluates interaction trust based on visual aesthetics and novelty parameters (Scarpi).
bool evaluate_scarpi_hedonic_trust(const teddy_bear_geometry_t *geom, double aesthetic_rating, double novelty_scale, double *trust_out);

// Models user interaction ratings based on latency speeds and coordinate consistency checking times (Scarpi).
bool evaluate_scarpi_utilitarian_efficiency(const teddy_bear_geometry_t *geom, double latency_sec, double *efficiency_out);

// Evaluates perceived conversational naturalness based on vocal pitch frequency jitter (Hyde et al.).
bool evaluate_hyde_vocal_jitter_naturalness(const teddy_bear_geometry_t *geom, double pitch_jitter, double *naturalness_out);

// Models engagement rates based on vocal intonation amplitude variance (Hyde et al.).
bool evaluate_hyde_intonation_amplitude(const teddy_bear_geometry_t *geom, double intonation_variance, double *engagement_out);

// Models the variance in conversational vocal naturalness ratings based on vocal jitter and intonation amplitude mismatch levels (Hyde et al.).
bool evaluate_hyde_vocal_naturalness_variance(const teddy_bear_geometry_t *geom, double vocal_jitter, double amplitude_mismatch, double *naturalness_variance_out);

// Models the decay rate of intonation amplitude variance over dynamic interaction durations (Hyde et al.).
bool evaluate_hyde_intonation_amplitude_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_amplitude_out);

// Models the variance in conversational synchronization ratings based on vocal tempo matching and size mismatch levels (Hyde et al.).
bool evaluate_hyde_vocal_tempo_sync_variance(const teddy_bear_geometry_t *geom, double tempo_sync_val, double size_mismatch, double *sync_variance_out);

// Models the decay rate of vocal size jitter variation over dynamic interaction durations (Hyde et al.).
bool evaluate_hyde_vocal_size_jitter_decay_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_size_jitter_out);

// Models the variance in vocal warmth ratings based on conversational interruption frequency variations and vocal pitch (Hyde et al.).
bool evaluate_hyde_vocal_warmth_variance(const teddy_bear_geometry_t *geom, double interruption_freq, double pitch_hz, double *warmth_variance_out);

// Models the decay rate of vocal size pitch variation over dynamic interaction durations (Hyde et al.).
bool evaluate_hyde_vocal_size_pitch_decay_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_size_pitch_out);

// Models the variance in conversational synchronization ratings based on vocal tempo range variations (Hyde et al.).
bool evaluate_hyde_vocal_tempo_range_variance(const teddy_bear_geometry_t *geom, double tempo_range_val, double size_mismatch, double *range_variance_out);

// Models the variance in decay rates of vocal size jitter variation over dynamic interaction durations (Hyde et al.).
bool evaluate_hyde_vocal_size_jitter_decay_mod_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in vocal warmth ratings based on conversational turn interruption frequency variations (Hyde et al.).
bool evaluate_hyde_vocal_turn_interruption_variance(const teddy_bear_geometry_t *geom, double interruption_freq, double *variance_out);

// Models the variance in vocal size pitch values under dynamic pitch range variations (Hyde et al.).
bool evaluate_hyde_vocal_size_pitch_variance(const teddy_bear_geometry_t *geom, double size_pitch_val, double pitch_range, double *variance_out);

// Models the variance in decay rates of vocal tempo range variations over dynamic interaction durations (Hyde et al.).
bool evaluate_hyde_vocal_tempo_range_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in conversational synchronization ratings based on vocal size sync variations (Hyde et al.).
bool evaluate_hyde_vocal_size_sync_variance(const teddy_bear_geometry_t *geom, double size_sync_val, double size_mismatch, double *variance_out);

// Models the variance in decay rates of vocal warmth ratings based on dynamic interaction durations (Hyde et al.).
bool evaluate_hyde_vocal_warmth_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in conversational synchronization ratings based on speech tempo sync variations (Hyde et al.).
bool evaluate_hyde_vocal_tempo_sync_variance_mod(const teddy_bear_geometry_t *geom, double tempo_sync_val, double *variance_out);

// Models the variance in decay rates of vocal warmth range ratings based on prolonged tracking intervals (Hyde et al.).
bool evaluate_hyde_vocal_warmth_range_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in conversational synchronization ratings based on speech warmth modulation variations (Hyde et al.).
bool evaluate_hyde_vocal_warmth_modulation_variance_mod(const teddy_bear_geometry_t *geom, double warmth_mod_val, double *variance_out);

// Models the variance in vocal warmth range ratings under dynamic layout symmetry variations (Hyde et al.).
bool evaluate_hyde_vocal_warmth_range_variance_mod(const teddy_bear_geometry_t *geom, double warmth_range_val, double *variance_out);

// Models the variance in conversational synchronization ratings based on speech size pitch variations (Hyde et al.).
bool evaluate_hyde_vocal_size_pitch_variance_mod(const teddy_bear_geometry_t *geom, double size_pitch_val, double *variance_out);

// Models the variance in decay rates of vocal size jitter variations based on prolonged tracking intervals (Hyde et al.).
bool evaluate_hyde_vocal_size_jitter_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in conversational synchronization ratings based on speech size sync variations (Hyde et al.).
bool evaluate_hyde_vocal_size_sync_variance_mod(const teddy_bear_geometry_t *geom, double size_sync_val, double *variance_out);

// Models the variance in vocal size jitter modulator ratings under dynamic layout symmetry variations (Hyde et al.).
bool evaluate_hyde_vocal_size_jitter_variance_mod(const teddy_bear_geometry_t *geom, double size_jitter_val, double *variance_out);

// Models the variance in vocal warmth modulator ratings under dynamic layout symmetry variations (Hyde et al.).
bool evaluate_hyde_vocal_warmth_variance_mod(const teddy_bear_geometry_t *geom, double warmth_val, double *variance_out);

// Models the variance in conversational synchronization ratings based on speech tempo range variations (Hyde et al.).
bool evaluate_hyde_vocal_tempo_range_variance_mod(const teddy_bear_geometry_t *geom, double tempo_range_val, double *variance_out);

// Models the variance in decay rates of vocal warmth modulator ratings based on prolonged tracking intervals (Hyde et al.).
bool evaluate_hyde_vocal_warmth_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of conversational synchronization ratings based on speech tempo range decay variations (Hyde et al.).
bool evaluate_hyde_vocal_tempo_range_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of vocal size jitter modulator ratings based on prolonged tracking intervals (Hyde et al.).
bool evaluate_hyde_vocal_size_jitter_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of conversational synchronization ratings based on speech tempo sync decay variations (Hyde et al.).
bool evaluate_hyde_vocal_tempo_sync_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of vocal size pitch modulator ratings based on prolonged tracking intervals (Hyde et al.).
bool evaluate_hyde_vocal_size_pitch_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of conversational synchronization ratings based on speech size sync decay variations (Hyde et al.).
bool evaluate_hyde_vocal_size_sync_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Simulates the low-level diode reflex and relative capacitor charge decay loop.
bool simulate_diode_capacitor_loop(double input_voltage, double resistance, double capacitance, double time_step, double *charge_state);

// QR Decomposition implemented directly at the H-bridge izotope ozone circuit near the RED sustain flyback diode.
bool hbridge_izotope_ozone_diode_qr(const double *A, double *Q, double *R, int n, const char *method);

// QR-Rotations implemented on a Tremolo (Hessenberg) matrix, connected to the BLACK flyback diode with a 2-3 tree.
bool hbridge_izotope_black_diode_graph_qr(const double *Tremolo_in, double *Tremolo_out, int n);

// Evaluates the H-bridge switching flyback transient mismatch for izotope filtering.
bool evaluate_hbridge_izotope_mismatch(const teddy_bear_geometry_t *geom, double switching_frequency, double *flyback_mismatch_out);

// Simulates the snubber-clamped inductive flyback transient voltage decay.
bool simulate_snubber_clamped_flyback(double peak_voltage, double inductance, double snubber_resistance, double time_step, double *clamped_voltage_out);

// Simulates the RCD snubber resonant capacitor decay stage.
bool simulate_rcd_snubber_decay(double peak_voltage, double resistance, double capacitance, double inductance, double time_step, double *voltage_state);

// Calculates the nerve-like Gumbel transient diyat tax on top of base gas costs.
bool calculate_diyat_tax(const teddy_bear_geometry_t *geom, double switching_frequency, double base_gas_cost, double *total_cost_out);

// Calculates the nerve-like Gumbel transient diyat tax with biological refractory recovery periods.
bool calculate_diyat_tax_with_refractory(const teddy_bear_geometry_t *geom, double switching_frequency, double base_gas_cost, double time_since_last_event, double *total_cost_out);

// Calculates the nerve-like Gumbel transient diyat tax modulated by tremolo and sustain envelope dynamics.
bool calculate_diyat_tax_with_envelope(const teddy_bear_geometry_t *geom, double switching_frequency, double base_gas_cost, double tremolo_freq, double sustain_time, double *total_cost_out);

// Simulates the phase-modulated flyback noise distortion index for izotope filtering.
bool simulate_phase_flyback_noise(const teddy_bear_geometry_t *geom, double phase_angle, double *noise_out);

// Evaluates symmetric threshold (tremolo) and group scale (sustain) constraints in the izotope system.
bool evaluate_izotope_constrained_parameters(const teddy_bear_geometry_t *geom, int group_id, double *tremolo_spacing_out, double *sustain_decay_out);

// Evaluates the izotope/ozone-adjusted habituation decay logic (Christensen).
bool evaluate_izotope_ozone_habituation_decay(const teddy_bear_geometry_t *geom, double base_decay, double *decayed_out);

// Evaluates relationship warmth ratings based on conversational turn-taking sync and posture pitch alignment metrics (Castle).
bool evaluate_castle_diplomatic_alignment(const teddy_bear_geometry_t *geom, double turn_taking_sync, double posture_alignment, double *alignment_out);

// Models communication consistency ratings based on message repetition and alignment bounds (Castle).
bool evaluate_castle_policy_consistency(const teddy_bear_geometry_t *geom, double consistency_score, double *policy_consistency_out);

// Models the decay rate of relationship warmth ratings based on prolonged lack of interaction (Castle).
bool evaluate_castle_diplomatic_alignment_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_alignment_out);

// Models the variance in communication consistency ratings under dynamic boundary conditions (Castle).
bool evaluate_castle_policy_consistency_variance(const teddy_bear_geometry_t *geom, double consistency_score, double *variance_out);

// Evaluates policy implementation progress ratings based on dynamic target metrics (Castle).
bool evaluate_castle_policy_target_engagement(const teddy_bear_geometry_t *geom, double target_metric, double *engagement_out);

// Models trust boundary configurations based on visual and vocal symmetry matching (Castle).
bool evaluate_castle_diplomatic_trust_bounds(const teddy_bear_geometry_t *geom, double visual_symmetry, double vocal_symmetry, double *bounds_out);

// Models the decay rate of diplomatic trust boundary configurations over prolonged lack of verification (Castle).
bool evaluate_castle_diplomatic_trust_bounds_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_bounds_out);

// Models the variance in progress ratings based on dynamic target metrics (Castle).
bool evaluate_castle_policy_target_engagement_variance(const teddy_bear_geometry_t *geom, double target_metric, double *variance_out);

// Models the variance in trust boundary configurations under dynamic context variations (Castle).
bool evaluate_castle_diplomatic_trust_bounds_variance(const teddy_bear_geometry_t *geom, double visual_symmetry, double vocal_symmetry, double *bounds_variance_out);

// Models the decay rate of target engagement ratings over prolonged tracking durations (Castle).
bool evaluate_castle_policy_target_engagement_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_engagement_out);

// Models the variance in diplomatic alignment ratings under dynamic boundary variations (Castle).
bool evaluate_castle_diplomatic_alignment_variance(const teddy_bear_geometry_t *geom, double visual_symmetry, double vocal_symmetry, double *alignment_variance_out);

// Models the decay rate of policy consistency ratings over prolonged tracking intervals (Castle).
bool evaluate_castle_policy_consistency_decay(const teddy_bear_geometry_t *geom, double duration_sec, double *decayed_consistency_out);

// Models the variance in diplomatic alignment decay rates under dynamic interaction durations (Castle).
bool evaluate_castle_diplomatic_alignment_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in policy consistency ratings based on visual design asymmetry (Castle).
bool evaluate_castle_policy_consistency_variance_mod(const teddy_bear_geometry_t *geom, double consistency_score, double *variance_out);

// Models the variance in diplomatic trust bounds decay rates under dynamic interaction durations (Castle).
bool evaluate_castle_diplomatic_trust_bounds_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in policy target engagement ratings based on visual design asymmetry (Castle).
bool evaluate_castle_policy_target_engagement_variance_mod(const teddy_bear_geometry_t *geom, double engagement_score, double *variance_out);

// Models the variance in diplomatic alignment ratings under dynamic boundary variations based on design layout symmetry (Castle).
bool evaluate_castle_diplomatic_alignment_variance_mod(const teddy_bear_geometry_t *geom, double alignment_score, double *variance_out);

// Models the variance in decay rates of policy consistency ratings based on prolonged tracking intervals (Castle).
bool evaluate_castle_policy_consistency_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in diplomatic trust bounds ratings under dynamic boundary variations based on design layout symmetry (Castle).
bool evaluate_castle_diplomatic_trust_bounds_variance_mod(const teddy_bear_geometry_t *geom, double bounds_score, double *variance_out);

// Models the variance in decay rates of policy target engagement ratings based on prolonged tracking intervals (Castle).
bool evaluate_castle_policy_target_engagement_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in policy consistency decay modulator ratings over prolonged tracking intervals (Castle).
bool evaluate_castle_policy_consistency_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of diplomatic alignment modulator ratings under dynamic layout symmetry variations (Castle).
bool evaluate_castle_diplomatic_alignment_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of diplomatic trust bounds modulator ratings under dynamic layout symmetry variations (Castle).
bool evaluate_castle_diplomatic_trust_bounds_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of policy target engagement modulator ratings under dynamic layout symmetry variations (Castle).
bool evaluate_castle_policy_target_engagement_decay_variance_mod(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);


// Models the relationship between facial width-to-height ratio variations and social dominance ratings (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_dominance(const teddy_bear_geometry_t *geom, double fwhr_val, double *dominance_out);

// Models the relationship between facial width-to-height ratio variations and trustworthiness ratings (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_trustworthiness(const teddy_bear_geometry_t *geom, double fwhr_val, double *trustworthiness_out);

// Models the variance in decay rates of FWHR dominance modulator ratings under dynamic layout symmetry variations (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_dominance_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of FWHR trustworthiness modulator ratings under dynamic layout symmetry variations (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_trustworthiness_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the relationship between facial width-to-height ratio variations and perceived threat ratings (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_threat(const teddy_bear_geometry_t *geom, double fwhr_val, double *threat_out);

// Models the relationship between facial width-to-height ratio variations and expected cooperation ratings (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_cooperation(const teddy_bear_geometry_t *geom, double fwhr_val, double *cooperation_out);

// Models the variance in decay rates of FWHR threat modulator ratings under dynamic layout symmetry variations (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_threat_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in decay rates of FWHR cooperation modulator ratings under dynamic layout symmetry variations (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_cooperation_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);


// Models the discrepancy/distortion between 2D photographic fWHR and direct 3D scan geometry (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_3d_scan_discrepancy(const teddy_bear_geometry_t *geom, double fwhr_val, double *discrepancy_out);

// Models the null-hypothesis scale factor (lack of significant sexual dimorphism) between male/female fWHR values (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_sexual_dimorphism_null_hypothesis(const teddy_bear_geometry_t *geom, double male_fwhr, double female_fwhr, double *p_value_out);

// Models how pitch-angle head tilt (upward or downward) distorts the apparent fWHR projected from 3D scan to 2D photograph (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_head_tilt_effect(const teddy_bear_geometry_t *geom, double fwhr_val, double pitch_degrees, double *apparent_fwhr_out);

// Models the mismatch between perceived dominance/threat ratings and actual cooperative/aggressive outputs (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_actual_vs_perceived_aggression_mismatch(const teddy_bear_geometry_t *geom, double perceived_dominance, double actual_aggression, double *mismatch_out);

// Models the perceptual sensitivity decay under noise, blurring, or features occlusion (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_perceptual_noise_sensitivity(const teddy_bear_geometry_t *geom, double noise_level, double *threshold_se_multiplier_out);

// Models contrast-effect threshold shifts based on sequential history of processed faces (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_sequential_adaptation_bias(const teddy_bear_geometry_t *geom, const double *recent_fwhr_history, int history_len, double *adaptation_offset_out);

// Models the weighting bias prioritizing right hemiface fWHR cues over left hemiface cues in dominance judgments (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_hemiface_asymmetry_bias(const teddy_bear_geometry_t *geom, double left_fwhr, double right_fwhr, double *weighted_fwhr_out);

// Models apparent fWHR cheekbone edge warping under directional lighting and fur roughness highlights (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_ambient_shading_effect(const teddy_bear_geometry_t *geom, double original_fwhr, double light_angle_degrees, double *apparent_fwhr_out);

// Models dynamic temporal rating integration over sequential video frame captures (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_dynamic_temporal_smoothing(const teddy_bear_geometry_t *geom, const double *frame_fwhr_sequence, int sequence_len, double *smoothed_fwhr_out);

// Models observer judgment confidence distributions relative to fWHR structural deviation from average (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_judgment_confidence_calibration(const teddy_bear_geometry_t *geom, double fwhr_val, double *confidence_rating_out);

// Models the discrepancy between perceived health and actual physiological health (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_perceived_vs_actual_health(const teddy_bear_geometry_t *geom, double perceived_health, double actual_health, double *mismatch_out);

// Models the geometric modulation of cheekbone prominence and cheek height (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_cheekbone_prominence_modulation(const teddy_bear_geometry_t *geom, double cheekbone_val, double *prominence_mod_out);

// Models perspective distortion of apparent fWHR as a function of lens focal length (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_perspective_distortion(const teddy_bear_geometry_t *geom, double fwhr_val, double focal_length_mm, double *distorted_fwhr_out);

// Models the accuracy limit of fWHR trait resolution under rapid micro-exposure (Kramer & Ward).
bool evaluate_kramer_ward_fwhr_micro_exposure_resolution(const teddy_bear_geometry_t *geom, double exposure_ms, double *resolution_accuracy_out);

// Models perceived trustworthiness and cooperation from human static face fWHR structures (Kramer & Ward).
bool evaluate_kramer_ward_human_fwhr_trustworthiness(const teddy_bear_geometry_t *geom, double fwhr_val, double *trustworthiness_out);

// Evaluates how left-right asymmetry on human faces modulates perceived dominance and aggression (Kramer & Ward).
bool evaluate_kramer_ward_human_face_asymmetry_aggression(const teddy_bear_geometry_t *geom, double asymmetry_val, double *aggression_rating_out);

// Models how relative eye size and pupil dilation dimensions modulate perceived social traits (Kramer & Ward).
bool evaluate_kramer_ward_human_eye_to_face_proportion(const teddy_bear_geometry_t *geom, double eye_size_val, double pupil_dilation_val, double *social_trait_rating_out);

// Models the cross-cultural consensus in dominance attribution from facial gestures (Keating).
bool evaluate_keating_cross_cultural_dominance_consensus(const teddy_bear_geometry_t *geom, double gesture_prominence, double *consensus_out);

// Models how the interaction of target status and observer gender affects dominance perception (Keating).
bool evaluate_keating_gender_status_interaction(const teddy_bear_geometry_t *geom, double status_cue, int observer_gender, double *dominance_out);

// Models the babyfacedness attraction decoupling effect where dominance is traded for warmth/attractiveness (Keating).
bool evaluate_keating_babyfacedness_attractiveness_decoupling(const teddy_bear_geometry_t *geom, double babyface_index, int target_gender, double *attractiveness_out);

// Models the smile-status congruence effect on observer trust (Keating).
bool evaluate_keating_smile_status_congruence(const teddy_bear_geometry_t *geom, double smile_intensity, double status_rank, double *trust_score_out);

// Models how human fWHR predicts perceived dominance and threat in human faces (Kramer & Ward).
bool evaluate_kramer_ward_human_fwhr_dominance(const teddy_bear_geometry_t *geom, double fwhr_val, double *dominance_out);

// Models the relationship between human facial elongation and perceived attractiveness or dominance (Kramer & Ward).
bool evaluate_kramer_ward_human_face_elongation(const teddy_bear_geometry_t *geom, double elongation_val, double *elongation_score_out);

// Models how human eye-to-brow distance signals perceived trust or aggression (Kramer & Ward).
bool evaluate_kramer_ward_human_eye_to_brow_trust(const teddy_bear_geometry_t *geom, double eye_to_brow_distance, double *trust_out);

// Models perceived trustworthiness decay over repeated interactions (Kramer & Ward).
bool evaluate_kramer_ward_human_fwhr_trustworthiness_decay(const teddy_bear_geometry_t *geom, double initial_trust, double duration_sec, double *decayed_trust_out);

// Models how static facial symmetry in humans relates to perceived trustworthiness (Kramer & Ward).
bool evaluate_kramer_ward_human_face_symmetry_trust(const teddy_bear_geometry_t *geom, double symmetry_val, double *trustworthiness_out);

// Models eye size and pupil dilation modulating perceived attractiveness and social interest (Kramer & Ward).
bool evaluate_kramer_ward_human_eye_dilation_arousal(const teddy_bear_geometry_t *geom, double eye_size_val, double pupil_dilation_val, double *arousal_out);

// Models the alignment of pitch fluctuations with status expectations (Keating).
bool evaluate_keating_pitch_vocal_status_congruence(const teddy_bear_geometry_t *geom, double pitch_variation, double status_rank, double *congruence_score_out);

// Models status attribution based on asymmetrical arm/torso postures (Keating).
bool evaluate_keating_dynamic_pose_asymmetry_consensus(const teddy_bear_geometry_t *geom, double posture_asymmetry, double group_size, double *consensus_score_out);

// Models dominance metrics derived from sclera exposure changes during rapid gaze aversion (Keating).
bool evaluate_keating_sclera_gaze_decoupling(const teddy_bear_geometry_t *geom, double sclera_exposure, double aversion_speed, double *dominance_score_out);

// Models the consensus of perceived agreeableness rating (Kramer, King & Ward).
bool evaluate_kramer_king_ward_perceived_agreeableness_consensus(const teddy_bear_geometry_t *geom, double mouth_curvature, double eye_size, double *consensus_out);

// Models extraversion correlation metrics from face elongation parameters (Kramer, King & Ward).
bool evaluate_kramer_king_ward_actual_extraversion_modulator(const teddy_bear_geometry_t *geom, double elongation_val, double cheekbone_prominence, double *extraversion_mod_out);

// Models perceived threat mapped from static asymmetry index metrics (Kramer, King & Ward).
bool evaluate_kramer_king_ward_neuroticism_threat_mapping(const teddy_bear_geometry_t *geom, double asymmetry_val, double vertical_offset, double *threat_out);

// Models smile dominance attenuation effects on status ratings (Keating).
bool evaluate_keating_smile_dominance_attenuation(const teddy_bear_geometry_t *geom, double smile_intensity, int gender, double *attenuated_dominance_out);

// Models the negative correlation of babyfacedness with leadership potential (Keating).
bool evaluate_keating_babyfacedness_leadership_decoupling(const teddy_bear_geometry_t *geom, double babyface_index, double *leadership_score_out);

// Models torso asymmetry status attribution metrics (Keating).
bool evaluate_keating_torso_asymmetry_status(const teddy_bear_geometry_t *geom, double torso_asymmetry_val, double *status_score_out);

// Models the consensus of perceived conscientiousness rating (Kramer, King & Ward).
bool evaluate_kramer_king_ward_perceived_conscientiousness_consensus(const teddy_bear_geometry_t *geom, double symmetry_val, double forehead_proportion, double *consensus_out);

// Models the relationship between perceived openness and static facial metrics (Kramer, King & Ward).
bool evaluate_kramer_king_ward_perceived_openness_morphological_mapping(const teddy_bear_geometry_t *geom, double eye_size, double elongation_val, double *openness_score_out);

// Models actual agreeableness correlation metrics against facial curvature metrics (Kramer, King & Ward).
bool evaluate_kramer_king_ward_actual_agreeableness_correlation(const teddy_bear_geometry_t *geom, double mouth_upturn, double fwhr_val, double *agreeableness_score_out);

// Models the effect of smiling behavior on perceived cooperative intent (Keating).
bool evaluate_keating_smile_cooperation_modulator(const teddy_bear_geometry_t *geom, double smile_intensity, double *cooperation_score_out);

// Models how eyebrow gestures decouple or highlight fWHR dominance markers (Keating).
bool evaluate_keating_eyebrow_dominance_decoupling(const teddy_bear_geometry_t *geom, double eyebrow_asymmetry, double fwhr_val, double *dominance_score_out);

// Models the interaction between jaw scale and babyfacedness attractiveness (Keating).
bool evaluate_keating_jaw_attractiveness_interaction(const teddy_bear_geometry_t *geom, double jaw_scale, int target_gender, double *attractiveness_score_out);

// Models the consensus of perceived extraversion rating (Kramer, King & Ward).
bool evaluate_kramer_king_ward_perceived_extraversion_consensus(const teddy_bear_geometry_t *geom, double forehead_proportion, double eye_size, double *consensus_out);

// Models the consensus of perceived neuroticism rating (Kramer, King & Ward).
bool evaluate_kramer_king_ward_perceived_neuroticism_consensus(const teddy_bear_geometry_t *geom, double asymmetry_val, double eyebrow_height, double *consensus_out);

// Models actual self-reported conscientiousness scores against static facial metrics (Kramer, King & Ward).
bool evaluate_kramer_king_ward_actual_conscientiousness_correlation(const teddy_bear_geometry_t *geom, double elongation_val, double symmetry_val, double *conscientiousness_score_out);

// Models perceived virtual character warmth from interactive cues (Wang, Geigel & Herbert).
bool evaluate_wang_geigel_character_warmth(const teddy_bear_geometry_t *geom, double gaze_shift_freq, double head_tilt_val, double *warmth_score_out);

// Models perceived competence in virtual actors based on postures and cues (Wang, Geigel & Herbert).
bool evaluate_wang_geigel_competence_projection(const teddy_bear_geometry_t *geom, double posture_pitch, double dominance_cue_val, double *competence_score_out);

// Models emotional contagion factor based on expression cues (Wang, Geigel & Herbert).
bool evaluate_wang_geigel_emotional_contagion(const teddy_bear_geometry_t *geom, double expression_intensity, double pupil_dilation_sync, double *contagion_score_out);

// Models warmth attenuation from erratic gaze shift parameters (Wang, Geigel & Herbert).
bool evaluate_wang_geigel_warmth_attenuation(const teddy_bear_geometry_t *geom, double gaze_erraticness, double *attenuated_warmth_out);

// Models gaze-induced dominance metrics in virtual agents (Wang, Geigel & Herbert).
bool evaluate_wang_geigel_gaze_dominance_modulator(const teddy_bear_geometry_t *geom, double direct_gaze_ratio, double *dominance_score_out);

// Models character realism and expression attraction interactions (Wang, Geigel & Herbert).
bool evaluate_wang_geigel_realism_attraction_congruence(const teddy_bear_geometry_t *geom, double realism_index, double expression_intensity, double *attraction_score_out);

// Models character competence from interactive gaze metrics (Wang, Geigel & Herbert).
bool evaluate_wang_geigel_gaze_competence_modulator(const teddy_bear_geometry_t *geom, double direct_gaze_ratio, double aversion_speed, double *competence_score_out);

// Models contagion arousal scores from pupil and tilt inputs (Wang, Geigel & Herbert).
bool evaluate_wang_geigel_contagion_arousal(const teddy_bear_geometry_t *geom, double pupil_dilation_sync, double head_tilt_val, double *arousal_score_out);

// Models realism trust congruence metrics in virtual agents (Wang, Geigel & Herbert).
bool evaluate_wang_geigel_realism_trust_congruence(const teddy_bear_geometry_t *geom, double realism_index, double smile_intensity, double *trustworthiness_out);

// Models emotional contagion decay over duration (Wang, Geigel & Herbert).
bool evaluate_wang_geigel_emotional_contagion_decay(const teddy_bear_geometry_t *geom, double initial_contagion, double duration_sec, double *decayed_contagion_out);

// Models competence projection decay over duration (Wang, Geigel & Herbert).
bool evaluate_wang_geigel_competence_decay_variance(const teddy_bear_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models mismatch metrics between gaze frequency and realism index (Wang, Geigel & Herbert).
bool evaluate_wang_geigel_gaze_realism_mismatch(const teddy_bear_geometry_t *geom, double gaze_shift_freq, double realism_index, double *mismatch_score_out);

// Models avatar blink rate effect on perceived trust (Wang, Geigel & Herbert).
bool evaluate_wang_geigel_avatar_blink_trust(const teddy_bear_geometry_t *geom, double blink_rate, double *trust_score_out);

// Models perceived negativity from avatar head shake gestures (Wang, Geigel & Herbert).
bool evaluate_wang_geigel_avatar_head_shake(const teddy_bear_geometry_t *geom, double head_shake_intensity, double *negativity_score_out);

// Models gaze-avoidance induced submissiveness in virtual actors (Wang, Geigel & Herbert).
bool evaluate_wang_geigel_avatar_gaze_submissiveness(const teddy_bear_geometry_t *geom, double averted_gaze_ratio, double posture_pitch, double *submissiveness_score_out);

// Models avatar smile attractiveness modulator (Wang, Geigel & Herbert).
bool evaluate_wang_geigel_avatar_smile_attractiveness(const teddy_bear_geometry_t *geom, double smile_intensity, double head_tilt_val, double *attractiveness_score_out);

// Models perceived threat from avatar eyebrow furrow (Wang, Geigel & Herbert).
bool evaluate_wang_geigel_avatar_eyebrow_furrow_threat(const teddy_bear_geometry_t *geom, double furrow_intensity, double direct_gaze_ratio, double *threat_score_out);

// Models realism warmth interaction in virtual agents (Wang, Geigel & Herbert).
bool evaluate_wang_geigel_avatar_realism_warmth_interaction(const teddy_bear_geometry_t *geom, double realism_index, double smile_intensity, double *warmth_score_out);

// Models perceived avatar naturalness from expression synchronization metrics (Masuda, Kunizawa & Takahashi).
bool evaluate_masuda_perceived_naturalness(const teddy_bear_geometry_t *geom, double sync_delay_ms, double smile_intensity, double *naturalness_out);

// Models conversational familiarity from gaze feedback sync and symmetry (Masuda, Kunizawa & Takahashi).
bool evaluate_masuda_conversational_familiarity(const teddy_bear_geometry_t *geom, double reciprocal_gaze_sync, double symmetry_val, double *familiarity_out);

// Models perceived empathy from gesture and pupil sync rate (Masuda, Kunizawa & Takahashi).
bool evaluate_masuda_empathic_synchronization(const teddy_bear_geometry_t *geom, double blink_sync_rate, double pupil_dilation_sync, double *empathy_out);

// Models naturalness attenuation from expression sync delays (Masuda, Kunizawa & Takahashi).
bool evaluate_masuda_naturalness_attenuation(const teddy_bear_geometry_t *geom, double sync_delay_ms, double *attenuated_naturalness_out);

// Models conversational familiarity decay over elapsed silent durations (Masuda, Kunizawa & Takahashi).
bool evaluate_masuda_conversational_familiarity_decay(const teddy_bear_geometry_t *geom, double initial_familiarity, double duration_sec, double *decayed_familiarity_out);

// Models empathic synchronization variance based on interactants (Masuda, Kunizawa & Takahashi).
bool evaluate_masuda_empathic_sync_variance(const teddy_bear_geometry_t *geom, double blink_sync_rate, double pupil_dilation_sync, double *sync_variance_out);

// Models naturalness variance based on gaze sync (Masuda, Kunizawa & Takahashi).
bool evaluate_masuda_naturalness_variance(const teddy_bear_geometry_t *geom, double reciprocal_gaze_sync, double *naturalness_variance_out);

// Models reciprocal gaze attenuation ratings (Masuda, Kunizawa & Takahashi).
bool evaluate_masuda_reciprocal_gaze_attenuation(const teddy_bear_geometry_t *geom, double gaze_return_delay_ms, double *attenuated_gaze_score_out);

// Models empathic contagion interactions (Masuda, Kunizawa & Takahashi).
bool evaluate_masuda_empathic_contagion_interaction(const teddy_bear_geometry_t *geom, double sync_level, double contagion_rate, double *interaction_score_out);

#endif // TSFI_PERSONALITY_MODELS_ADV_H

