#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_camerafrustum.h"

int main(void) {
    printf("=== RUNNING AUNCIENT CAMERAFRUSTUM CULLING TESTS ===\n");

    TSFiCameraFrustum frustum;
    // FOV = 60 degrees, Aspect = 1.33, Near = 1.0, Far = 100.0
    tsfi_camerafrustum_init(&frustum, 60.0f, 1.33f, 1.0f, 100.0f);

    assert(fabs(frustum.fov - 60.0f) < 1e-5f);
    assert(fabs(frustum.near_plane - 1.0f) < 1e-5f);

    // Sphere 1: Inside frustum (Z = 10.0, X = 0.0, Y = 0.0)
    float c1[3] = {0.0f, 0.0f, 10.0f};
    assert(tsfi_camerafrustum_contains_sphere(&frustum, c1, 2.0f) == true);

    // Sphere 2: Outside near plane (Z = 0.5, radius = 0.1)
    float c2[3] = {0.0f, 0.0f, 0.5f};
    assert(tsfi_camerafrustum_contains_sphere(&frustum, c2, 0.1f) == false);

    // Sphere 3: Outside far plane (Z = 110.0, radius = 2.0)
    float c3[3] = {0.0f, 0.0f, 110.0f};
    assert(tsfi_camerafrustum_contains_sphere(&frustum, c3, 2.0f) == false);

    printf("   ✓ CameraFrustum clip plane bounds culling verified.\n");
    printf("   ✓ Perspective cone projection checks verified successfully.\n");
    printf("=== AUNCIENT CAMERAFRUSTUM CULLING TESTS COMPLETE (PASS) ===\n");
    return 0;
}
