#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/tsfi_displacementshader.h"
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

// Integrand for Euler Chapter 9 Section 1: f(t) = 1 / (1 + t)
static double euler_ch9_log1p_integrand(double t, void *user_data) {
    return 1.0 / (1.0 + t);
}

int main(void) {
    printf("================================================================================\n");
    printf(" PROOF: W. M. MCKEEMAN ADAPTIVE QUADRATURE DISPLACEMENT SHADER ENGINE\n");
    printf("================================================================================\n\n");

    TSFiDisplacementShader ds = {0};
    tsfi_displacementshader_init(&ds, 1.0, 1.0);

    // Integrate f(t) = 1/(1+t) from 0 to 0.2 using McKeeman Adaptive Quadrature over plane phi
    double a = 0.0;
    double b = 0.2;
    double tol = 1e-9;

    double result = tsfi_displacementshader_eval_mckeeman_adaptive_quadrature(&ds, a, b, tol, euler_ch9_log1p_integrand, NULL);
    double expected = log(1.2); // \ln(1 + 0.2) = 0.1823215567939546...

    printf("[MCKEEMAN ADAPTIVE QUADRATURE INTEGRATION OVER PLANE PHI (§ 286)]\n");
    printf("  - Domain Interval:             [%.1f, %.1f]\n", a, b);
    printf("  - Quadrature Tolerance:        %.1e\n", tol);
    printf("  - McKeeman Quad Result:        %.15f\n", result);
    printf("  - Expected ln(1.2) Value:      %.15f\n", expected);
    printf("  - Absolute Precision Error:    %.2e\n", fabs(result - expected));

    assert(fabs(result - expected) < 1e-8);
    printf("  - Integration Accuracy:        100%% SOUND (Sub-Nanosecond Precision)\n\n");

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] W. M. McKeeman Adaptive Quadrature Displacement Shader sound.\n");
    printf("================================================================================\n");

    return 0;
}
