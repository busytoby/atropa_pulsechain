#ifndef AUNCIENT_TEDDY_PERSONALITY_BASE_MODELS_H
#define AUNCIENT_TEDDY_PERSONALITY_BASE_MODELS_H
void resolve_teddy_geometry(teddy_personality_t trait, teddy_geometry_t *geom);

// Evaluates an ordinal perception rating (1 to 7) using a cumulative link model.
int evaluate_ordinal_link_rating(const teddy_geometry_t *geom);

// Evaluates profile-likelihood thresholds bounds for category validation.
bool evaluate_profile_likelihood_bounds(const teddy_geometry_t *geom, int category, double *lower_bound, double *upper_bound);

// Evaluates a scale-adjusted ordinal rating using a heteroscedastic cumulative link model.
int evaluate_ordinal_scale_rating(const teddy_geometry_t *geom, double scale_multiplier);

// Evaluates an ordinal rating with nominal effects violating parallel slopes.
int evaluate_ordinal_nominal_rating(const teddy_geometry_t *geom);

// Evaluates model fitting stability via Hessian standard error diagnostics.
bool evaluate_hessian_diagnostics(const teddy_geometry_t *geom, double *variance_out);

// Evaluates the full parameter covariance matrix derived from the Hessian inverse.
bool evaluate_parameter_covariance(const teddy_geometry_t *geom, double *covariance_matrix_out);

// Evaluates an ordinal rating using a complementary log-log (cloglog) link model.
int evaluate_ordinal_cloglog_rating(const teddy_geometry_t *geom);

// Evaluates an ordinal rating using a scale-adjusted cloglog link model (Christensen).
int evaluate_ordinal_cloglog_scale_rating(const teddy_geometry_t *geom, double scale_covariate);

// Evaluates an ordinal rating using a probit link model.
int evaluate_ordinal_probit_rating(const teddy_geometry_t *geom);

// Evaluates an asymmetrical ordinal rating using a Gumbel (extreme value) link model.
int evaluate_ordinal_gumbel_rating(const teddy_geometry_t *geom);

// Evaluates an asymmetrical ordinal rating using a log-log link model.
int evaluate_ordinal_loglog_rating(const teddy_geometry_t *geom);

// Evaluates the cumulative probability under a chosen ordinal link model type.
bool evaluate_ordinal_link_probability(double latent_val, double threshold_val, int link_type, double *probability_out);

// Evaluates the expected response value from link probabilities.
bool evaluate_ordinal_link_expectation(const double *probabilities, int count, double *expectation_out);

// Evaluates the standard error of the expected response value.
bool evaluate_ordinal_link_expectation_se(const double *probabilities, const double *covariance_matrix, int count, double *se_out);

// Evaluates the mixture link expectation standard error (Christensen).
bool evaluate_ordinal_mixture_expectation_se(const double *probabilities, const double *covariance_matrix, double mixture_weight, int count, double *se_out);

// Evaluates the log-likelihood of the cumulative link model given observations.
bool evaluate_ordinal_link_loglik(const teddy_geometry_t *geom, const int *observed_ratings, int count, double *loglik_out);

// Evaluates an ordinal rating using a Cauchy link model.
int evaluate_ordinal_cauchy_rating(const teddy_geometry_t *geom);

// Evaluates an ordinal rating using a log-gamma link model.
int evaluate_ordinal_loggamma_rating(const teddy_geometry_t *geom, double lambda);

// Evaluates an ordinal rating using a Gumbel link model.
int evaluate_ordinal_gumbel_rating(const teddy_geometry_t *geom);

// Evaluates the flexible mixture link function mapping ordinal boundaries (Christensen).
bool evaluate_ordinal_flexible_mixture_link(const teddy_geometry_t *geom, double mixture_weight, int *rating_out);

// Evaluates the mixture link nominal-adjusted threshold bounds (Christensen).
bool evaluate_ordinal_mixture_nominal_thresholds(const teddy_geometry_t *geom, double mixture_weight, const double *nominal_covariates, double *thresholds_out);

// Evaluates Cauchy/Gumbel mixture link mappings. a customized flexible link mixture weight (0.0 cloglog, 1.0 logit).
int evaluate_ordinal_flexible_rating(const teddy_geometry_t *geom, double link_mixture_weight);

// Evaluates an ordinal rating using a Cauchy and Gumbel mixture link model.
int evaluate_ordinal_cauchy_gumbel_mixture(const teddy_geometry_t *geom, double cauchy_weight);

// Evaluates an ordinal rating using a scale-adjusted Gumbel link model (Christensen).
int evaluate_ordinal_gumbel_scale_rating(const teddy_geometry_t *geom, double scale_covariate);

// Diagnoses whether ordinal thresholds fit equidistant constraints.
bool evaluate_threshold_equidistancy(const teddy_geometry_t *geom, double tolerance, double *spacing_error);

// Evaluates profile log-likelihood bounds for scale parameter estimation.
bool evaluate_scale_profile_bounds(const teddy_geometry_t *geom, double *lower_scale_bound, double *upper_scale_bound);

// Evaluates structured covariate scale parameters under custom link models.
bool evaluate_scale_structured_covariates(const teddy_geometry_t *geom, double age_covariate, double *scale_out);

// Evaluates nominal effects violating parallel scale parameter bounds.
bool evaluate_scale_nominal_effects(const teddy_geometry_t *geom, const double *nominal_covariates, int df, double *effects_out);

// Evaluates profile log-likelihood bounds for a specific threshold parameter.
bool evaluate_threshold_profile_bounds(const teddy_geometry_t *geom, int threshold_index, double *lower_bound, double *upper_bound);

// Evaluates nested models via Likelihood Ratio Test (LRT) diagnostics.
bool evaluate_lrt_nested_models(double null_loglik, double alt_loglik, int df_diff, double *chi_sq_out, double *p_value_out);

// Computes category predicted probability and confidence interval bounds.
bool evaluate_predicted_probability_bounds(const teddy_geometry_t *geom, int category, double *prob_out, double *lower_prob_bound, double *upper_prob_bound);

// Evaluates nominal effect parameter violations via the Wald test statistic.
bool evaluate_wald_nominal_test(const double *beta_vector, const double *covariance_matrix, int df, double *wald_stat_out, double *p_value_out);

// Calculates surrogate residuals for model specification diagnostics.
bool evaluate_surrogate_residuals(const teddy_geometry_t *geom, int observed_rating, double *residual_out);

// Calculates surrogate residuals for nominal scale ordinal regression models.
bool evaluate_nominal_surrogate_residuals(const teddy_geometry_t *geom, int observed_rating, double *residual_out);

// Evaluates individual threshold parameters stability via the Wald test.
bool evaluate_threshold_wald_test(double threshold_est, double baseline, double variance, double *wald_stat_out, double *p_value_out);

// Evaluates scale parameter variance stability via the Wald test.
bool evaluate_scale_wald_test(double scale_est, double baseline, double variance, double *wald_stat_out, double *p_value_out);

// Evaluates nominal effects on scale parameters via the Wald test statistic.
bool evaluate_scale_nominal_wald_test(const double *gamma_vector, const double *covariance_matrix, int df, double *wald_stat_out, double *p_value_out);

// Evaluates nominal effects on threshold parameters via the Wald test statistic.
bool evaluate_threshold_nominal_wald_test(const double *theta_vector, const double *covariance_matrix, int df, double *wald_stat_out, double *p_value_out);

// Evaluates mixed-link threshold parameters via the Wald test (Christensen).
bool evaluate_ordinal_mixture_threshold_wald_test(const double *theta_vector, const double *covariance_matrix, double mixture_weight, int df, double *wald_stat_out, double *p_value_out);

// Evaluates threshold parameters under scale-adjusted Wald tests (Christensen).
bool evaluate_scale_adjusted_threshold_wald(double threshold_est, double scale_multiplier, double baseline, double variance, double *wald_stat_out, double *p_value_out);

// Evaluates nominal parameters under flexible mixture link Wald tests (Christensen).
bool evaluate_mixture_link_nominal_wald(const double *beta_vector, const double *covariance_matrix, int df, double *wald_stat_out, double *p_value_out);

// Evaluates standard single parameter Wald statistics.
bool evaluate_parameter_wald_test(double estimate, double baseline, double variance, double *wald_stat_out, double *p_value_out);

// Evaluates scale-adjusted threshold parameter Wald tests under link model constraints.
bool evaluate_scale_adjusted_threshold_wald(double threshold_est, double scale_multiplier, double baseline, double variance, double *wald_stat_out, double *p_value_out);

// Calculates perceived threat level based on fWHR dominance cues (Geniole et al.).
double evaluate_fw_threat_level(const teddy_geometry_t *geom);

// Computes visual-vocal mismatch threat scores (Geniole et al.).
bool evaluate_behavioral_threat_mismatch(const teddy_geometry_t *geom, double *mismatch_score);

// Computes reactive mismatch retaliation scores based on fWHR and status aggression (Geniole et al.).
double evaluate_reactive_mismatch_retaliation(const teddy_geometry_t *geom);

// Evaluates rapid exposure threat judgment consistency under exposure constraints (Geniole et al.).
bool evaluate_exposure_threat_consistency(const teddy_geometry_t *geom, double exposure_ms, double *perceived_threat_out);

// Evaluates cooperative cheating risk and trustworthiness index (Geniole et al.).
bool evaluate_cooperative_cheating_risk(const teddy_geometry_t *geom, double social_trust_factor, double *cheating_risk_out);

// Evaluates the rapid exposure threat detection limit (Geniole et al.).
bool evaluate_rapid_threat_limit(const teddy_geometry_t *geom, double exposure_ms, double *detected_threat_out);

// Evaluates the reactive retaliation aggression profile (Geniole et al.).
bool evaluate_reactive_retaliation_profile(const teddy_geometry_t *geom, double provocation_intensity, double *retaliation_out);

// Evaluates the provocation aggression threshold boundary (Geniole et al.).
bool evaluate_provocation_aggression_threshold(const teddy_geometry_t *geom, double base_provocation, double *aggression_threshold_out);

// Evaluates the exposure duration adjusted threat threshold (Geniole et al.).
bool evaluate_exposure_adjusted_threat_threshold(const teddy_geometry_t *geom, double exposure_ms, double *adjusted_threat_threshold_out);

// Evaluates status-dominance threat interaction under provocation (Geniole et al.).
bool evaluate_status_dominance_provocation(const teddy_geometry_t *geom, double provocation_scale, double *dominance_threat_out);

// Evaluates statistical threat assessment consistency across repeated dominance trials (Geniole et al.).
bool evaluate_threat_replication_consistency(const teddy_geometry_t *geom, const double *threat_observations, int count, double *consistency_out);

// Evaluates the provocation-modulated reactive retaliation boundary (Geniole et al.).
bool evaluate_reactive_retaliation_boundary(const teddy_geometry_t *geom, double provocation_scale, double *retaliation_boundary_out);

// Evaluates the fWHR retaliation aggression scaling parameter (Geniole et al.).
bool evaluate_retaliation_aggression_scaling(const teddy_geometry_t *geom, double baseline_aggression, double *scaled_aggression_out);

// Evaluates the fWHR retaliation threshold decay over exposure duration (Geniole et al.).
bool evaluate_retaliation_threshold_decay(const teddy_geometry_t *geom, double exposure_ms, double *decayed_threshold_out);

// Evaluates threat threshold incorporating interaction decay over exposure durations (Geniole et al.).
bool evaluate_provocation_exposure_decay(const teddy_geometry_t *geom, double provocation_scale, double exposure_ms, double *decayed_threat_out);

// Evaluates the provocation-modulated fWHR retaliation aggression ceiling (Geniole et al.).
bool evaluate_retaliation_aggression_ceiling(const teddy_geometry_t *geom, double provocation_scale, double *aggression_ceiling_out);

// Evaluates status challenge threat multipliers (Geniole et al.).
bool evaluate_status_dominance_multiplier(const teddy_geometry_t *geom, double provocation_scale, double *multiplier_out);

// Evaluates the provocation-modulated cheating threat index (Geniole et al.).
bool evaluate_cheating_threat_index(const teddy_geometry_t *geom, double provocation_scale, double *cheating_threat_out);

// Evaluates threat replication consistency under provocation (Geniole et al.).
bool evaluate_provocation_replication_consistency(const teddy_geometry_t *geom, double replication_scale, double *consistency_out);

// Evaluates the rapid threat limit boundary under provocation (Geniole et al.).
bool evaluate_provocation_rapid_threat_limit(const teddy_geometry_t *geom, double provocation_scale, double *limit_out);

// Evaluates the provocation-modulated reactive retaliation boundary (Geniole et al.).
bool evaluate_provocation_retaliation_boundary(const teddy_geometry_t *geom, double exposure_ms, double *boundary_out);

// Evaluates the vocal-visual uncanny mismatch index based on geometry and pitch (Hyde et al.).
bool evaluate_uncanny_mismatch_index(const teddy_geometry_t *geom, double *uncanny_score_out);

// Evaluates the motion-mismatch uncanny valley response based on stiffness dynamics (Hyde et al.).
bool evaluate_motion_uncanny_index(const teddy_geometry_t *geom, double movement_stiffness, double *motion_uncanny_out);

// Evaluates the micro-expression jitter uncanny response (Hyde et al.).
bool evaluate_expression_jitter_uncanny(const teddy_geometry_t *geom, double jitter_frequency, double *uncanny_score_out);

// Evaluates the exposure decay uncanny response (Hyde et al.).
bool evaluate_exposure_decay_uncanny(const teddy_geometry_t *geom, double exposure_duration_sec, double *decayed_uncanny_out);

// Evaluates uncanny habituation decay over animation freeze durations (Hyde et al.).
bool evaluate_freeze_habituation_decay(const teddy_geometry_t *geom, double exposure_duration_sec, double *decayed_uncanny_out);

// Evaluates the velocity jitter pacing uncanny response (Hyde et al.).
bool evaluate_velocity_jitter_uncanny(const teddy_geometry_t *geom, double velocity_variance, double *uncanny_score_out);

// Evaluates the unpredictable micro-expression freeze uncanny response (Hyde et al.).
bool evaluate_expression_freeze_uncanny(const teddy_geometry_t *geom, double freeze_duration_sec, double *uncanny_score_out);

// Evaluates the unpredictable micro-expression freeze frequency uncanny response (Hyde et al.).
bool evaluate_expression_freeze_frequency(const teddy_geometry_t *geom, double freeze_frequency_hz, double *uncanny_score_out);

// Executes an H-bridge switching thunk with real-time safety feedback logic.
bool execute_hbridge_thunk_with_feedback(const teddy_geometry_t *geom, double load_inductance, double (*callback)(void), double *safety_margin_out);

// Executes a complementary log-log gated thunk with real-time feedback logic (Christensen).
bool execute_cloglog_thunk_with_feedback(const teddy_geometry_t *geom, double scale_covariate, double (*callback)(void), double *safety_margin_out);

// Executes a maturity-adjusted cloglog gated thunk with feedback (Christensen).
bool execute_maturity_cloglog_thunk_with_feedback(const teddy_geometry_t *geom, double (*callback)(void), double *safety_margin_out);

// Evaluates the unpredictable micro-expression asymmetry uncanny response (Hyde et al.).
bool evaluate_expression_asymmetry_uncanny(const teddy_geometry_t *geom, double asymmetry_deviation, double *uncanny_score_out);

// Evaluates the unpredictable micro-expression asymmetry duration uncanny response (Hyde et al.).
bool evaluate_expression_asymmetry_duration(const teddy_geometry_t *geom, double asymmetry_duration_ms, double *uncanny_score_out);

// Evaluates the unpredictable micro-expression synchronization uncanny response (Hyde et al.).
bool evaluate_expression_sync_uncanny(const teddy_geometry_t *geom, double sync_delay_ms, double *uncanny_score_out);

// Evaluates the unpredictable micro-expression synchronization habituation decay (Hyde et al.).
bool evaluate_sync_habituation_decay(const teddy_geometry_t *geom, double exposure_duration_sec, double *decayed_uncanny_out);

// Evaluates the unpredictable micro-expression acceleration jitter uncanny response (Hyde et al.).
bool evaluate_acceleration_jitter_uncanny(const teddy_geometry_t *geom, double acceleration_variance, double *uncanny_score_out);

// Evaluates the unpredictable micro-expression acceleration jitter habituation decay (Hyde et al.).
bool evaluate_acceleration_habituation_decay(const teddy_geometry_t *geom, double exposure_duration_sec, double *decayed_uncanny_out);

// Evaluates the unpredictable animation acceleration jitter uncanny response (Hyde et al.).
bool evaluate_animation_acceleration_jitter(const teddy_geometry_t *geom, double acceleration_jitter, double *uncanny_score_out);

// Evaluates the unpredictable micro-expression amplitude jitter uncanny response (Hyde et al.).
bool evaluate_expression_amplitude_jitter(const teddy_geometry_t *geom, double amplitude_variance, double *uncanny_score_out);

// Evaluates the unpredictable micro-expression amplitude jitter habituation decay (Hyde et al.).
bool evaluate_amplitude_habituation_decay(const teddy_geometry_t *geom, double exposure_duration_sec, double *decayed_uncanny_out);

// Evaluates the unpredictable micro-expression amplitude jitter uncanny response (Hyde et al.).
bool evaluate_amplitude_jitter_uncanny(const teddy_geometry_t *geom, double amplitude_variance, double *uncanny_score_out);

// Evaluates the vocal-visual synchronization uncanny response (Hyde et al.).
bool evaluate_vocal_visual_sync_uncanny(const teddy_geometry_t *geom, double audio_lag_ms, double *uncanny_score_out);

// Evaluates vocal-visual acceleration synchrony uncanny response (Hyde et al.).
bool evaluate_vocal_visual_acceleration_sync(const teddy_geometry_t *geom, double acceleration_delay_ms, double *uncanny_score_out);

// Evaluates the vocal-visual pitch transition velocity mismatch uncanny response (Hyde et al.).
bool evaluate_vocal_visual_pitch_mismatch(const teddy_geometry_t *geom, double pitch_velocity_mismatch, double *uncanny_score_out);

// Evaluates the combined pitch mismatch and micro-expression freeze uncanny response (Hyde et al.).
bool evaluate_pitch_freeze_uncanny(const teddy_geometry_t *geom, double pitch_velocity_mismatch, double freeze_duration_sec, double *uncanny_score_out);

// Evaluates the combined pitch mismatch and micro-expression jitter uncanny response (Hyde et al.).
bool evaluate_pitch_jitter_uncanny(const teddy_geometry_t *geom, double pitch_velocity_mismatch, double jitter_deviation, double *uncanny_score_out);

// Evaluates the vocal-visual motion acceleration mismatch uncanny response (Hyde et al.).
bool evaluate_motion_acceleration_mismatch(const teddy_geometry_t *geom, double acceleration_mismatch, double *uncanny_score_out);

// Evaluates the combined motion mismatch and micro-expression freeze uncanny response (Hyde et al.).
bool evaluate_motion_freeze_uncanny(const teddy_geometry_t *geom, double motion_mismatch, double freeze_duration_sec, double *uncanny_score_out);

bool evaluate_pitch_velocity_mismatch(const teddy_geometry_t *geom, double pitch_velocity_mismatch, double *uncanny_score_out);

// Evaluates the vocal-visual pitch transition velocity mismatch habituation decay (Hyde et al.).
bool evaluate_pitch_mismatch_habituation_decay(const teddy_geometry_t *geom, double exposure_duration_sec, double *decayed_uncanny_out);

// Evaluates the spatial interaction distance proximity uncanny response (Hyde et al.).
bool evaluate_spatial_interaction_distance(const teddy_geometry_t *geom, double physical_distance_meters, double *interaction_uncanny_out);

// Evaluates the dynamic auditory offset matching vocal gain to ambient noise levels (Hyde et al.).
bool evaluate_dynamic_auditory_offset(const teddy_geometry_t *geom, double ambient_noise_db, double vocal_gain_db, double *auditory_offset_out);

// Evaluates the babyfacedness index based on large eyes, narrow jaw, and low vertical features (Keating, C. F.).
bool evaluate_keating_babyfacedness_index(const teddy_geometry_t *geom, double *babyfacedness_out);

// Evaluates the dominance physiognomy cue index based on high features, small eyes, and wide jaw (Keating, C. F.).
bool evaluate_keating_dominance_cue_index(const teddy_geometry_t *geom, double *dominance_out);

// Evaluates gaze alignment deviations relative to target interaction coordinates (Hyde et al.).
bool evaluate_hyde_gaze_direction_index(const teddy_geometry_t *geom, double target_angle, double *gaze_alignment_out);

// Evaluates vocal speed/pitch stability variations against nominal values (Hyde et al.).
bool evaluate_hyde_vocal_tempo_variance(const teddy_geometry_t *geom, double vocal_tempo_bpm, double *vocal_uncanny_out);

// Evaluates the provocation aggression limit based on fWHR and status-dominance (Geniole et al.).
bool evaluate_geniole_provocation_aggression_limit(const teddy_geometry_t *geom, double provocation_scale, double *aggression_limit_out);

// Models dynamic visual changes driven by baseline testosterone ratings (Geniole et al.).
bool evaluate_geniole_testosterone_modulator(const teddy_geometry_t *geom, double baseline_testosterone, double *modulator_out);

// Evaluates perceived dominance based on low/lowered eyebrows (Keating, C. F.).
bool evaluate_keating_brow_dominance(const teddy_geometry_t *geom, double brow_height, double *brow_dominance_out);

// Models the association between lip thickness/fullness and trustworthiness (Keating, C. F.).
bool evaluate_keating_lip_trustworthiness(const teddy_geometry_t *geom, double lip_thickness, double *lip_trustworthiness_out);

// Evaluates conversational turn-taking reaction delays and response lag (Hyde et al.).
bool evaluate_hyde_conversational_latency(const teddy_geometry_t *geom, double lag_seconds, double *latency_uncanny_out);

// Models the discrepancy between physical body size indicators and vocal pitch (Hyde et al.).
bool evaluate_hyde_vocal_size_mismatch(const teddy_geometry_t *geom, double voice_pitch_hz, double *size_mismatch_out);

// Calculates perceived dominance based on the brow-to-eye spatial distance ratio (Keating, C. F.).
bool evaluate_keating_brow_eye_ratio(const teddy_geometry_t *geom, double brow_eye_distance, double *ratio_dominance_out);

// Maps fWHR threat ratings to dynamic geometry projection dilation rates (Geniole et al.).
bool evaluate_geniole_fwhr_dilation_map(const teddy_geometry_t *geom, double base_dilation, double *mapped_dilation_out);

// Evaluates head roundness babyface index (Keating, C. F.).
bool evaluate_keating_head_roundness_index(const teddy_geometry_t *geom, double *roundness_out);

// Models the relationship between bilateral facial symmetry and perceived babyfacedness (Keating, C. F.).
bool evaluate_keating_symmetry_trust(const teddy_geometry_t *geom, double *trust_out);

// Models the discrepancy between turn-taking conversational status and vocal amplitude (Hyde et al.).
bool evaluate_hyde_vocal_amplitude_mismatch(const teddy_geometry_t *geom, double voice_amplitude_db, double *amplitude_mismatch_out);

// Evaluates the alignment of mouth animation speed to vocal fundamental frequency acceleration curves (Hyde et al.).
bool evaluate_hyde_mouth_speed_synchrony(const teddy_geometry_t *geom, double mouth_speed, double pitch_acceleration, double *sync_mismatch_out);

// Calculates perceived dominance based on the brow-to-chin vertical distance proportion (Keating, C. F.).
bool evaluate_keating_brow_chin_proportion(const teddy_geometry_t *geom, double brow_chin_distance, double *proportion_dominance_out);

// Maps fWHR dominance ratings to micro-expression coordinate jitter bounds (Geniole et al.).
bool evaluate_geniole_fwhr_jitter_mod(const teddy_geometry_t *geom, double base_jitter, double *mapped_jitter_out);

// Evaluates perceived dominance based on direct gaze duration versus eye aversion angles (Keating, C. F.).
bool evaluate_keating_gaze_dominance(const teddy_geometry_t *geom, double gaze_duration_sec, double aversion_angle, double *gaze_dominance_out);

// Models how brow-raise gestures scale perceived submissiveness and babyfacedness (Keating, C. F.).
bool evaluate_keating_brow_gesture(const teddy_geometry_t *geom, double brow_raise_amplitude, double *brow_submissiveness_out);

// Evaluates turn-taking overlap and conversational collision ratings (Hyde et al.).
bool evaluate_hyde_turn_interruption(const teddy_geometry_t *geom, double overlap_duration_sec, double *interruption_uncanny_out);

// Models micro-tremors in voice pitch associated with submissiveness or distress (Hyde et al.).
bool evaluate_hyde_vocal_tremor_index(const teddy_geometry_t *geom, double pitch_variance, double *tremor_uncanny_out);

// Calculates perceived maturity and dominance based on torso-to-head spatial scaling ratio (Keating, C. F.).
bool evaluate_keating_torso_head_ratio(const teddy_geometry_t *geom, double torso_span, double *ratio_dominance_out);

// Maps fWHR aggression thresholds directly to WinchesterMQ register boundary constraints (Geniole et al.).
bool evaluate_geniole_fwhr_boundary_map(const teddy_geometry_t *geom, double threshold_scale, double *mapped_boundary_out);

// Evaluates symmetrical eye dilation balances on perceived babyfacedness (Keating, C. F.).
bool evaluate_keating_eye_dilation_sync(const teddy_geometry_t *geom, double left_dilation, double right_dilation, double *babyface_sync_out);

// Models how head pitch tilt modulates perceived social rank and submissiveness (Keating, C. F.).
bool evaluate_keating_posture_pitch(const teddy_geometry_t *geom, double pitch_angle, double *submissiveness_out);

// Evaluates the coordination rate between visual chin vibrations and audio frequency tremors (Hyde et al.).
bool evaluate_hyde_tremor_frequency_sync(const teddy_geometry_t *geom, double chin_vibration_hz, double audio_tremor_hz, double *sync_rating_out);

// Models perceived conversational engagement based on pitch range variations (Hyde et al.).
bool evaluate_hyde_pitch_range_engagement(const teddy_geometry_t *geom, double pitch_range_hz, double *engagement_rating_out);

// Evaluates how unilateral jaw or chin asymmetry structures alter perceived dominance and threat ratings (Keating, C. F.).
bool evaluate_keating_chin_asymmetry(const teddy_geometry_t *geom, double left_jaw_width, double right_jaw_width, double *asymmetry_dominance_out);

// Models the relationship between upturned mouth curvature and perceived babyfacedness (Keating, C. F.).
bool evaluate_keating_mouth_curvature(const teddy_geometry_t *geom, double upturn_curvature, double *warmth_rating_out);

// Evaluates how average voice pitch values align with dynamic warmth perception offsets (Hyde et al.).
bool evaluate_hyde_vocal_warmth_pitch(const teddy_geometry_t *geom, double average_pitch_hz, double *warmth_offset_out);

// Models user aversion ratings based on conversational collision frequencies (Hyde et al.).
bool evaluate_hyde_interruption_frequency(const teddy_geometry_t *geom, double collision_rate, double *aversion_rating_out);

// Evaluates how unilateral chin asymmetry interacts with dynamic vertex dilation constraints (Keating, C. F.).
bool evaluate_keating_chin_asymmetry_dilation(const teddy_geometry_t *geom, double base_dilation, double *asymmetry_dilation_out);

// Models the habituation and decay of retaliation limits in response to provocation exposure based on fWHR (Geniole et al.).
bool evaluate_geniole_fwhr_retaliation_decay(const teddy_geometry_t *geom, double exposure_duration_sec, double *decayed_retaliation_out);

// Evaluates visible white sclera ratio effects on perceived submissiveness (Keating, C. F.).
bool evaluate_keating_sclera_size(const teddy_geometry_t *geom, double sclera_ratio, double *submissiveness_out);

// Models perceived dominance based on bilateral facial width deviations (Keating, C. F.).
bool evaluate_keating_width_asymmetry(const teddy_geometry_t *geom, double left_width, double right_width, double *asymmetry_dominance_out);

// Models playfulness, warmth, and visual pleasure based on friendly facial parameters (Scarpi).
bool evaluate_scarpi_hedonic_orientation(const teddy_geometry_t *geom, double playfulness_scale, double *hedonic_out);

// Models functional efficiency and task-oriented focus based on structured features (Scarpi).
bool evaluate_scarpi_utilitarian_orientation(const teddy_geometry_t *geom, double efficiency_scale, double *utilitarian_out);

// Evaluates how design symmetry and hedonic orientation interact to modify trust ratings (Scarpi).
bool evaluate_scarpi_aesthetic_trust(const teddy_geometry_t *geom, double base_trust, double *aesthetic_trust_out);

// Models task-oriented utilitarian efficiency decay and user fatigue over time (Scarpi).
bool evaluate_scarpi_utilitarian_decay(const teddy_geometry_t *geom, double interaction_duration_sec, double *decayed_efficiency_out);

// Models coordinate projection offsets based on heliocentric orbital phases and planetary scale ratios (Cellarius).
bool evaluate_cellarius_heliocentric_alignment(const teddy_geometry_t *geom, double orbital_phase, double *alignment_offset_out);

// Calculates boundary limits mapping 3D coordinate paths to star-envelope constraints (Cellarius).
bool evaluate_cellarius_constellation_boundary(const teddy_geometry_t *geom, double celestial_longitude, double *boundary_limit_out);

// Evaluates perceived conversational naturalness based on audio-visual latency jitter (Hyde et al.).
bool evaluate_hyde_av_latency_jitter(const teddy_geometry_t *geom, double latency_jitter_sec, double *naturalness_out);

// Models engagement levels based on conversational vocal energy delivery variance (Hyde et al.).
bool evaluate_hyde_vocal_energy_variance(const teddy_geometry_t *geom, double energy_variance, double *engagement_out);

// Evaluates perceived social dominance and babyfacedness based on facial width-to-height ratio (Keating, C. F.).
bool evaluate_keating_fwhr_dominance(const teddy_geometry_t *geom, double fwhr_value, double *dominance_out);

// Models perceived submissiveness based on mouth-to-jaw width ratios (Keating, C. F.).
bool evaluate_keating_mouth_width_ratio(const teddy_geometry_t *geom, double mouth_width, double jaw_width, double *submissiveness_out);

// Evaluates perceived trustworthiness based on dynamic facial width symmetry (Keating, C. F.).
bool evaluate_keating_width_symmetry_trust(const teddy_geometry_t *geom, double symmetry_ratio, double *trustworthiness_out);

// Models submissiveness and babyfacedness based on lip-to-facial height ratios (Keating, C. F.).
bool evaluate_keating_lip_submissiveness(const teddy_geometry_t *geom, double lip_height_ratio, double *submissiveness_out);

// Evaluates playfulness and design novelty effects on perceived emotional arousal (Scarpi).
bool evaluate_scarpi_hedonic_arousal(const teddy_geometry_t *geom, double playfulness_scale, double novelty_index, double *arousal_out);

// Models utilitarian quality evaluation based on physical layout symmetry and stability indices (Scarpi).
bool evaluate_scarpi_utilitarian_quality(const teddy_geometry_t *geom, double stability_index, double *quality_rating_out);

// Models playfulness arousal decay over dynamic interaction durations (Scarpi).
bool evaluate_scarpi_hedonic_arousal_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_arousal_out);

// Models user interaction loyalty ratings based on task success and structural consistency checks (Scarpi).
bool evaluate_scarpi_utilitarian_loyalty(const teddy_geometry_t *geom, double success_rate, double *loyalty_out);

// Models coordinate translation offsets derived from planetary orbit eccentricity ratios (Cellarius).
bool evaluate_cellarius_planetary_eccentricity(const teddy_geometry_t *geom, double eccentricity_ratio, double *translation_offset_out);

// Calculates frequency modifiers based on epicycle-to-deferent planetary orbit ratios (Cellarius).
bool evaluate_cellarius_epicycle_modulation(const teddy_geometry_t *geom, double epicycle_ratio, double *frequency_modifier_out);

// Models coordinate radial offsets based on orbital velocity variations (Cellarius).
bool evaluate_cellarius_planet_velocity(const teddy_geometry_t *geom, double velocity_val, double *radial_offset_out);

// Calculates coordinate offsets based on epicycle angular phase rotations (Cellarius).
bool evaluate_cellarius_epicycle_phase(const teddy_geometry_t *geom, double phase_angle, double *phase_offset_out);

// Models changes in planetary orbital velocity coordinates based on eccentricity ratios (Cellarius).
bool evaluate_cellarius_eccentricity_velocity(const teddy_geometry_t *geom, double eccentricity_ratio, double *velocity_mod_out);

// Calculates coordinate frequency scale shifts based on radial orbital distances (Cellarius).
bool evaluate_cellarius_radial_frequency(const teddy_geometry_t *geom, double radial_distance, double *frequency_shift_out);

// Models spatial boundary alignment based on constellation distance constraints (Cellarius).
bool evaluate_cellarius_alignment_boundary(const teddy_geometry_t *geom, double constellation_distance, double *boundary_out);

// Calculates coordinate frequency scale shifts based on epicycle orbit radius ratios (Cellarius).
bool evaluate_cellarius_epicycle_radius(const teddy_geometry_t *geom, double epicycle_radius, double *frequency_scale_out);

// Models coordinate shifts based on orbital velocity aligned with epicycle angular phase rotations (Cellarius).
bool evaluate_cellarius_velocity_phase(const teddy_geometry_t *geom, double velocity_val, double phase_angle, double *shift_out);

// Calculates coordinate scale shifts based on radial orbital distances (Cellarius).
bool evaluate_cellarius_radial_scale(const teddy_geometry_t *geom, double radial_distance, double *scale_shift_out);

// Models coordinate scale shifts based on orbital eccentricity values (Cellarius).
bool evaluate_cellarius_eccentricity_scale(const teddy_geometry_t *geom, double eccentricity_val, double *scale_mod_out);

// Models the decay rate of epicycle coordinate offset amplitudes over prolonged tracking cycles (Cellarius).
bool evaluate_cellarius_epicycle_phase_decay(const teddy_geometry_t *geom, double tracking_cycles, double *decayed_amplitude_out);

// Models spatial coordinate frequency shifts based on planetary orbital velocity variations and eccentricity (Cellarius).
bool evaluate_cellarius_velocity_eccentricity(const teddy_geometry_t *geom, double velocity_val, double eccentricity_val, double *frequency_shift_out);

// Models the decay rate of planetary orbit alignment over prolonged tracking durations (Cellarius).
bool evaluate_cellarius_alignment_decay(const teddy_geometry_t *geom, double tracking_duration, double *decayed_boundary_out);

// Models the variance in planet coordinate scale shifts based on orbital velocity variations and eccentricity values (Cellarius).
bool evaluate_cellarius_velocity_scale_variance(const teddy_geometry_t *geom, double velocity_val, double eccentricity_val, double *scale_variance_out);

// Models the decay rate of epicycle coordinate offset amplitudes over prolonged tracking cycles (Cellarius).
bool evaluate_cellarius_epicycle_phase_decay_mod(const teddy_geometry_t *geom, double tracking_duration, double *decayed_offset_out);

// Evaluates coordinate scaling based on heliocentric phase values (Cellarius).
bool evaluate_cellarius_heliocentric_alignment_scale(const teddy_geometry_t *geom, double orbital_phase, double *scale_out);

// Models the variance in epicycle velocity changes under dynamic gravity perturbations (Cellarius).
bool evaluate_cellarius_epicycle_acceleration_variance(const teddy_geometry_t *geom, double velocity_val, double eccentricity_val, double *variance_out);

// Models the variance in phase angle values under dynamic trajectory shifts (Cellarius).
bool evaluate_cellarius_heliocentric_phase_variance(const teddy_geometry_t *geom, double orbital_phase, double *variance_out);

// Models the decay rate of epicycle coordinate offset velocities over prolonged tracking durations (Cellarius).
bool evaluate_cellarius_epicycle_velocity_decay(const teddy_geometry_t *geom, double tracking_duration, double *decayed_velocity_out);

// Models the variance in epicycle velocity shifts under dynamic gravity perturbations (Cellarius).
bool evaluate_cellarius_epicycle_velocity_variance(const teddy_geometry_t *geom, double velocity_val, double eccentricity_val, double *variance_out);

// Models the decay rate of heliocentric phase angle alignment coordinates over prolonged tracking cycles (Cellarius).
bool evaluate_cellarius_heliocentric_phase_decay(const teddy_geometry_t *geom, double tracking_duration, double *decayed_phase_out);

// Models the variance in constellation alignment boundary settings based on orbital velocity variations and eccentricity values (Cellarius).
bool evaluate_cellarius_alignment_boundary_variance(const teddy_geometry_t *geom, double velocity_val, double eccentricity_val, double *variance_out);

// Models the variance in epicycle offset decay rates based on tracking duration variations (Cellarius).
bool evaluate_cellarius_epicycle_phase_decay_mod_variance(const teddy_geometry_t *geom, double tracking_duration, double *decay_variance_out);

// Models the variance in constellation boundary settings based on orbital eccentricity variations and constellation scale factors (Cellarius).
bool evaluate_cellarius_constellation_boundary_variance(const teddy_geometry_t *geom, double eccentricity_val, double scale_factor, double *variance_out);

// Models the variance in heliocentric phase angle velocity settings under orbital alignment perturbations (Cellarius).
bool evaluate_cellarius_heliocentric_velocity_variance(const teddy_geometry_t *geom, double phase_val, double alignment_factor, double *variance_out);

// Models the variance in planetary orbital velocity decay rates based on tracking duration variations (Cellarius).
bool evaluate_cellarius_planetary_velocity_decay_variance(const teddy_geometry_t *geom, double tracking_duration, double *decay_variance_out);

// Models the variance in epicycle orbital acceleration settings based on design layout symmetry variations (Cellarius).
bool evaluate_cellarius_epicycle_acceleration_variance_mod(const teddy_geometry_t *geom, double acceleration_val, double *variance_out);

// Models the variance in constellation scale factors based on orbital alignment perturbations (Cellarius).
bool evaluate_cellarius_constellation_scale_variance_mod(const teddy_geometry_t *geom, double scale_factor, double *variance_out);

// Models the variance in epicycle orbital acceleration decay rates based on tracking duration variations (Cellarius).
bool evaluate_cellarius_epicycle_acceleration_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in planetary eccentricity scale factors based on orbital alignment perturbations (Cellarius).
bool evaluate_cellarius_planetary_eccentricity_variance_mod(const teddy_geometry_t *geom, double eccentricity_val, double *variance_out);

// Models the variance in epicycle orbital radius decay rates based on tracking duration variations (Cellarius).
bool evaluate_cellarius_epicycle_radius_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in planetary eccentricity decay rates based on prolonged tracking intervals (Cellarius).
bool evaluate_cellarius_planetary_eccentricity_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in heliocentric alignment scale decay rates based on prolonged tracking intervals (Cellarius).
bool evaluate_cellarius_heliocentric_alignment_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in planetary velocity modulator ratings under dynamic layout symmetry variations (Cellarius).
bool evaluate_cellarius_planetary_velocity_variance_mod(const teddy_geometry_t *geom, double velocity_val, double *variance_out);

// Models the variance in heliocentric alignment modulator ratings under dynamic layout symmetry variations (Cellarius).
bool evaluate_cellarius_heliocentric_alignment_variance_mod(const teddy_geometry_t *geom, double alignment_val, double *variance_out);

// Models the variance in constellation scale modulator ratings under dynamic layout symmetry variations (Cellarius).
bool evaluate_cellarius_constellation_scale_variance_mod_single(const teddy_geometry_t *geom, double scale_val, double *variance_out);

// Models the variance in decay rates of epicycle orbital radius modulator ratings under dynamic layout symmetry variations (Cellarius).
bool evaluate_cellarius_epicycle_radius_decay_variance_mod(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

#endif // AUNCIENT_TEDDY_PERSONALITY_BASE_MODELS_H
