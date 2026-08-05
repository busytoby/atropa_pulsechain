#ifndef TSFI_PERSONALITY_LINK_H
#define TSFI_PERSONALITY_LINK_H

#include "tsfi_personality_core.h"

// Evaluates an ordinal perception rating (1 to 7) using a cumulative link model.
int evaluate_ordinal_link_rating(const teddy_geometry_t *geom);

// Evaluates profile-likelihood thresholds bounds for category validation.
bool evaluate_profile_likelihood_bounds(const teddy_geometry_t *geom, int category, double *lower_bound, double *upper_bound);

// Evaluates a scale-adjusted ordinal rating using a heteroscedastic cumulative link model.
int evaluate_ordinal_scale_rating(const teddy_geometry_t *geom, double scale_multiplier);

// Evaluates an ordinal rating with nominal effects violating parallel slopes.
int evaluate_ordinal_nominal_rating(const teddy_geometry_t *geom);

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

// Evaluates the flexible mixture link function mapping ordinal boundaries (Christensen).
bool evaluate_ordinal_flexible_mixture_link(const teddy_geometry_t *geom, double mixture_weight, int *rating_out);

// Evaluates the mixture link nominal-adjusted threshold bounds (Christensen).
bool evaluate_ordinal_mixture_nominal_thresholds(const teddy_geometry_t *geom, double mixture_weight, const double *nominal_covariates, double *thresholds_out);

#endif // TSFI_PERSONALITY_LINK_H
