#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_curvestension.h"

int main(void) {
    printf("=== RUNNING AUNCIENT CURVESTENSION TESTS ===\n");

    TSFiCurvesTension ct;
    tsfi_curvestension_init(&ct, 0.5);

    assert(fabs(ct.tension - 0.5) < 1e-5);
    assert(fabs(ct.scale_factor - 0.25) < 1e-5);

    double p0[3] = {10.0, 20.0, 30.0};
    double p2[3] = {14.0, 28.0, 42.0};
    double tangent[3];

    // Verify Catmull-Rom tangent calculation math
    tsfi_curvestension_calculate_tangent(&ct, p0, p2, tangent);
    assert(fabs(tangent[0] - 1.0) < 1e-5);
    assert(fabs(tangent[1] - 2.0) < 1e-5);
    assert(fabs(tangent[2] - 3.0) < 1e-5);

    // Verify maximum tension (tangent goes to zero)
    tsfi_curvestension_init(&ct, 1.0);
    tsfi_curvestension_calculate_tangent(&ct, p0, p2, tangent);
    assert(fabs(tangent[0]) < 1e-5);

    printf("   ✓ Spline tangent calculations verified.\n");
    printf("   ✓ Catmull-Rom tension scaling factors verified successfully.\n");
    printf("=== AUNCIENT CURVESTENSION TESTS COMPLETE (PASS) ===\n");
    return 0;
}
