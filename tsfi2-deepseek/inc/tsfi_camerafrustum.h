#ifndef TSFI_CAMERAFRUSTUM_H
#define TSFI_CAMERAFRUSTUM_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float fov; // Field of view in degrees
    float aspect_ratio;
    float near_plane;
    float far_plane;
    
    // View bounds parameters
    float width_near;
    float height_near;
} TSFiCameraFrustum;

// Initialize the CameraFrustum context
void tsfi_camerafrustum_init(TSFiCameraFrustum *frustum, float fov, float aspect, float near_p, float far_p);

// Checks if a 3D sphere coordinate falls within the near and far clipping planes
bool tsfi_camerafrustum_contains_sphere(const TSFiCameraFrustum *frustum, const float center[3], float radius);

#endif // TSFI_CAMERAFRUSTUM_H
