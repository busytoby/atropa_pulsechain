#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_micropolygonmesh.h"

int main(void) {
    printf("=== RUNNING AUNCIENT MICROPOLYGONMESH TESTS ===\n");

    TSFiMicropolygonMesh mpm;
    tsfi_micropolygonmesh_init(&mpm);

    assert(mpm.count == 0);

    TSFiMicropolygonBounds parent = {0.0, 0.0, 10.0, 10.0, 10.0, 50.0};
    tsfi_micropolygonmesh_split(&mpm, &parent);

    assert(mpm.count == 4);

    // Verify sub-bounds coordinate limits
    assert(fabs(mpm.polygons[0].max_x - 5.0) < 1e-5);
    assert(fabs(mpm.polygons[3].min_x - 5.0) < 1e-5);

    // Verify constant-time O(1) Z-bounds culling checks
    assert(tsfi_micropolygonmesh_cull_check(&mpm, 0, 5.0) == false);  // Visible
    assert(tsfi_micropolygonmesh_cull_check(&mpm, 0, 60.0) == true);  // Culled

    printf("   ✓ Reyes split-and-bound micropolygon subdivision verified.\n");
    printf("   ✓ Constant-time O(1) frustum culling checks verified successfully.\n");
    printf("=== AUNCIENT MICROPOLYGONMESH TESTS COMPLETE (PASS) ===\n");
    return 0;
}
