#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_singularityring.h"

int main(void) {
    printf("=== RUNNING AUNCIENT SINGULARITYRING MAGNETIC TESTS ===\n");

    TSFiSingularityRing sr;
    // Radius = 70.0, Intensity = 2.5 Tesla
    tsfi_singularityring_init(&sr, 70.0, 2.5);

    assert(fabs(sr.ring_radius - 70.0) < 1e-5);
    assert(fabs(sr.magnetic_intensity - 2.5) < 1e-5);

    // Initial check (no update yet)
    assert(fabs(tsfi_singularityring_get_flux(&sr) - 0.0) < 1e-5);

    // Update with close distance (1.0) and aligned angle (0.0 -> cos = 1.0)
    tsfi_singularityring_update(&sr, 0.01, 0.0, 1.0);
    double flux_close = tsfi_singularityring_get_flux(&sr);
    assert(flux_close > 0.0);

    // Update with far distance (10.0)
    tsfi_singularityring_update(&sr, 0.01, 0.0, 10.0);
    double flux_far = tsfi_singularityring_get_flux(&sr);
    assert(flux_far < flux_close);

    printf("   ✓ Spatial magnetic attenuation verified.\n");
    printf("   ✓ Singularity coupling flux equations verified successfully.\n");
    printf("=== AUNCIENT SINGULARITYRING MAGNETIC TESTS COMPLETE (PASS) ===\n");
    return 0;
}
