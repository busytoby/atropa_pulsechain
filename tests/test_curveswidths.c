#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_curveswidths.h"

int main(void) {
    printf("=== RUNNING AUNCIENT CURVESWIDTHS TESTS ===\n");

    TSFiCurvesWidths cw;
    tsfi_curveswidths_init(&cw);

    assert(cw.count == 0);

    tsfi_curveswidths_add(&cw, 2.0);
    tsfi_curveswidths_add(&cw, 4.0);
    tsfi_curveswidths_add(&cw, 8.0);

    assert(cw.count == 3);
    assert(fabs(cw.widths[0] - 2.0) < 1e-5);
    assert(fabs(cw.widths[2] - 8.0) < 1e-5);

    // Verify width interpolation math along the curve
    double w_start = tsfi_curveswidths_interpolate(&cw, 0.0);
    double w_mid = tsfi_curveswidths_interpolate(&cw, 0.25);
    double w_end = tsfi_curveswidths_interpolate(&cw, 1.0);

    assert(fabs(w_start - 2.0) < 1e-5);
    assert(fabs(w_mid - 3.0) < 1e-5);
    assert(fabs(w_end - 8.0) < 1e-5);

    printf("   ✓ Varying spline widths addition verified.\n");
    printf("   ✓ Curve widths segment interpolation verified successfully.\n");
    printf("=== AUNCIENT CURVESWIDTHS TESTS COMPLETE (PASS) ===\n");
    return 0;
}
