#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_geomxform.h"

int main(void) {
    printf("=== RUNNING AUNCIENT GEOMXFORM MATRIX TESTS ===\n");

    TSFiGeomXform xform;
    tsfi_geomxform_init(&xform, "/auncient/xform/torus_mesh");

    assert(strcmp(xform.xform_path, "/auncient/xform/torus_mesh") == 0);

    // Apply translation: X += 5.0, Y += 2.0
    tsfi_geomxform_translate(&xform, 5.0f, 2.0f, 0.0f);

    // Apply scaling: X *= 2.0, Y *= 3.0
    tsfi_geomxform_scale(&xform, 2.0f, 3.0f, 1.0f);

    float input[3] = {1.0f, 1.0f, 0.0f};
    float output[3];
    tsfi_geomxform_transform_vector(&xform, input, output);

    // Expected transformation sequence on input (1,1,0):
    // 1. Scaled: X = 1.0 * 2.0 = 2.0; Y = 1.0 * 3.0 = 3.0
    // 2. Translated: X = 2.0 + 5.0 = 7.0; Y = 3.0 + 2.0 = 5.0
    assert(fabs(output[0] - 7.0f) < 1e-5f);
    assert(fabs(output[1] - 5.0f) < 1e-5f);

    printf("   ✓ GeomXform translation and scale multiplication verified.\n");
    printf("   ✓ Multi-layer matrix coordinate transformations verified.\n");
    printf("=== AUNCIENT GEOMXFORM MATRIX TESTS COMPLETE (PASS) ===\n");
    return 0;
}
