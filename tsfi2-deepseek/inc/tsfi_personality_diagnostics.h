#ifndef TSFI_PERSONALITY_DIAGNOSTICS_H
#define TSFI_PERSONALITY_DIAGNOSTICS_H

#include "tsfi_personality_core.h"

// Evaluates model fitting stability via Hessian standard error diagnostics.
bool evaluate_hessian_diagnostics(const teddy_geometry_t *geom, double *variance_out);

// Evaluates the full parameter covariance matrix derived from the Hessian inverse.
bool evaluate_parameter_covariance(const teddy_geometry_t *geom, double *covariance_matrix_out);

// Evaluates the equidistancy of thresholds for parallel link checks.
bool evaluate_threshold_equidistancy(const teddy_geometry_t *geom, double tolerance, double *spacing_error);

// Evaluates profile scale bounds for structured model stability check.
bool evaluate_scale_profile_bounds(const teddy_geometry_t *geom, double *lower_scale_bound, double *upper_scale_bound);

// Evaluates structured scale covariates (Christensen).
bool evaluate_scale_structured_covariates(const teddy_geometry_t *geom, double age_covariate, double *scale_out);

// Evaluates nominal scale effects for unstructured link variance.
bool evaluate_scale_nominal_effects(const teddy_geometry_t *geom, const double *nominal_covariates, int df, double *effects_out);

// Evaluates profile threshold bounds for categories.
bool evaluate_threshold_profile_bounds(const teddy_geometry_t *geom, int threshold_index, double *lower_bound, double *upper_bound);

// Performs a Likelihood Ratio Test comparing nested models.
bool evaluate_lrt_nested_models(double null_loglik, double alt_loglik, int df_diff, double *chi_sq_out, double *p_value_out);

// Evaluates the predicted probability bounds for categories.
bool evaluate_predicted_probability_bounds(const teddy_geometry_t *geom, int category, double *prob_out, double *lower_prob_bound, double *upper_prob_bound);

// Performs a Wald test on nominal effects vector.
bool evaluate_wald_nominal_test(const double *beta_vector, const double *covariance_matrix, int df, double *wald_stat_out, double *p_value_out);

// Computes surrogate residuals for validation.
bool evaluate_surrogate_residuals(const teddy_geometry_t *geom, int observed_rating, double *residual_out);

// Computes nominal-adjusted surrogate residuals.
bool evaluate_nominal_surrogate_residuals(const teddy_geometry_t *geom, int observed_rating, double *residual_out);

// Performs a Wald test on single threshold parameters.
bool evaluate_threshold_wald_test(double threshold_est, double baseline, double variance, double *wald_stat_out, double *p_value_out);

// Performs a Wald test on single scale parameters.
bool evaluate_scale_wald_test(double scale_est, double baseline, double variance, double *wald_stat_out, double *p_value_out);

// Performs a nominal scale Wald test.
bool evaluate_scale_nominal_wald_test(const double *gamma_vector, const double *covariance_matrix, int df, double *wald_stat_out, double *p_value_out);

// Performs a nominal threshold Wald test.
bool evaluate_threshold_nominal_wald_test(const double *theta_vector, const double *covariance_matrix, int df, double *wald_stat_out, double *p_value_out);

// Performs a mixture link threshold Wald test.
bool evaluate_ordinal_mixture_threshold_wald_test(const double *theta_vector, const double *covariance_matrix, double mixture_weight, int df, double *wald_stat_out, double *p_value_out);

// Performs a scale-adjusted threshold Wald test.
bool evaluate_scale_adjusted_threshold_wald(double threshold_est, double scale_multiplier, double baseline, double variance, double *wald_stat_out, double *p_value_out);

// Performs a mixture link nominal Wald test.
bool evaluate_mixture_link_nominal_wald(const double *beta_vector, const double *covariance_matrix, int df, double *wald_stat_out, double *p_value_out);

// Performs a general Wald test on any single model parameter.
bool evaluate_parameter_wald_test(double estimate, double baseline, double variance, double *wald_stat_out, double *p_value_out);

// Computes model selection diagnostics via AIC and BIC information criteria.
bool evaluate_information_criteria(const teddy_geometry_t *geom, int param_count, int sample_size, double *aic_out, double *bic_out);

#endif // TSFI_PERSONALITY_DIAGNOSTICS_H
