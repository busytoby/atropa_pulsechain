#include "tsfi_camerafrustum.h"
#include <math.h>

void tsfi_camerafrustum_init(TSFiCameraFrustum *frustum, float fov, float aspect, float near_p, float far_p) {
    if (!frustum) return;
    
    frustum->fov = fov;
    frustum->aspect_ratio = aspect;
    frustum->near_plane = near_p;
    frustum->far_plane = far_p;
    
    // Calculate near plane width and height boundaries
    float rad = (fov * 0.5f) * (3.14159265f / 180.0f);
    frustum->height_near = 2.0f * tanf(rad) * near_p;
    frustum->width_near = frustum->height_near * aspect;
}

bool tsfi_camerafrustum_contains_sphere(const TSFiCameraFrustum *frustum, const float center[3], float radius) {
    if (!frustum || !center) return false;
    
    float z = center[2];
    
    // Check if the sphere coordinates fall completely outside the near or far clipping boundaries
    if ((z + radius) < frustum->near_plane) {
        return false;
    }
    
    if ((z - radius) > frustum->far_plane) {
        return false;
    }
    
    // Check horizontal and vertical perspective cone limits (simplified projection check)
    float depth_scale = z / frustum->near_plane;
    if (depth_scale > 0.0f) {
        float max_h = (frustum->height_near * 0.5f) * depth_scale;
        float max_w = (frustum->width_near * 0.5f) * depth_scale;
        
        if (fabsf(center[1]) - radius > max_h) {
            return false;
        }
        if (fabsf(center[0]) - radius > max_w) {
            return false;
        }
    }
    
    return true;
}
