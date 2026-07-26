#ifndef TSFI_GEOMXFORM_H
#define TSFI_GEOMXFORM_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char xform_path[64];
    float matrix[4][4];
} TSFiGeomXform;

// Initialize the GeomXform context with identity matrix
void tsfi_geomxform_init(TSFiGeomXform *xform, const char *path);

// Multiplies the current matrix by a translation matrix
void tsfi_geomxform_translate(TSFiGeomXform *xform, float tx, float ty, float tz);

// Multiplies the current matrix by a scaling matrix
void tsfi_geomxform_scale(TSFiGeomXform *xform, float sx, float sy, float sz);

// Transforms a 3D coordinate vector using the active matrix stack
void tsfi_geomxform_transform_vector(const TSFiGeomXform *xform, const float input[3], float output[3]);

#endif // TSFI_GEOMXFORM_H
