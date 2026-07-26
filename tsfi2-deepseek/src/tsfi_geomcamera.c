#include "tsfi_geomcamera.h"
#include <string.h>
#include <math.h>

void tsfi_geomcamera_init(TSFiGeomCamera *camera, const char *path) {
    if (!camera) return;
    
    if (path) {
        strncpy(camera->camera_path, path, sizeof(camera->camera_path) - 1);
    } else {
        strcpy(camera->camera_path, "/auncient/camera/main");
    }
    
    camera->focal_length = 50.0f;
    camera->horizontal_aperture = 36.0f;
    camera->vertical_aperture = 24.0f;
    camera->clipping_range[0] = 0.1f;
    camera->clipping_range[1] = 1000.0f;
    
    // Set identity transform matrix
    memset(camera->camera_transform, 0, sizeof(camera->camera_transform));
    camera->camera_transform[0][0] = 1.0f;
    camera->camera_transform[1][1] = 1.0f;
    camera->camera_transform[2][2] = 1.0f;
    camera->camera_transform[3][3] = 1.0f;
}

void tsfi_geomcamera_set_transform(TSFiGeomCamera *camera, const float translation[3], const float rotation[3]) {
    if (!camera) return;
    
    float cos_x = cosf(rotation[0]);
    float sin_x = sinf(rotation[0]);
    float cos_y = cosf(rotation[1]);
    float sin_y = sinf(rotation[1]);
    float cos_z = cosf(rotation[2]);
    float sin_z = sinf(rotation[2]);
    
    // Euler angles rotation matrix assembly
    camera->camera_transform[0][0] = cos_y * cos_z;
    camera->camera_transform[0][1] = -cos_y * sin_z;
    camera->camera_transform[0][2] = sin_y;
    camera->camera_transform[0][3] = translation[0];
    
    camera->camera_transform[1][0] = sin_x * sin_y * cos_z + cos_x * sin_z;
    camera->camera_transform[1][1] = -sin_x * sin_y * sin_z + cos_x * cos_z;
    camera->camera_transform[1][2] = -sin_x * cos_y;
    camera->camera_transform[1][3] = translation[1];
    
    camera->camera_transform[2][0] = -cos_x * sin_y * cos_z + sin_x * sin_z;
    camera->camera_transform[2][1] = cos_x * sin_y * sin_z + sin_x * cos_z;
    camera->camera_transform[2][2] = cos_x * cos_y;
    camera->camera_transform[2][3] = translation[2];
    
    camera->camera_transform[3][0] = 0.0f;
    camera->camera_transform[3][1] = 0.0f;
    camera->camera_transform[3][2] = 0.0f;
    camera->camera_transform[3][3] = 1.0f;
}

void tsfi_geomcamera_project(const TSFiGeomCamera *camera, const float vertex[3], float out_projected[3]) {
    if (!camera || !vertex || !out_projected) return;
    
    // Transform vertex to camera coordinate system
    float cx = camera->camera_transform[0][0] * vertex[0] + camera->camera_transform[0][1] * vertex[1] + camera->camera_transform[0][2] * vertex[2] + camera->camera_transform[0][3];
    float cy = camera->camera_transform[1][0] * vertex[0] + camera->camera_transform[1][1] * vertex[1] + camera->camera_transform[1][2] * vertex[2] + camera->camera_transform[1][3];
    float cz = camera->camera_transform[2][0] * vertex[0] + camera->camera_transform[2][1] * vertex[1] + camera->camera_transform[2][2] * vertex[2] + camera->camera_transform[2][3];
    
    // Perspective projection using focal length
    float f_scale = camera->focal_length / 35.0f;
    if (fabsf(cz) > 1e-4f) {
        out_projected[0] = (cx * f_scale) / cz;
        out_projected[1] = (cy * f_scale) / cz;
    } else {
        out_projected[0] = cx * f_scale;
        out_projected[1] = cy * f_scale;
    }
    out_projected[2] = cz;
}
