#include "tsfi_geompoints.h"
#include <string.h>

void tsfi_geompoints_init(TSFiGeomPoints *gp, const char *path) {
    if (!gp) return;
    
    if (path) {
        strncpy(gp->points_path, path, sizeof(gp->points_path) - 1);
    } else {
        strcpy(gp->points_path, "/auncient/points/main");
    }
    
    gp->point_count = 0;
    memset(gp->points, 0, sizeof(gp->points));
}

bool tsfi_geompoints_add(TSFiGeomPoints *gp, const float pos[3], const float vel[3], float width) {
    if (!gp || !pos || !vel || gp->point_count >= TSFI_MAX_POINTS) return false;
    
    TSFiGeomPoint *pt = &gp->points[gp->point_count];
    pt->position[0] = pos[0];
    pt->position[1] = pos[1];
    pt->position[2] = pos[2];
    
    pt->velocity[0] = vel[0];
    pt->velocity[1] = vel[1];
    pt->velocity[2] = vel[2];
    
    pt->width = width;
    gp->point_count++;
    return true;
}

void tsfi_geompoints_update_physics(TSFiGeomPoints *gp, float delta_time) {
    if (!gp || delta_time <= 0.0f) return;
    
    for (int i = 0; i < gp->point_count; i++) {
        TSFiGeomPoint *pt = &gp->points[i];
        
        // Apply a gentle simulated downward gravity force on the Z axis (Semi-implicit Euler)
        pt->velocity[2] -= 9.8f * delta_time;
        
        // Update positions using velocity vectors
        pt->position[0] += pt->velocity[0] * delta_time;
        pt->position[1] += pt->velocity[1] * delta_time;
        pt->position[2] += pt->velocity[2] * delta_time;
    }
}
