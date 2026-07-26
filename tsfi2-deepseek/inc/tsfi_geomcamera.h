#ifndef TSFI_GEOMCAMERA_H
#define TSFI_GEOMCAMERA_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char camera_path[64];
    float focal_length;
    float horizontal_aperture;
    float vertical_aperture;
    float clipping_range[2]; // Near, Far planes
    
    // Projection matrix parameters
    float camera_transform[4][4];
} TSFiGeomCamera;

// Initialize the GeomCamera context
void tsfi_geomcamera_init(TSFiGeomCamera *camera, const char *path);

// Sets the camera rotation and translation vectors
void tsfi_geomcamera_set_transform(TSFiGeomCamera *camera, const float translation[3], const float rotation[3]);

// Computes the 3D projection matrix (perspective projection)
void tsfi_geomcamera_project(const TSFiGeomCamera *camera, const float vertex[3], float out_projected[3]);

#endif // TSFI_GEOMCAMERA_H
