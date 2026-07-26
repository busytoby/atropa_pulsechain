#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_geompoints.h"

int main(void) {
    printf("=== RUNNING AUNCIENT GEOMPOINTS PARTICLE TESTS ===\n");

    TSFiGeomPoints gp;
    tsfi_geompoints_init(&gp, "/auncient/points/emitter_1");

    assert(strcmp(gp.points_path, "/auncient/points/emitter_1") == 0);
    assert(gp.point_count == 0);

    float pos[3] = {0.0f, 0.0f, 10.0f};
    float vel[3] = {1.0f, 0.0f, 0.0f};
    float width = 2.5f;

    assert(tsfi_geompoints_add(&gp, pos, vel, width));
    assert(gp.point_count == 1);

    // Initial check
    assert(fabs(gp.points[0].position[2] - 10.0f) < 1e-5f);

    // Update physics by 0.1s
    tsfi_geompoints_update_physics(&gp, 0.1f);

    // Position X should have advanced, and Z should have decayed slightly due to gravity
    assert(gp.points[0].position[0] > 0.0f);
    assert(gp.points[0].position[2] < 10.0f);

    printf("   ✓ GeomPoints particle additions verified successfully.\n");
    printf("   ✓ Euler integration and downward gravity acceleration verified.\n");
    printf("=== AUNCIENT GEOMPOINTS PARTICLE TESTS COMPLETE (PASS) ===\n");
    return 0;
}
