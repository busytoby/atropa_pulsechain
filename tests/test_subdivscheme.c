#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_subdivscheme.h"

int main(void) {
    printf("=== RUNNING AUNCIENT SUBDIVSCHEME REFINEMENT TESTS ===\n");

    TSFiSubdivScheme ss;
    tsfi_subdivscheme_init(&ss, TSFI_SCHEME_CATMULLCLARK, TSFI_BOUNDARY_INTERPOLATE_EDGES);

    assert(ss.scheme == TSFI_SCHEME_CATMULLCLARK);
    assert(ss.boundary_rule == TSFI_BOUNDARY_INTERPOLATE_EDGES);
    assert(fabs(ss.smoothness_factor - 0.625) < 1e-5);

    double current_pos[3] = {10.0, 20.0, 30.0};
    double neighbors_avg[3] = {12.0, 22.0, 32.0};
    double out_pos[3];

    // Verify refinement output coordinate calculation
    tsfi_subdivscheme_refine_vertex(&ss, current_pos, neighbors_avg, out_pos);
    assert(out_pos[0] > 10.0 && out_pos[0] < 12.0);

    // Verify boundary culling override
    tsfi_subdivscheme_init(&ss, TSFI_SCHEME_CATMULLCLARK, TSFI_BOUNDARY_INTERPOLATE_NONE);
    tsfi_subdivscheme_refine_vertex(&ss, current_pos, neighbors_avg, out_pos);
    assert(fabs(out_pos[0] - 10.0) < 1e-5);

    printf("   ✓ Subdivision scheme vertex coordinate refinements verified.\n");
    printf("   ✓ Boundary rules coordinates interpolation overrides verified successfully.\n");
    printf("=== AUNCIENT SUBDIVSCHEME REFINEMENT TESTS COMPLETE (PASS) ===\n");
    return 0;
}
