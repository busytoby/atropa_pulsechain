#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "../tsfi2-deepseek/inc/tsfi_hairshading.h"

int main(void) {
    printf("=== RUNNING AUNCIENT ANISOTROPIC HAIR SHADING TESTS ===\n");

    TSFiHairShading hs;
    tsfi_hairshading_init(&hs, 8.0, 0.7);

    assert(fabs(hs.specular_exponent - 8.0) < 1e-5);
    assert(fabs(hs.reflection_coeff - 0.7) < 1e-5);

    // Test parallel vectors
    double tangent[3] = {1.0, 0.0, 0.0};
    double light_dir[3] = {0.0, 1.0, 0.0};
    double view_dir[3] = {0.0, 1.0, 0.0};

    double intensity1 = tsfi_hairshading_eval(&hs, tangent, light_dir, view_dir);
    assert(intensity1 > 0.0);

    // Apply animatronic configurations (secondary reflection peak and mechanical jitter)
    tsfi_hairshading_set_animatronic(&hs, 0.1, 0.5);
    assert(fabs(hs.secondary_shift - 0.1) < 1e-5);
    assert(fabs(hs.jitter_amplitude - 0.5) < 1e-5);

    double intensity2 = tsfi_hairshading_eval(&hs, tangent, light_dir, view_dir);
    assert(intensity2 > 0.0);
    // Specular intensity changes under vibration/secondary peaks
    assert(fabs(intensity1 - intensity2) > 1e-5);

    printf("   ✓ Anisotropic hair/fiber parameters initialized successfully.\n");
    printf("   ✓ Kajiya-Kay specular intensity verified successfully.\n");
    printf("   ✓ Animatronic synthetic dual-peak and mechanical jitter verified successfully.\n");
    printf("=== AUNCIENT ANISOTROPIC HAIR SHADING TESTS COMPLETE (PASS) ===\n");
    return 0;
}
