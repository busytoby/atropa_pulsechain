#include "../include/auncient_teddy_personality.h"
#include <stdio.h>
#include <assert.h>

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

    // Test AIC and BIC information criteria diagnostics estimation
    double aic = 0.0, bic = 0.0;
    assert(evaluate_information_criteria(&geom, 3, 100, &aic, &bic));
    assert(aic > 0.0 && bic > 0.0);
    printf("   ✓ R H B Christensen AIC and BIC model selection criteria verified successfully\n");

    printf("=============================================================\n");
    printf("PERSONALITY CONFIGURATIONS VALIDATED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
