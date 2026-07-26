#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "../tsfi2-deepseek/inc/tsfi_sss.h"

int main(void) {
    printf("=== RUNNING AUNCIENT SUBSURFACE SHADING TESTS ===\n");

    TSFiSubsurfaceScattering sss;
    tsfi_sss_init(&sss, 2.0, 0.2);

    assert(fabs(sss.scatter_radius - 2.0) < 1e-5);
    assert(fabs(sss.absorption_coeff - 0.2) < 1e-5);

    // Test zero thickness transmittance has no attenuation
    double t0 = tsfi_sss_eval(&sss, 0.0, 1.0);
    assert(fabs(t0 - 1.0) < 1e-5);

    // Test light decay through translucent material
    double t1 = tsfi_sss_eval(&sss, 1.0, 1.0);
    assert(t1 < 1.0);
    assert(t1 > 0.0);

    printf("   ✓ Sub-surface scattering parameters initialized successfully.\n");
    printf("   ✓ Dipole-decay transmissive light attenuation verified successfully.\n");
    printf("=== AUNCIENT SUBSURFACE SHADING TESTS COMPLETE (PASS) ===\n");
    return 0;
}
