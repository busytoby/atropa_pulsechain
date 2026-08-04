#ifndef AUNCIENT_TEDDY_PERSONALITY_H
#define AUNCIENT_TEDDY_PERSONALITY_H

#include <stdbool.h>
#include <stdint.h>

// Personality profiles matching the paper's findings
typedef enum {
    PERSONALITY_TRUSTWORTHY, // Round head, round eyes, medium eye size
    PERSONALITY_AGGRESSIVE,   // Narrow head, almond eyes, medium eye size
    PERSONALITY_EERIE,         // Narrow head, almond eyes, large eye size
    PERSONALITY_SKEPTICAL      // Narrow head, crooked-face symmetry
} teddy_personality_t;

typedef struct {
    double head_fwhr;        // Facial Width-to-Height Ratio (0.7 narrow, 1.0 round)
    double eye_eccentricity; // Eye shape (1.0 round, 0.5 almond/squint)
    double eye_scale;        // Eye size scaling factor (1.0 medium, 1.8 large)
    double stiffness;        // Soft body spring stiffness for clothing/fur dynamics
    double damping;          // Soft body motion damping coefficient
    double taper;            // Taper factor for sharp primitive deformations (cylinders/cones)
    double curvature;        // Surface curvature rounding factor (1.0 round, 0.2 flat/sharp)
    double symmetry;         // Bilateral face/eye symmetry (1.0 symmetric, 0.8 crooked-face)
    double jaw_scale;        // Lower jaw width scaling for perceived strength
    double neck_thickness;   // Neck capsule thickness for formidability cues
    double focal_length;     // Camera lens focal length (35mm baby-face, 85mm flat/dominant)
    double camera_elevation; // Camera vertical translation offset (positive high, negative low)
    double torso_ratio;      // Shoulder-to-hip width ratio (1.5 broad/formidable, 0.7 squat)
    double fur_roughness;    // Specular roughness coefficient for hair shading
    double feature_vertical_offset; // Vertical feature placement (negative lower/baby-face)
    double behavioral_mismatch;     // Cognitive mismatch factor (0.0 aligned, 1.0 high discrepancy)
    double vocal_pitch;             // Vocal fundamental frequency in Hz (85Hz deep, 250Hz child-like)
    double expression_scale;        // Animation expression sensitivity multiplier (e.g. 1.5 amplified)
    double leadership_profile;       // Leadership divergence (0.0 supportive, 1.0 dominant)
    double maturity_index;           // Perceived age maturity (0.0 young, 1.0 mature/experienced)
    double persuasion_strategy;      // Persuasion strategy (0.0 sincerity/trust, 1.0 expertise/capability)
    double empathy_index;            // Perceived empathy and gentleness (0.0 low, 1.0 high/nurturing)
    double protection_index;         // Perceived physical vulnerability (0.0 independent, 1.0 vulnerable)
    double size_index;               // Perceived height/size (0.0 smaller/shorter, 1.0 larger/taller)
    double intellect_index;          // Perceived intellectual capability (0.0 lower, 1.0 higher)
    double resilience_index;         // Perceived physical resilience (0.0 lower, 1.0 higher)
    double agility_index;            // Perceived physical agility/speed (0.0 lower, 1.0 higher)
    double safety_index;             // Perceived safety (0.0 dangerous/threat, 1.0 safe/harmless)
    double purity_index;             // Perceived purity/cleanliness (0.0 contaminated, 1.0 pure/clean)
    double honesty_index;            // Perceived honesty/truthfulness (0.0 deceptive, 1.0 honest/sincere)
    double friendliness_index;       // Perceived friendliness/warmth (0.0 hostile, 1.0 friendly/approachable)
    double strength_index;           // Perceived physical strength (0.0 weaker, 1.0 stronger)
    double dominance_index;          // Perceived dominance/authority (0.0 submissive, 1.0 dominant)
    double naivety_index;            // Perceived naivety/gullibility (0.0 shrewd, 1.0 naive/gullible)
    double approachability_index;    // Perceived approachability (0.0 unapproachable, 1.0 approachable)
    double vocal_visual_mismatch;    // Vocal-visual cue incongruence (0.0 congruent, 1.0 high mismatch)
    double cooperative_negotiation;  // Cooperative negotiation style (0.0 competitive, 1.0 cooperative)
    double attractiveness_bias;      // Attractiveness interaction (0.0 babyface warmth, 1.0 mature competence)
    double head_body_ratio;          // Head-to-body proportion (0.0 mature small head, 1.0 babyface large head)
    double behavioral_expectation;   // Expected behavior profile (0.0 adult-like analytical, 1.0 child-like playful)
    double transgression_intent;     // Perceived transgression intent (0.0 accidental, 1.0 intentional/hostile)
    double affordance_profile;       // Ecological affordance (0.0 nurturing/protective, 1.0 threat avoidance)
    double social_status;            // Perceived social status (0.0 lower status/authority, 1.0 higher status)
    double head_tilt;                // Perceived head tilt (negative downward/aggressive, positive upward/submissive)
    double emotional_stability;      // Perceived emotional stability (0.0 unstable/unpredictable, 1.0 stable/predictable)
    double physical_vigor;           // Perceived physical vigor (0.0 fragile/weak, 1.0 robust/vigorous)
    double stress_coping;            // Perceived stress coping capability (0.0 low coping/dependent, 1.0 high coping)
    double parenting_capability;     // Perceived parenting capability (0.0 low expected warmth, 1.0 high warmth/cooperation)
    double naive_trust;              // Perceived naive trust (0.0 suspicious caution, 1.0 naive trust/openness)
    double social_submissiveness;    // Perceived submissiveness (0.0 mature dominance/assertiveness, 1.0 submissiveness)
    double relationship_commitment;  // Perceived commitment (0.0 low expected fidelity, 1.0 high commitment/fidelity)
    double health_robustness;        // Perceived health status (0.0 fragile health, 1.0 robust health/resistance)
    double vocal_visual_harmony;     // Vocal-visual harmony (0.0 low friendliness/competence bias, 1.0 high friendliness)
    double relationship_quality;     // Expected relationship quality (0.0 lower quality/cooperativeness, 1.0 higher)
    double immunological_strength;   // Perceived immunological strength (0.0 lower resistance, 1.0 robust resistance)
    double relationship_length;      // Perceived relationship length preference (0.0 short-term, 1.0 long-term)
    double physical_aggression;      // Perceived physical aggression (0.0 cooperative non-violent, 1.0 aggressive/violent)
    double task_diligence;           // Perceived task diligence (0.0 lower/emotional, 1.0 high systematic diligence)
    double social_supportiveness;    // Perceived social supportiveness (0.0 lower altruism, 1.0 high altruism/support)
    double relationship_permanence;  // Perceived relationship permanence (0.0 opportunistic dissolution, 1.0 stable)
    double defense_capability;       // Perceived defense capability (0.0 lower self-defense/vulnerable, 1.0 high self-defense)
    double social_extraversion;      // Perceived social extraversion (0.0 reticent/introverted, 1.0 extraverted)
    double hazard_resilience;        // Perceived hazard resilience (0.0 fragile/low trauma resilience, 1.0 highly resilient)
    double routine_leadership;       // Perceived routine leadership efficiency (0.0 crisis-only, 1.0 routine management)
    double parental_investment;      // Perceived parental investment duration (0.0 short-term, 1.0 long-term)
    double hazard_avoidance;         // Perceived hazard avoidance (0.0 risk-taking/hazard-seeking, 1.0 cautious/avoidant)
    double threat_confrontation;     // Perceived threat confrontation (0.0 submissive/retreat, 1.0 confrontational)
    double pain_tolerance;           // Perceived pain tolerance (0.0 lower tolerance/toughness, 1.0 high pain tolerance)
    double relationship_exclusivity; // Perceived relationship exclusivity (0.0 low/extra-pair, 1.0 monogamous/exclusive)
    double exhaustion_vulnerability; // Perceived exhaustion vulnerability (0.0 low/high stamina, 1.0 high fatigue)
    double offspring_survival;       // Perceived offspring survival probability (0.0 lower, 1.0 higher survival)
    double stress_expressiveness;    // Perceived stress expressiveness (0.0 analytical detachment, 1.0 emotional expressiveness)
    double parental_effort;          // Perceived parental effort (0.0 lower effort, 1.0 high parental effort/investment)
    double relationship_fidelity;    // Perceived relationship fidelity (0.0 low fidelity/promiscuous, 1.0 high fidelity)
    double motor_fatigue_resistance; // Perceived motor fatigue resistance (0.0 low resistance, 1.0 high fatigue resistance)
    double work_ethic;               // Perceived work ethic (0.0 passive/lazy, 1.0 high work ethic/active labor)
    double compliance;               // Perceived compliance (0.0 defiance/resistance, 1.0 naive compliance/obedience)
    double industriousness;          // Perceived industriousness (0.0 low/opportunism, 1.0 high industriousness)
    double gullibility;              // Perceived gullibility (0.0 shrewd skepticism, 1.0 naive gullibility)
    double stress_recovery;          // Perceived stress recovery (0.0 group-reliant/low recovery, 1.0 self-reliant/fast recovery)
    double work_output;              // Perceived work output during campaigns (0.0 low/avoidance, 1.0 high output)
    double reassurance_capability;   // Perceived reassurance capability (0.0 stoic/low comfort, 1.0 high comfort/warmth)
    double paternal_protection_style; // Perceived protection style (0.0 aggressive over-protection, 1.0 cooperative care)
    double competitive_persistence;   // Perceived competitive persistence (0.0 lower, 1.0 high competitive persistence)
    double family_altruism;          // Perceived family altruism (0.0 lower support, 1.0 high family support/altruism)
    double social_conformity;        // Perceived social conformity (0.0 non-conformity, 1.0 convention conformity)
    double status_aggression;        // Perceived status aggression (0.0 submissive, 1.0 aggressive retaliation)
    double workplace_mentorship;     // Perceived workplace mentorship (0.0 individual competitive, 1.0 patient mentoring)
    double verbal_arbitration;       // Perceived verbal arbitration capacity (0.0 emotional, 1.0 objective arbiter)
    double isolation_resilience;     // Perceived isolation resilience (0.0 lower, 1.0 high coping under isolation)
    double confrontational_assertiveness; // Perceived assertiveness (0.0 cooperative/submissive, 1.0 confrontational)
    double neighborhood_altruism;    // Perceived neighborhood altruism (0.0 lower support, 1.0 high neighborhood support/altruism)
    double rescue_compliance;        // Perceived rescue compliance (0.0 independent action, 1.0 high compliance)
    double mismatch_retaliation;     // Perceived retaliation on mismatch (0.0 tolerant, 1.0 hostile retaliation)
    double resource_conservation;    // Perceived resource conservation (0.0 wasteful, 1.0 high conservation)
    double spatial_orientation;      // Perceived spatial orientation capacity (0.0 disorientation-prone, 1.0 objective pathfinding)
    double defense_vigilance;        // Perceived defense vigilance (0.0 lower vigilance, 1.0 high protective action)
    double conflict_mediation;       // Perceived conflict mediation (0.0 escalatory, 1.0 cooperative de-escalation)
    double command_authority;        // Perceived command authority (0.0 collaborative, 1.0 high command authority)
    double partner_caregiving;       // Perceived caregiving support (0.0 lower support, 1.0 high caregiving support)
    double relationship_permanence_strategy; // Perceived permanence strategy (0.0 lower effort, 1.0 high partner retention)
} teddy_geometry_t;

// ACID Transaction container for evaluation stability.
typedef struct {
    teddy_geometry_t *target;
    teddy_geometry_t backup;
    bool active;
} evaluation_tx_t;

// Pixar RenderMan Avatar Agent definition upon the cooperative boundary
typedef struct {
    uint32_t sdk_state;          // SDK typestate transition tracker
    uint64_t dna_seed;           // FNV-1a DNA signature seed
    teddy_geometry_t geometry;   // Geometric and dynamic physics variables
    char usd_path[256];          // Target output USD asset path
} agent_avatar_t;

// Maps abstract traits to geometric parameters based on study results
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

// Evaluates an asymmetrical ordinal rating using a complementary log-log (cloglog) link model.
int evaluate_ordinal_cloglog_rating(const teddy_geometry_t *geom);

// Evaluates an ordinal rating using a probit link model.
int evaluate_ordinal_probit_rating(const teddy_geometry_t *geom);

// Evaluates an asymmetrical ordinal rating using a log-log link model.
int evaluate_ordinal_loglog_rating(const teddy_geometry_t *geom);

// Evaluates an ordinal rating using a Cauchy link model.
int evaluate_ordinal_cauchy_rating(const teddy_geometry_t *geom);

// Evaluates an ordinal rating using a log-gamma link model.
int evaluate_ordinal_loggamma_rating(const teddy_geometry_t *geom, double lambda);

// Evaluates an ordinal rating using a Gumbel link model.
int evaluate_ordinal_gumbel_rating(const teddy_geometry_t *geom);

// Evaluates an ordinal rating using a customized flexible link mixture weight (0.0 cloglog, 1.0 logit).
int evaluate_ordinal_flexible_rating(const teddy_geometry_t *geom, double link_mixture_weight);

// Evaluates an ordinal rating using a Cauchy and Gumbel mixture link model.
int evaluate_ordinal_cauchy_gumbel_mixture(const teddy_geometry_t *geom, double cauchy_weight);

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

// Evaluates individual threshold parameters stability via the Wald test.
bool evaluate_threshold_wald_test(double threshold_est, double baseline, double variance, double *wald_stat_out, double *p_value_out);

// Evaluates scale parameter variance stability via the Wald test.
bool evaluate_scale_wald_test(double scale_est, double baseline, double variance, double *wald_stat_out, double *p_value_out);

// Evaluates nominal effects on scale parameters via the Wald test statistic.
bool evaluate_scale_nominal_wald_test(const double *gamma_vector, const double *covariance_matrix, int df, double *wald_stat_out, double *p_value_out);

// Evaluates nominal effects on threshold parameters via the Wald test statistic.
bool evaluate_threshold_nominal_wald_test(const double *theta_vector, const double *covariance_matrix, int df, double *wald_stat_out, double *p_value_out);

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

// Evaluates the vocal-visual uncanny mismatch index based on geometry and pitch (Hyde et al.).
bool evaluate_uncanny_mismatch_index(const teddy_geometry_t *geom, double *uncanny_score_out);

// Evaluates the motion-mismatch uncanny valley response based on stiffness dynamics (Hyde et al.).
bool evaluate_motion_uncanny_index(const teddy_geometry_t *geom, double movement_stiffness, double *motion_uncanny_out);

// Evaluates the micro-expression jitter uncanny response (Hyde et al.).
bool evaluate_expression_jitter_uncanny(const teddy_geometry_t *geom, double jitter_frequency, double *uncanny_score_out);

// Evaluates the exposure duration habituation decay of the uncanny response (Hyde et al.).
bool evaluate_exposure_decay_uncanny(const teddy_geometry_t *geom, double exposure_duration_sec, double *decayed_uncanny_out);

// Evaluates the velocity jitter pacing uncanny response (Hyde et al.).
bool evaluate_velocity_jitter_uncanny(const teddy_geometry_t *geom, double velocity_variance, double *uncanny_score_out);

// Evaluates the unpredictable micro-expression freeze uncanny response (Hyde et al.).
bool evaluate_expression_freeze_uncanny(const teddy_geometry_t *geom, double freeze_duration_sec, double *uncanny_score_out);

// Executes a deferred H-bridge thunk under Gumbel flyback and emotional-scale adjusted Wald test bounds.
bool execute_hbridge_thunk_with_feedback(const teddy_geometry_t *geom, double switching_frequency, double (*thunk_fn)(void), double *safety_margin_out);

// Evaluates the unpredictable micro-expression asymmetry uncanny response (Hyde et al.).
bool evaluate_expression_asymmetry_uncanny(const teddy_geometry_t *geom, double asymmetry_deviation, double *uncanny_score_out);

// Evaluates the unpredictable micro-expression synchronization uncanny response (Hyde et al.).
bool evaluate_expression_sync_uncanny(const teddy_geometry_t *geom, double sync_delay_ms, double *uncanny_score_out);

// Simulates the low-level diode reflex and relative capacitor charge decay loop.
bool simulate_diode_capacitor_loop(double input_voltage, double resistance, double capacitance, double time_step, double *charge_state);

// Evaluates the H-bridge switching flyback transient mismatch for izotope filtering.
bool evaluate_hbridge_izotope_mismatch(const teddy_geometry_t *geom, double switching_frequency, double *flyback_mismatch_out);

// Simulates the snubber-clamped inductive flyback transient voltage decay.
bool simulate_snubber_clamped_flyback(double peak_voltage, double inductance, double snubber_resistance, double time_step, double *clamped_voltage_out);

// Simulates the RCD snubber resonant capacitor decay stage.
bool simulate_rcd_snubber_decay(double peak_voltage, double resistance, double capacitance, double inductance, double time_step, double *voltage_state);

// Commits an izotope H-bridge flyback transaction under ACID safety constraints.
bool commit_izotope_flyback_transaction(evaluation_tx_t *tx, double switching_frequency, double max_safe_voltage);

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

// Computes model selection diagnostics via AIC and BIC information criteria.
bool evaluate_information_criteria(const teddy_geometry_t *geom, int param_count, int sample_size, double *aic_out, double *bic_out);

evaluation_tx_t begin_evaluation_transaction(teddy_geometry_t *target);
bool commit_evaluation_transaction(evaluation_tx_t *tx);
void rollback_evaluation_transaction(evaluation_tx_t *tx);

// ACID Transaction container for end-to-end avatar state.
typedef struct {
    agent_avatar_t *target;
    agent_avatar_t backup;
    bool active;
} avatar_tx_t;

avatar_tx_t begin_avatar_transaction(agent_avatar_t *avatar);
bool commit_avatar_transaction(avatar_tx_t *tx, const char *bin_filepath);
void rollback_avatar_transaction(avatar_tx_t *tx);

// Engages systems via the SDK typestate and calculates parameters.
bool engage_system_boundary(agent_avatar_t *avatar, teddy_personality_t personality);

// Authorizes the system boundary via GOST-encrypted state payloads when USD is not linked.
bool authorize_boundary_via_gost(agent_avatar_t *avatar, const uint32_t *key_8words, uint32_t token_left, uint32_t token_right);

// Validates the SDK typestate sequence
bool validate_sdk_typestate(const agent_avatar_t *avatar);

// Synchronizes vertex displacement math scales with WinchesterMQ register boundary constraints
void execute_displacement_shader_sync(const agent_avatar_t *avatar, double *vertex_offsets, uint32_t count);

// Emulates low-level WinchesterMQ SCSI register boundary handshake loop
bool emulate_winchester_scsi_loop(agent_avatar_t *avatar);

// Simulates Verlet soft-body dynamics of FET discharge cycles
void simulate_fet_verlet_discharge(agent_avatar_t *avatar, double time_step);

#endif // AUNCIENT_TEDDY_PERSONALITY_H
