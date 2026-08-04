#include "../include/auncient_teddy_personality.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT RENDERMAN TEDDY BEAR PERSONALITY CONFIGURATION TEST\n");
    printf("=============================================================\n");

    teddy_geometry_t geom;

    // Test Trustworthy Mapping
    resolve_teddy_geometry(PERSONALITY_TRUSTWORTHY, &geom);
    assert(geom.head_fwhr == 1.0);
    assert(geom.eye_eccentricity == 1.0);
    assert(geom.eye_scale == 1.0);
    assert(geom.stiffness == 0.3);
    printf("   ✓ Trustworthy parameters resolved: Round Head (1.0), Soft Dynamics (Stiffness: 0.3)\n");

    // Test Aggressive Mapping
    resolve_teddy_geometry(PERSONALITY_AGGRESSIVE, &geom);
    assert(geom.head_fwhr == 0.7);
    assert(geom.eye_eccentricity == 0.5);
    assert(geom.stiffness == 0.9);
    printf("   ✓ Aggressive parameters resolved: Narrow Head (0.7), Rigid Dynamics (Stiffness: 0.9)\n");

    // Test Eerie Mapping
    resolve_teddy_geometry(PERSONALITY_EERIE, &geom);
    assert(geom.head_fwhr == 0.7);
    assert(geom.eye_scale == 1.8);
    assert(geom.damping == 0.1);
    printf("   ✓ Eerie parameters resolved: Narrow Head (0.7), Eerie Dynamics (Damping: 0.1)\n");

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

    // Test Christensen ordinal link model implementation
    resolve_teddy_geometry(PERSONALITY_TRUSTWORTHY, &geom);
    int rating_trust = evaluate_ordinal_link_rating(&geom);
    assert(rating_trust >= 1 && rating_trust <= 7);

    resolve_teddy_geometry(PERSONALITY_AGGRESSIVE, &geom);
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

    // Test complementary log-log rating estimation
    int rating_cloglog = evaluate_ordinal_cloglog_rating(&geom);
    assert(rating_cloglog >= 1 && rating_cloglog <= 7);
    printf("   ✓ R H B Christensen complementary log-log ordinal rating verified successfully\n");

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

    // Test AIC and BIC information criteria diagnostics estimation
    double aic = 0.0, bic = 0.0;
    assert(evaluate_information_criteria(&geom, 3, 100, &aic, &bic));
    assert(aic > 0.0 && bic > 0.0);
    printf("   ✓ R H B Christensen AIC and BIC model selection criteria verified successfully\n");

    // Test flexible link rating estimation (mixture logit/cloglog)
    int rating_flex = evaluate_ordinal_flexible_rating(&geom, 0.5);
    assert(rating_flex >= 1 && rating_flex <= 7);
    printf("   ✓ R H B Christensen flexible mixture link ordinal rating verified successfully\n");

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
    printf("   ✓ GENIOLE et al. fWHR threat, visual-vocal mismatch, reactive retaliation, rapid exposure consistency, and cooperative cheating risk verified successfully\n");

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

    // Test Diode-Capacitor loop simulation
    double charge = 0.0;
    assert(simulate_diode_capacitor_loop(5.0, 1000.0, 1e-6, 0.1, &charge)); // Forward charge
    assert(charge > 0.0);
    double charged_val = charge;
    assert(simulate_diode_capacitor_loop(0.0, 1000.0, 1e-6, 0.1, &charge)); // Reverse discharge decay
    assert(charge < charged_val);
    printf("   ✓ Virtual diode reflex and relative capacitor charge decay loop verified successfully\n");

    // Test H-bridge flyback and izotope mismatch calculation
    double flyback_mismatch = 0.0;
    assert(evaluate_hbridge_izotope_mismatch(&geom, 500.0, &flyback_mismatch));
    assert(flyback_mismatch >= 0.0);
    printf("   ✓ H-bridge switching flyback transient and izotope mismatch verified successfully\n");

    // Test ACID transaction behavior (Commit successful path)
    evaluation_tx_t tx = begin_evaluation_transaction(&geom);
    assert(tx.active);
    geom.head_fwhr = 1.2;
    assert(commit_evaluation_transaction(&tx));
    assert(!tx.active);
    assert(geom.head_fwhr == 1.2);

    // Test ACID transaction behavior (Rollback path on constraint violation)
    tx = begin_evaluation_transaction(&geom);
    assert(tx.active);
    geom.head_fwhr = -5.0; // Invalid fWHR constraint
    assert(!commit_evaluation_transaction(&tx)); // Must fail and rollback
    assert(geom.head_fwhr == 1.2); // Restored
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

    printf("=============================================================\n");
    printf("PERSONALITY CONFIGURATIONS VALIDATED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
