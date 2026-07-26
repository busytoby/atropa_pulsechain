#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_subdivinterpolateboundary.h"

int main(void) {
    printf("=== RUNNING AUNCIENT SUBDIVINTERPOLATEBOUNDARY TESTS ===\n");

    TSFiSubdivInterpolateBoundary sib;
    tsfi_subdivinterpolateboundary_init(&sib, TSFI_INTERPOLATE_EDGE_ONLY);

    assert(sib.rule == TSFI_INTERPOLATE_EDGE_ONLY);
    assert(fabs(sib.rom_coefficient - 0.5) < 1e-5);

    double input_coord[3] = {10.0, 20.0, 30.0};
    double tangent[3] = {2.0, 4.0, 6.0};
    double out_coord[3];

    // Verify boundary coordinate adjustment math
    tsfi_subdivinterpolateboundary_adjust(&sib, input_coord, tangent, out_coord);
    assert(fabs(out_coord[0] - 11.0) < 1e-5);
    assert(fabs(out_coord[1] - 22.0) < 1e-5);
    assert(fabs(out_coord[2] - 33.0) < 1e-5);

    // Verify TSFI_INTERPOLATE_NONE rule
    tsfi_subdivinterpolateboundary_init(&sib, TSFI_INTERPOLATE_NONE);
    tsfi_subdivinterpolateboundary_adjust(&sib, input_coord, tangent, out_coord);
    assert(fabs(out_coord[0] - 10.0) < 1e-5);

    printf("   ✓ Boundary coordinate adjustment verified.\n");
    printf("   ✓ Catmull-Rom deterministic coefficients verified successfully.\n");
    printf("=== AUNCIENT SUBDIVINTERPOLATEBOUNDARY TESTS COMPLETE (PASS) ===\n");
    return 0;
}
