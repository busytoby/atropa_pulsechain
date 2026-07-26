#ifndef TSFI_MICROPOLYGONMESH_H
#define TSFI_MICROPOLYGONMESH_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_MICROPOLYGONS 256

typedef struct {
    double min_x, min_y, min_z;
    double max_x, max_y, max_z;
} TSFiMicropolygonBounds;

typedef struct {
    TSFiMicropolygonBounds polygons[TSFI_MAX_MICROPOLYGONS];
    int count;
} TSFiMicropolygonMesh;

// Initialize the MicropolygonMesh context
void tsfi_micropolygonmesh_init(TSFiMicropolygonMesh *mpm);

// Splits a parent polygon bounds into four micropolygons and adds them to the array
bool tsfi_micropolygonmesh_split(TSFiMicropolygonMesh *mpm, const TSFiMicropolygonBounds *parent);

// Performs constant-time O(1) visibility check of a micropolygon against a viewport frustum boundary
bool tsfi_micropolygonmesh_cull_check(const TSFiMicropolygonMesh *mpm, int index, double frustum_z_limit);

#endif // TSFI_MICROPOLYGONMESH_H
