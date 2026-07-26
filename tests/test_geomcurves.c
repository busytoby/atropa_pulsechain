#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_geomcurves.h"

int main(void) {
    printf("=== RUNNING AUNCIENT GEOMCURVES SPLINE TESTS ===\n");

    TSFiGeomCurves curves;
    tsfi_geomcurves_init(&curves, "/auncient/curves/hair_strand_1");

    assert(strcmp(curves.curve_path, "/auncient/curves/hair_strand_1") == 0);
    assert(curves.control_point_count == 0);

    float p0[3] = {0.0f, 0.0f, 0.0f};
    float p1[3] = {1.0f, 2.0f, 0.0f};
    float p2[3] = {2.0f, 0.0f, 3.0f};
    float p3[3] = {3.0f, 4.0f, 1.0f};

    assert(tsfi_geomcurves_add_point(&curves, p0));
    assert(tsfi_geomcurves_add_point(&curves, p1));
    assert(tsfi_geomcurves_add_point(&curves, p2));
    assert(tsfi_geomcurves_add_point(&curves, p3));
    assert(curves.control_point_count == 4);

    float out_pt[3];
    bool ok = tsfi_geomcurves_interpolate_b_spline(&curves, 0.5f, out_pt);
    assert(ok == true);

    // Verify coordinate logic
    assert(out_pt[0] > 0.0f);
    assert(out_pt[1] > 0.0f);

    printf("   ✓ GeomCurves control point buffer additions verified.\n");
    printf("   ✓ Cubic B-spline matrix coordinate interpolation verified.\n");
    printf("=== AUNCIENT GEOMCURVES SPLINE TESTS COMPLETE (PASS) ===\n");
    return 0;
}
