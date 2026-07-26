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

    double intensity = tsfi_hairshading_eval(&hs, tangent, light_dir, view_dir);
    assert(intensity > 0.0);
    assert(intensity <= 0.7);

    printf("   ✓ Anisotropic hair/fiber parameters initialized successfully.\n");
    printf("   ✓ Kajiya-Kay specular intensity verified successfully.\n");
    printf("=== AUNCIENT ANISOTROPIC HAIR SHADING TESTS COMPLETE (PASS) ===\n");
    return 0;
}
