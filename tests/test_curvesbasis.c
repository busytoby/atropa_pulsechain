#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_curvesbasis.h"

int main(void) {
    printf("=== RUNNING AUNCIENT CURVESBASIS SPLINE TESTS ===\n");

    TSFiCurvesBasis cb;
    tsfi_curvesbasis_init(&cb, TSFI_BASIS_BEZIER, TSFI_WRAP_NONPERIODIC);

    assert(cb.basis == TSFI_BASIS_BEZIER);
    assert(cb.wrap == TSFI_WRAP_NONPERIODIC);

    double p0[3] = {0.0, 0.0, 0.0};
    double p1[3] = {10.0, 20.0, 30.0};
    double p2[3] = {20.0, 40.0, 60.0};
    double p3[3] = {30.0, 60.0, 90.0};
    double out[3];

    // Evaluate Bezier at t = 0.0 (should equal p0)
    tsfi_curvesbasis_interpolate(&cb, 0.0, p0, p1, p2, p3, out);
    assert(fabs(out[0] - 0.0) < 1e-5);
    assert(fabs(out[1] - 0.0) < 1e-5);

    // Evaluate Bezier at t = 1.0 (should equal p3)
    tsfi_curvesbasis_interpolate(&cb, 1.0, p0, p1, p2, p3, out);
    assert(fabs(out[0] - 30.0) < 1e-5);
    assert(fabs(out[1] - 60.0) < 1e-5);

    // Switch to B-spline basis
    tsfi_curvesbasis_init(&cb, TSFI_BASIS_BSPLINE, TSFI_WRAP_NONPERIODIC);
    tsfi_curvesbasis_interpolate(&cb, 0.5, p0, p1, p2, p3, out);
    assert(out[0] > 0.0);

    printf("   ✓ Bezier basis coordinate interpolation verified.\n");
    printf("   ✓ B-spline basis coordinate interpolation verified successfully.\n");
    printf("=== AUNCIENT CURVESBASIS SPLINE TESTS COMPLETE (PASS) ===\n");
    return 0;
}
