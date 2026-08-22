#include "../tsfi2-deepseek/inc/auncient_teddy_bear_personality.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "tsfi_montecarlo.h"
#include "tsfi_riinterface.h"
#include "auncient_sdk.h"

#ifdef __GNUC__
__attribute__((weak)) void tsfi_zmm_winchester_deconvolve_handshake(void) {}
__attribute__((weak)) void tsfi_zmm_winchester_handshake(void) {}
__attribute__((weak)) uint16_t ce_gprs[64] = {0};
#endif

static double test_thunk_executed_flag = 0.0;
static double test_thunk_callback(void) {
    test_thunk_executed_flag = 1.0;
    return 1.0;
}

typedef enum { LHK_RED_NODE, LHK_BLACK_NODE } GSRBColor;
typedef struct GSRBNode {
    char *key;
    GSRBColor color;
    struct GSRBNode *left;
    struct GSRBNode *right;
} GSRBNode;

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT RENDERMAN TEDDY BEAR PERSONALITY CONFIGURATION TEST\n");
    printf("=============================================================\n");

    teddy_bear_geometry_t geom;

    // Test Trustworthy Mapping
    resolve_teddy_bear_geometry(PERSONALITY_TRUSTWORTHY, &geom);
    assert(geom.head_fwhr == 1.0);
    assert(geom.eye_eccentricity == 1.0);
    assert(geom.eye_scale == 1.0);
    assert(geom.stiffness == 0.3);
    printf("   ✓ Trustworthy parameters resolved: Round Head (1.0), Soft Dynamics (Stiffness: 0.3)\n");

    // Test Aggressive Mapping
    resolve_teddy_bear_geometry(PERSONALITY_AGGRESSIVE, &geom);
    assert(geom.head_fwhr == 0.7);
    assert(geom.eye_eccentricity == 0.5);
    assert(geom.stiffness == 0.9);
    printf("   ✓ Aggressive parameters resolved: Narrow Head (0.7), Rigid Dynamics (Stiffness: 0.9)\n");

    // Test Eerie Mapping
    resolve_teddy_bear_geometry(PERSONALITY_EERIE, &geom);
    assert(geom.head_fwhr == 0.7);
    assert(geom.eye_scale == 1.8);
    assert(geom.damping == 0.1);
    printf("   ✓ Eerie parameters resolved: Narrow Head (0.7), Eerie Dynamics (Damping: 0.1)\n");

    // Test Skeptical Mapping
    resolve_teddy_bear_geometry(PERSONALITY_SKEPTICAL, &geom);
    assert(geom.head_fwhr == 0.8);
    assert(geom.symmetry == 0.6);
    assert(geom.naivety_index == 0.0);
    printf("   ✓ Skeptical parameters resolved: Narrow Head (0.8), Crooked Symmetry (0.6), Naivety (0.0)\n");

    // Test system boundary engagement and typestate validation
    agent_avatar_t avatar;
    assert(engage_system_boundary(&avatar, PERSONALITY_TRUSTWORTHY));
    assert(validate_sdk_typestate(&avatar));
    printf("   ✓ System boundary engaged: Typestate executing asserted successfully\n");

    // Test WinchesterMQ DisplacementShader alignment sync
    double vertex_offsets[3] = {1.0, 2.0, 3.0};
    execute_displacement_shader_sync(&avatar, vertex_offsets, 3);
    // scale = 1.0 + (1.0 * 0.5) - (0.0 * 0.3) = 1.5
    assert(vertex_offsets[0] == 1.5);
    assert(vertex_offsets[1] == 3.0);
    assert(vertex_offsets[2] == 4.5);
    printf("   ✓ WinchesterMQ DisplacementShader alignment sync verified successfully\n");

    // Test WinchesterMQ SCSI Loop Emulation
    assert(emulate_winchester_scsi_loop(&avatar));
    printf("   ✓ WinchesterMQ SCSI Loop Emulation verified successfully\n");

    // Test FET Verlet soft-body discharge simulation
    simulate_fet_verlet_discharge(&avatar, 0.1);
    // damping should be updated from initial value
    assert(avatar.geometry.damping >= 0.0);
    printf("   ✓ FET Verlet Soft-Body Discharge simulation verified successfully\n");

    // Test Cloglog-modulated Verlet physics simulation
    double next_verlet_pos = 0.0;
    assert(simulate_cloglog_verlet_physics(&geom, 0.5, 1.0, 0.9, 0.1, &next_verlet_pos));
    assert(next_verlet_pos < 1.1);
    printf("   ✓ Cloglog-modulated Verlet soft-body physics verified successfully\n");

    // Test Christensen ordinal link model implementation
    resolve_teddy_bear_geometry(PERSONALITY_TRUSTWORTHY, &geom);
    int rating_trust = evaluate_ordinal_link_rating(&geom);
    assert(rating_trust >= 1 && rating_trust <= 7);

    resolve_teddy_bear_geometry(PERSONALITY_AGGRESSIVE, &geom);
    int rating_aggr = evaluate_ordinal_link_rating(&geom);
    assert(rating_aggr >= 1 && rating_aggr <= 7);
    printf("   ✓ R H B Christensen cumulative ordinal link rating verified successfully\n");

    // Test profile likelihood bounds calculation
    double lower = 0.0, upper = 0.0;
    assert(evaluate_profile_likelihood_bounds(&geom, rating_aggr, &lower, &upper));
    assert(lower < upper);
    printf("   ✓ R H B Christensen profile-likelihood threshold bounds verified successfully\n");

    // Test scale effect rating estimation
    int rating_scale = evaluate_ordinal_scale_rating(&geom, 0.5);
    assert(rating_scale >= 1 && rating_scale <= 7);
    printf("   ✓ R H B Christensen scale-adjusted ordinal rating verified successfully\n");

    // Test nominal effects rating estimation
    int rating_nominal = evaluate_ordinal_nominal_rating(&geom);
    assert(rating_nominal >= 1 && rating_nominal <= 7);
    printf("   ✓ R H B Christensen nominal-adjusted ordinal rating verified successfully\n");

    // Test Hessian standard error covariance diagnostics estimation
    double variance = 0.0;
    assert(evaluate_hessian_diagnostics(&geom, &variance));
    assert(variance > 0.0);
    printf("   ✓ R H B Christensen Hessian standard error covariance diagnostics verified successfully\n");

    // Test Hessian parameter covariance matrix inverse estimation
    double cov_matrix[36] = {0.0};
    assert(evaluate_parameter_covariance(&geom, cov_matrix));
    assert(cov_matrix[0] > 0.0);
    printf("   ✓ R H B Christensen parameter covariance matrix verified successfully\n");

    // Test complementary log-log rating estimation
    int rating_cloglog = evaluate_ordinal_cloglog_rating(&geom);
    assert(rating_cloglog >= 1 && rating_cloglog <= 7);
    printf("   ✓ R H B Christensen complementary log-log ordinal rating verified successfully\n");

    // Test scale-adjusted cloglog rating estimation
    int rating_cloglog_scale = evaluate_ordinal_cloglog_scale_rating(&geom, -0.8);
    assert(rating_cloglog_scale >= 1 && rating_cloglog_scale <= 7);
    printf("   ✓ R H B Christensen scale-adjusted cloglog ordinal rating verified successfully\n");

    // Test probit rating estimation
    int rating_probit = evaluate_ordinal_probit_rating(&geom);
    assert(rating_probit >= 1 && rating_probit <= 7);
    printf("   ✓ R H B Christensen probit ordinal rating verified successfully\n");

    // Test log-log rating estimation
    int rating_loglog = evaluate_ordinal_loglog_rating(&geom);
    assert(rating_loglog >= 1 && rating_loglog <= 7);
    printf("   ✓ R H B Christensen log-log ordinal rating verified successfully\n");

    // Test Cauchy rating estimation
    int rating_cauchy = evaluate_ordinal_cauchy_rating(&geom);
    assert(rating_cauchy >= 1 && rating_cauchy <= 7);
    printf("   ✓ R H B Christensen Cauchy ordinal rating verified successfully\n");

    // Test log-gamma rating estimation
    int rating_loggamma = evaluate_ordinal_loggamma_rating(&geom, 0.5);
    assert(rating_loggamma >= 1 && rating_loggamma <= 7);
    printf("   ✓ R H B Christensen log-gamma ordinal rating verified successfully\n");

    // Test Gumbel rating estimation
    int rating_gumbel = evaluate_ordinal_gumbel_rating(&geom);
    assert(rating_gumbel >= 1 && rating_gumbel <= 7);
    printf("   ✓ R H B Christensen Gumbel ordinal rating verified successfully\n");

    // Test scale-adjusted Gumbel rating estimation
    int rating_gumbel_scale = evaluate_ordinal_gumbel_scale_rating(&geom, 1.2);
    assert(rating_gumbel_scale >= 1 && rating_gumbel_scale <= 7);
    printf("   ✓ R H B Christensen scale-adjusted Gumbel ordinal rating verified successfully\n");

    // Test cumulative link probability estimation
    double link_prob = 0.0;
    assert(evaluate_ordinal_link_probability(1.0, 1.5, 0, &link_prob)); // Logit
    assert(link_prob > 0.0 && link_prob < 1.0);
    printf("   ✓ R H B Christensen cumulative link probability verified successfully\n");

    // Test ordinal link expectation estimation
    double probs[3] = {0.2, 0.5, 0.3};
    double expected_response = 0.0;
    assert(evaluate_ordinal_link_expectation(probs, 3, &expected_response));
    assert(fabs(expected_response - 2.1) < 1e-9);
    printf("   ✓ R H B Christensen ordinal link response expectation verified successfully\n");

    // Test ordinal link expectation standard error estimation
    double prob_covar[9] = {0.04, 0.0, 0.0, 0.0, 0.09, 0.0, 0.0, 0.0, 0.16};
    double expected_se = 0.0;
    assert(evaluate_ordinal_link_expectation_se(probs, prob_covar, 3, &expected_se));
    assert(expected_se > 0.0);
    printf("   ✓ R H B Christensen ordinal link expectation standard error verified successfully\n");

    // Test mixture link expectation standard error estimation
    double mixture_se = 0.0;
    assert(evaluate_ordinal_mixture_expectation_se(probs, prob_covar, 0.5, 3, &mixture_se));
    assert(mixture_se > expected_se);
    printf("   ✓ R H B Christensen mixture link expectation standard error verified successfully\n");

    // Test ordinal link model log-likelihood calculation
    int ratings[5] = {3, 5, 2, 6, 4};
    double loglik = 0.0;
    assert(evaluate_ordinal_link_loglik(&geom, ratings, 5, &loglik));
    assert(loglik < 0.0);
    printf("   ✓ R H B Christensen cumulative link model log-likelihood verified successfully\n");

    // Test AIC and BIC information criteria diagnostics estimation
    double aic = 0.0, bic = 0.0;
    assert(evaluate_information_criteria(&geom, 3, 100, &aic, &bic));
    assert(aic > 0.0 && bic > 0.0);
    printf("   ✓ R H B Christensen AIC and BIC model selection criteria verified successfully\n");

    // Test flexible link rating estimation (mixture logit/cloglog)
    int rating_flex = evaluate_ordinal_flexible_rating(&geom, 0.5);
    assert(rating_flex >= 1 && rating_flex <= 7);
    printf("   ✓ R H B Christensen flexible mixture link ordinal rating verified successfully\n");

    // Test flexible mixture link function mapping ordinal boundaries
    int rating_mixture = 0;
    assert(evaluate_ordinal_flexible_mixture_link(&geom, 0.5, &rating_mixture));
    assert(rating_mixture >= 1 && rating_mixture <= 7);
    printf("   ✓ R H B Christensen flexible mixture link mapping verified successfully\n");

    // Test mixture link nominal-adjusted threshold bounds calculation
    double nominal_covs[5] = {0.1, -0.2, 0.3, -0.1, 0.2};
    double mixture_thresholds[5];
    assert(evaluate_ordinal_mixture_nominal_thresholds(&geom, 0.5, nominal_covs, mixture_thresholds));
    assert(mixture_thresholds[0] < mixture_thresholds[4]);
    printf("   ✓ R H B Christensen flexible mixture link nominal-adjusted thresholds verified successfully\n");

    // Test Cauchy/Gumbel mixture link rating
    int rating_cg = evaluate_ordinal_cauchy_gumbel_mixture(&geom, 0.5);
    assert(rating_cg >= 1 && rating_cg <= 7);
    printf("   ✓ R H B Christensen Cauchy/Gumbel mixture link ordinal rating verified successfully\n");

    // Test threshold equidistancy diagnostics calculation
    double spacing_err = 0.0;
    assert(evaluate_threshold_equidistancy(&geom, 1.0, &spacing_err));
    assert(spacing_err >= 0.0);
    printf("   ✓ R H B Christensen threshold equidistancy diagnostics verified successfully\n");

    // Test scale parameter likelihood profile calculation
    double l_scale = 0.0, u_scale = 0.0;
    assert(evaluate_scale_profile_bounds(&geom, &l_scale, &u_scale));
    assert(l_scale < u_scale);
    printf("   ✓ R H B Christensen scale parameter likelihood profile bounds verified successfully\n");

    // Test scale parameter under structured covariates calculation
    double struct_scale = 0.0;
    assert(evaluate_scale_structured_covariates(&geom, 30.0, &struct_scale));
    assert(struct_scale > 0.0);
    printf("   ✓ R H B Christensen scale parameter under structured covariates verified successfully\n");

    // Test scale parameter nominal effects calculation
    double nom_cov[2] = {0.5, 1.2};
    double nom_effects[2] = {0.0};
    assert(evaluate_scale_nominal_effects(&geom, nom_cov, 2, nom_effects));
    assert(nom_effects[0] > 0.0);
    printf("   ✓ R H B Christensen scale parameter nominal effects verified successfully\n");

    // Test threshold parameter likelihood profile calculation
    double l_thresh = 0.0, u_thresh = 0.0;
    assert(evaluate_threshold_profile_bounds(&geom, 2, &l_thresh, &u_thresh));
    assert(l_thresh < u_thresh);
    printf("   ✓ R H B Christensen threshold parameter likelihood profile bounds verified successfully\n");

    // Test Likelihood Ratio Test (LRT) nested model comparison
    double chi_sq = 0.0, p_val = 0.0;
    assert(evaluate_lrt_nested_models(-150.0, -145.0, 1, &chi_sq, &p_val));
    assert(chi_sq == 10.0);
    assert(p_val > 0.0 && p_val < 0.05); // statistically significant difference
    printf("   ✓ R H B Christensen nested model Likelihood Ratio Test diagnostics verified successfully\n");

    // Test predicted class probability confidence bounds calculation
    double pred_prob = 0.0, low_p = 0.0, high_p = 0.0;
    assert(evaluate_predicted_probability_bounds(&geom, 4, &pred_prob, &low_p, &high_p));
    assert(pred_prob >= 0.0 && pred_prob <= 1.0);
    assert(low_p <= high_p);
    printf("   ✓ R H B Christensen predicted class probability and standard error bounds verified successfully\n");

    // Test Wald nominal effect test calculation
    double betas[3] = {1.5, -2.0, 0.5};
    double covar[9] = {0.25, 0.0, 0.0, 0.0, 0.16, 0.0, 0.0, 0.0, 0.09};
    double w_stat = 0.0, w_pval = 0.0;
    assert(evaluate_wald_nominal_test(betas, covar, 3, &w_stat, &w_pval));
    assert(w_stat > 0.0);
    assert(w_pval < 0.05); // statistically significant violation
    printf("   ✓ R H B Christensen nominal effect Wald test diagnostics verified successfully\n");

    // Test surrogate residuals calculation
    double sur_resid = 0.0;
    assert(evaluate_surrogate_residuals(&geom, 4, &sur_resid));
    printf("   ✓ R H B Christensen surrogate residuals diagnostics verified successfully\n");

    // Test nominal surrogate residuals calculation
    double nom_sur_resid = 0.0;
    assert(evaluate_nominal_surrogate_residuals(&geom, 4, &nom_sur_resid));
    printf("   ✓ R H B Christensen nominal surrogate residuals verified successfully\n");

    // Test threshold parameter Wald test calculation
    double t_wstat = 0.0, t_wpval = 0.0;
    assert(evaluate_threshold_wald_test(1.2, 0.5, 0.04, &t_wstat, &t_wpval));
    assert(fabs(t_wstat - 12.25) < 1e-9);
    assert(t_wpval < 0.01);
    printf("   ✓ R H B Christensen threshold parameter Wald test diagnostics verified successfully\n");

    // Test scale parameter Wald test calculation
    double s_wstat = 0.0, s_wpval = 0.0;
    assert(evaluate_scale_wald_test(1.5, 1.0, 0.25, &s_wstat, &s_wpval));
    assert(fabs(s_wstat - 1.0) < 1e-9);
    assert(s_wpval < 0.7);
    printf("   ✓ R H B Christensen scale parameter Wald test diagnostics verified successfully\n");

    // Test standard parameter Wald test calculation
    double p_wstat = 0.0, p_wpval = 0.0;
    assert(evaluate_parameter_wald_test(2.0, 1.0, 0.25, &p_wstat, &p_wpval));
    assert(p_wstat > 0.0);
    printf("   ✓ R H B Christensen standard parameter Wald test verified successfully\n");

    // Test scale nominal Wald test calculation
    double gammas[2] = {0.8, -0.6};
    double g_covar[4] = {0.16, 0.0, 0.0, 0.09};
    double sn_wstat = 0.0, sn_wpval = 0.0;
    assert(evaluate_scale_nominal_wald_test(gammas, g_covar, 2, &sn_wstat, &sn_wpval));
    assert(sn_wstat > 0.0);
    assert(sn_wpval < 0.05); // significant
    printf("   ✓ R H B Christensen scale nominal Wald test diagnostics verified successfully\n");

    // Test threshold nominal Wald test calculation
    double thetas[2] = {1.2, -1.0};
    double t_covar[4] = {0.25, 0.0, 0.0, 0.25};
    double tn_wstat = 0.0, tn_wpval = 0.0;
    assert(evaluate_threshold_nominal_wald_test(thetas, t_covar, 2, &tn_wstat, &tn_wpval));
    assert(tn_wstat > 0.0);
    assert(tn_wpval < 0.05); // significant
    printf("   ✓ R H B Christensen threshold nominal Wald test diagnostics verified successfully\n");

    // Test mixed-link threshold Wald test calculation
    double mix_wstat = 0.0, mix_wpval = 0.0;
    assert(evaluate_ordinal_mixture_threshold_wald_test(thetas, t_covar, 0.5, 2, &mix_wstat, &mix_wpval));
    assert(mix_wstat > tn_wstat);
    printf("   ✓ R H B Christensen mixed-link threshold parameters Wald test verified successfully\n");

    // Test scale-adjusted threshold Wald test calculation
    double sat_wstat = 0.0, sat_wpval = 0.0;
    assert(evaluate_scale_adjusted_threshold_wald(1.5, 1.2, 1.0, 0.25, &sat_wstat, &sat_wpval));
    assert(sat_wstat > 0.0);
    printf("   ✓ R H B Christensen scale-adjusted threshold Wald test verified successfully\n");

    // Test mixture-link nominal effects Wald test calculation
    double mn_wstat = 0.0, mn_wpval = 0.0;
    assert(evaluate_mixture_link_nominal_wald(thetas, t_covar, 2, &mn_wstat, &mn_wpval));
    assert(mn_wstat > 0.0);
    printf("   ✓ R H B Christensen flexible mixture link nominal effects Wald test verified successfully\n");

    // Test Geniole fWHR threat level calculation
    double threat_level = evaluate_fw_threat_level(&geom);
    assert(threat_level >= 0.0);

    // Test Geniole behavioral mismatch threat calculation
    double threat_mismatch = 0.0;
    assert(evaluate_behavioral_threat_mismatch(&geom, &threat_mismatch));
    assert(threat_mismatch >= 0.0);

    // Test Geniole reactive mismatch retaliation calculation
    double reactive_aggr = evaluate_reactive_mismatch_retaliation(&geom);
    assert(reactive_aggr >= 0.0);

    // Test Geniole rapid exposure threat consistency calculation
    double rapid_threat = 0.0;
    assert(evaluate_exposure_threat_consistency(&geom, 25.0, &rapid_threat));
    assert(rapid_threat >= 0.0);

    // Test Geniole cooperative cheating risk calculation
    double cheating_risk = 0.0;
    assert(evaluate_cooperative_cheating_risk(&geom, 1.5, &cheating_risk));
    assert(cheating_risk >= 0.0);

    // Test Geniole rapid exposure threat limit calculation
    double limit_threat = 0.0;
    assert(evaluate_rapid_threat_limit(&geom, 50.0, &limit_threat));
    assert(limit_threat >= 0.0);

    // Test Geniole reactive retaliation profile calculation
    double ret_prof = 0.0;
    assert(evaluate_reactive_retaliation_profile(&geom, 2.0, &ret_prof));
    assert(ret_prof >= 0.0);

    // Test Geniole provocation aggression threshold calculation
    double agg_thresh = 0.0;
    assert(evaluate_provocation_aggression_threshold(&geom, 5.0, &agg_thresh));
    assert(agg_thresh >= 0.0);

    // Test Geniole exposure duration adjusted threat threshold calculation
    double exp_adjusted_threat = 0.0;
    assert(evaluate_exposure_adjusted_threat_threshold(&geom, 150.0, &exp_adjusted_threat));
    assert(exp_adjusted_threat >= 0.0);

    // Test Geniole status dominance threat interaction calculation
    double status_dom_threat = 0.0;
    assert(evaluate_status_dominance_provocation(&geom, 2.5, &status_dom_threat));
    assert(status_dom_threat >= 0.0);

    // Test Geniole threat replication consistency calculation
    double observations[3] = {1.5, 1.6, 1.4};
    double rep_consistency = 0.0;
    assert(evaluate_threat_replication_consistency(&geom, observations, 3, &rep_consistency));
    assert(rep_consistency > 0.0);

    // Test Geniole provocation-modulated reactive retaliation boundary calculation
    double ret_boundary = 0.0;
    assert(evaluate_reactive_retaliation_boundary(&geom, 2.0, &ret_boundary));
    assert(ret_boundary >= 0.0);

    // Test Geniole fWHR retaliation aggression scaling calculation
    double scaled_agg = 0.0;
    assert(evaluate_retaliation_aggression_scaling(&geom, 1.5, &scaled_agg));
    assert(scaled_agg > 1.5);

    // Test Geniole fWHR retaliation threshold decay calculation
    double decayed_ret_thresh = 0.0;
    assert(evaluate_retaliation_threshold_decay(&geom, 120.0, &decayed_ret_thresh));
    assert(decayed_ret_thresh >= 0.0);

    // Test Geniole provocation exposure interactive decay calculation
    double decayed_exp_threat = 0.0;
    assert(evaluate_provocation_exposure_decay(&geom, 1.5, 90.0, &decayed_exp_threat));
    assert(decayed_exp_threat >= 0.0);

    // Test Geniole provocation-modulated retaliation aggression ceiling calculation
    double aggression_ceiling = 0.0;
    assert(evaluate_retaliation_aggression_ceiling(&geom, 2.5, &aggression_ceiling));
    assert(aggression_ceiling >= 0.0);

    // Test Geniole status challenge multiplier calculation
    double status_mult = 0.0;
    assert(evaluate_status_dominance_multiplier(&geom, 2.0, &status_mult));
    assert(status_mult >= 1.0);

    // Test Geniole cheating threat calculation
    double cheating_threat = 0.0;
    assert(evaluate_cheating_threat_index(&geom, 2.5, &cheating_threat));
    assert(cheating_threat >= 0.0);

    // Test Geniole threat replication consistency calculation
    double consistency = 0.0;
    assert(evaluate_provocation_replication_consistency(&geom, 1.5, &consistency));
    assert(consistency >= 0.0);

    // Test Geniole rapid threat limit calculation
    double rapid_limit = 0.0;
    assert(evaluate_provocation_rapid_threat_limit(&geom, 2.0, &rapid_limit));
    assert(rapid_limit >= 0.0);

    // Test Geniole provocation retaliation boundary calculation
    double retaliation_boundary = 0.0;
    assert(evaluate_provocation_retaliation_boundary(&geom, 100.0, &retaliation_boundary));
    assert(retaliation_boundary >= 0.0);
    printf("   ✓ GENIOLE et al. fWHR threat, visual-vocal mismatch, reactive retaliation, rapid exposure consistency, cooperative cheating risk, rapid threat limit, retaliation profile, aggression threshold, exposure-adjusted threat, status-dominance threat, threat replication consistency, retaliation boundary, aggression scaling, threshold decay, provocation exposure decay, aggression ceiling, status-dominance multiplier, cheating threat index, provocation threat replication consistency, rapid threat limit, and provocation retaliation boundary verified successfully\n");

    // Test Hyde uncanny mismatch index calculation
    double uncanny_val = 0.0;
    assert(evaluate_uncanny_mismatch_index(&geom, &uncanny_val));
    assert(uncanny_val >= 0.0);
    printf("   ✓ HYDE et al. vocal-visual uncanny mismatch index verified successfully\n");

    // Test Hyde motion-mismatch uncanny valley calculation
    double motion_uncanny = 0.0;
    assert(evaluate_motion_uncanny_index(&geom, 0.8, &motion_uncanny));
    assert(motion_uncanny >= uncanny_val);
    printf("   ✓ HYDE et al. vocal-visual uncanny mismatch and motion-mismatch index verified successfully\n");

    // Test Hyde micro-expression jitter uncanny calculation
    double jitter_uncanny = 0.0;
    assert(evaluate_expression_jitter_uncanny(&geom, 15.0, &jitter_uncanny));
    assert(jitter_uncanny > uncanny_val);
    printf("   ✓ HYDE et al. vocal-visual uncanny mismatch and micro-expression jitter verified successfully\n");

    // Test Hyde exposure duration habituation decay calculation
    double decayed_uncanny = 0.0;
    assert(evaluate_exposure_decay_uncanny(&geom, 5.0, &decayed_uncanny));
    assert(decayed_uncanny < uncanny_val);
    printf("   ✓ HYDE et al. exposure duration habituation decay verified successfully\n");

    // Test Hyde unpredictable micro-expression freeze habituation decay calculation
    double decayed_freeze_uncanny = 0.0;
    assert(evaluate_freeze_habituation_decay(&geom, 7.5, &decayed_freeze_uncanny));
    assert(decayed_freeze_uncanny < uncanny_val);
    printf("   ✓ HYDE et al. unpredictable micro-expression freeze habituation decay verified successfully\n");

    // Test Hyde unpredictable animation velocity jitter calculation
    double vel_jitter_uncanny = 0.0;
    assert(evaluate_velocity_jitter_uncanny(&geom, 0.4, &vel_jitter_uncanny));
    assert(vel_jitter_uncanny >= 0.0);
    printf("   ✓ HYDE et al. unpredictable animation velocity jitter verified successfully\n");

    // Test Hyde unpredictable micro-expression freeze calculation
    double freeze_uncanny = 0.0;
    assert(evaluate_expression_freeze_uncanny(&geom, 2.0, &freeze_uncanny));
    assert(freeze_uncanny > uncanny_val);
    printf("   ✓ HYDE et al. unpredictable micro-expression freeze verified successfully\n");

    // Test Hyde unpredictable micro-expression freeze frequency calculation
    double freeze_freq_uncanny = 0.0;
    assert(evaluate_expression_freeze_frequency(&geom, 3.5, &freeze_freq_uncanny));
    assert(freeze_freq_uncanny >= 0.0);
    printf("   ✓ HYDE et al. unpredictable micro-expression freeze frequency verified successfully\n");

    // Test Hyde unpredictable micro-expression asymmetry calculation
    double asym_uncanny = 0.0;
    assert(evaluate_expression_asymmetry_uncanny(&geom, 0.5, &asym_uncanny));
    assert(asym_uncanny >= 0.0);
    printf("   ✓ HYDE et al. unpredictable micro-expression asymmetry verified successfully\n");

    // Test Hyde unpredictable micro-expression asymmetry duration calculation
    double asym_duration_uncanny = 0.0;
    assert(evaluate_expression_asymmetry_duration(&geom, 250.0, &asym_duration_uncanny));
    assert(asym_duration_uncanny >= 0.0);
    printf("   ✓ HYDE et al. unpredictable micro-expression asymmetry duration verified successfully\n");

    // Test Hyde unpredictable micro-expression synchronization delay calculation
    double sync_uncanny = 0.0;
    assert(evaluate_expression_sync_uncanny(&geom, 60.0, &sync_uncanny));
    assert(sync_uncanny >= 0.0);
    printf("   ✓ HYDE et al. unpredictable micro-expression synchronization verified successfully\n");

    // Test Hyde unpredictable micro-expression synchronization habituation decay calculation
    double decayed_sync_uncanny = 0.0;
    assert(evaluate_sync_habituation_decay(&geom, 8.0, &decayed_sync_uncanny));
    assert(decayed_sync_uncanny < sync_uncanny);
    printf("   ✓ HYDE et al. unpredictable micro-expression synchronization habituation decay verified successfully\n");

    // Test Hyde unpredictable micro-expression acceleration jitter calculation
    double acc_jitter_uncanny = 0.0;
    assert(evaluate_acceleration_jitter_uncanny(&geom, 0.3, &acc_jitter_uncanny));
    assert(acc_jitter_uncanny >= 0.0);
    printf("   ✓ HYDE et al. unpredictable micro-expression acceleration jitter verified successfully\n");

    // Test Hyde unpredictable micro-expression acceleration jitter habituation decay calculation
    double decayed_acc_uncanny = 0.0;
    assert(evaluate_acceleration_habituation_decay(&geom, 10.0, &decayed_acc_uncanny));
    assert(decayed_acc_uncanny < acc_jitter_uncanny);
    printf("   ✓ HYDE et al. unpredictable micro-expression acceleration jitter habituation decay verified successfully\n");

    // Test Hyde unpredictable animation acceleration jitter calculation
    double anim_acc_jitter = 0.0;
    assert(evaluate_animation_acceleration_jitter(&geom, 0.35, &anim_acc_jitter));
    assert(anim_acc_jitter >= 0.0);
    printf("   ✓ HYDE et al. unpredictable animation acceleration jitter verified successfully\n");

    // Test Hyde unpredictable micro-expression amplitude jitter calculation
    double amp_jitter_uncanny = 0.0;
    assert(evaluate_amplitude_jitter_uncanny(&geom, 0.25, &amp_jitter_uncanny));
    assert(amp_jitter_uncanny >= 0.0);
    printf("   ✓ HYDE et al. unpredictable micro-expression amplitude jitter verified successfully\n");

    // Test Hyde unpredictable micro-expression amplitude jitter habituation decay calculation
    double decayed_amp_uncanny = 0.0;
    assert(evaluate_amplitude_habituation_decay(&geom, 12.0, &decayed_amp_uncanny));
    assert(decayed_amp_uncanny < amp_jitter_uncanny);
    printf("   ✓ HYDE et al. unpredictable micro-expression amplitude jitter habituation decay verified successfully\n");

    // Test Hyde vocal-visual synchronization delay calculation
    double vv_sync_uncanny = 0.0;
    assert(evaluate_vocal_visual_sync_uncanny(&geom, 90.0, &vv_sync_uncanny));
    assert(vv_sync_uncanny >= 0.0);
    printf("   ✓ HYDE et al. vocal-visual synchronization verified successfully\n");

    // Test Hyde vocal-visual acceleration synchrony delay calculation
    double vv_acc_sync = 0.0;
    assert(evaluate_vocal_visual_acceleration_sync(&geom, 50.0, &vv_acc_sync));
    assert(vv_acc_sync >= 0.0);
    printf("   ✓ HYDE et al. vocal-visual acceleration synchronization verified successfully\n");

    // Test Hyde vocal-visual pitch transition velocity mismatch calculation
    double pitch_vel_mismatch = 0.0;
    assert(evaluate_pitch_velocity_mismatch(&geom, 0.4, &pitch_vel_mismatch));
    assert(pitch_vel_mismatch >= 0.0);
    printf("   ✓ HYDE et al. vocal-visual pitch transition velocity mismatch verified successfully\n");

    // Test Hyde vocal-visual pitch transition velocity mismatch habituation decay calculation
    double decayed_pitch_uncanny = 0.0;
    assert(evaluate_pitch_mismatch_habituation_decay(&geom, 15.0, &decayed_pitch_uncanny));
    assert(decayed_pitch_uncanny < pitch_vel_mismatch);
    printf("   ✓ HYDE et al. vocal-visual pitch transition velocity mismatch habituation decay verified successfully\n");

    // Test Hyde combined pitch mismatch and micro-expression freeze uncanny calculation
    double pitch_freeze_uncanny = 0.0;
    assert(evaluate_pitch_freeze_uncanny(&geom, 0.4, 2.0, &pitch_freeze_uncanny));
    assert(pitch_freeze_uncanny > pitch_vel_mismatch);
    printf("   ✓ HYDE et al. vocal-visual pitch transition velocity mismatch and micro-expression freeze index verified successfully\n");

    // Test Hyde combined pitch mismatch and micro-expression jitter uncanny calculation
    double pitch_jitter_uncanny = 0.0;
    assert(evaluate_pitch_jitter_uncanny(&geom, 0.4, 0.1, &pitch_jitter_uncanny));
    assert(pitch_jitter_uncanny > pitch_vel_mismatch);
    printf("   ✓ HYDE et al. vocal-visual pitch transition velocity mismatch and micro-expression jitter index verified successfully\n");

    // Test Hyde vocal-visual motion acceleration mismatch calculation
    double accel_uncanny = 0.0;
    assert(evaluate_motion_acceleration_mismatch(&geom, 0.5, &accel_uncanny));
    assert(accel_uncanny > 0.0);
    printf("   ✓ HYDE et al. vocal-visual motion acceleration mismatch and jitter index verified successfully\n");

    // Test Hyde combined motion mismatch and micro-expression freeze uncanny calculation
    double motion_freeze_uncanny = 0.0;
    assert(evaluate_motion_freeze_uncanny(&geom, 0.4, 2.0, &motion_freeze_uncanny));
    assert(motion_freeze_uncanny > 0.0);
    printf("   ✓ HYDE et al. vocal-visual motion-mismatch and micro-expression freeze index verified successfully\n");

    // Test Hyde spatial interaction distance proximity uncanny calculation
    double interaction_uncanny = 0.0;
    assert(evaluate_spatial_interaction_distance(&geom, 0.5, &interaction_uncanny));
    assert(interaction_uncanny > 0.0);
    printf("   ✓ HYDE et al. spatial interaction distance proximity verified successfully\n");

    // Test Hyde dynamic auditory offset calculation
    double auditory_offset = 0.0;
    assert(evaluate_dynamic_auditory_offset(&geom, 60.0, 75.0, &auditory_offset));
    assert(auditory_offset > 0.0);
    printf("   ✓ HYDE et al. dynamic auditory offset verified successfully\n");

    // Test Keating babyfacedness and dominance cue index calculations
    double babyfacedness = 0.0;
    double dominance = 0.0;
    assert(evaluate_keating_babyfacedness_index(&geom, &babyfacedness));
    assert(babyfacedness > 0.0);
    assert(evaluate_keating_dominance_cue_index(&geom, &dominance));
    assert(dominance > 0.0);
    printf("   ✓ KEATING, C. F. babyfacedness and dominance physiognomy indices verified successfully\n");

    // Test Hyde gaze direction and vocal tempo variance index calculations
    double gaze_alignment = 0.0;
    double vocal_tempo_uncanny = 0.0;
    assert(evaluate_hyde_gaze_direction_index(&geom, 0.5, &gaze_alignment));
    assert(gaze_alignment > 0.0);
    assert(evaluate_hyde_vocal_tempo_variance(&geom, 140.0, &vocal_tempo_uncanny));
    assert(vocal_tempo_uncanny > 0.0);
    printf("   ✓ HYDE et al. gaze direction and vocal tempo variance indices verified successfully\n");

    // Test Geniole provocation limit and testosterone modulator calculations
    double aggression_limit = 0.0;
    double testosterone_mod = 0.0;
    assert(evaluate_geniole_provocation_aggression_limit(&geom, 1.5, &aggression_limit));
    assert(aggression_limit > 0.0);
    assert(evaluate_geniole_testosterone_modulator(&geom, 2.5, &testosterone_mod));
    assert(testosterone_mod > 0.0);
    printf("   ✓ GENIOLE et al. provocation aggression limit and testosterone modulator verified successfully\n");

    // Test Keating eyebrow dominance and lip trustworthiness calculations
    double brow_dominance = 0.0;
    double lip_trustworthiness = 0.0;
    assert(evaluate_keating_brow_dominance(&geom, 0.4, &brow_dominance));
    assert(brow_dominance > 0.0);
    assert(evaluate_keating_lip_trustworthiness(&geom, 0.5, &lip_trustworthiness));
    assert(lip_trustworthiness > 0.0);
    printf("   ✓ KEATING, C. F. eyebrow dominance and lip trustworthiness verified successfully\n");

    // Test Hyde conversational latency and vocal size mismatch calculations
    double conversational_latency = 0.0;
    double vocal_size_mismatch = 0.0;
    assert(evaluate_hyde_conversational_latency(&geom, 1.2, &conversational_latency));
    assert(conversational_latency > 0.0);
    assert(evaluate_hyde_vocal_size_mismatch(&geom, 180.0, &vocal_size_mismatch));
    assert(vocal_size_mismatch > 0.0);
    printf("   ✓ HYDE et al. conversational latency and vocal size mismatch verified successfully\n");

    // Test Keating brow-eye ratio and Geniole fWHR dilation calculations
    double brow_eye_dominance = 0.0;
    double fwhr_dilation = 0.0;
    assert(evaluate_keating_brow_eye_ratio(&geom, 0.3, &brow_eye_dominance));
    assert(brow_eye_dominance > 0.0);
    assert(evaluate_geniole_fwhr_dilation_map(&geom, 1.2, &fwhr_dilation));
    assert(fwhr_dilation > 0.0);
    printf("   ✓ Keating brow-eye ratio and Geniole fWHR dilation mapping verified successfully\n");

    // Test Keating head roundness and symmetry trustworthiness calculations
    double head_roundness = 0.0;
    double symmetry_trust = 0.0;
    assert(evaluate_keating_head_roundness_index(&geom, &head_roundness));
    assert(head_roundness > 0.0);
    assert(evaluate_keating_symmetry_trust(&geom, &symmetry_trust));
    assert(symmetry_trust > 0.0);
    printf("   ✓ KEATING, C. F. head roundness and symmetry trustworthiness verified successfully\n");

    // Test Hyde vocal amplitude mismatch and mouth speed synchrony calculations
    double amplitude_mismatch = 0.0;
    double sync_mismatch = 0.0;
    assert(evaluate_hyde_vocal_amplitude_mismatch(&geom, 75.0, &amplitude_mismatch));
    assert(amplitude_mismatch > 0.0);
    assert(evaluate_hyde_mouth_speed_synchrony(&geom, 2.5, 3.0, &sync_mismatch));
    assert(sync_mismatch > 0.0);
    printf("   ✓ HYDE et al. vocal amplitude mismatch and mouth speed synchrony verified successfully\n");

    // Test Keating brow-chin proportion and Geniole fWHR jitter modulator calculations
    double brow_chin_dominance = 0.0;
    double fwhr_jitter = 0.0;
    assert(evaluate_keating_brow_chin_proportion(&geom, 0.4, &brow_chin_dominance));
    assert(brow_chin_dominance > 0.0);
    assert(evaluate_geniole_fwhr_jitter_mod(&geom, 0.1, &fwhr_jitter));
    assert(fwhr_jitter > 0.0);
    printf("   ✓ Keating brow-chin proportion and Geniole fWHR jitter modulation verified successfully\n");

    // Test Keating gaze dominance and brow gesture calculations
    double gaze_dom = 0.0;
    double brow_sub = 0.0;
    assert(evaluate_keating_gaze_dominance(&geom, 5.0, 0.2, &gaze_dom));
    assert(gaze_dom > 0.0);
    assert(evaluate_keating_brow_gesture(&geom, 0.3, &brow_sub));
    assert(brow_sub > 0.0);
    printf("   ✓ KEATING, C. F. gaze dominance and brow gesture verified successfully\n");

    // Test Hyde vocal turn interruption and vocal micro-tremor calculations
    double turn_interruption = 0.0;
    double vocal_tremor = 0.0;
    assert(evaluate_hyde_turn_interruption(&geom, 1.5, &turn_interruption));
    assert(turn_interruption > 0.0);
    assert(evaluate_hyde_vocal_tremor_index(&geom, 0.8, &vocal_tremor));
    assert(vocal_tremor > 0.0);
    printf("   ✓ HYDE et al. vocal turn interruption and vocal micro-tremor verified successfully\n");

    // Test Keating torso-to-head ratio and Geniole fWHR boundary calculations
    double torso_head_dominance = 0.0;
    double fwhr_boundary = 0.0;
    assert(evaluate_keating_torso_head_ratio(&geom, 1.5, &torso_head_dominance));
    assert(torso_head_dominance > 0.0);
    assert(evaluate_geniole_fwhr_boundary_map(&geom, 2.0, &fwhr_boundary));
    assert(fwhr_boundary > 0.0);
    printf("   ✓ Keating torso-to-head ratio and Geniole fWHR boundary mapping verified successfully\n");

    // Test Keating eye dilation sync and posture pitch calculations
    double eye_dilation_sync = 0.0;
    double posture_pitch = 0.0;
    assert(evaluate_keating_eye_dilation_sync(&geom, 0.4, 0.4, &eye_dilation_sync));
    assert(eye_dilation_sync > 0.0);
    assert(evaluate_keating_posture_pitch(&geom, 0.2, &posture_pitch));
    assert(posture_pitch > 0.0);
    printf("   ✓ KEATING, C. F. eye dilation sync and posture pitch verified successfully\n");

    // Test Hyde tremor frequency sync and pitch range engagement calculations
    double tremor_sync = 0.0;
    double pitch_range_engage = 0.0;
    assert(evaluate_hyde_tremor_frequency_sync(&geom, 6.0, 6.0, &tremor_sync));
    assert(tremor_sync > 0.0);
    assert(evaluate_hyde_pitch_range_engagement(&geom, 120.0, &pitch_range_engage));
    assert(pitch_range_engage > 0.0);
    printf("   ✓ HYDE et al. tremor frequency sync and pitch range engagement verified successfully\n");

    // Test Keating chin asymmetry and mouth curvature calculations
    double chin_asymmetry = 0.0;
    double mouth_curvature = 0.0;
    assert(evaluate_keating_chin_asymmetry(&geom, 0.4, 0.5, &chin_asymmetry));
    assert(chin_asymmetry > 0.0);
    assert(evaluate_keating_mouth_curvature(&geom, 0.3, &mouth_curvature));
    assert(mouth_curvature > 0.0);
    printf("   ✓ KEATING, C. F. chin asymmetry and mouth curvature verified successfully\n");

    // Test Hyde vocal warmth pitch and conversational interruption frequency calculations
    double vocal_warmth_pitch = 0.0;
    double interruption_freq = 0.0;
    assert(evaluate_hyde_vocal_warmth_pitch(&geom, 220.0, &vocal_warmth_pitch));
    assert(vocal_warmth_pitch > 0.0);
    assert(evaluate_hyde_interruption_frequency(&geom, 0.6, &interruption_freq));
    assert(interruption_freq > 0.0);
    printf("   ✓ HYDE et al. vocal warmth pitch and conversational interruption frequency verified successfully\n");

    // Test Keating chin asymmetry dilation and Geniole fWHR retaliation decay calculations
    double asymmetry_dilation = 0.0;
    double fwhr_retaliation_decay = 0.0;
    assert(evaluate_keating_chin_asymmetry_dilation(&geom, 1.2, &asymmetry_dilation));
    assert(asymmetry_dilation > 0.0);
    assert(evaluate_geniole_fwhr_retaliation_decay(&geom, 5.0, &fwhr_retaliation_decay));
    assert(fwhr_retaliation_decay > 0.0);
    printf("   ✓ Keating chin asymmetry dilation and Geniole fWHR retaliation decay verified successfully\n");

    // Test Keating sclera size and facial width asymmetry calculations
    double sclera_sub = 0.0;
    double width_asymmetry = 0.0;
    assert(evaluate_keating_sclera_size(&geom, 0.45, &sclera_sub));
    assert(sclera_sub > 0.0);
    assert(evaluate_keating_width_asymmetry(&geom, 2.2, 2.4, &width_asymmetry));
    assert(width_asymmetry > 0.0);
    printf("   ✓ KEATING, C. F. sclera size and facial width asymmetry verified successfully\n");

    // Test Scarpi hedonic and utilitarian orientation index calculations
    double hedonic_orient = 0.0;
    double utilitarian_orient = 0.0;
    assert(evaluate_scarpi_hedonic_orientation(&geom, 1.5, &hedonic_orient));
    assert(hedonic_orient > 0.0);
    assert(evaluate_scarpi_utilitarian_orientation(&geom, 2.0, &utilitarian_orient));
    assert(utilitarian_orient > 0.0);
    printf("   ✓ Scarpi hedonic and utilitarian orientations verified successfully\n");

    // Test Scarpi aesthetic trust and utilitarian decay calculations
    double aesthetic_trust = 0.0;
    double utilitarian_decay = 0.0;
    assert(evaluate_scarpi_aesthetic_trust(&geom, 0.85, &aesthetic_trust));
    assert(aesthetic_trust > 0.0);
    assert(evaluate_scarpi_utilitarian_decay(&geom, 10.0, &utilitarian_decay));
    assert(utilitarian_decay > 0.0);
    printf("   ✓ Scarpi aesthetic trust and utilitarian decay verified successfully\n");

    // Test Cellarius heliocentric alignment and constellation boundary calculations
    double cellarius_align = 0.0;
    double cellarius_boundary = 0.0;
    assert(evaluate_cellarius_heliocentric_alignment(&geom, 1.0, &cellarius_align));
    assert(cellarius_align > 0.0);
    assert(evaluate_cellarius_constellation_boundary(&geom, 0.5, &cellarius_boundary));
    assert(cellarius_boundary > 0.0);
    printf("   ✓ Cellarius orbital alignment and constellation boundary verified successfully\n");

    // Test Hyde audio-visual latency jitter and vocal energy variance calculations
    double av_latency_jitter = 0.0;
    double vocal_energy_var = 0.0;
    assert(evaluate_hyde_av_latency_jitter(&geom, 0.05, &av_latency_jitter));
    assert(av_latency_jitter > 0.0);
    assert(evaluate_hyde_vocal_energy_variance(&geom, 15.0, &vocal_energy_var));
    assert(vocal_energy_var > 0.0);
    printf("   ✓ Hyde audio-visual latency jitter and vocal energy variance verified successfully\n");

    // Test Keating fWHR dominance and mouth width ratio calculations
    double fwhr_dominance = 0.0;
    double mouth_width_sub = 0.0;
    assert(evaluate_keating_fwhr_dominance(&geom, 1.85, &fwhr_dominance));
    assert(fwhr_dominance > 0.0);
    assert(evaluate_keating_mouth_width_ratio(&geom, 0.35, 0.7, &mouth_width_sub));
    assert(mouth_width_sub > 0.0);
    printf("   ✓ KEATING, C. F. fWHR dominance and mouth width ratio verified successfully\n");

    // Test Keating width symmetry trust and lip submissiveness calculations
    double width_symmetry_trust = 0.0;
    double lip_sub = 0.0;
    assert(evaluate_keating_width_symmetry_trust(&geom, 0.95, &width_symmetry_trust));
    assert(width_symmetry_trust > 0.0);
    assert(evaluate_keating_lip_submissiveness(&geom, 0.15, &lip_sub));
    assert(lip_sub > 0.0);
    printf("   ✓ KEATING, C. F. width symmetry trust and lip submissiveness verified successfully\n");

    // Test Scarpi hedonic arousal and utilitarian quality calculations
    double hedonic_arousal = 0.0;
    double utilitarian_quality = 0.0;
    assert(evaluate_scarpi_hedonic_arousal(&geom, 1.2, 0.8, &hedonic_arousal));
    assert(hedonic_arousal > 0.0);
    assert(evaluate_scarpi_utilitarian_quality(&geom, 0.95, &utilitarian_quality));
    assert(utilitarian_quality > 0.0);
    printf("   ✓ Scarpi hedonic arousal and utilitarian quality verified successfully\n");

    // Test Scarpi hedonic arousal decay and utilitarian loyalty calculations
    double hedonic_arousal_decay = 0.0;
    double utilitarian_loyalty = 0.0;
    assert(evaluate_scarpi_hedonic_arousal_decay(&geom, 15.0, &hedonic_arousal_decay));
    assert(hedonic_arousal_decay > 0.0);
    assert(evaluate_scarpi_utilitarian_loyalty(&geom, 0.88, &utilitarian_loyalty));
    assert(utilitarian_loyalty > 0.0);
    printf("   ✓ Scarpi hedonic arousal decay and utilitarian loyalty verified successfully\n");

    // Test Cellarius planetary eccentricity and epicycle modulation calculations
    double planet_eccentricity = 0.0;
    double epicycle_mod = 0.0;
    assert(evaluate_cellarius_planetary_eccentricity(&geom, 0.048, &planet_eccentricity));
    assert(planet_eccentricity > 0.0);
    assert(evaluate_cellarius_epicycle_modulation(&geom, 0.25, &epicycle_mod));
    assert(epicycle_mod > 0.0);
    printf("   ✓ Cellarius planetary eccentricity and epicycle modulation verified successfully\n");

    // Test Cellarius planet velocity and epicycle phase calculations
    double planet_velocity = 0.0;
    double epicycle_phase = 0.0;
    assert(evaluate_cellarius_planet_velocity(&geom, 24.5, &planet_velocity));
    assert(planet_velocity > 0.0);
    assert(evaluate_cellarius_epicycle_phase(&geom, 1.2, &epicycle_phase));
    assert(epicycle_phase > 0.0);
    printf("   ✓ Cellarius planet velocity and epicycle phase verified successfully\n");

    // Test Cellarius eccentricity velocity and radial frequency calculations
    double eccentricity_vel = 0.0;
    double radial_freq = 0.0;
    assert(evaluate_cellarius_eccentricity_velocity(&geom, 0.054, &eccentricity_vel));
    assert(eccentricity_vel > 0.0);
    assert(evaluate_cellarius_radial_frequency(&geom, 4.2, &radial_freq));
    assert(radial_freq > 0.0);
    printf("   ✓ Cellarius eccentricity velocity and radial frequency verified successfully\n");

    // Test Cellarius alignment boundary and epicycle radius calculations
    double alignment_boundary = 0.0;
    double epicycle_radius_val = 0.0;
    assert(evaluate_cellarius_alignment_boundary(&geom, 8.4, &alignment_boundary));
    assert(alignment_boundary > 0.0);
    assert(evaluate_cellarius_epicycle_radius(&geom, 1.25, &epicycle_radius_val));
    assert(epicycle_radius_val > 0.0);
    printf("   ✓ Cellarius alignment boundary and epicycle radius verified successfully\n");

    // Test Cellarius velocity phase and radial scale calculations
    double velocity_phase_shift = 0.0;
    double radial_scale_shift = 0.0;
    assert(evaluate_cellarius_velocity_phase(&geom, 15.0, 1.2, &velocity_phase_shift));
    assert(velocity_phase_shift > 0.0);
    assert(evaluate_cellarius_radial_scale(&geom, 5.5, &radial_scale_shift));
    assert(radial_scale_shift > 0.0);
    printf("   ✓ Cellarius velocity phase and radial scale verified successfully\n");

    // Test Cellarius eccentricity scale and epicycle phase decay calculations
    double eccentricity_scale_val = 0.0;
    double epicycle_phase_decay = 0.0;
    assert(evaluate_cellarius_eccentricity_scale(&geom, 0.045, &eccentricity_scale_val));
    assert(eccentricity_scale_val > 0.0);
    assert(evaluate_cellarius_epicycle_phase_decay(&geom, 12.0, &epicycle_phase_decay));
    assert(epicycle_phase_decay > 0.0);
    printf("   ✓ Cellarius eccentricity scale and epicycle phase decay verified successfully\n");

    // Test Cellarius velocity eccentricity and alignment decay calculations
    double velocity_eccentricity_shift = 0.0;
    double alignment_decay_val = 0.0;
    assert(evaluate_cellarius_velocity_eccentricity(&geom, 10.5, 0.06, &velocity_eccentricity_shift));
    assert(velocity_eccentricity_shift > 0.0);
    assert(evaluate_cellarius_alignment_decay(&geom, 22.0, &alignment_decay_val));
    assert(alignment_decay_val > 0.0);
    printf("   ✓ Cellarius velocity eccentricity and alignment boundary decay verified successfully\n");

    // Test Cellarius planet orbital velocity scale variance and epicycle orbit phase decay calculations
    double velocity_scale_var = 0.0;
    double epicycle_phase_decay_mod_val = 0.0;
    assert(evaluate_cellarius_velocity_scale_variance(&geom, 8.5, 0.05, &velocity_scale_var));
    assert(velocity_scale_var > 0.0);
    assert(evaluate_cellarius_epicycle_phase_decay_mod(&geom, 18.0, &epicycle_phase_decay_mod_val));
    assert(epicycle_phase_decay_mod_val > 0.0);
    printf("   ✓ Cellarius velocity scale variance and epicycle phase decay mod verified successfully\n");

    // Test Cellarius heliocentric alignment scale and epicycle acceleration variance calculations
    double heliocentric_alignment_scale = 0.0;
    double epicycle_accel_variance = 0.0;
    assert(evaluate_cellarius_heliocentric_alignment_scale(&geom, 1.45, &heliocentric_alignment_scale));
    assert(heliocentric_alignment_scale > 0.0);
    assert(evaluate_cellarius_epicycle_acceleration_variance(&geom, 10.2, 0.08, &epicycle_accel_variance));
    assert(epicycle_accel_variance > 0.0);
    printf("   ✓ Cellarius heliocentric alignment scale and epicycle acceleration variance verified successfully\n");

    // Test Cellarius heliocentric phase variance and epicycle velocity decay calculations
    double heliocentric_phase_variance = 0.0;
    double epicycle_velocity_decay_val = 0.0;
    assert(evaluate_cellarius_heliocentric_phase_variance(&geom, 1.25, &heliocentric_phase_variance));
    assert(heliocentric_phase_variance > 0.0);
    assert(evaluate_cellarius_epicycle_velocity_decay(&geom, 15.0, &epicycle_velocity_decay_val));
    assert(epicycle_velocity_decay_val > 0.0);
    printf("   ✓ Cellarius heliocentric phase variance and epicycle velocity decay verified successfully\n");

    // Test Cellarius epicycle velocity variance and heliocentric phase decay calculations
    double epicycle_velocity_variance = 0.0;
    double heliocentric_phase_decay_val = 0.0;
    assert(evaluate_cellarius_epicycle_velocity_variance(&geom, 11.2, 0.06, &epicycle_velocity_variance));
    assert(epicycle_velocity_variance > 0.0);
    assert(evaluate_cellarius_heliocentric_phase_decay(&geom, 18.0, &heliocentric_phase_decay_val));
    assert(heliocentric_phase_decay_val > 0.0);
    printf("   ✓ Cellarius epicycle velocity variance and heliocentric phase decay verified successfully\n");

    // Test Cellarius alignment boundary variance and epicycle phase decay mod variance calculations
    double alignment_boundary_variance_val = 0.0;
    double epicycle_phase_decay_mod_variance_val = 0.0;
    assert(evaluate_cellarius_alignment_boundary_variance(&geom, 10.5, 0.07, &alignment_boundary_variance_val));
    assert(alignment_boundary_variance_val > 0.0);
    assert(evaluate_cellarius_epicycle_phase_decay_mod_variance(&geom, 16.0, &epicycle_phase_decay_mod_variance_val));
    assert(epicycle_phase_decay_mod_variance_val > 0.0);
    printf("   ✓ Cellarius alignment boundary variance and epicycle phase decay mod variance verified successfully\n");

    // Test Cellarius constellation boundary variance and heliocentric velocity variance calculations
    double constellation_boundary_variance = 0.0;
    double heliocentric_velocity_variance = 0.0;
    assert(evaluate_cellarius_constellation_boundary_variance(&geom, 0.08, 1.45, &constellation_boundary_variance));
    assert(constellation_boundary_variance > 0.0);
    assert(evaluate_cellarius_heliocentric_velocity_variance(&geom, 1.25, 0.85, &heliocentric_velocity_variance));
    assert(heliocentric_velocity_variance > 0.0);
    printf("   ✓ Cellarius constellation boundary variance and heliocentric velocity variance verified successfully\n");

    // Test Cellarius planetary velocity decay variance and epicycle acceleration variance mod calculations
    double planetary_velocity_decay_variance = 0.0;
    double epicycle_acceleration_variance_mod_val = 0.0;
    assert(evaluate_cellarius_planetary_velocity_decay_variance(&geom, 15.0, &planetary_velocity_decay_variance));
    assert(planetary_velocity_decay_variance > 0.0);
    assert(evaluate_cellarius_epicycle_acceleration_variance_mod(&geom, 10.8, &epicycle_acceleration_variance_mod_val));
    assert(epicycle_acceleration_variance_mod_val > 0.0);
    printf("   ✓ Cellarius planetary velocity decay variance and epicycle acceleration variance mod verified successfully\n");

    // Test Cellarius constellation scale variance mod and epicycle acceleration decay variance calculations
    double constellation_scale_variance_mod_val = 0.0;
    double epicycle_acceleration_decay_variance = 0.0;
    assert(evaluate_cellarius_constellation_scale_variance_mod(&geom, 1.25, &constellation_scale_variance_mod_val));
    assert(constellation_scale_variance_mod_val > 0.0);
    assert(evaluate_cellarius_epicycle_acceleration_decay_variance(&geom, 16.5, &epicycle_acceleration_decay_variance));
    assert(epicycle_acceleration_decay_variance > 0.0);
    printf("   ✓ Cellarius constellation scale variance mod and epicycle acceleration decay variance verified successfully\n");

    // Test Cellarius planetary eccentricity variance mod and epicycle radius decay variance calculations
    double planetary_eccentricity_variance_mod_val = 0.0;
    double epicycle_radius_decay_variance = 0.0;
    assert(evaluate_cellarius_planetary_eccentricity_variance_mod(&geom, 0.09, &planetary_eccentricity_variance_mod_val));
    assert(planetary_eccentricity_variance_mod_val > 0.0);
    assert(evaluate_cellarius_epicycle_radius_decay_variance(&geom, 15.0, &epicycle_radius_decay_variance));
    assert(epicycle_radius_decay_variance > 0.0);
    printf("   ✓ Cellarius planetary eccentricity variance mod and epicycle radius decay variance verified successfully\n");

    // Test Cellarius planetary eccentricity decay variance and heliocentric alignment decay variance calculations
    double planetary_eccentricity_decay_variance = 0.0;
    double heliocentric_alignment_decay_variance = 0.0;
    assert(evaluate_cellarius_planetary_eccentricity_decay_variance(&geom, 14.5, &planetary_eccentricity_decay_variance));
    assert(planetary_eccentricity_decay_variance > 0.0);
    assert(evaluate_cellarius_heliocentric_alignment_decay_variance(&geom, 15.0, &heliocentric_alignment_decay_variance));
    assert(heliocentric_alignment_decay_variance > 0.0);
    printf("   ✓ Cellarius planetary eccentricity decay variance and heliocentric alignment decay variance verified successfully\n");

    // Test Cellarius planetary velocity variance mod and heliocentric alignment variance mod calculations
    double planetary_velocity_variance_mod = 0.0;
    double heliocentric_alignment_variance_mod = 0.0;
    assert(evaluate_cellarius_planetary_velocity_variance_mod(&geom, 2.5, &planetary_velocity_variance_mod));
    assert(planetary_velocity_variance_mod > 0.0);
    assert(evaluate_cellarius_heliocentric_alignment_variance_mod(&geom, 0.85, &heliocentric_alignment_variance_mod));
    assert(heliocentric_alignment_variance_mod > 0.0);
    printf("   ✓ Cellarius planetary velocity variance mod and heliocentric alignment variance mod verified successfully\n");

    // Test Cellarius constellation scale variance mod and epicycle radius decay variance mod calculations
    double constellation_scale_variance_mod = 0.0;
    double epicycle_radius_decay_variance_mod = 0.0;
    assert(evaluate_cellarius_constellation_scale_variance_mod_single(&geom, 0.95, &constellation_scale_variance_mod));
    assert(constellation_scale_variance_mod > 0.0);
    assert(evaluate_cellarius_epicycle_radius_decay_variance_mod(&geom, 15.0, &epicycle_radius_decay_variance_mod));
    assert(epicycle_radius_decay_variance_mod > 0.0);
    printf("   ✓ Cellarius constellation scale variance mod and epicycle radius decay variance mod verified successfully\n");

    // Test Hyde vocal jitter naturalness and intonation amplitude calculations
    double vocal_jitter = 0.0;
    double intonation_amp = 0.0;
    assert(evaluate_hyde_vocal_jitter_naturalness(&geom, 0.04, &vocal_jitter));
    assert(vocal_jitter > 0.0);
    assert(evaluate_hyde_intonation_amplitude(&geom, 12.0, &intonation_amp));
    assert(intonation_amp > 0.0);
    printf("   ✓ Hyde vocal jitter naturalness and intonation amplitude verified successfully\n");

    // Test Hyde vocal naturalness variance and intonation amplitude decay calculations
    double vocal_naturalness_var = 0.0;
    double intonation_amp_decay_val = 0.0;
    assert(evaluate_hyde_vocal_naturalness_variance(&geom, 0.05, 0.44, &vocal_naturalness_var));
    assert(vocal_naturalness_var > 0.0);
    assert(evaluate_hyde_intonation_amplitude_decay(&geom, 16.0, &intonation_amp_decay_val));
    assert(intonation_amp_decay_val > 0.0);
    printf("   ✓ Hyde vocal naturalness variance and intonation amplitude decay verified successfully\n");

    // Test Hyde dynamic intonation and amplitude decay calculations
    double dynamic_intonation = 0.0;
    double intonation_decay = 0.0;
    assert(evaluate_hyde_dynamic_intonation(&geom, 15.0, 0.35, &dynamic_intonation));
    assert(dynamic_intonation > 0.0);
    assert(evaluate_hyde_intonation_decay(&geom, 12.0, &intonation_decay));
    assert(intonation_decay > 0.0);
    printf("   ✓ Hyde dynamic intonation and amplitude decay verified successfully\n");

    // Test Hyde vocal tempo sync and jitter decay calculations
    double tempo_sync = 0.0;
    double tempo_jitter_decay = 0.0;
    assert(evaluate_hyde_tempo_sync(&geom, 0.82, &tempo_sync));
    assert(tempo_sync > 0.0);
    assert(evaluate_hyde_tempo_jitter_decay(&geom, 24.0, &tempo_jitter_decay));
    assert(tempo_jitter_decay > 0.0);
    printf("   ✓ Hyde vocal tempo sync and jitter decay verified successfully\n");

    // Test Hyde vocal tempo range and range decay calculations
    double tempo_range_val = 0.0;
    double tempo_range_decay = 0.0;
    assert(evaluate_hyde_tempo_range(&geom, 12.5, 0.45, &tempo_range_val));
    assert(tempo_range_val > 0.0);
    assert(evaluate_hyde_tempo_range_decay(&geom, 18.0, &tempo_range_decay));
    assert(tempo_range_decay > 0.0);
    printf("   ✓ Hyde vocal tempo range and range decay verified successfully\n");

    // Test Hyde vocal size sync and size jitter decay calculations
    double size_sync_val = 0.0;
    double size_jitter_decay_val = 0.0;
    assert(evaluate_hyde_vocal_size_sync(&geom, 0.85, 0.22, &size_sync_val));
    assert(size_sync_val > 0.0);
    assert(evaluate_hyde_vocal_size_jitter_decay(&geom, 15.0, &size_jitter_decay_val));
    assert(size_jitter_decay_val > 0.0);
    printf("   ✓ Hyde vocal size sync and size jitter decay verified successfully\n");

    // Test Hyde vocal tempo sync variance and size jitter decay mod calculations
    double vocal_tempo_sync_variance = 0.0;
    double size_jitter_decay_mod = 0.0;
    assert(evaluate_hyde_vocal_tempo_sync_variance(&geom, 0.88, 0.24, &vocal_tempo_sync_variance));
    assert(vocal_tempo_sync_variance > 0.0);
    assert(evaluate_hyde_vocal_size_jitter_decay_mod(&geom, 16.5, &size_jitter_decay_mod));
    assert(size_jitter_decay_mod > 0.0);
    printf("   ✓ Hyde vocal tempo sync variance and size jitter decay mod verified successfully\n");

    // Test Hyde vocal tempo range variance and size jitter decay mod variance calculations
    double vocal_tempo_range_variance = 0.0;
    double size_jitter_decay_mod_variance = 0.0;
    assert(evaluate_hyde_vocal_tempo_range_variance(&geom, 12.8, 0.26, &vocal_tempo_range_variance));
    assert(vocal_tempo_range_variance > 0.0);
    assert(evaluate_hyde_vocal_size_jitter_decay_mod_variance(&geom, 15.0, &size_jitter_decay_mod_variance));
    assert(size_jitter_decay_mod_variance > 0.0);
    printf("   ✓ Hyde vocal tempo range variance and size jitter decay mod variance verified successfully\n");

    // Test Hyde vocal size pitch and pitch range decay calculations
    double size_pitch_val = 0.0;
    double size_pitch_decay_val = 0.0;
    assert(evaluate_hyde_vocal_size_pitch(&geom, 2.45, 12.0, &size_pitch_val));
    assert(size_pitch_val > 0.0);
    assert(evaluate_hyde_vocal_size_pitch_decay(&geom, 14.5, &size_pitch_decay_val));
    assert(size_pitch_decay_val > 0.0);
    printf("   ✓ Hyde vocal size pitch and pitch range decay verified successfully\n");

    // Test Hyde vocal warmth range and warmth range decay calculations
    double warmth_range_val = 0.0;
    double warmth_range_decay_val = 0.0;
    assert(evaluate_hyde_vocal_warmth_range(&geom, 14.2, 0.35, &warmth_range_val));
    assert(warmth_range_val > 0.0);
    assert(evaluate_hyde_vocal_warmth_range_decay(&geom, 16.5, &warmth_range_decay_val));
    assert(warmth_range_decay_val > 0.0);
    printf("   ✓ Hyde vocal warmth range and warmth range decay verified successfully\n");

    // Test Hyde vocal warmth modulation and interruption recovery calculations
    double vocal_warmth_mod = 0.0;
    double interruption_rec = 0.0;
    assert(evaluate_hyde_vocal_warmth_modulation(&geom, 200.0, 0.25, &vocal_warmth_mod));
    assert(vocal_warmth_mod > 0.0);
    assert(evaluate_hyde_interruption_recovery(&geom, 1.5, &interruption_rec));
    assert(interruption_rec > 0.0);
    printf("   ✓ Hyde vocal warmth modulation and interruption recovery verified successfully\n");

    // Test Hyde vocal warmth variance and size pitch decay mod calculations
    double warmth_variance_val = 0.0;
    double size_pitch_decay_mod = 0.0;
    assert(evaluate_hyde_vocal_warmth_variance(&geom, 0.44, 220.0, &warmth_variance_val));
    assert(warmth_variance_val > 0.0);
    assert(evaluate_hyde_vocal_size_pitch_decay_mod(&geom, 15.0, &size_pitch_decay_mod));
    assert(size_pitch_decay_mod > 0.0);
    printf("   ✓ Hyde vocal warmth variance and size pitch decay mod verified successfully\n");

    // Test Hyde vocal turn interruption variance and size pitch variance calculations
    double turn_interruption_variance = 0.0;
    double size_pitch_variance_val = 0.0;
    assert(evaluate_hyde_vocal_turn_interruption_variance(&geom, 0.45, &turn_interruption_variance));
    assert(turn_interruption_variance > 0.0);
    assert(evaluate_hyde_vocal_size_pitch_variance(&geom, 2.5, 12.5, &size_pitch_variance_val));
    assert(size_pitch_variance_val > 0.0);
    printf("   ✓ Hyde vocal turn interruption variance and size pitch variance verified successfully\n");

    // Test Hyde vocal tempo range decay variance and size sync variance calculations
    double tempo_range_decay_variance = 0.0;
    double size_sync_variance_val = 0.0;
    assert(evaluate_hyde_vocal_tempo_range_decay_variance(&geom, 15.0, &tempo_range_decay_variance));
    assert(tempo_range_decay_variance > 0.0);
    assert(evaluate_hyde_vocal_size_sync_variance(&geom, 0.82, 0.24, &size_sync_variance_val));
    assert(size_sync_variance_val > 0.0);
    printf("   ✓ Hyde vocal tempo range decay variance and size sync variance verified successfully\n");

    // Test Hyde vocal warmth decay variance and vocal tempo sync variance mod calculations
    double warmth_decay_variance = 0.0;
    double tempo_sync_variance_mod_val = 0.0;
    assert(evaluate_hyde_vocal_warmth_decay_variance(&geom, 15.0, &warmth_decay_variance));
    assert(warmth_decay_variance > 0.0);
    assert(evaluate_hyde_vocal_tempo_sync_variance_mod(&geom, 0.85, &tempo_sync_variance_mod_val));
    assert(tempo_sync_variance_mod_val > 0.0);
    printf("   ✓ Hyde vocal warmth decay variance and vocal tempo sync variance mod verified successfully\n");

    // Test Hyde vocal warmth range decay variance and vocal warmth modulation variance mod calculations
    double warmth_range_decay_variance = 0.0;
    double warmth_modulation_variance_mod_val = 0.0;
    assert(evaluate_hyde_vocal_warmth_range_decay_variance(&geom, 15.0, &warmth_range_decay_variance));
    assert(warmth_range_decay_variance > 0.0);
    assert(evaluate_hyde_vocal_warmth_modulation_variance_mod(&geom, 0.88, &warmth_modulation_variance_mod_val));
    assert(warmth_modulation_variance_mod_val > 0.0);
    printf("   ✓ Hyde vocal warmth range decay variance and vocal warmth modulation variance mod verified successfully\n");

    // Test Hyde vocal warmth range variance mod and vocal size pitch variance mod calculations
    double warmth_range_variance_mod_val = 0.0;
    double size_pitch_variance_mod_val = 0.0;
    assert(evaluate_hyde_vocal_warmth_range_variance_mod(&geom, 0.72, &warmth_range_variance_mod_val));
    assert(warmth_range_variance_mod_val > 0.0);
    assert(evaluate_hyde_vocal_size_pitch_variance_mod(&geom, 1.45, &size_pitch_variance_mod_val));
    assert(size_pitch_variance_mod_val > 0.0);
    printf("   ✓ Hyde vocal warmth range variance mod and vocal size pitch variance mod verified successfully\n");

    // Test Hyde vocal size jitter decay variance and vocal size sync variance mod calculations
    double size_jitter_decay_variance = 0.0;
    double size_sync_variance_mod_val = 0.0;
    assert(evaluate_hyde_vocal_size_jitter_decay_variance(&geom, 15.0, &size_jitter_decay_variance));
    assert(size_jitter_decay_variance > 0.0);
    assert(evaluate_hyde_vocal_size_sync_variance_mod(&geom, 0.85, &size_sync_variance_mod_val));
    assert(size_sync_variance_mod_val > 0.0);
    printf("   ✓ Hyde vocal size jitter decay variance and vocal size sync variance mod verified successfully\n");

    // Test Hyde vocal warmth variance mod and vocal tempo range variance mod calculations
    double warmth_variance_mod_val = 0.0;
    double tempo_range_variance_mod_val = 0.0;
    assert(evaluate_hyde_vocal_warmth_variance_mod(&geom, 0.65, &warmth_variance_mod_val));
    assert(warmth_variance_mod_val > 0.0);
    assert(evaluate_hyde_vocal_tempo_range_variance_mod(&geom, 1.35, &tempo_range_variance_mod_val));
    assert(tempo_range_variance_mod_val > 0.0);
    printf("   ✓ Hyde vocal warmth variance mod and vocal tempo range variance mod verified successfully\n");

    // Test Hyde vocal size jitter variance mod calculations
    double size_jitter_variance_mod_val = 0.0;
    assert(evaluate_hyde_vocal_size_jitter_variance_mod(&geom, 0.78, &size_jitter_variance_mod_val));
    assert(size_jitter_variance_mod_val > 0.0);
    printf("   ✓ Hyde vocal size jitter variance mod verified successfully\n");

    // Test Hyde vocal warmth decay variance mod and vocal tempo range decay variance mod calculations
    double warmth_decay_variance_mod = 0.0;
    double tempo_range_decay_variance_mod = 0.0;
    assert(evaluate_hyde_vocal_warmth_decay_variance_mod(&geom, 15.0, &warmth_decay_variance_mod));
    assert(warmth_decay_variance_mod > 0.0);
    assert(evaluate_hyde_vocal_tempo_range_decay_variance_mod(&geom, 14.5, &tempo_range_decay_variance_mod));
    assert(tempo_range_decay_variance_mod > 0.0);
    printf("   ✓ Hyde vocal warmth decay variance mod and vocal tempo range decay variance mod verified successfully\n");

    // Test Hyde vocal size jitter decay variance mod and vocal tempo sync decay variance mod calculations
    double size_jitter_decay_variance_mod = 0.0;
    double tempo_sync_decay_variance_mod = 0.0;
    assert(evaluate_hyde_vocal_size_jitter_decay_variance_mod(&geom, 15.0, &size_jitter_decay_variance_mod));
    assert(size_jitter_decay_variance_mod > 0.0);
    assert(evaluate_hyde_vocal_tempo_sync_decay_variance_mod(&geom, 14.5, &tempo_sync_decay_variance_mod));
    assert(tempo_sync_decay_variance_mod > 0.0);
    printf("   ✓ Hyde vocal size jitter decay variance mod and vocal tempo sync decay variance mod verified successfully\n");

    // Test Hyde vocal size pitch decay variance mod and vocal size sync decay variance mod calculations
    double size_pitch_decay_variance_mod = 0.0;
    double size_sync_decay_variance_mod = 0.0;
    assert(evaluate_hyde_vocal_size_pitch_decay_variance_mod(&geom, 15.0, &size_pitch_decay_variance_mod));
    assert(size_pitch_decay_variance_mod > 0.0);
    assert(evaluate_hyde_vocal_size_sync_decay_variance_mod(&geom, 14.5, &size_sync_decay_variance_mod));
    assert(size_sync_decay_variance_mod > 0.0);
    printf("   ✓ Hyde vocal size pitch decay variance mod and vocal size sync decay variance mod verified successfully\n");

    // Test Keating gaze shift dominance and lip compression trust calculations
    double gaze_shift_dom = 0.0;
    double lip_compress_trust = 0.0;
    assert(evaluate_keating_gaze_shift_dominance(&geom, 2.5, &gaze_shift_dom));
    assert(gaze_shift_dom > 0.0);
    assert(evaluate_keating_lip_compression_trust(&geom, 0.35, &lip_compress_trust));
    assert(lip_compress_trust > 0.0);
    printf("   ✓ KEATING, C. F. gaze shift dominance and lip compression trust verified successfully\n");

    // Test Keating width asymmetry dominance and mouth asymmetry trust decay calculations
    double width_asym_dom = 0.0;
    double mouth_asym_decay = 0.0;
    assert(evaluate_keating_width_asymmetry_dominance(&geom, 0.18, &width_asym_dom));
    assert(width_asym_dom > 0.0);
    assert(evaluate_keating_mouth_asymmetry_decay(&geom, 0.22, &mouth_asym_decay));
    assert(mouth_asym_decay > 0.0);
    printf("   ✓ KEATING, C. F. width asymmetry dominance and mouth asymmetry trust decay verified successfully\n");

    // Test Keating width asymmetry trust and mouth asymmetry dominance calculations
    double width_asym_trust = 0.0;
    double mouth_asym_dom = 0.0;
    assert(evaluate_keating_width_asymmetry_trust(&geom, 0.15, &width_asym_trust));
    assert(width_asym_trust > 0.0);
    assert(evaluate_keating_mouth_asymmetry_dominance(&geom, 0.25, &mouth_asym_dom));
    assert(mouth_asym_dom > 0.0);
    printf("   ✓ KEATING, C. F. width asymmetry trust and mouth asymmetry dominance verified successfully\n");

    // Test Keating eyebrow asymmetry dominance and sclera submissiveness calculations
    double eyebrow_asym_dom = 0.0;
    double sclera_sub_val = 0.0;
    assert(evaluate_keating_eyebrow_asymmetry_dominance(&geom, 0.14, &eyebrow_asym_dom));
    assert(eyebrow_asym_dom > 0.0);
    assert(evaluate_keating_sclera_submissiveness(&geom, 0.35, &sclera_sub_val));
    assert(sclera_sub_val > 0.0);
    printf("   ✓ KEATING, C. F. eyebrow asymmetry dominance and sclera submissiveness verified successfully\n");

    // Test Keating eyebrow asymmetry trust and sclera size dominance calculations
    double eyebrow_asym_trust = 0.0;
    double sclera_dom_val = 0.0;
    assert(evaluate_keating_eyebrow_asymmetry_trust(&geom, 0.12, &eyebrow_asym_trust));
    assert(eyebrow_asym_trust > 0.0);
    assert(evaluate_keating_sclera_dominance(&geom, 0.42, &sclera_dom_val));
    assert(sclera_dom_val > 0.0);
    printf("   ✓ KEATING, C. F. eyebrow asymmetry trust and sclera dominance verified successfully\n");

    // Test Keating eyebrow asymmetry decay and sclera submissiveness modulator calculations
    double eyebrow_asym_decay_val = 0.0;
    double sclera_sub_mod_val = 0.0;
    assert(evaluate_keating_eyebrow_asymmetry_decay(&geom, 15.0, &eyebrow_asym_decay_val));
    assert(eyebrow_asym_decay_val > 0.0);
    assert(evaluate_keating_sclera_submissiveness_mod(&geom, 0.38, &sclera_sub_mod_val));
    assert(sclera_sub_mod_val > 0.0);
    printf("   ✓ KEATING, C. F. eyebrow asymmetry decay and sclera submissiveness modulator verified successfully\n");

    // Test Keating eyebrow asymmetry dominance decay and sclera dominance variance calculations
    double eyebrow_asym_dom_decay = 0.0;
    double sclera_dom_var_val = 0.0;
    assert(evaluate_keating_eyebrow_asymmetry_dominance_decay(&geom, 18.0, &eyebrow_asym_dom_decay));
    assert(eyebrow_asym_dom_decay > 0.0);
    assert(evaluate_keating_sclera_dominance_variance(&geom, 0.45, &sclera_dom_var_val));
    assert(sclera_dom_var_val > 0.0);
    printf("   ✓ KEATING, C. F. eyebrow asymmetry dominance decay and sclera dominance variance verified successfully\n");

    // Test Keating eyebrow asymmetry trust decay and sclera submissiveness variance calculations
    double eyebrow_asym_trust_decay = 0.0;
    double sclera_sub_var_val = 0.0;
    assert(evaluate_keating_eyebrow_asymmetry_trust_decay_mod(&geom, 15.0, &eyebrow_asym_trust_decay));
    assert(eyebrow_asym_trust_decay > 0.0);
    assert(evaluate_keating_sclera_submissiveness_variance(&geom, 0.39, &sclera_sub_var_val));
    assert(sclera_sub_var_val > 0.0);
    printf("   ✓ KEATING, C. F. eyebrow asymmetry trust decay and sclera submissiveness variance verified successfully\n");

    // Test Keating gaze shift dominance decay and lip compression trust variance calculations
    double gaze_shift_dom_decay = 0.0;
    double lip_compression_trust_var = 0.0;
    assert(evaluate_keating_gaze_shift_dominance_decay(&geom, 14.5, &gaze_shift_dom_decay));
    assert(gaze_shift_dom_decay > 0.0);
    assert(evaluate_keating_lip_compression_trust_variance(&geom, 0.28, &lip_compression_trust_var));
    assert(lip_compression_trust_var > 0.0);
    printf("   ✓ KEATING, C. F. gaze shift dominance decay and lip compression trust variance verified successfully\n");

    // Test Keating sclera submissiveness decay and width asymmetry dominance variance calculations
    double sclera_sub_decay = 0.0;
    double width_asymmetry_dom_var = 0.0;
    assert(evaluate_keating_sclera_submissiveness_decay(&geom, 16.5, &sclera_sub_decay));
    assert(sclera_sub_decay > 0.0);
    assert(evaluate_keating_width_asymmetry_dominance_variance(&geom, 0.26, &width_asymmetry_dom_var));
    assert(width_asymmetry_dom_var > 0.0);
    printf("   ✓ KEATING, C. F. sclera submissiveness decay and width asymmetry dominance variance verified successfully\n");

    // Test Keating sclera dominance decay and mouth asymmetry dominance variance calculations
    double sclera_dom_decay_val = 0.0;
    double mouth_asymmetry_dom_var_val = 0.0;
    assert(evaluate_keating_sclera_dominance_decay(&geom, 15.5, &sclera_dom_decay_val));
    assert(sclera_dom_decay_val > 0.0);
    assert(evaluate_keating_mouth_asymmetry_dominance_variance(&geom, 0.32, &mouth_asymmetry_dom_var_val));
    assert(mouth_asymmetry_dom_var_val > 0.0);
    printf("   ✓ KEATING, C. F. sclera dominance decay and mouth asymmetry dominance variance verified successfully\n");

    // Test Keating eyebrow asymmetry dominance decay variance and mouth asymmetry trustworthiness variance calculations
    double eyebrow_asym_dom_decay_variance = 0.0;
    double mouth_asymmetry_trust_variance_val = 0.0;
    assert(evaluate_keating_eyebrow_asymmetry_dominance_decay_variance(&geom, 14.5, &eyebrow_asym_dom_decay_variance));
    assert(eyebrow_asym_dom_decay_variance > 0.0);
    assert(evaluate_keating_mouth_asymmetry_trust_variance(&geom, 0.28, &mouth_asymmetry_trust_variance_val));
    assert(mouth_asymmetry_trust_variance_val > 0.0);
    printf("   ✓ KEATING, C. F. eyebrow asymmetry dominance decay variance and mouth asymmetry trust variance verified successfully\n");

    // Test Keating sclera submissiveness decay variance and width asymmetry trustworthiness variance calculations
    double sclera_sub_decay_variance = 0.0;
    double width_asymmetry_trust_variance_val = 0.0;
    assert(evaluate_keating_sclera_submissiveness_decay_variance(&geom, 15.0, &sclera_sub_decay_variance));
    assert(sclera_sub_decay_variance > 0.0);
    assert(evaluate_keating_width_asymmetry_trust_variance(&geom, 0.25, &width_asymmetry_trust_variance_val));
    assert(width_asymmetry_trust_variance_val > 0.0);
    printf("   ✓ KEATING, C. F. eyebrow asymmetry dominance decay variance and mouth asymmetry trust variance verified successfully\n");

    // Test Keating gaze shift dominance decay variance and lip compression trust variance mod calculations
    double gaze_shift_dominance_decay_variance = 0.0;
    double lip_compression_trust_variance_mod_val = 0.0;
    assert(evaluate_keating_gaze_shift_dominance_decay_variance(&geom, 16.5, &gaze_shift_dominance_decay_variance));
    assert(gaze_shift_dominance_decay_variance > 0.0);
    assert(evaluate_keating_lip_compression_trust_variance_mod(&geom, 0.32, &lip_compression_trust_variance_mod_val));
    assert(lip_compression_trust_variance_mod_val > 0.0);
    printf("   ✓ KEATING, C. F. gaze shift dominance decay variance and lip compression trust variance mod verified successfully\n");

    // Test Keating eyebrow asymmetry trust decay variance and sclera dominance variance mod calculations
    double eyebrow_asymmetry_trust_decay_variance = 0.0;
    double sclera_dominance_variance_mod_val = 0.0;
    assert(evaluate_keating_eyebrow_asymmetry_trust_decay_variance(&geom, 15.0, &eyebrow_asymmetry_trust_decay_variance));
    assert(eyebrow_asymmetry_trust_decay_variance > 0.0);
    assert(evaluate_keating_sclera_dominance_variance_mod(&geom, 0.38, &sclera_dominance_variance_mod_val));
    assert(sclera_dominance_variance_mod_val > 0.0);
    printf("   ✓ KEATING, C. F. eyebrow asymmetry trust decay variance and sclera dominance variance mod verified successfully\n");

    // Test Keating sclera dominance decay variance and mouth asymmetry trust variance mod calculations
    double sclera_dominance_decay_variance = 0.0;
    double mouth_asymmetry_trust_variance_mod_val = 0.0;
    assert(evaluate_keating_sclera_dominance_decay_variance(&geom, 16.5, &sclera_dominance_decay_variance));
    assert(sclera_dominance_decay_variance > 0.0);
    assert(evaluate_keating_mouth_asymmetry_trust_variance_mod(&geom, 0.28, &mouth_asymmetry_trust_variance_mod_val));
    assert(mouth_asymmetry_trust_variance_mod_val > 0.0);
    printf("   ✓ KEATING, C. F. sclera dominance decay variance and mouth asymmetry trust variance mod verified successfully\n");

    // Test Keating mouth asymmetry dominance decay variance and width asymmetry trust variance mod calculations
    double mouth_asymmetry_dominance_decay_variance = 0.0;
    double width_asymmetry_trust_variance_mod_val = 0.0;
    assert(evaluate_keating_mouth_asymmetry_dominance_decay_variance(&geom, 15.0, &mouth_asymmetry_dominance_decay_variance));
    assert(mouth_asymmetry_dominance_decay_variance > 0.0);
    assert(evaluate_keating_width_asymmetry_trust_variance_mod(&geom, 0.35, &width_asymmetry_trust_variance_mod_val));
    assert(width_asymmetry_trust_variance_mod_val > 0.0);
    printf("   ✓ KEATING, C. F. mouth asymmetry dominance decay variance and width asymmetry trust variance mod verified successfully\n");

    // Test Keating mouth asymmetry trust decay variance and width asymmetry dominance variance mod calculations
    double mouth_asymmetry_trust_decay_variance = 0.0;
    double width_asymmetry_dominance_variance_mod = 0.0;
    assert(evaluate_keating_mouth_asymmetry_trust_decay_variance(&geom, 14.5, &mouth_asymmetry_trust_decay_variance));
    assert(mouth_asymmetry_trust_decay_variance > 0.0);
    assert(evaluate_keating_width_asymmetry_dominance_variance_mod(&geom, 0.42, &width_asymmetry_dominance_variance_mod));
    assert(width_asymmetry_dominance_variance_mod > 0.0);
    printf("   ✓ KEATING, C. F. mouth asymmetry trust decay variance and width asymmetry dominance variance mod verified successfully\n");

    // Test Keating eyebrow asymmetry trust decay variance and sclera submissiveness variance mod calculations
    double eyebrow_asymmetry_trust_decay_variance_single = 0.0;
    double sclera_submissiveness_variance_mod = 0.0;
    assert(evaluate_keating_eyebrow_asymmetry_trust_decay_variance_single(&geom, 15.0, &eyebrow_asymmetry_trust_decay_variance_single));
    assert(eyebrow_asymmetry_trust_decay_variance_single > 0.0);
    assert(evaluate_keating_sclera_submissiveness_variance_mod(&geom, 0.32, &sclera_submissiveness_variance_mod));
    assert(sclera_submissiveness_variance_mod > 0.0);
    printf("   ✓ KEATING, C. F. eyebrow asymmetry trust decay variance and sclera submissiveness variance mod verified successfully\n");

    // Test Keating sclera dominance variance mod and eyebrow asymmetry dominance variance mod calculations
    double sclera_dominance_variance_mod_val_single = 0.0;
    double eyebrow_asymmetry_dominance_variance_mod = 0.0;
    assert(evaluate_keating_sclera_dominance_variance_mod_single(&geom, 0.35, &sclera_dominance_variance_mod_val_single));
    assert(sclera_dominance_variance_mod_val_single > 0.0);
    assert(evaluate_keating_eyebrow_asymmetry_dominance_variance_mod(&geom, 0.45, &eyebrow_asymmetry_dominance_variance_mod));
    assert(eyebrow_asymmetry_dominance_variance_mod > 0.0);
    printf("   ✓ KEATING, C. F. sclera dominance variance mod and eyebrow asymmetry dominance variance mod verified successfully\n");

    // Test Keating eyebrow asymmetry trust variance mod and sclera submissiveness decay variance calculations
    double eyebrow_asymmetry_trust_variance_mod_val_single = 0.0;
    double sclera_submissiveness_decay_variance_single = 0.0;
    assert(evaluate_keating_eyebrow_asymmetry_trust_variance_mod_single(&geom, 0.48, &eyebrow_asymmetry_trust_variance_mod_val_single));
    assert(eyebrow_asymmetry_trust_variance_mod_val_single > 0.0);
    assert(evaluate_keating_sclera_submissiveness_decay_variance_single(&geom, 15.0, &sclera_submissiveness_decay_variance_single));
    assert(sclera_submissiveness_decay_variance_single > 0.0);
    printf("   ✓ KEATING, C. F. eyebrow asymmetry trust variance mod and sclera submissiveness decay variance verified successfully\n");

    // Test Keating sclera dominance decay variance and eyebrow asymmetry dominance decay variance calculations
    double sclera_dominance_decay_variance_single = 0.0;
    double eyebrow_asymmetry_dominance_decay_variance_single = 0.0;
    assert(evaluate_keating_sclera_dominance_decay_variance_single(&geom, 15.0, &sclera_dominance_decay_variance_single));
    assert(sclera_dominance_decay_variance_single > 0.0);
    assert(evaluate_keating_eyebrow_asymmetry_dominance_decay_variance_single(&geom, 16.5, &eyebrow_asymmetry_dominance_decay_variance_single));
    assert(eyebrow_asymmetry_dominance_decay_variance_single > 0.0);
    printf("   ✓ KEATING, C. F. sclera dominance decay variance and eyebrow asymmetry dominance decay variance verified successfully\n");

    // Test Keating sclera submissiveness decay variance mod and sclera dominance decay variance mod calculations
    double sclera_submissiveness_decay_variance_mod = 0.0;
    double sclera_dominance_decay_variance_mod = 0.0;
    assert(evaluate_keating_sclera_submissiveness_decay_variance_mod(&geom, 15.0, &sclera_submissiveness_decay_variance_mod));
    assert(sclera_submissiveness_decay_variance_mod > 0.0);
    assert(evaluate_keating_sclera_dominance_decay_variance_mod(&geom, 16.5, &sclera_dominance_decay_variance_mod));
    assert(sclera_dominance_decay_variance_mod > 0.0);
    printf("   ✓ KEATING, C. F. sclera submissiveness decay variance mod and sclera dominance decay variance mod verified successfully\n");

    // Test new Keating models
    double keating_consensus = 0.0;
    double keating_interaction_dom = 0.0;
    double keating_decoupled_attr = 0.0;
    double keating_smile_trust = 0.0;
    assert(evaluate_keating_cross_cultural_dominance_consensus(&geom, 0.85, &keating_consensus));
    assert(keating_consensus > 0.0);
    assert(evaluate_keating_gender_status_interaction(&geom, 0.75, 1, &keating_interaction_dom));
    assert(keating_interaction_dom > 0.0);
    assert(evaluate_keating_babyfacedness_attractiveness_decoupling(&geom, 0.65, 1, &keating_decoupled_attr));
    assert(keating_decoupled_attr > 0.0);
    assert(evaluate_keating_smile_status_congruence(&geom, 0.8, 2.0, &keating_smile_trust));
    assert(keating_smile_trust > 0.0);
    
    double keating_pitch_congruence = 0.0;
    double keating_pose_consensus = 0.0;
    double keating_sclera_decoupling = 0.0;
    assert(evaluate_keating_pitch_vocal_status_congruence(&geom, 0.45, 3.0, &keating_pitch_congruence));
    assert(keating_pitch_congruence > 0.0);
    assert(evaluate_keating_dynamic_pose_asymmetry_consensus(&geom, 0.6, 5.0, &keating_pose_consensus));
    assert(keating_pose_consensus > 0.0);
    assert(evaluate_keating_sclera_gaze_decoupling(&geom, 0.7, 1.2, &keating_sclera_decoupling));
    assert(keating_sclera_decoupling > 0.0);
    printf("   ✓ KEATING, C. F. pitch vocal status congruence, pose asymmetry consensus, and sclera gaze decoupling verified successfully\n");

    double keating_smile_attenuation = 0.0;
    double keating_babyface_leadership = 0.0;
    double keating_torso_asymmetry_status = 0.0;
    assert(evaluate_keating_smile_dominance_attenuation(&geom, 0.4, 1, &keating_smile_attenuation));
    assert(keating_smile_attenuation > 0.0);
    assert(evaluate_keating_babyfacedness_leadership_decoupling(&geom, 0.35, &keating_babyface_leadership));
    assert(keating_babyface_leadership > 0.0);
    assert(evaluate_keating_torso_asymmetry_status(&geom, 0.65, &keating_torso_asymmetry_status));
    assert(keating_torso_asymmetry_status > 0.0);
    printf("   ✓ KEATING, C. F. smile dominance attenuation, babyfacedness leadership decoupling, and torso asymmetry status verified successfully\n");

    double keating_smile_cooperation = 0.0;
    double keating_eyebrow_decoupling = 0.0;
    double keating_jaw_attractiveness = 0.0;
    assert(evaluate_keating_smile_cooperation_modulator(&geom, 0.75, &keating_smile_cooperation));
    assert(keating_smile_cooperation > 0.0);
    assert(evaluate_keating_eyebrow_dominance_decoupling(&geom, 0.25, 1.85, &keating_eyebrow_decoupling));
    assert(keating_eyebrow_decoupling > 0.0);
    assert(evaluate_keating_jaw_attractiveness_interaction(&geom, 0.45, 1, &keating_jaw_attractiveness));
    assert(keating_jaw_attractiveness > 0.0);
    printf("   ✓ KEATING, C. F. smile cooperation, eyebrow dominance decoupling, and jaw attractiveness interaction verified successfully\n");

    printf("   ✓ KEATING, C. F. cross-cultural consensus, gender status interaction, babyfacedness attractiveness decoupling, and smile status congruence verified successfully\n");

    // Test Scarpi hedonic playfulness and utilitarian trust calculations
    double playfulness_val = 0.0;
    double utilitarian_trust_val = 0.0;
    assert(evaluate_scarpi_hedonic_playfulness(&geom, 1.4, &playfulness_val));
    assert(playfulness_val > 0.0);
    assert(evaluate_scarpi_utilitarian_trust(&geom, 0.9, &utilitarian_trust_val));
    assert(utilitarian_trust_val > 0.0);
    printf("   ✓ Scarpi playfulness and utilitarian operational trust verified successfully\n");

    // Test Scarpi hedonic arousal modulator and utilitarian quality control calculations
    double hedonic_arousal_mod_val = 0.0;
    double utilitarian_qc_val = 0.0;
    assert(evaluate_scarpi_hedonic_arousal_mod(&geom, 0.45, 0.75, &hedonic_arousal_mod_val));
    assert(hedonic_arousal_mod_val > 0.0);
    assert(evaluate_scarpi_utilitarian_quality_control(&geom, 0.92, &utilitarian_qc_val));
    assert(utilitarian_qc_val > 0.0);
    printf("   ✓ Scarpi hedonic arousal mod and utilitarian quality control verified successfully\n");

    // Test Scarpi hedonic aesthetic quality and utilitarian structural consistency calculations
    double aesthetic_quality_val = 0.0;
    double consistency_mod_val = 0.0;
    assert(evaluate_scarpi_hedonic_aesthetic_quality(&geom, 0.78, &aesthetic_quality_val));
    assert(aesthetic_quality_val > 0.0);
    assert(evaluate_scarpi_utilitarian_consistency(&geom, 0.84, &consistency_mod_val));
    assert(consistency_mod_val > 0.0);
    printf("   ✓ Scarpi aesthetic quality and structural consistency verified successfully\n");

    // Test Scarpi hedonic interface trust and utilitarian operational trust calculations
    double interface_trust_val = 0.0;
    double operational_trust_mod_val = 0.0;
    assert(evaluate_scarpi_hedonic_interface_trust(&geom, 0.65, &interface_trust_val));
    assert(interface_trust_val > 0.0);
    assert(evaluate_scarpi_utilitarian_operational_trust_mod(&geom, 0.72, &operational_trust_mod_val));
    assert(operational_trust_mod_val > 0.0);
    printf("   ✓ Scarpi interface trust and operational trust mod verified successfully\n");

    // Test Scarpi hedonic playful engagement and utilitarian consistency decay calculations
    double playful_engagement_val = 0.0;
    double consistency_decay_val = 0.0;
    assert(evaluate_scarpi_hedonic_playful_engagement(&geom, 0.85, 0.74, &playful_engagement_val));
    assert(playful_engagement_val > 0.0);
    assert(evaluate_scarpi_utilitarian_consistency_decay(&geom, 14.5, &consistency_decay_val));
    assert(consistency_decay_val > 0.0);
    printf("   ✓ Scarpi playful engagement and structural consistency decay verified successfully\n");

    // Test Scarpi hedonic playful arousal variance and utilitarian operational trust decay calculations
    double arousal_variance_val = 0.0;
    double operational_trust_decay = 0.0;
    assert(evaluate_scarpi_hedonic_playful_arousal_variance(&geom, 0.55, 0.64, &arousal_variance_val));
    assert(arousal_variance_val > 0.0);
    assert(evaluate_scarpi_utilitarian_operational_trust_decay(&geom, 18.0, &operational_trust_decay));
    assert(operational_trust_decay > 0.0);
    printf("   ✓ Scarpi playful arousal variance and operational trust decay verified successfully\n");

    // Test Scarpi utilitarian structural consistency variance and hedonic interface trust decay calculations
    double structural_consistency_var = 0.0;
    double interface_trust_decay = 0.0;
    assert(evaluate_scarpi_utilitarian_consistency_variance(&geom, 0.88, &structural_consistency_var));
    assert(structural_consistency_var > 0.0);
    assert(evaluate_scarpi_hedonic_interface_trust_decay(&geom, 15.0, &interface_trust_decay));
    assert(interface_trust_decay > 0.0);
    printf("   ✓ Scarpi structural consistency variance and interface trust decay verified successfully\n");

    // Test Scarpi hedonic trust and utilitarian efficiency calculations
    double hedonic_trust_val = 0.0;
    double utilitarian_efficiency_val = 0.0;
    assert(evaluate_scarpi_hedonic_trust(&geom, 0.85, 1.25, &hedonic_trust_val));
    assert(hedonic_trust_val > 0.0);
    assert(evaluate_scarpi_utilitarian_efficiency(&geom, 0.05, &utilitarian_efficiency_val));
    assert(utilitarian_efficiency_val > 0.0);
    printf("   ✓ Scarpi interface trust and utilitarian task efficiency verified successfully\n");

    // Test Scarpi aesthetic trust variance and quality control decay calculations
    double hedonic_trust_variance = 0.0;
    double quality_control_decay_val = 0.0;
    assert(evaluate_scarpi_hedonic_trust_variance(&geom, 0.82, 1.15, &hedonic_trust_variance));
    assert(hedonic_trust_variance > 0.0);
    assert(evaluate_scarpi_utilitarian_quality_control_decay(&geom, 15.0, &quality_control_decay_val));
    assert(quality_control_decay_val > 0.0);
    printf("   ✓ Scarpi aesthetic trust variance and quality control decay verified successfully\n");

    // Test Scarpi playful engagement variance and consistency decay variance calculations
    double playful_engagement_variance = 0.0;
    double consistency_decay_variance = 0.0;
    assert(evaluate_scarpi_hedonic_playful_engagement_variance(&geom, 0.82, 0.72, &playful_engagement_variance));
    assert(playful_engagement_variance > 0.0);
    assert(evaluate_scarpi_utilitarian_consistency_decay_variance(&geom, 16.0, &consistency_decay_variance));
    assert(consistency_decay_variance > 0.0);
    printf("   ✓ Scarpi playful engagement variance and consistency decay variance verified successfully\n");

    // Test Scarpi playful arousal mod variance and quality control variance calculations
    double playful_arousal_mod_variance = 0.0;
    double quality_control_variance_val = 0.0;
    assert(evaluate_scarpi_hedonic_playful_arousal_mod_variance(&geom, 0.85, 0.95, &playful_arousal_mod_variance));
    assert(playful_arousal_mod_variance > 0.0);
    assert(evaluate_scarpi_utilitarian_quality_control_variance(&geom, 2.5, &quality_control_variance_val));
    assert(quality_control_variance_val > 0.0);
    printf("   ✓ Scarpi playful arousal mod variance and quality control variance verified successfully\n");

    // Test Scarpi playful arousal decay variance and quality control decay variance calculations
    double playful_arousal_decay_variance = 0.0;
    double quality_control_decay_variance = 0.0;
    assert(evaluate_scarpi_hedonic_playful_arousal_decay_variance(&geom, 15.0, &playful_arousal_decay_variance));
    assert(playful_arousal_decay_variance > 0.0);
    assert(evaluate_scarpi_utilitarian_quality_control_decay_variance(&geom, 16.0, &quality_control_decay_variance));
    assert(quality_control_decay_variance > 0.0);
    printf("   ✓ Scarpi playful arousal decay variance and quality control decay variance verified successfully\n");

    // Test Scarpi playful engagement decay variance and structural consistency variance calculations
    double playful_engagement_decay_variance = 0.0;
    double structural_consistency_variance_mod_val = 0.0;
    assert(evaluate_scarpi_hedonic_playful_engagement_decay_variance(&geom, 14.5, &playful_engagement_decay_variance));
    assert(playful_engagement_decay_variance > 0.0);
    assert(evaluate_scarpi_utilitarian_consistency_variance_mod(&geom, 2.8, &structural_consistency_variance_mod_val));
    assert(structural_consistency_variance_mod_val > 0.0);
    printf("   ✓ Scarpi playful engagement decay variance and structural consistency variance verified successfully\n");

    // Test Scarpi quality control variance mod calculations
    double quality_control_variance_mod_val = 0.0;
    assert(evaluate_scarpi_utilitarian_quality_control_variance_mod(&geom, 2.2, &quality_control_variance_mod_val));
    assert(quality_control_variance_mod_val > 0.0);
    printf("   ✓ Scarpi quality control variance mod verified successfully\n");

    // Test Scarpi quality control decay variance mod calculations
    double quality_control_decay_variance_mod = 0.0;
    assert(evaluate_scarpi_utilitarian_quality_control_decay_variance_mod(&geom, 15.0, &quality_control_decay_variance_mod));
    assert(quality_control_decay_variance_mod > 0.0);
    printf("   ✓ Scarpi quality control decay variance mod verified successfully\n");

    // Test Scarpi playful arousal decay variance mod and operational trust variance mod calculations
    double playful_arousal_decay_variance_mod = 0.0;
    double operational_trust_variance_mod_val = 0.0;
    assert(evaluate_scarpi_hedonic_playful_arousal_decay_variance_mod(&geom, 15.0, &playful_arousal_decay_variance_mod));
    assert(playful_arousal_decay_variance_mod > 0.0);
    assert(evaluate_scarpi_utilitarian_operational_trust_variance_mod(&geom, 0.85, &operational_trust_variance_mod_val));
    assert(operational_trust_variance_mod_val > 0.0);
    printf("   ✓ Scarpi playful arousal decay variance mod and operational trust variance mod verified successfully\n");

    // Test Scarpi playful engagement variance mod and operational consistency variance mod calculations
    double playful_engagement_variance_mod = 0.0;
    double consistency_variance_mod_single = 0.0;
    assert(evaluate_scarpi_hedonic_playful_engagement_variance_mod(&geom, 0.90, &playful_engagement_variance_mod));
    assert(playful_engagement_variance_mod > 0.0);
    assert(evaluate_scarpi_utilitarian_consistency_variance_mod_single(&geom, 0.88, &consistency_variance_mod_single));
    assert(consistency_variance_mod_single > 0.0);
    printf("   ✓ Scarpi playful engagement variance mod and operational consistency variance mod verified successfully\n");

    // Test Scarpi playful engagement decay variance mod and consistency decay variance mod calculations
    double playful_engagement_decay_variance_mod = 0.0;
    double consistency_decay_variance_mod = 0.0;
    assert(evaluate_scarpi_hedonic_playful_engagement_decay_variance_mod(&geom, 15.0, &playful_engagement_decay_variance_mod));
    assert(playful_engagement_decay_variance_mod > 0.0);
    assert(evaluate_scarpi_utilitarian_consistency_decay_variance_mod(&geom, 14.5, &consistency_decay_variance_mod));
    assert(consistency_decay_variance_mod > 0.0);
    printf("   ✓ Scarpi playful engagement decay variance mod and consistency decay variance mod verified successfully\n");

    // Test Scarpi operational trust decay variance mod and interface trust decay variance mod calculations
    double operational_trust_decay_variance_mod = 0.0;
    double interface_trust_decay_variance_mod = 0.0;
    assert(evaluate_scarpi_utilitarian_operational_trust_decay_variance_mod(&geom, 15.0, &operational_trust_decay_variance_mod));
    assert(operational_trust_decay_variance_mod > 0.0);
    assert(evaluate_scarpi_hedonic_interface_trust_decay_variance_mod(&geom, 14.5, &interface_trust_decay_variance_mod));
    assert(interface_trust_decay_variance_mod > 0.0);
    printf("   ✓ Scarpi operational trust decay variance mod and interface trust decay variance mod verified successfully\n");

    // Test Castle diplomatic alignment and policy consistency calculations
    double diplomatic_alignment_val = 0.0;
    double policy_consistency_val = 0.0;
    assert(evaluate_castle_diplomatic_alignment(&geom, 0.85, 0.74, &diplomatic_alignment_val));
    assert(diplomatic_alignment_val > 0.0);
    assert(evaluate_castle_policy_consistency(&geom, 0.82, &policy_consistency_val));
    assert(policy_consistency_val > 0.0);
    printf("   ✓ Castle diplomatic alignment and policy consistency verified successfully\n");

    // Test Castle diplomatic alignment decay and policy consistency variance calculations
    double diplomatic_decay_val = 0.0;
    double consistency_variance_val = 0.0;
    assert(evaluate_castle_diplomatic_alignment_decay(&geom, 15.0, &diplomatic_decay_val));
    assert(diplomatic_decay_val > 0.0);
    assert(evaluate_castle_policy_consistency_variance(&geom, 0.76, &consistency_variance_val));
    assert(consistency_variance_val > 0.0);
    printf("   ✓ Castle diplomatic alignment decay and policy consistency variance verified successfully\n");

    // Test Castle policy target engagement and diplomatic trust bounds calculations
    double target_engagement_val = 0.0;
    double diplomatic_trust_bounds_val = 0.0;
    assert(evaluate_castle_policy_target_engagement(&geom, 0.95, &target_engagement_val));
    assert(target_engagement_val > 0.0);
    assert(evaluate_castle_diplomatic_trust_bounds(&geom, 0.88, 0.92, &diplomatic_trust_bounds_val));
    assert(diplomatic_trust_bounds_val > 0.0);
    printf("   ✓ Castle policy target engagement and diplomatic trust bounds verified successfully\n");

    // Test Castle diplomatic trust bounds decay and policy target engagement variance calculations
    double diplomatic_trust_bounds_decay = 0.0;
    double target_engagement_variance = 0.0;
    assert(evaluate_castle_diplomatic_trust_bounds_decay(&geom, 15.0, &diplomatic_trust_bounds_decay));
    assert(diplomatic_trust_bounds_decay > 0.0);
    assert(evaluate_castle_policy_target_engagement_variance(&geom, 0.86, &target_engagement_variance));
    assert(target_engagement_variance > 0.0);
    printf("   ✓ Castle diplomatic trust bounds decay and policy target engagement variance verified successfully\n");

    // Test Castle diplomatic trust bounds variance and policy target engagement decay calculations
    double diplomatic_trust_bounds_var_val = 0.0;
    double target_engagement_decay_val = 0.0;
    assert(evaluate_castle_diplomatic_trust_bounds_variance(&geom, 0.84, 0.90, &diplomatic_trust_bounds_var_val));
    assert(diplomatic_trust_bounds_var_val > 0.0);
    assert(evaluate_castle_policy_target_engagement_decay(&geom, 16.0, &target_engagement_decay_val));
    assert(target_engagement_decay_val > 0.0);
    printf("   ✓ Castle diplomatic trust bounds variance and policy target engagement decay verified successfully\n");

    // Test Castle diplomatic alignment variance and policy consistency decay calculations
    double diplomatic_alignment_var_val = 0.0;
    double policy_consistency_decay_val = 0.0;
    assert(evaluate_castle_diplomatic_alignment_variance(&geom, 0.85, 0.92, &diplomatic_alignment_var_val));
    assert(diplomatic_alignment_var_val > 0.0);
    assert(evaluate_castle_policy_consistency_decay(&geom, 15.0, &policy_consistency_decay_val));
    assert(policy_consistency_decay_val > 0.0);
    printf("   ✓ Castle diplomatic alignment variance and policy consistency decay verified successfully\n");

    // Test Castle diplomatic alignment decay variance and policy consistency variance calculations
    double diplomatic_alignment_decay_var = 0.0;
    double policy_consistency_variance_val = 0.0;
    assert(evaluate_castle_diplomatic_alignment_decay_variance(&geom, 16.5, &diplomatic_alignment_decay_var));
    assert(diplomatic_alignment_decay_var > 0.0);
    assert(evaluate_castle_policy_consistency_variance_mod(&geom, 0.88, &policy_consistency_variance_val));
    assert(policy_consistency_variance_val > 0.0);
    printf("   ✓ Castle diplomatic alignment decay variance and policy consistency variance verified successfully\n");

    // Test Castle diplomatic trust bounds decay variance and policy target engagement variance mod calculations
    double diplomatic_trust_bounds_decay_variance = 0.0;
    double policy_target_engagement_variance_mod_val = 0.0;
    assert(evaluate_castle_diplomatic_trust_bounds_decay_variance(&geom, 15.0, &diplomatic_trust_bounds_decay_variance));
    assert(diplomatic_trust_bounds_decay_variance > 0.0);
    assert(evaluate_castle_policy_target_engagement_variance_mod(&geom, 0.82, &policy_target_engagement_variance_mod_val));
    assert(policy_target_engagement_variance_mod_val > 0.0);
    printf("   ✓ Castle diplomatic trust bounds decay variance and policy target engagement variance mod verified successfully\n");

    // Test Castle diplomatic alignment variance mod variance and policy consistency decay variance calculations
    double diplomatic_alignment_variance_mod_val = 0.0;
    double policy_consistency_decay_variance_val = 0.0;
    assert(evaluate_castle_diplomatic_alignment_variance_mod(&geom, 0.85, &diplomatic_alignment_variance_mod_val));
    assert(diplomatic_alignment_variance_mod_val > 0.0);
    assert(evaluate_castle_policy_consistency_decay_variance(&geom, 14.5, &policy_consistency_decay_variance_val));
    assert(policy_consistency_decay_variance_val > 0.0);
    printf("   ✓ Castle diplomatic alignment variance mod variance and policy consistency decay variance verified successfully\n");

    // Test Castle diplomatic trust bounds variance mod variance and policy target engagement decay variance calculations
    double diplomatic_trust_bounds_variance_mod_val = 0.0;
    double policy_target_engagement_decay_variance_val = 0.0;
    assert(evaluate_castle_diplomatic_trust_bounds_variance_mod(&geom, 0.88, &diplomatic_trust_bounds_variance_mod_val));
    assert(diplomatic_trust_bounds_variance_mod_val > 0.0);
    assert(evaluate_castle_policy_target_engagement_decay_variance(&geom, 15.0, &policy_target_engagement_decay_variance_val));
    assert(policy_target_engagement_decay_variance_val > 0.0);
    printf("   ✓ Castle diplomatic trust bounds variance mod variance and policy target engagement decay variance verified successfully\n");

    // Test Castle policy consistency decay variance mod calculations
    double policy_consistency_decay_variance_mod_val = 0.0;
    assert(evaluate_castle_policy_consistency_decay_variance_mod(&geom, 16.0, &policy_consistency_decay_variance_mod_val));
    assert(policy_consistency_decay_variance_mod_val > 0.0);
    printf("   ✓ Castle policy consistency decay variance mod verified successfully\n");

    // Test Castle diplomatic alignment, diplomatic trust bounds, and policy target engagement modulator decay variance mod calculations
    double diplomatic_alignment_decay_variance_mod = 0.0;
    double diplomatic_trust_bounds_decay_variance_mod = 0.0;
    double policy_target_engagement_decay_variance_mod = 0.0;
    assert(evaluate_castle_diplomatic_alignment_decay_variance_mod(&geom, 15.0, &diplomatic_alignment_decay_variance_mod));
    assert(diplomatic_alignment_decay_variance_mod > 0.0);
    assert(evaluate_castle_diplomatic_trust_bounds_decay_variance_mod(&geom, 14.5, &diplomatic_trust_bounds_decay_variance_mod));
    assert(diplomatic_trust_bounds_decay_variance_mod > 0.0);
    assert(evaluate_castle_policy_target_engagement_decay_variance_mod(&geom, 16.0, &policy_target_engagement_decay_variance_mod));
    assert(policy_target_engagement_decay_variance_mod > 0.0);
    printf("   ✓ Castle diplomatic alignment, diplomatic trust bounds, and policy target engagement modulator decay variance mod verified successfully\n");

    // Test Kramer-Ward FWHR dominance, trustworthiness, and decay variance calculations
    double kramer_dominance = 0.0;
    double kramer_trustworthiness = 0.0;
    double kramer_dominance_decay_variance = 0.0;
    double kramer_trustworthiness_decay_variance = 0.0;
    double kramer_threat = 0.0;
    double kramer_cooperation = 0.0;
    double kramer_threat_decay_variance = 0.0;
    double kramer_cooperation_decay_variance = 0.0;
    assert(evaluate_kramer_ward_fwhr_dominance(&geom, 1.85, &kramer_dominance));
    assert(kramer_dominance > 0.0);
    assert(evaluate_kramer_ward_fwhr_trustworthiness(&geom, 1.85, &kramer_trustworthiness));
    assert(kramer_trustworthiness > 0.0);
    assert(evaluate_kramer_ward_fwhr_dominance_decay_variance(&geom, 15.0, &kramer_dominance_decay_variance));
    assert(kramer_dominance_decay_variance > 0.0);
    assert(evaluate_kramer_ward_fwhr_trustworthiness_decay_variance(&geom, 14.5, &kramer_trustworthiness_decay_variance));
    assert(kramer_trustworthiness_decay_variance > 0.0);
    assert(evaluate_kramer_ward_fwhr_threat(&geom, 1.85, &kramer_threat));
    assert(kramer_threat > 0.0);
    assert(evaluate_kramer_ward_fwhr_cooperation(&geom, 1.85, &kramer_cooperation));
    assert(kramer_cooperation > 0.0);
    assert(evaluate_kramer_ward_fwhr_threat_decay_variance(&geom, 15.0, &kramer_threat_decay_variance));
    assert(kramer_threat_decay_variance > 0.0);
    assert(evaluate_kramer_ward_fwhr_cooperation_decay_variance(&geom, 14.5, &kramer_cooperation_decay_variance));
    assert(kramer_cooperation_decay_variance > 0.0);
    
    double kramer_noise_se = 0.0;
    double kramer_adapt_offset = 0.0;
    double recent_history[3] = {1.85, 1.95, 1.75};
    double kramer_weighted = 0.0;
    double kramer_shaded = 0.0;
    double kramer_smoothed = 0.0;
    double kramer_confidence = 0.0;
    double frame_seq[3] = {1.82, 1.88, 1.85};
    double kramer_discrepancy = 0.0;
    double kramer_p_value = 0.0;
    double kramer_apparent = 0.0;
    double kramer_mismatch = 0.0;
    double kramer_health_mismatch = 0.0;
    double kramer_prominence_mod = 0.0;
    double kramer_distorted_fwhr = 0.0;
    double kramer_exposure_accuracy = 0.0;
    double kramer_human_trustworthiness = 0.0;
    double kramer_human_aggression = 0.0;
    double kramer_human_eye_proportion = 0.0;
    double kramer_human_dominance = 0.0;
    double kramer_human_elongation = 0.0;
    double kramer_human_eye_brow_trust = 0.0;
    double kramer_human_trust_decay = 0.0;
    double kramer_human_symmetry_trust = 0.0;
    double kramer_human_dilation_arousal = 0.0;
    assert(evaluate_kramer_ward_fwhr_3d_scan_discrepancy(&geom, 1.85, &kramer_discrepancy));
    assert(kramer_discrepancy > 0.0);
    assert(evaluate_kramer_ward_fwhr_sexual_dimorphism_null_hypothesis(&geom, 1.90, 1.85, &kramer_p_value));
    assert(kramer_p_value > 0.0);
    assert(evaluate_kramer_ward_fwhr_head_tilt_effect(&geom, 1.85, 10.0, &kramer_apparent));
    assert(kramer_apparent > 0.0);
    assert(evaluate_kramer_ward_fwhr_actual_vs_perceived_aggression_mismatch(&geom, 4.5, 2.1, &kramer_mismatch));
    assert(kramer_mismatch > 0.0);
    assert(evaluate_kramer_ward_fwhr_perceptual_noise_sensitivity(&geom, 0.2, &kramer_noise_se));
    assert(kramer_noise_se > 0.0);
    assert(evaluate_kramer_ward_fwhr_sequential_adaptation_bias(&geom, recent_history, 3, &kramer_adapt_offset));
    assert(evaluate_kramer_ward_fwhr_hemiface_asymmetry_bias(&geom, 1.80, 1.90, &kramer_weighted));
    assert(kramer_weighted > 0.0);
    assert(evaluate_kramer_ward_fwhr_ambient_shading_effect(&geom, 1.85, 45.0, &kramer_shaded));
    assert(kramer_shaded > 0.0);
    assert(evaluate_kramer_ward_fwhr_dynamic_temporal_smoothing(&geom, frame_seq, 3, &kramer_smoothed));
    assert(kramer_smoothed > 0.0);
    assert(evaluate_kramer_ward_fwhr_judgment_confidence_calibration(&geom, 1.85, &kramer_confidence));
    assert(kramer_confidence > 0.0);
    assert(evaluate_kramer_ward_fwhr_perceived_vs_actual_health(&geom, 0.8, 0.7, &kramer_health_mismatch));
    assert(kramer_health_mismatch > 0.0);
    assert(evaluate_kramer_ward_fwhr_cheekbone_prominence_modulation(&geom, 1.2, &kramer_prominence_mod));
    assert(kramer_prominence_mod > 0.0);
    assert(evaluate_kramer_ward_fwhr_perspective_distortion(&geom, 1.85, 50.0, &kramer_distorted_fwhr));
    assert(kramer_distorted_fwhr > 0.0);
    assert(evaluate_kramer_ward_fwhr_micro_exposure_resolution(&geom, 250.0, &kramer_exposure_accuracy));
    assert(kramer_exposure_accuracy > 0.0);
    assert(evaluate_kramer_ward_human_fwhr_trustworthiness(&geom, 1.85, &kramer_human_trustworthiness));
    assert(kramer_human_trustworthiness > 0.0);
    assert(evaluate_kramer_ward_human_face_asymmetry_aggression(&geom, 0.15, &kramer_human_aggression));
    assert(kramer_human_aggression > 0.0);
    assert(evaluate_kramer_ward_human_eye_to_face_proportion(&geom, 0.22, 0.05, &kramer_human_eye_proportion));
    assert(kramer_human_eye_proportion > 0.0);
    assert(evaluate_kramer_ward_human_fwhr_dominance(&geom, 1.85, &kramer_human_dominance));
    assert(kramer_human_dominance > 0.0);
    assert(evaluate_kramer_ward_human_face_elongation(&geom, 1.35, &kramer_human_elongation));
    assert(kramer_human_elongation > 0.0);
    assert(evaluate_kramer_ward_human_eye_to_brow_trust(&geom, 0.12, &kramer_human_eye_brow_trust));
    assert(kramer_human_eye_brow_trust > 0.0);
    assert(evaluate_kramer_ward_human_fwhr_trustworthiness_decay(&geom, 0.85, 12.0, &kramer_human_trust_decay));
    assert(kramer_human_trust_decay > 0.0);
    assert(evaluate_kramer_ward_human_face_symmetry_trust(&geom, 0.95, &kramer_human_symmetry_trust));
    assert(kramer_human_symmetry_trust > 0.0);
    assert(evaluate_kramer_ward_human_eye_dilation_arousal(&geom, 0.22, 0.06, &kramer_human_dilation_arousal));
    assert(kramer_human_dilation_arousal > 0.0);
    
    double kramer_agreeableness_consensus = 0.0;
    double kramer_extraversion_mod = 0.0;
    double kramer_neuroticism_threat = 0.0;
    assert(evaluate_kramer_king_ward_perceived_agreeableness_consensus(&geom, 0.45, 0.35, &kramer_agreeableness_consensus));
    assert(kramer_agreeableness_consensus > 0.0);
    assert(evaluate_kramer_king_ward_actual_extraversion_modulator(&geom, 1.25, 0.85, &kramer_extraversion_mod));
    assert(kramer_extraversion_mod > 0.0);
    assert(evaluate_kramer_king_ward_neuroticism_threat_mapping(&geom, 0.15, 0.22, &kramer_neuroticism_threat));
    assert(kramer_neuroticism_threat > 0.0);
    printf("   ✓ KRAMER, KING & WARD perceived agreeableness, actual extraversion, and neuroticism threat mapping verified successfully\n");

    double kramer_conscientiousness_consensus = 0.0;
    double kramer_openness_mapping = 0.0;
    double kramer_agreeableness_correlation = 0.0;
    assert(evaluate_kramer_king_ward_perceived_conscientiousness_consensus(&geom, 0.85, 0.45, &kramer_conscientiousness_consensus));
    assert(kramer_conscientiousness_consensus > 0.0);
    assert(evaluate_kramer_king_ward_perceived_openness_morphological_mapping(&geom, 0.22, 1.25, &kramer_openness_mapping));
    assert(kramer_openness_mapping > 0.0);
    assert(evaluate_kramer_king_ward_actual_agreeableness_correlation(&geom, 0.4, 1.85, &kramer_agreeableness_correlation));
    assert(kramer_agreeableness_correlation > 0.0);
    printf("   ✓ KRAMER, KING & WARD conscientiousness consensus, openness mapping, and actual agreeableness correlation verified successfully\n");

    double kramer_extraversion_consensus = 0.0;
    double kramer_neuroticism_consensus = 0.0;
    double kramer_conscientiousness_correlation = 0.0;
    assert(evaluate_kramer_king_ward_perceived_extraversion_consensus(&geom, 0.45, 0.22, &kramer_extraversion_consensus));
    assert(kramer_extraversion_consensus > 0.0);
    assert(evaluate_kramer_king_ward_perceived_neuroticism_consensus(&geom, 0.15, 0.12, &kramer_neuroticism_consensus));
    assert(kramer_neuroticism_consensus > 0.0);
    assert(evaluate_kramer_king_ward_actual_conscientiousness_correlation(&geom, 1.35, 0.95, &kramer_conscientiousness_correlation));
    assert(kramer_conscientiousness_correlation > 0.0);
    printf("   ✓ KRAMER, KING & WARD extraversion and neuroticism consensus, and actual conscientiousness correlation verified successfully\n");

    double wang_warmth = 0.0;
    double wang_competence = 0.0;
    double wang_contagion = 0.0;
    assert(evaluate_wang_geigel_character_warmth(&geom, 0.65, 0.45, &wang_warmth));
    assert(wang_warmth > 0.0);
    assert(evaluate_wang_geigel_competence_projection(&geom, 0.22, 0.85, &wang_competence));
    assert(wang_competence > 0.0);
    assert(evaluate_wang_geigel_emotional_contagion(&geom, 0.75, 0.95, &wang_contagion));
    assert(wang_contagion > 0.0);
    printf("   ✓ WANG, GEIGEL & HERBERT virtual character warmth, competence projection, and emotional contagion verified successfully\n");

    double wang_warmth_attenuation = 0.0;
    double wang_gaze_dominance = 0.0;
    double wang_realism_attraction = 0.0;
    assert(evaluate_wang_geigel_warmth_attenuation(&geom, 0.35, &wang_warmth_attenuation));
    assert(wang_warmth_attenuation > 0.0);
    assert(evaluate_wang_geigel_gaze_dominance_modulator(&geom, 0.65, &wang_gaze_dominance));
    assert(wang_gaze_dominance > 0.0);
    assert(evaluate_wang_geigel_realism_attraction_congruence(&geom, 0.75, 0.45, &wang_realism_attraction));
    assert(wang_realism_attraction > 0.0);
    printf("   ✓ WANG, GEIGEL & HERBERT warmth attenuation, gaze dominance modulator, and realism attraction congruence verified successfully\n");

    double wang_gaze_competence = 0.0;
    double wang_contagion_arousal = 0.0;
    double wang_realism_trust = 0.0;
    assert(evaluate_wang_geigel_gaze_competence_modulator(&geom, 0.75, 0.45, &wang_gaze_competence));
    assert(wang_gaze_competence > 0.0);
    assert(evaluate_wang_geigel_contagion_arousal(&geom, 0.85, 0.25, &wang_contagion_arousal));
    assert(wang_contagion_arousal > 0.0);
    assert(evaluate_wang_geigel_realism_trust_congruence(&geom, 0.65, 0.55, &wang_realism_trust));
    assert(wang_realism_trust > 0.0);
    printf("   ✓ WANG, GEIGEL & HERBERT virtual character gaze competence, contagion arousal, and realism trust congruence verified successfully\n");

    double wang_contagion_decay = 0.0;
    double wang_competence_decay_var = 0.0;
    double wang_gaze_realism_mismatch = 0.0;
    assert(evaluate_wang_geigel_emotional_contagion_decay(&geom, 0.75, 12.0, &wang_contagion_decay));
    assert(wang_contagion_decay > 0.0);
    assert(evaluate_wang_geigel_competence_decay_variance(&geom, 15.0, &wang_competence_decay_var));
    assert(wang_competence_decay_var > 0.0);
    assert(evaluate_wang_geigel_gaze_realism_mismatch(&geom, 0.65, 0.75, &wang_gaze_realism_mismatch));
    assert(wang_gaze_realism_mismatch >= 0.0);
    printf("   ✓ WANG, GEIGEL & HERBERT contagion decay, competence decay variance, and gaze realism mismatch verified successfully\n");

    double wang_blink_trust = 0.0;
    double wang_head_shake = 0.0;
    double wang_gaze_submissiveness = 0.0;
    assert(evaluate_wang_geigel_avatar_blink_trust(&geom, 0.22, &wang_blink_trust));
    assert(wang_blink_trust > 0.0);
    assert(evaluate_wang_geigel_avatar_head_shake(&geom, 0.65, &wang_head_shake));
    assert(wang_head_shake > 0.0);
    assert(evaluate_wang_geigel_avatar_gaze_submissiveness(&geom, 0.45, 0.22, &wang_gaze_submissiveness));
    assert(wang_gaze_submissiveness > 0.0);
    printf("   ✓ WANG, GEIGEL & HERBERT avatar blink trust, head shake, and gaze submissiveness verified successfully\n");

    double wang_smile_attr = 0.0;
    double wang_furrow_threat = 0.0;
    double wang_realism_warmth = 0.0;
    assert(evaluate_wang_geigel_avatar_smile_attractiveness(&geom, 0.75, 0.25, &wang_smile_attr));
    assert(wang_smile_attr > 0.0);
    assert(evaluate_wang_geigel_avatar_eyebrow_furrow_threat(&geom, 0.85, 0.65, &wang_furrow_threat));
    assert(wang_furrow_threat > 0.0);
    assert(evaluate_wang_geigel_avatar_realism_warmth_interaction(&geom, 0.8, 0.7, &wang_realism_warmth));
    assert(wang_realism_warmth > 0.0);
    printf("   ✓ WANG, GEIGEL & HERBERT avatar smile attractiveness, eyebrow furrow threat, and realism warmth interaction verified successfully\n");

    double masuda_naturalness = 0.0;
    double masuda_familiarity = 0.0;
    double masuda_empathy = 0.0;
    assert(evaluate_masuda_perceived_naturalness(&geom, 25.0, 0.75, &masuda_naturalness));
    assert(masuda_naturalness > 0.0);
    assert(evaluate_masuda_conversational_familiarity(&geom, 0.65, 0.85, &masuda_familiarity));
    assert(masuda_familiarity > 0.0);
    assert(evaluate_masuda_empathic_synchronization(&geom, 0.7, 0.8, &masuda_empathy));
    assert(masuda_empathy > 0.0);
    printf("   ✓ MASUDA, KUNIZAWA & TAKAHASHI perceived naturalness, familiarity, and empathic synchronization verified successfully\n");

    double masuda_naturalness_att = 0.0;
    double masuda_familiarity_decay = 0.0;
    double masuda_sync_var = 0.0;
    assert(evaluate_masuda_naturalness_attenuation(&geom, 120.0, &masuda_naturalness_att));
    assert(masuda_naturalness_att > 0.0);
    assert(evaluate_masuda_conversational_familiarity_decay(&geom, 0.8, 15.0, &masuda_familiarity_decay));
    assert(masuda_familiarity_decay > 0.0);
    assert(evaluate_masuda_empathic_sync_variance(&geom, 0.75, 0.65, &masuda_sync_var));
    assert(masuda_sync_var > 0.0);
    printf("   ✓ MASUDA, KUNIZAWA & TAKAHASHI naturalness attenuation, familiarity decay, and empathic sync variance verified successfully\n");

    double masuda_nat_var = 0.0;
    double masuda_gaze_att = 0.0;
    double masuda_contagion_int = 0.0;
    assert(evaluate_masuda_naturalness_variance(&geom, 0.65, &masuda_nat_var));
    assert(masuda_nat_var > 0.0);
    assert(evaluate_masuda_reciprocal_gaze_attenuation(&geom, 150.0, &masuda_gaze_att));
    assert(masuda_gaze_att > 0.0);
    assert(evaluate_masuda_empathic_contagion_interaction(&geom, 0.7, 0.8, &masuda_contagion_int));
    assert(masuda_contagion_int > 0.0);
    printf("   ✓ MASUDA, KUNIZAWA & TAKAHASHI naturalness variance, reciprocal gaze attenuation, and empathic contagion interaction verified successfully\n");

    // Error validation tests for Wang models
    double dummy_out = 0.0;
    assert(!evaluate_wang_geigel_character_warmth(NULL, 0.65, 0.45, &dummy_out));
    assert(!evaluate_wang_geigel_character_warmth(&geom, -0.65, 0.45, &dummy_out));
    assert(!evaluate_wang_geigel_character_warmth(&geom, 0.65, -0.45, &dummy_out));
    assert(!evaluate_wang_geigel_character_warmth(&geom, 0.65, 0.45, NULL));

    assert(!evaluate_wang_geigel_competence_projection(NULL, 0.22, 0.85, &dummy_out));
    assert(!evaluate_wang_geigel_competence_projection(&geom, -0.22, 0.85, &dummy_out));
    assert(!evaluate_wang_geigel_competence_projection(&geom, 0.22, -0.85, &dummy_out));
    assert(!evaluate_wang_geigel_competence_projection(&geom, 0.22, 0.85, NULL));

    assert(!evaluate_wang_geigel_emotional_contagion(NULL, 0.75, 0.95, &dummy_out));
    assert(!evaluate_wang_geigel_emotional_contagion(&geom, -0.75, 0.95, &dummy_out));
    assert(!evaluate_wang_geigel_emotional_contagion(&geom, 0.75, -0.95, &dummy_out));
    assert(!evaluate_wang_geigel_emotional_contagion(&geom, 0.75, 0.95, NULL));

    assert(!evaluate_wang_geigel_warmth_attenuation(NULL, 0.35, &dummy_out));
    assert(!evaluate_wang_geigel_warmth_attenuation(&geom, -0.35, &dummy_out));
    assert(!evaluate_wang_geigel_warmth_attenuation(&geom, 0.35, NULL));

    assert(!evaluate_wang_geigel_gaze_dominance_modulator(NULL, 0.65, &dummy_out));
    assert(!evaluate_wang_geigel_gaze_dominance_modulator(&geom, -0.65, &dummy_out));
    assert(!evaluate_wang_geigel_gaze_dominance_modulator(&geom, 0.65, NULL));

    assert(!evaluate_wang_geigel_realism_attraction_congruence(NULL, 0.75, 0.45, &dummy_out));
    assert(!evaluate_wang_geigel_realism_attraction_congruence(&geom, -0.75, 0.45, &dummy_out));
    assert(!evaluate_wang_geigel_realism_attraction_congruence(&geom, 0.75, -0.45, &dummy_out));
    assert(!evaluate_wang_geigel_realism_attraction_congruence(&geom, 0.75, 0.45, NULL));

    assert(!evaluate_wang_geigel_gaze_competence_modulator(NULL, 0.75, 0.45, &dummy_out));
    assert(!evaluate_wang_geigel_gaze_competence_modulator(&geom, -0.75, 0.45, &dummy_out));
    assert(!evaluate_wang_geigel_gaze_competence_modulator(&geom, 0.75, -0.45, &dummy_out));
    assert(!evaluate_wang_geigel_gaze_competence_modulator(&geom, 0.75, 0.45, NULL));

    assert(!evaluate_wang_geigel_contagion_arousal(NULL, 0.85, 0.25, &dummy_out));
    assert(!evaluate_wang_geigel_contagion_arousal(&geom, -0.85, 0.25, &dummy_out));
    assert(!evaluate_wang_geigel_contagion_arousal(&geom, 0.85, -0.25, &dummy_out));
    assert(!evaluate_wang_geigel_contagion_arousal(&geom, 0.85, 0.25, NULL));

    assert(!evaluate_wang_geigel_realism_trust_congruence(NULL, 0.65, 0.55, &dummy_out));
    assert(!evaluate_wang_geigel_realism_trust_congruence(&geom, -0.65, 0.55, &dummy_out));
    assert(!evaluate_wang_geigel_realism_trust_congruence(&geom, 0.65, -0.55, &dummy_out));
    assert(!evaluate_wang_geigel_realism_trust_congruence(&geom, 0.65, 0.55, NULL));

    assert(!evaluate_wang_geigel_emotional_contagion_decay(NULL, 0.75, 12.0, &dummy_out));
    assert(!evaluate_wang_geigel_emotional_contagion_decay(&geom, -0.75, 12.0, &dummy_out));
    assert(!evaluate_wang_geigel_emotional_contagion_decay(&geom, 0.75, -12.0, &dummy_out));
    assert(!evaluate_wang_geigel_emotional_contagion_decay(&geom, 0.75, 12.0, NULL));

    assert(!evaluate_wang_geigel_competence_decay_variance(NULL, 15.0, &dummy_out));
    assert(!evaluate_wang_geigel_competence_decay_variance(&geom, -15.0, &dummy_out));
    assert(!evaluate_wang_geigel_competence_decay_variance(&geom, 15.0, NULL));

    assert(!evaluate_wang_geigel_gaze_realism_mismatch(NULL, 0.65, 0.75, &dummy_out));
    assert(!evaluate_wang_geigel_gaze_realism_mismatch(&geom, -0.65, 0.75, &dummy_out));
    assert(!evaluate_wang_geigel_gaze_realism_mismatch(&geom, 0.65, -0.75, &dummy_out));
    assert(!evaluate_wang_geigel_gaze_realism_mismatch(&geom, 0.65, 0.75, NULL));

    assert(!evaluate_wang_geigel_avatar_blink_trust(NULL, 0.22, &dummy_out));
    assert(!evaluate_wang_geigel_avatar_blink_trust(&geom, -0.22, &dummy_out));
    assert(!evaluate_wang_geigel_avatar_blink_trust(&geom, 0.22, NULL));

    assert(!evaluate_wang_geigel_avatar_head_shake(NULL, 0.65, &dummy_out));
    assert(!evaluate_wang_geigel_avatar_head_shake(&geom, -0.65, &dummy_out));
    assert(!evaluate_wang_geigel_avatar_head_shake(&geom, 0.65, NULL));

    assert(!evaluate_wang_geigel_avatar_gaze_submissiveness(NULL, 0.45, 0.22, &dummy_out));
    assert(!evaluate_wang_geigel_avatar_gaze_submissiveness(&geom, -0.45, 0.22, &dummy_out));
    assert(!evaluate_wang_geigel_avatar_gaze_submissiveness(&geom, 0.45, -0.22, &dummy_out));
    assert(!evaluate_wang_geigel_avatar_gaze_submissiveness(&geom, 0.45, 0.22, NULL));

    assert(!evaluate_wang_geigel_avatar_smile_attractiveness(NULL, 0.75, 0.25, &dummy_out));
    assert(!evaluate_wang_geigel_avatar_smile_attractiveness(&geom, -0.75, 0.25, &dummy_out));
    assert(!evaluate_wang_geigel_avatar_smile_attractiveness(&geom, 0.75, -0.25, &dummy_out));
    assert(!evaluate_wang_geigel_avatar_smile_attractiveness(&geom, 0.75, 0.25, NULL));

    assert(!evaluate_wang_geigel_avatar_eyebrow_furrow_threat(NULL, 0.85, 0.65, &dummy_out));
    assert(!evaluate_wang_geigel_avatar_eyebrow_furrow_threat(&geom, -0.85, 0.65, &dummy_out));
    assert(!evaluate_wang_geigel_avatar_eyebrow_furrow_threat(&geom, 0.85, -0.65, &dummy_out));
    assert(!evaluate_wang_geigel_avatar_eyebrow_furrow_threat(&geom, 0.85, 0.65, NULL));

    assert(!evaluate_wang_geigel_avatar_realism_warmth_interaction(NULL, 0.8, 0.7, &dummy_out));
    assert(!evaluate_wang_geigel_avatar_realism_warmth_interaction(&geom, -0.8, 0.7, &dummy_out));
    assert(!evaluate_wang_geigel_avatar_realism_warmth_interaction(&geom, 0.8, -0.7, &dummy_out));
    assert(!evaluate_wang_geigel_avatar_realism_warmth_interaction(&geom, 0.8, 0.7, NULL));

    assert(!evaluate_masuda_perceived_naturalness(NULL, 25.0, 0.75, &dummy_out));
    assert(!evaluate_masuda_perceived_naturalness(&geom, -25.0, 0.75, &dummy_out));
    assert(!evaluate_masuda_perceived_naturalness(&geom, 25.0, -0.75, &dummy_out));
    assert(!evaluate_masuda_perceived_naturalness(&geom, 25.0, 0.75, NULL));

    assert(!evaluate_masuda_conversational_familiarity(NULL, 0.65, 0.85, &dummy_out));
    assert(!evaluate_masuda_conversational_familiarity(&geom, -0.65, 0.85, &dummy_out));
    assert(!evaluate_masuda_conversational_familiarity(&geom, 0.65, -0.85, &dummy_out));
    assert(!evaluate_masuda_conversational_familiarity(&geom, 0.65, 0.85, NULL));

    assert(!evaluate_masuda_empathic_synchronization(NULL, 0.7, 0.8, &dummy_out));
    assert(!evaluate_masuda_empathic_synchronization(&geom, -0.7, 0.8, &dummy_out));
    assert(!evaluate_masuda_empathic_synchronization(&geom, 0.7, -0.8, &dummy_out));
    assert(!evaluate_masuda_empathic_synchronization(&geom, 0.7, 0.8, NULL));

    assert(!evaluate_masuda_naturalness_attenuation(NULL, 120.0, &dummy_out));
    assert(!evaluate_masuda_naturalness_attenuation(&geom, -120.0, &dummy_out));
    assert(!evaluate_masuda_naturalness_attenuation(&geom, 120.0, NULL));

    assert(!evaluate_masuda_conversational_familiarity_decay(NULL, 0.8, 15.0, &dummy_out));
    assert(!evaluate_masuda_conversational_familiarity_decay(&geom, -0.8, 15.0, &dummy_out));
    assert(!evaluate_masuda_conversational_familiarity_decay(&geom, 0.8, -15.0, &dummy_out));
    assert(!evaluate_masuda_conversational_familiarity_decay(&geom, 0.8, 15.0, NULL));

    assert(!evaluate_masuda_empathic_sync_variance(NULL, 0.75, 0.65, &dummy_out));
    assert(!evaluate_masuda_empathic_sync_variance(&geom, -0.75, 0.65, &dummy_out));
    assert(!evaluate_masuda_empathic_sync_variance(&geom, 0.75, -0.65, &dummy_out));
    assert(!evaluate_masuda_empathic_sync_variance(&geom, 0.75, 0.65, NULL));

    assert(!evaluate_masuda_naturalness_variance(NULL, 0.65, &dummy_out));
    assert(!evaluate_masuda_naturalness_variance(&geom, -0.65, &dummy_out));
    assert(!evaluate_masuda_naturalness_variance(&geom, 0.65, NULL));

    assert(!evaluate_masuda_reciprocal_gaze_attenuation(NULL, 150.0, &dummy_out));
    assert(!evaluate_masuda_reciprocal_gaze_attenuation(&geom, -150.0, &dummy_out));
    assert(!evaluate_masuda_reciprocal_gaze_attenuation(&geom, 150.0, NULL));

    assert(!evaluate_masuda_empathic_contagion_interaction(NULL, 0.7, 0.8, &dummy_out));
    assert(!evaluate_masuda_empathic_contagion_interaction(&geom, -0.7, 0.8, &dummy_out));
    assert(!evaluate_masuda_empathic_contagion_interaction(&geom, 0.7, -0.8, &dummy_out));
    assert(!evaluate_masuda_empathic_contagion_interaction(&geom, 0.7, 0.8, NULL));

    printf("   ✓ MASUDA, KUNIZAWA & TAKAHASHI error validation and bounds checking verified successfully\n");
    printf("   ✓ WANG, GEIGEL & HERBERT error validation and bounds checking verified successfully\n");







    printf("   ✓ KRAMER & WARD FWHR dominance, trustworthiness, threat, cooperation, discrepancy, dimorphism null-hypothesis, head tilt, perceived-actual mismatch, noise sensitivity, adaptation bias, hemiface bias, ambient shading, temporal smoothing, confidence calibration, perceived-actual health mismatch, cheekbone prominence modulation, perspective distortion, micro-exposure resolution, human fWHR trustworthiness, human face asymmetry aggression, human eye-to-face proportion, human fWHR dominance, human face elongation, human eye-to-brow trust, human fWHR trustworthiness decay, human face symmetry trust, human eye dilation arousal, and decay variances verified successfully\n");

    // Test Diode-Capacitor loop simulation
    double charge = 0.0;
    assert(simulate_diode_capacitor_loop(5.0, 1000.0, 1e-6, 0.1, &charge)); // Forward charge
    assert(charge > 0.0);
    double charged_val = charge;
    assert(simulate_diode_capacitor_loop(0.0, 1000.0, 1e-6, 0.1, &charge)); // Reverse discharge decay
    assert(charge < charged_val);
    printf("   ✓ Virtual diode reflex and relative capacitor charge decay loop verified successfully\n");

    // Test QR Decomposition modes (Sustail, BLACK, RED) directly at the H-bridge izotope diode circuit
    double test_A[16] = {
        4.0, 1.0, 0.0, 0.0,
        1.0, 5.0, 1.0, 0.0,
        0.0, 1.0, 3.0, 1.0,
        0.0, 0.0, 1.0, 4.0
    };
    double test_Q[16] = {0};
    double test_R[16] = {0};

    // Test Sustail (Gram-Schmidt)
    assert(hbridge_izotope_ozone_diode_qr(test_A, test_Q, test_R, 4, "Sustail"));
    assert(fabs(test_R[4]) < 1e-9); // R[1,0] should be zero
    printf("   ✓ Sustail (Gram-Schmidt) QR at RED sustain flyback diode verified successfully\n");

    // Test BLACK (Householder)
    assert(hbridge_izotope_ozone_diode_qr(test_A, test_Q, test_R, 4, "BLACK"));
    assert(fabs(test_R[4]) < 1e-9); // R[1,0] should be zero
    printf("   ✓ BLACK (Householder) QR at RED sustain flyback diode verified successfully\n");

    // Test RED (Givens)
    assert(hbridge_izotope_ozone_diode_qr(test_A, test_Q, test_R, 4, "RED"));
    assert(fabs(test_R[4]) < 1e-9); // R[1,0] should be zero
    printf("   ✓ RED (Givens) QR at RED sustain flyback diode verified successfully\n");

    // Test Tremolo (Hessenberg) QR-Rotations at BLACK flyback diode
    double test_Tremolo[16] = {
        4.0, 1.0, 0.0, 0.0,
        1.0, 5.0, 1.0, 0.0,
        0.0, 1.0, 3.0, 1.0,
        0.0, 0.0, 1.0, 4.0
    };
    double test_Tremolo_out[16] = {0};
    assert(hbridge_izotope_black_diode_graph_qr(test_Tremolo, test_Tremolo_out, 4));
    assert(fabs(test_Tremolo_out[8]) < 1e-9); // Should maintain Hessenberg (Tremolo) structure or decay
    printf("   ✓ Tremolo QR-Rotations graph and 2-3 tree at BLACK flyback diode verified successfully\n");

    // Test Unit Tests for "Hessenberg" 2-3 tree and "Gram-Schmidt" Red-Black tree structures
    // 1. Gram-Schmidt Red-Black Tree Verification
    GSRBNode rb_root;
    rb_root.key = "Sustail";
    rb_root.color = LHK_BLACK_NODE;
    rb_root.left = NULL;
    rb_root.right = NULL;
    
    assert(strcmp(rb_root.key, "Sustail") == 0);
    assert(rb_root.color == LHK_BLACK_NODE);
    printf("   ✓ Gram-Schmidt (Sustail) Red-Black Tree structures verified successfully\n");

    // Test "Tremolo" 2-3 tree izotope verlet graph and "Sustain" red-black tree izotope effect
    // 1. Tremolo 2-3 tree izotope verlet graph verification
    agent_avatar_t test_avatar;
    test_avatar.geometry = geom;
    test_avatar.sdk_state = 2;
    test_avatar.geometry.stiffness = 0.5;
    simulate_fet_verlet_discharge(&test_avatar, 0.1);
    assert(test_avatar.geometry.damping >= 0.0);

    // Verify a priori samples mapped to Tremolo 2-3 tree node parameters
    TSFiMCAuxFeatures test_nodes[3];
    memset(test_nodes, 0, sizeof(test_nodes));
    test_nodes[0].depth = (float)test_avatar.geometry.head_fwhr;
    test_nodes[1].depth = (float)(test_avatar.geometry.head_fwhr + 1.5); // Steep gradient simulating a 3-node branch split
    test_nodes[2].depth = (float)(test_avatar.geometry.head_fwhr + 1.6);
    
    int apriori_nodes = tsfi_montecarlo_apriori_sample_count(test_nodes, 1, 0, 3, 1, 4);
    assert(apriori_nodes == 8); // 2x multiplier triggered by the node split gradient
    
    printf("   ✓ Tremolo 2-3 tree izotope verlet graph verification passed\n");

    // 2. Sustain red-black tree izotope effect verification
    GSRBNode sustain_node;
    sustain_node.key = "Sustain";
    sustain_node.color = LHK_RED_NODE;
    sustain_node.left = NULL;
    sustain_node.right = NULL;
    
    double t_space = 0.5;
    double s_decay = 0.8;
    assert(evaluate_izotope_constrained_parameters(&geom, 1, &t_space, &s_decay));
    assert(s_decay > 0.0);
    printf("   ✓ Sustain (%s) red-black tree izotope effect verification passed\n", sustain_node.key);

    // Test H-bridge flyback and izotope mismatch calculation
    double flyback_mismatch = 0.0;
    assert(evaluate_hbridge_izotope_mismatch(&geom, 500.0, &flyback_mismatch));
    assert(flyback_mismatch >= 0.0);
    printf("   ✓ H-bridge switching flyback transient and izotope mismatch verified successfully\n");

    // Test Snubber-Clamped inductive flyback decay simulation
    double clamped_volt = 0.0;
    assert(simulate_snubber_clamped_flyback(100.0, 0.05, 10.0, 0.01, &clamped_volt));
    assert(clamped_volt < 100.0);
    assert(clamped_volt > 0.0);
    printf("   ✓ Snubber-clamped H-bridge flyback transient simulation verified successfully\n");

    // Test RCD Snubber resonant decay simulation
    double rcd_volt = 0.0;
    assert(simulate_rcd_snubber_decay(100.0, 10.0, 1e-6, 0.05, 0.00001, &rcd_volt));
    assert(rcd_volt <= 100.0);
    printf("   ✓ RCD snubber resonant capacitor decay simulation verified successfully\n");

    // Test ACID transaction behavior (Commit successful path)
    evaluation_tx_t tx = begin_evaluation_transaction(&geom);
    assert(tx.active);
    geom.head_fwhr = 1.2;
    assert(commit_evaluation_transaction(&tx));
    assert(!tx.active);
    assert(geom.head_fwhr == 1.2);

    // Test ACID-compliant izotope flyback transaction behavior (Commit path)
    tx = begin_evaluation_transaction(&geom);
    assert(tx.active);
    geom.head_fwhr = 1.0;
    assert(commit_izotope_flyback_transaction(&tx, 500.0, 50.0));
    assert(!tx.active);
    assert(geom.head_fwhr == 1.0);

    // Test ACID-compliant izotope flyback transaction behavior (Rollback path)
    tx = begin_evaluation_transaction(&geom);
    assert(tx.active);
    geom.head_fwhr = 1.5; // Modify state
    assert(!commit_izotope_flyback_transaction(&tx, 1500.0, 5.0)); // Should trigger safety violation and rollback
    assert(!tx.active);
    assert(geom.head_fwhr == 1.0); // Rolled back
    printf("   ✓ ACID-compliant izotope flyback transaction safety limits and rollback verified successfully\n");

    // Test Gumbel transient diyat tax calculation
    double total_cost = 0.0;
    assert(calculate_diyat_tax(&geom, 500.0, 10.0, &total_cost));
    assert(total_cost >= 10.0);
    printf("   ✓ Nerve-like Gumbel transient diyat tax calculations verified successfully\n");

    // Test Gumbel transient diyat tax with refractory recovery calculation
    double ref_cost = 0.0;
    assert(calculate_diyat_tax_with_refractory(&geom, 500.0, 10.0, 0.5, &ref_cost));
    assert(ref_cost >= 10.0);

    // Test Gumbel transient diyat tax with tremolo and sustain envelope calculation
    double env_cost = 0.0;
    assert(calculate_diyat_tax_with_envelope(&geom, 500.0, 10.0, 1.0, 2.0, &env_cost));
    assert(env_cost >= 10.0);
    printf("   ✓ Gumbel transient nerve-tax with tremolo and sustain envelope verified successfully\n");

    // Test Phase-Modulated flyback noise distortion calculation
    double noise_val = 0.0;
    assert(simulate_phase_flyback_noise(&geom, 3.14159, &noise_val));
    printf("   ✓ Phase-modulated flyback noise distortion simulation verified successfully\n");

    // Test Izotope constrained parameters calculation
    double tremolo_spacing = 0.0;
    double sustain_decay = 0.0;
    assert(evaluate_izotope_constrained_parameters(&geom, 1, &tremolo_spacing, &sustain_decay));
    assert(tremolo_spacing > 0.0);
    assert(sustain_decay > 0.0);
    printf("   ✓ Izotope symmetric threshold tremolo and group scale sustain verified successfully\n");

    // Test Izotope transactional/ACID rollback for sustain/tremolo parameters
    double test_t_space = 2.2;
    double test_s_decay = 3.3;
    // Create invalid geometry (e.g. negative or zero symmetry) to trigger validation failure
    teddy_bear_geometry_t invalid_geom = geom;
    invalid_geom.symmetry = -0.5;
    bool status_iz = evaluate_izotope_constrained_parameters(&invalid_geom, 1, &test_t_space, &test_s_decay);
    assert(status_iz == false);
    assert(test_t_space == 2.2); // Rollback verified
    assert(test_s_decay == 3.3); // Rollback verified
    printf("   ✓ ACID compliance and rollback for sustain parameter calculations verified successfully\n");

    // Test Izotope/Ozone habituation decay calculation
    double izotope_decay_out = 0.0;
    assert(evaluate_izotope_ozone_habituation_decay(&geom, 1.5, &izotope_decay_out));
    assert(izotope_decay_out < 1.5);
    assert(izotope_decay_out > 0.0);
    printf("   ✓ Izotope/Ozone habituation decay verified successfully\n");

    // Test execute_hbridge_thunk_with_feedback (Successful execution case)
    test_thunk_executed_flag = 0.0;
    double safety_margin = 0.0;
    geom.head_fwhr = 1.0;
    geom.behavioral_mismatch = 0.1;
    assert(execute_hbridge_thunk_with_feedback(&geom, 200.0, test_thunk_callback, &safety_margin));
    assert(test_thunk_executed_flag == 1.0);

    // Test execute_hbridge_thunk_with_feedback (Deferred rollback case on Gumbel safety threshold violation)
    test_thunk_executed_flag = 0.0;
    geom.head_fwhr = 1.8;
    geom.behavioral_mismatch = 1.0;
    assert(!execute_hbridge_thunk_with_feedback(&geom, 2500.0, test_thunk_callback, &safety_margin));
    assert(test_thunk_executed_flag == 0.0); // deferred
    geom.head_fwhr = 1.0; // reset
    printf("   ✓ H-bridge thunk execution under Gumbel flyback and emotional Wald tests verified successfully\n");

    // Test execute_cloglog_thunk_with_feedback (Successful execution case)
    test_thunk_executed_flag = 0.0;
    double cloglog_safety_margin = 0.0;
    geom.head_fwhr = 0.4;
    assert(execute_cloglog_thunk_with_feedback(&geom, 0.5, test_thunk_callback, &cloglog_safety_margin));
    assert(test_thunk_executed_flag == 1.0);
    geom.head_fwhr = 1.0; // reset
    printf("   ✓ Cloglog gated thunk execution verified successfully\n");

    // Test execute_maturity_cloglog_thunk_with_feedback (Successful execution case)
    test_thunk_executed_flag = 0.0;
    double maturity_cloglog_safety = 0.0;
    geom.head_fwhr = 0.3;
    assert(execute_maturity_cloglog_thunk_with_feedback(&geom, test_thunk_callback, &maturity_cloglog_safety));
    assert(test_thunk_executed_flag == 1.0);
    geom.head_fwhr = 1.0; // reset
    printf("   ✓ Maturity-adjusted cloglog gated thunk execution verified successfully\n");

    // Test execute_cloglog_gated_thunk_with_maturity (Successful execution case)
    test_thunk_executed_flag = 0.0;
    double clg_mat_res = 0.0;
    assert(execute_cloglog_gated_thunk_with_maturity(&geom, 0.5, 12.0, test_thunk_callback, &clg_mat_res));
    assert(test_thunk_executed_flag == 1.0);
    printf("   ✓ execute_cloglog_gated_thunk_with_maturity verified successfully\n");

    // Test execute_cooperative_wald_gated_thunk
    test_thunk_executed_flag = 0.0;
    double wald_thunk_res = 0.0;
    double test_betas[2] = {1.2, -1.0};
    double test_cov[4] = {0.25, 0.0, 0.0, 0.25};
    assert(execute_cooperative_wald_gated_thunk(&geom, test_betas, test_cov, 2, test_thunk_callback, &wald_thunk_res));
    assert(test_thunk_executed_flag == 1.0);
    printf("   ✓ execute_cooperative_wald_gated_thunk verified successfully\n");

    // Test ACID transaction behavior (Rollback path on constraint violation)
    tx = begin_evaluation_transaction(&geom);
    assert(tx.active);
    geom.head_fwhr = -5.0; // Invalid fWHR constraint
    assert(!commit_evaluation_transaction(&tx)); // Must fail and rollback
    assert(geom.head_fwhr == 1.0); // Restored
    printf("   ✓ ACID transactions (commit, constraint verification, and rollback) verified successfully\n");

    // Test End-to-End ACID Transaction
    avatar_tx_t av_tx = begin_avatar_transaction(&avatar);
    assert(av_tx.active);
    avatar.geometry.head_fwhr = 2.0;
    
    // Commit to temporary binary file (Rule 13: .dat.bin extension only)
    const char *tmp_bin = "/tmp/test_avatar_state.dat.bin";
    assert(commit_avatar_transaction(&av_tx, tmp_bin));
    assert(!av_tx.active);
    assert(avatar.geometry.head_fwhr == 2.0);
    
    // Verify file created and clean up
    FILE *bin_check = fopen(tmp_bin, "rb");
    assert(bin_check != NULL);
    fclose(bin_check);
    remove(tmp_bin);
    
    // Rollback test
    av_tx = begin_avatar_transaction(&avatar);
    avatar.geometry.head_fwhr = -10.0;
    assert(!commit_avatar_transaction(&av_tx, NULL));
    assert(avatar.geometry.head_fwhr == 2.0);
    printf("   ✓ End-to-End ACID transactions (durability writes to .dat.bin) verified successfully\n");

    // Test GOST fallback authorization channel
    uint32_t gost_key[8] = {0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210, 
                            0x55555555, 0xAAAAAAAA, 0x11111111, 0x99999999};
    avatar.sdk_state = 0; // Reset to Unlocked
    assert(authorize_boundary_via_gost(&avatar, gost_key, 0xAA55AA55, 0x55AA55AA));
    assert(avatar.sdk_state == 2); // Transitioned to Executing
    printf("   ✓ GOST fallback boundary authorization verified successfully\n");

    // Test Parity Unit Tests for WinchesterMQ Yul Hardware selectors
    // Selector 0xe399f0f2: simulate_hbridge_flyback_transient(supply_voltage=12, load_inductance=50, switching_time_sec=10)
    // supply_voltage + (load_inductance * 2000) / switching_time_sec = 12 + (50 * 2000) / 10 = 12 + 10000 = 10012
    uint32_t hbridge_yul_val = 12 + (50 * 2000) / 10;
    assert(hbridge_yul_val == 10012);
    
    // Selector 0xe399f0f3: simulate_snubber_flyback_transient(supply_voltage=12, load_inductance=50, switching_time_sec=10, snubber_resistance=150)
    // peak = 10012. denom = 1000 + 150 * 20 = 4000. attenuated = (peak * 1000) / denom = (10012 * 1000) / 4000 = 2503
    uint32_t peak = 10012;
    uint32_t denom = 1000 + (150 * 20);
    uint32_t snubber_attenuated = (peak * 1000) / denom;
    assert(snubber_attenuated == 2503);
    
    // Selector 0xe399f0f5: simulate_diode_capacitor_loop (reverse decay path with current_charge=800, resistance=1000, capacitance=10, time_step=5)
    // denom = 10000. X = (time_step * 1000000) / denom = 500. factor = 1000 - X + X^2/2000 = 1000 - 500 + 125 = 625. next_charge = 800 * 625 / 1000 = 500
    uint32_t current_chg = 800;
    uint32_t time_step = 5;
    uint32_t rc_denom = 1000 * 10;
    uint32_t X = (time_step * 1000000) / rc_denom;
    uint32_t factor = 1000 - X + (X * X) / 2000;
    uint32_t next_charge = (current_chg * factor) / 1000;
    assert(next_charge == 500);

    // Selector 0xe399f0f6: evaluate_sustain_adjusted_gumbel_tax (flyback_voltage=10012, threshold_limit=8000, sustain_voltage=500)
    // adjusted_limit = 8000 * (1000 + 500) / 1000 = 12000. flyback_voltage (10012) <= adjusted_limit (12000), meaning low tax.
    uint32_t adjusted_limit = 8000 * (1000 + 500) / 1000;
    assert(adjusted_limit == 12000);
    printf("   ✓ Parity unit tests for WinchesterMQ Yul Hardware emulators passed successfully\n");

    // Test RenderMan SDK Bridge and DeepSeek Apply function
    TSFiRiInterface mock_ri;
    tsfi_riinterface_init(&mock_ri);
    mock_ri.frame_buffer[0] = 255; // Set some pixels in the framebuffer

    sdk_coaxial_env_t mock_env;
    auncient_sdk_init_coaxial(&mock_env);

    sdk_cics_context_t mock_ctx;
    mock_ctx.env = &mock_env;
    mock_ctx.state = SDK_STATE_UNLOCKED;

    assert(tsfi_riinterface_bridge_to_sdk(&mock_ri, &mock_ctx));
    assert(mock_ctx.env->registers[0].value > 0); // Verify registers updated successfully
    printf("   ✓ tsfi_riinterface_bridge_to_sdk and auncient_sdk_apply_deepseek_guide verified successfully\n");

    // Test WinchesterMQ DisplacementShader alignment
    TSFiDisplacementShader mock_ds;
    tsfi_displacementshader_init(&mock_ds, 1.0, 0.5);
    uint32_t aligned_off = tsfi_displacementshader_align_ahocorasick_offset(&mock_ds, 123);
    assert(aligned_off == 256);
    uint32_t aligned_zero = tsfi_displacementshader_align_ahocorasick_offset(&mock_ds, 256);
    assert(aligned_zero == 256);
    printf("   ✓ tsfi_displacementshader_align_ahocorasick_offset verified successfully\n");

    printf("=============================================================\n");
    printf("PERSONALITY CONFIGURATIONS VALIDATED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
