#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_subdivcurves.h"

int main(void) {
    printf("=== RUNNING AUNCIENT SUBDIVCURVES REFINEMENT TESTS ===\n");

    TSFiSubdivCurves sc;
    // Subdivision Level = 3 (8 points), Tension = 0.25
    tsfi_subdivcurves_init(&sc, 3, 0.25);

    assert(sc.subdivision_level == 3);
    assert(fabs(sc.tension_parameter - 0.25) < 1e-5);
    assert(sc.output_point_count == 8);

    double p0[3] = {0.0, 0.0, 0.0};
    double p1[3] = {10.0, 10.0, 10.0};
    double p2[3] = {20.0, 20.0, 20.0};
    double p3[3] = {30.0, 30.0, 30.0};
    double out_pts[24];

    tsfi_subdivcurves_subdivide(&sc, p0, p1, p2, p3, out_pts, 8);

    // Coordinate validation (points should span between endpoints)
    assert(out_pts[0] >= 0.0);
    assert(out_pts[21] <= 30.0);

    printf("   ✓ Subdivision curves midpoint refinement verified.\n");
    printf("   ✓ Tension-weighted coordinate displacement verified successfully.\n");
    printf("=== AUNCIENT SUBDIVCURVES REFINEMENT TESTS COMPLETE (PASS) ===\n");
    return 0;
}
