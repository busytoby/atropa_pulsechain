#include "auncient_teddy_bear_personality.h"
#include <math.h>

int evaluate_ordinal_link_rating(const teddy_bear_geometry_t *geom) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    for (int i = 0; i < 6; ++i) {
        double logit_prob = 1.0 / (1.0 + exp(-(thresholds[i] - latent)));
        if (logit_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

bool evaluate_profile_likelihood_bounds(const teddy_bear_geometry_t *geom, int category, double *lower_bound, double *upper_bound) {
    if (!geom || category < 1 || category > 7 || !lower_bound || !upper_bound) {
        return false;
    }
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    double t_val = (category == 7) ? thresholds[5] : thresholds[category - 1];
    double se = 0.25;
    double z = 1.96;
    *lower_bound = t_val - (z * se) + (latent * 0.05);
    *upper_bound = t_val + (z * se) + (latent * 0.05);
    return true;
}

int evaluate_ordinal_scale_rating(const teddy_bear_geometry_t *geom, double scale_multiplier) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double scale = exp(geom->vocal_visual_mismatch * scale_multiplier);
    if (scale < 0.01) scale = 0.01;
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    for (int i = 0; i < 6; ++i) {
        double logit_prob = 1.0 / (1.0 + exp(-((thresholds[i] - latent) / scale)));
        if (logit_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

int evaluate_ordinal_nominal_rating(const teddy_bear_geometry_t *geom) {
    if (!geom) return 1;
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    double nominal_coefficients[6] = {0.1, 0.3, 0.6, 1.0, 1.5, 2.0};
    double latent_base = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    for (int i = 0; i < 6; ++i) {
        double latent = latent_base + (geom->symmetry * nominal_coefficients[i]);
        double logit_prob = 1.0 / (1.0 + exp(-(thresholds[i] - latent)));
        if (logit_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

bool evaluate_hessian_diagnostics(const teddy_bear_geometry_t *geom, double *variance_out) {
    if (!geom || !variance_out) {
        return false;
    }
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double info = 16.0 - (latent * 0.5);
    if (info < 0.1) info = 0.1;
    *variance_out = 1.0 / info;
    return true;
}

bool evaluate_parameter_covariance(const teddy_bear_geometry_t *geom, double *covariance_matrix_out) {
    if (!geom || !covariance_matrix_out) {
        return false;
    }
    double base_variance = 0.0;
    if (!evaluate_hessian_diagnostics(geom, &base_variance)) {
        return false;
    }
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 6; ++j) {
            if (i == j) {
                covariance_matrix_out[i * 6 + j] = base_variance * (1.0 + i * 0.1);
            } else {
                covariance_matrix_out[i * 6 + j] = base_variance * 0.05;
            }
        }
    }
    return true;
}

int evaluate_ordinal_cloglog_rating(const teddy_bear_geometry_t *geom) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    for (int i = 0; i < 6; ++i) {
        double cloglog_prob = 1.0 - exp(-exp(thresholds[i] - latent));
        if (cloglog_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

int evaluate_ordinal_cloglog_scale_rating(const teddy_bear_geometry_t *geom, double scale_covariate) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double scale = exp(scale_covariate * 0.5);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    for (int i = 0; i < 6; ++i) {
        double cloglog_prob = 1.0 - exp(-exp((thresholds[i] - latent) / scale));
        if (cloglog_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

int evaluate_ordinal_probit_rating(const teddy_bear_geometry_t *geom) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    for (int i = 0; i < 6; ++i) {
        double diff = thresholds[i] - latent;
        double probit_prob = 0.5 * (1.0 + erf(diff / sqrt(2.0)));
        if (probit_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

int evaluate_ordinal_loglog_rating(const teddy_bear_geometry_t *geom) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    for (int i = 0; i < 6; ++i) {
        double loglog_prob = exp(-exp(-(thresholds[i] - latent)));
        if (loglog_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

int evaluate_ordinal_cauchy_rating(const teddy_bear_geometry_t *geom) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    for (int i = 0; i < 6; ++i) {
        double cauchy_prob = 0.5 + atan(thresholds[i] - latent) / 3.141592653589793;
        if (cauchy_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

int evaluate_ordinal_loggamma_rating(const teddy_bear_geometry_t *geom, double lambda) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    for (int i = 0; i < 6; ++i) {
        double diff = thresholds[i] - latent;
        double val = exp(diff);
        double loggamma_prob = 1.0 - exp(-pow(val, lambda));
        if (lambda == 0.0) {
            loggamma_prob = 1.0 / (1.0 + exp(-diff));
        }
        if (loggamma_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

int evaluate_ordinal_gumbel_rating(const teddy_bear_geometry_t *geom) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    for (int i = 0; i < 6; ++i) {
        double gumbel_prob = 1.0 - exp(-exp(thresholds[i] - latent));
        if (gumbel_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

int evaluate_ordinal_gumbel_scale_rating(const teddy_bear_geometry_t *geom, double scale_covariate) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double scale = exp(scale_covariate * 0.5);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    for (int i = 0; i < 6; ++i) {
        double gumbel_prob = 1.0 - exp(-exp((thresholds[i] - latent) / scale));
        if (gumbel_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

bool evaluate_ordinal_link_probability(double latent_val, double threshold_val, int link_type, double *probability_out) {
    if (!probability_out) {
        return false;
    }
    double diff = threshold_val - latent_val;
    switch (link_type) {
        case 0: // Logit
            *probability_out = 1.0 / (1.0 + exp(-diff));
            break;
        case 1: // Probit
            *probability_out = 0.5 * (1.0 + erf(diff / sqrt(2.0)));
            break;
        case 2: // Cloglog
            *probability_out = 1.0 - exp(-exp(diff));
            break;
        case 3: // Loglog
            *probability_out = exp(-exp(-diff));
            break;
        case 4: // Gumbel
            *probability_out = 1.0 - exp(-exp(diff));
            break;
        default:
            return false;
    }
    return true;
}

bool evaluate_ordinal_link_expectation(const double *probabilities, int count, double *expectation_out) {
    if (!probabilities || count < 1 || !expectation_out) {
        return false;
    }
    double sum = 0.0;
    for (int i = 0; i < count; ++i) {
        sum += probabilities[i] * (i + 1);
    }
    *expectation_out = sum;
    return true;
}

bool evaluate_ordinal_link_expectation_se(const double *probabilities, const double *covariance_matrix, int count, double *se_out) {
    if (!probabilities || !covariance_matrix || count < 1 || !se_out) {
        return false;
    }
    double variance_sum = 0.0;
    for (int i = 0; i < count; ++i) {
        double g_i = (double)(i + 1);
        for (int j = 0; j < count; ++j) {
            double g_j = (double)(j + 1);
            variance_sum += g_i * covariance_matrix[i * count + j] * g_j;
        }
    }
    if (variance_sum < 0.0) variance_sum = 0.0;
    *se_out = sqrt(variance_sum);
    return true;
}

bool evaluate_ordinal_mixture_expectation_se(const double *probabilities, const double *covariance_matrix, double mixture_weight, int count, double *se_out) {
    if (!probabilities || !covariance_matrix || count < 1 || !se_out) {
        return false;
    }
    double base_se = 0.0;
    if (!evaluate_ordinal_link_expectation_se(probabilities, covariance_matrix, count, &base_se)) {
        return false;
    }
    *se_out = base_se * (1.0 + (mixture_weight * 0.1));
    return true;
}

bool evaluate_ordinal_link_loglik(const teddy_bear_geometry_t *geom, const int *observed_ratings, int count, double *loglik_out) {
    if (!geom || !observed_ratings || count < 1 || !loglik_out) {
        return false;
    }
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[8] = {-10.0, 0.5, 1.2, 2.0, 2.8, 3.5, 4.2, 10.0};
    double loglik = 0.0;
    for (int i = 0; i < count; ++i) {
        int r = observed_ratings[i];
        if (r < 1 || r > 7) return false;
        double p_upper = 1.0 / (1.0 + exp(-(thresholds[r] - latent)));
        double p_lower = 1.0 / (1.0 + exp(-(thresholds[r - 1] - latent)));
        double prob = p_upper - p_lower;
        if (prob < 1e-9) prob = 1e-9;
        loglik += log(prob);
    }
    *loglik_out = loglik;
    return true;
}

int evaluate_ordinal_flexible_rating(const teddy_bear_geometry_t *geom, double link_mixture_weight) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    double w = link_mixture_weight;
    if (w < 0.0) w = 0.0;
    if (w > 1.0) w = 1.0;
    for (int i = 0; i < 6; ++i) {
        double logit_prob = 1.0 / (1.0 + exp(-(thresholds[i] - latent)));
        double cloglog_prob = 1.0 - exp(-exp(thresholds[i] - latent));
        double mix_prob = (w * logit_prob) + ((1.0 - w) * cloglog_prob);
        if (mix_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

int evaluate_ordinal_cauchy_gumbel_mixture(const teddy_bear_geometry_t *geom, double cauchy_weight) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    double w = cauchy_weight;
    if (w < 0.0) w = 0.0;
    if (w > 1.0) w = 1.0;
    for (int i = 0; i < 6; ++i) {
        double cauchy_prob = atan(thresholds[i] - latent) / 3.141592653589793 + 0.5;
        double gumbel_prob = exp(-exp(-(thresholds[i] - latent)));
        double mix_prob = (w * cauchy_prob) + ((1.0 - w) * gumbel_prob);
        if (mix_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

bool evaluate_threshold_equidistancy(const teddy_bear_geometry_t *geom, double tolerance, double *spacing_error) {
    if (!geom || !spacing_error) {
        return false;
    }
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    double total_delta = 0.0;
    for (int i = 1; i < 6; ++i) {
        total_delta += (thresholds[i] - thresholds[i - 1]);
    }
    double mean_delta = total_delta / 5.0;
    double sum_sq_err = 0.0;
    for (int i = 0; i < 6; ++i) {
        double expected = thresholds[0] + (double)i * mean_delta;
        double err = thresholds[i] - expected;
        sum_sq_err += err * err;
    }
    *spacing_error = sum_sq_err + (geom->vocal_visual_mismatch * 0.02);
    return (*spacing_error <= tolerance);
}

bool evaluate_scale_profile_bounds(const teddy_bear_geometry_t *geom, double *lower_scale_bound, double *upper_scale_bound) {
    if (!geom || !lower_scale_bound || !upper_scale_bound) {
        return false;
    }
    double scale = exp(geom->vocal_visual_mismatch * 0.5);
    double z = 1.96;
    double se = 0.15;
    *lower_scale_bound = scale - (z * se);
    *upper_scale_bound = scale + (z * se);
    if (*lower_scale_bound < 0.01) *lower_scale_bound = 0.01;
    return true;
}

bool evaluate_scale_structured_covariates(const teddy_bear_geometry_t *geom, double age_covariate, double *scale_out) {
    if (!geom || !scale_out) {
        return false;
    }
    double gamma_1 = 0.4;
    double gamma_2 = 0.15;
    double log_scale = (gamma_1 * geom->vocal_visual_mismatch) + (gamma_2 * age_covariate);
    *scale_out = exp(log_scale);
    return true;
}

bool evaluate_scale_nominal_effects(const teddy_bear_geometry_t *geom, const double *nominal_covariates, int df, double *effects_out) {
    if (!geom || !nominal_covariates || df < 1 || !effects_out) {
        return false;
    }
    double scale = 1.0;
    evaluate_scale_structured_covariates(geom, geom->maturity_index, &scale);
    for (int i = 0; i < df; ++i) {
        effects_out[i] = nominal_covariates[i] * scale * (1.0 + geom->behavioral_mismatch);
    }
    return true;
}

bool evaluate_threshold_profile_bounds(const teddy_bear_geometry_t *geom, int threshold_index, double *lower_bound, double *upper_bound) {
    if (!geom || threshold_index < 0 || threshold_index >= 6 || !lower_bound || !upper_bound) {
        return false;
    }
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    double est = thresholds[threshold_index];
    double se = 0.12 * (1.0 + geom->behavioral_mismatch * 0.5);
    double z = 1.96;
    *lower_bound = est - (z * se);
    *upper_bound = est + (z * se);
    return true;
}

bool evaluate_lrt_nested_models(double null_loglik, double alt_loglik, int df_diff, double *chi_sq_out, double *p_value_out) {
    if (df_diff < 1 || !chi_sq_out || !p_value_out) {
        return false;
    }
    *chi_sq_out = -2.0 * (null_loglik - alt_loglik);
    if (*chi_sq_out < 0.0) *chi_sq_out = 0.0;
    *p_value_out = exp(-(*chi_sq_out) / 2.0);
    if (*p_value_out > 1.0) *p_value_out = 1.0;
    return true;
}

bool evaluate_predicted_probability_bounds(const teddy_bear_geometry_t *geom, int category, double *prob_out, double *lower_prob_bound, double *upper_prob_bound) {
    if (!geom || category < 1 || category > 7 || !prob_out || !lower_prob_bound || !upper_prob_bound) {
        return false;
    }
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[8] = {-10.0, 0.5, 1.2, 2.0, 2.8, 3.5, 4.2, 10.0};
    double p_upper = 1.0 / (1.0 + exp(-(thresholds[category] - latent)));
    double p_lower = 1.0 / (1.0 + exp(-(thresholds[category - 1] - latent)));
    *prob_out = p_upper - p_lower;
    if (*prob_out < 0.0) *prob_out = 0.0;
    double se = 0.05;
    double z = 1.96;
    *lower_prob_bound = *prob_out - (z * se);
    *upper_prob_bound = *prob_out + (z * se);
    if (*lower_prob_bound < 0.0) *lower_prob_bound = 0.0;
    if (*upper_prob_bound > 1.0) *upper_prob_bound = 1.0;
    return true;
}

bool evaluate_wald_nominal_test(const double *beta_vector, const double *covariance_matrix, int df, double *wald_stat_out, double *p_value_out) {
    if (!beta_vector || !covariance_matrix || df < 1 || !wald_stat_out || !p_value_out) {
        return false;
    }
    double sum_w = 0.0;
    for (int i = 0; i < df; ++i) {
        double var = covariance_matrix[i * df + i];
        if (var < 1e-9) var = 1e-9;
        sum_w += (beta_vector[i] * beta_vector[i]) / var;
    }
    *wald_stat_out = sum_w;
    *p_value_out = exp(-(*wald_stat_out) / 2.0);
    if (*p_value_out > 1.0) *p_value_out = 1.0;
    return true;
}

bool evaluate_surrogate_residuals(const teddy_bear_geometry_t *geom, int observed_rating, double *residual_out) {
    if (!geom || observed_rating < 1 || observed_rating > 7 || !residual_out) {
        return false;
    }
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[8] = {-10.0, 0.5, 1.2, 2.0, 2.8, 3.5, 4.2, 10.0};
    double mid = (thresholds[observed_rating] + thresholds[observed_rating - 1]) / 2.0;
    *residual_out = mid - latent;
    return true;
}

bool evaluate_nominal_surrogate_residuals(const teddy_bear_geometry_t *geom, int observed_rating, double *residual_out) {
    if (!geom || observed_rating < 1 || observed_rating > 7 || !residual_out) {
        return false;
    }
    double scale = 1.0;
    evaluate_scale_structured_covariates(geom, geom->maturity_index, &scale);
    double latent = ((geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0)) / scale;
    double thresholds[8] = {-10.0, 0.5, 1.2, 2.0, 2.8, 3.5, 4.2, 10.0};
    double mid = (thresholds[observed_rating] + thresholds[observed_rating - 1]) / 2.0;
    *residual_out = mid - latent;
    return true;
}

bool evaluate_threshold_wald_test(double threshold_est, double baseline, double variance, double *wald_stat_out, double *p_value_out) {
    if (variance < 1e-9 || !wald_stat_out || !p_value_out) {
        return false;
    }
    double diff = threshold_est - baseline;
    *wald_stat_out = (diff * diff) / variance;
    *p_value_out = exp(-(*wald_stat_out) / 2.0);
    if (*p_value_out > 1.0) *p_value_out = 1.0;
    return true;
}

bool evaluate_scale_wald_test(double scale_est, double baseline, double variance, double *wald_stat_out, double *p_value_out) {
    if (variance < 1e-9 || !wald_stat_out || !p_value_out) {
        return false;
    }
    double diff = scale_est - baseline;
    *wald_stat_out = (diff * diff) / variance;
    *p_value_out = exp(-(*wald_stat_out) / 2.0);
    if (*p_value_out > 1.0) *p_value_out = 1.0;
    return true;
}

bool evaluate_scale_nominal_wald_test(const double *gamma_vector, const double *covariance_matrix, int df, double *wald_stat_out, double *p_value_out) {
    if (!gamma_vector || !covariance_matrix || df < 1 || !wald_stat_out || !p_value_out) {
        return false;
    }
    double sum_w = 0.0;
    for (int i = 0; i < df; ++i) {
        double var = covariance_matrix[i * df + i];
        if (var < 1e-9) var = 1e-9;
        sum_w += (gamma_vector[i] * gamma_vector[i]) / var;
    }
    *wald_stat_out = sum_w;
    *p_value_out = exp(-(*wald_stat_out) / 2.0);
    if (*p_value_out > 1.0) *p_value_out = 1.0;
    return true;
}

bool evaluate_threshold_nominal_wald_test(const double *theta_vector, const double *covariance_matrix, int df, double *wald_stat_out, double *p_value_out) {
    if (!theta_vector || !covariance_matrix || df < 1 || !wald_stat_out || !p_value_out) {
        return false;
    }
    double sum_w = 0.0;
    for (int i = 0; i < df; ++i) {
        double var = covariance_matrix[i * df + i];
        if (var < 1e-9) var = 1e-9;
        sum_w += (theta_vector[i] * theta_vector[i]) / var;
    }
    *wald_stat_out = sum_w;
    *p_value_out = exp(-(*wald_stat_out) / 2.0);
    if (*p_value_out > 1.0) *p_value_out = 1.0;
    return true;
}

bool evaluate_ordinal_mixture_threshold_wald_test(const double *theta_vector, const double *covariance_matrix, double mixture_weight, int df, double *wald_stat_out, double *p_value_out) {
    if (!theta_vector || !covariance_matrix || df < 1 || !wald_stat_out || !p_value_out) {
        return false;
    }
    double base_wald = 0.0;
    double base_p = 1.0;
    if (!evaluate_threshold_nominal_wald_test(theta_vector, covariance_matrix, df, &base_wald, &base_p)) {
        return false;
    }
    *wald_stat_out = base_wald * (1.0 + (mixture_weight * 0.1));
    *p_value_out = exp(-(*wald_stat_out) / 2.0);
    if (*p_value_out > 1.0) *p_value_out = 1.0;
    return true;
}

bool evaluate_parameter_wald_test(double estimate, double baseline, double variance, double *wald_stat_out, double *p_value_out) {
    if (variance < 1e-9 || !wald_stat_out || !p_value_out) {
        return false;
    }
    double diff = estimate - baseline;
    *wald_stat_out = (diff * diff) / variance;
    *p_value_out = exp(-(*wald_stat_out) / 2.0);
    if (*p_value_out > 1.0) *p_value_out = 1.0;
    return true;
}

bool evaluate_scale_adjusted_threshold_wald(double threshold_est, double scale_multiplier, double baseline, double variance, double *wald_stat_out, double *p_value_out) {
    if (variance < 1e-9 || scale_multiplier < 1e-9 || !wald_stat_out || !p_value_out) {
        return false;
    }
    double adjusted_var = variance * (scale_multiplier * scale_multiplier);
    double diff = threshold_est - baseline;
    *wald_stat_out = (diff * diff) / adjusted_var;
    *p_value_out = exp(-(*wald_stat_out) / 2.0);
    if (*p_value_out > 1.0) *p_value_out = 1.0;
    return true;
}

bool evaluate_mixture_link_nominal_wald(const double *beta_vector, const double *covariance_matrix, int df, double *wald_stat_out, double *p_value_out) {
    if (!beta_vector || !covariance_matrix || df < 1 || !wald_stat_out || !p_value_out) {
        return false;
    }
    double sum_w = 0.0;
    for (int i = 0; i < df; ++i) {
        double var = covariance_matrix[i * df + i];
        if (var < 1e-9) var = 1e-9;
        sum_w += (beta_vector[i] * beta_vector[i]) / (var * 1.05);
    }
    *wald_stat_out = sum_w;
    *p_value_out = exp(-(*wald_stat_out) / 2.0);
    if (*p_value_out > 1.0) *p_value_out = 1.0;
    return true;
}

bool evaluate_information_criteria(const teddy_bear_geometry_t *geom, int param_count, int sample_size, double *aic_out, double *bic_out) {
    if (!geom || param_count < 1 || sample_size < 2 || !aic_out || !bic_out) {
        return false;
    }
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double log_lik = -150.0 + (latent * 5.0) - (geom->behavioral_mismatch * 10.0);
    *aic_out = -2.0 * log_lik + 2.0 * param_count;
    *bic_out = -2.0 * log_lik + (double)param_count * log((double)sample_size);
    return true;
}

bool evaluate_ordinal_flexible_mixture_link(const teddy_bear_geometry_t *geom, double mixture_weight, int *rating_out) {
    if (!geom || mixture_weight < 0.0 || mixture_weight > 1.0 || !rating_out) {
        return false;
    }
    int logit_rating = evaluate_ordinal_flexible_rating(geom, 1.0);
    int cloglog_rating = evaluate_ordinal_flexible_rating(geom, 0.0);
    double blended = ((double)logit_rating * mixture_weight) + ((double)cloglog_rating * (1.0 - mixture_weight));
    *rating_out = (int)(blended + 0.5);
    return true;
}

bool evaluate_ordinal_mixture_nominal_thresholds(const teddy_bear_geometry_t *geom, double mixture_weight, const double *nominal_covariates, double *thresholds_out) {
    if (!geom || mixture_weight < 0.0 || mixture_weight > 1.0 || !nominal_covariates || !thresholds_out) {
        return false;
    }
    for (int i = 0; i < 5; ++i) {
        double baseline = -2.0 + (double)i * 1.0;
        double nominal_adjustment = nominal_covariates[i] * (1.0 + mixture_weight);
        thresholds_out[i] = baseline - (geom->behavioral_mismatch * 0.5) + nominal_adjustment;
    }
    return true;
}
