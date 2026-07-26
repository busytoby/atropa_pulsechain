#ifndef TSFI_GEOMPOINTS_H
#define TSFI_GEOMPOINTS_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_POINTS 32

typedef struct {
    float position[3];
    float velocity[3];
    float width;
} TSFiGeomPoint;

typedef struct {
    char points_path[64];
    TSFiGeomPoint points[TSFI_MAX_POINTS];
    int point_count;
} TSFiGeomPoints;

// Initialize the GeomPoints particle context
void tsfi_geompoints_init(TSFiGeomPoints *gp, const char *path);

// Adds a particle to the geometry buffer
bool tsfi_geompoints_add(TSFiGeomPoints *gp, const float pos[3], const float vel[3], float width);

// Updates the particle positions based on their velocity vector and time step delta_time
void tsfi_geompoints_update_physics(TSFiGeomPoints *gp, float delta_time);

#endif // TSFI_GEOMPOINTS_H
