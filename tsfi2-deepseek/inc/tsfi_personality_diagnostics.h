#ifndef TSFI_PERSONALITY_DIAGNOSTICS_H
#define TSFI_PERSONALITY_DIAGNOSTICS_H

#include "tsfi_personality_core.h"

// Evaluates model fitting stability via Hessian standard error diagnostics.
bool evaluate_hessian_diagnostics(const teddy_bear_geometry_t *geom, double *variance_out);

// Evaluates the full parameter covariance matrix derived from the Hessian inverse.
bool evaluate_parameter_covariance(const teddy_bear_geometry_t *geom, double *covariance_matrix_out);

// Evaluates the mixture link expectation standard error (Christensen).
bool evaluate_ordinal_mixture_expectation_se(const double *probabilities, const double *covariance_matrix, double mixture_weight, int count, double *se_out);

// Diagnoses whether ordinal thresholds fit equidistant constraints.
bool evaluate_threshold_equidistancy(const teddy_bear_geometry_t *geom, double tolerance, double *spacing_error);

// Evaluates profile log-likelihood bounds for scale parameter estimation.
bool evaluate_scale_profile_bounds(const teddy_bear_geometry_t *geom, double *lower_scale_bound, double *upper_scale_bound);

// Evaluates structured covariate scale parameters under custom link models.
bool evaluate_scale_structured_covariates(const teddy_bear_geometry_t *geom, double age_covariate, double *scale_out);

// Evaluates nominal effects violating parallel scale parameter bounds.
bool evaluate_scale_nominal_effects(const teddy_bear_geometry_t *geom, const double *nominal_covariates, int df, double *effects_out);

// Evaluates profile log-likelihood bounds for a specific threshold parameter.
bool evaluate_threshold_profile_bounds(const teddy_bear_geometry_t *geom, int threshold_index, double *lower_bound, double *upper_bound);

// Evaluates nested models via Likelihood Ratio Test (LRT) diagnostics.
bool evaluate_lrt_nested_models(double null_loglik, double alt_loglik, int df_diff, double *chi_sq_out, double *p_value_out);

// Computes category predicted probability and confidence interval bounds.
bool evaluate_predicted_probability_bounds(const teddy_bear_geometry_t *geom, int category, double *prob_out, double *lower_prob_bound, double *upper_prob_bound);

// Evaluates nominal effect parameter violations via the Wald test statistic.
bool evaluate_wald_nominal_test(const double *beta_vector, const double *covariance_matrix, int df, double *wald_stat_out, double *p_value_out);

// Calculates surrogate residuals for model specification diagnostics.
bool evaluate_surrogate_residuals(const teddy_bear_geometry_t *geom, int observed_rating, double *residual_out);

// Calculates surrogate residuals for nominal scale ordinal regression models.
bool evaluate_nominal_surrogate_residuals(const teddy_bear_geometry_t *geom, int observed_rating, double *residual_out);

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

// Executes a Wald-gated cooperative validation check thunk (Christensen).
bool execute_cooperative_wald_gated_thunk(const teddy_bear_geometry_t *geom, const double *beta_vector, const double *covariance_matrix, int df, double (*thunk_fn)(void), double *result_out);

// Computes model selection diagnostics via AIC and BIC information criteria.
bool evaluate_information_criteria(const teddy_bear_geometry_t *geom, int param_count, int sample_size, double *aic_out, double *bic_out);

#endif // TSFI_PERSONALITY_DIAGNOSTICS_H
