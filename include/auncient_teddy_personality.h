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

// Evaluates dynamic changes in vocal intonation range alignment with eyebrow movements (Hyde et al.).
bool evaluate_hyde_dynamic_intonation(const teddy_geometry_t *geom, double intonation_range, double brow_movement, double *intonation_out);

// Models the decay rate of intonation amplitude ranges over prolonged interaction durations (Hyde et al.).
bool evaluate_hyde_intonation_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_intonation_out);

// Evaluates conversational speech tempo synchronization scaling on interaction ratings (Hyde et al.).
bool evaluate_hyde_tempo_sync(const teddy_geometry_t *geom, double tempo_sync_val, double *rating_out);

// Models vocal tempo variations decay over dynamic interaction durations (Hyde et al.).
bool evaluate_hyde_tempo_jitter_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_jitter_out);

// Evaluates conversational vocal tempo range alignment with eyebrow movements (Hyde et al.).
bool evaluate_hyde_tempo_range(const teddy_geometry_t *geom, double tempo_range_val, double brow_movement, double *tempo_range_out);

// Models the decay rate of vocal tempo variation range over prolonged interaction durations (Hyde et al.).
bool evaluate_hyde_tempo_range_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_range_out);

// Evaluates conversational speech tempo synchronization alignment with vocal size mismatch indicators (Hyde et al.).
bool evaluate_hyde_vocal_size_sync(const teddy_geometry_t *geom, double tempo_sync_val, double size_mismatch, double *sync_out);

// Models the decay rate of vocal size variation range over prolonged interaction durations (Hyde et al.).
bool evaluate_hyde_vocal_size_jitter_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_size_jitter_out);

// Evaluates conversational vocal size variance alignment with pitch range indicators (Hyde et al.).
bool evaluate_hyde_vocal_size_pitch(const teddy_geometry_t *geom, double size_variance, double pitch_range, double *pitch_out);

// Models the decay rate of vocal size pitch variation range over prolonged interaction durations (Hyde et al.).
bool evaluate_hyde_vocal_size_pitch_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_size_pitch_out);

// Evaluates conversational vocal warmth ratings based on pitch range variance and vocal size mismatch values (Hyde et al.).
bool evaluate_hyde_vocal_warmth_range(const teddy_geometry_t *geom, double pitch_range, double size_mismatch, double *warmth_range_out);

// Models the decay rate of vocal warmth range variation over dynamic interaction durations (Hyde et al.).
bool evaluate_hyde_vocal_warmth_range_decay(const teddy_geometry_t *geom, double duration_sec, double *decayed_warmth_range_out);

// Evaluates vocal warmth alignment with visual chin curvature adjustments (Hyde et al.).
bool evaluate_hyde_vocal_warmth_modulation(const teddy_geometry_t *geom, double pitch_hz, double chin_curvature, double *warmth_out);

// Models interaction recovery perception based on the timing duration of conversational resumption after collision events (Hyde et al.).
bool evaluate_hyde_interruption_recovery(const teddy_geometry_t *geom, double recovery_time_sec, double *recovery_rating_out);

// Evaluates perceived social dominance based on dynamic gaze shifts (Keating, C. F.).
bool evaluate_keating_gaze_shift_dominance(const teddy_geometry_t *geom, double shift_frequency, double *dominance_out);

// Models perceived trustworthiness based on lip compression and mouth surface area changes (Keating, C. F.).
bool evaluate_keating_lip_compression_trust(const teddy_geometry_t *geom, double compression_ratio, double *trustworthiness_out);

// Evaluates perceived social dominance based on facial width asymmetries (Keating, C. F.).
bool evaluate_keating_width_asymmetry_dominance(const teddy_geometry_t *geom, double width_asymmetry_val, double *dominance_out);

// Models trustworthiness decay rates based on mouth asymmetry values (Keating, C. F.).
bool evaluate_keating_mouth_asymmetry_decay(const teddy_geometry_t *geom, double mouth_asymmetry_val, double *trust_decay_out);

// Evaluates perceived trustworthiness based on facial width asymmetries (Keating, C. F.).
bool evaluate_keating_width_asymmetry_trust(const teddy_geometry_t *geom, double width_asymmetry_val, double *trustworthiness_out);

// Models perceived social dominance based on mouth asymmetries (Keating, C. F.).
bool evaluate_keating_mouth_asymmetry_dominance(const teddy_geometry_t *geom, double mouth_asymmetry_val, double *dominance_out);

// Evaluates perceived social dominance based on eyebrow asymmetries (Keating, C. F.).
bool evaluate_keating_eyebrow_asymmetry_dominance(const teddy_geometry_t *geom, double eyebrow_asymmetry_val, double *dominance_out);

// Models submissiveness ratings based on exposed sclera size ratios (Keating, C. F.).
bool evaluate_keating_sclera_submissiveness(const teddy_geometry_t *geom, double sclera_size_ratio, double *submissiveness_out);

// Evaluates perceived trustworthiness based on eyebrow asymmetries (Keating, C. F.).
bool evaluate_keating_eyebrow_asymmetry_trust(const teddy_geometry_t *geom, double eyebrow_asymmetry_val, double *trustworthiness_out);

// Models perceived social dominance based on exposed sclera size ratios (Keating, C. F.).
bool evaluate_keating_sclera_dominance(const teddy_geometry_t *geom, double sclera_size_ratio, double *dominance_out);

// Models the decay rate of trustworthiness ratings based on prolonged eyebrow gesture asymmetry durations (Keating, C. F.).
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

// Models the variance in dominance decay rates based on prolonged eyebrow asymmetry duration variations (Keating, C. F.).
bool evaluate_keating_eyebrow_asymmetry_dominance_decay_variance(const teddy_geometry_t *geom, double duration_sec, double *decay_variance_out);

// Models the variance in trustworthiness ratings based on visual mouth asymmetry ratio variations (Keating, C. F.).
bool evaluate_keating_mouth_asymmetry_trust_variance(const teddy_geometry_t *geom, double mouth_asymmetry_val, double *trust_variance_out);

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

// Executes a maturity-adjusted cloglog gated thunk.
bool execute_cloglog_gated_thunk_with_maturity(const teddy_geometry_t *geom, double scale_covariate, double age_months, double (*thunk_fn)(void), double *result_out);

// Executes a Wald-gated cooperative validation check thunk (Christensen).
bool execute_cooperative_wald_gated_thunk(const teddy_geometry_t *geom, const double *beta_vector, const double *covariance_matrix, int df, double (*thunk_fn)(void), double *result_out);

// Simulates the phase-modulated flyback noise distortion index for izotope filtering.
bool simulate_phase_flyback_noise(const teddy_geometry_t *geom, double phase_angle, double *noise_out);

// Evaluates symmetric threshold (tremolo) and group scale (sustain) constraints in the izotope system.
bool evaluate_izotope_constrained_parameters(const teddy_geometry_t *geom, int group_id, double *tremolo_spacing_out, double *sustain_decay_out);

// Evaluates the izotope/ozone-adjusted habituation decay logic (Christensen).
bool evaluate_izotope_ozone_habituation_decay(const teddy_geometry_t *geom, double base_decay, double *decayed_out);

// Simulates cloglog-modulated Verlet physics (Christensen).
bool simulate_cloglog_verlet_physics(const teddy_geometry_t *geom, double scale_covariate, double current_pos, double prev_pos, double time_step, double *next_pos_out);

// Computes model selection diagnostics via AIC and BIC information criteria.
bool evaluate_information_criteria(const teddy_geometry_t *geom, int param_count, int sample_size, double *aic_out, double *bic_out);

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
