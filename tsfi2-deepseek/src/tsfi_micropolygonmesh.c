#include "tsfi_micropolygonmesh.h"

void tsfi_micropolygonmesh_init(TSFiMicropolygonMesh *mpm) {
    if (!mpm) return;
    mpm->count = 0;
}

bool tsfi_micropolygonmesh_split(TSFiMicropolygonMesh *mpm, const TSFiMicropolygonBounds *parent) {
    if (!mpm || !parent || mpm->count + 4 > TSFI_MAX_MICROPOLYGONS) return false;
    
    double mid_x = (parent->min_x + parent->max_x) * 0.5;
    double mid_y = (parent->min_y + parent->max_y) * 0.5;
    
    // Subdivide parent into four sub-bounds representing micropolygon splits
    for (int i = 0; i < 4; i++) {
        TSFiMicropolygonBounds *sub = &mpm->polygons[mpm->count++];
        sub->min_z = parent->min_z;
        sub->max_z = parent->max_z;
        
        if (i == 0) {
            sub->min_x = parent->min_x; sub->max_x = mid_x;
            sub->min_y = parent->min_y; sub->max_y = mid_y;
        } else if (i == 1) {
            sub->min_x = mid_x;          sub->max_x = parent->max_x;
            sub->min_y = parent->min_y; sub->max_y = mid_y;
        } else if (i == 2) {
            sub->min_x = parent->min_x; sub->max_x = mid_x;
            sub->min_y = mid_y;          sub->max_y = parent->max_y;
        } else {
            sub->min_x = mid_x;          sub->max_x = parent->max_x;
            sub->min_y = mid_y;          sub->max_y = parent->max_y;
        }
    }
    return true;
}

bool tsfi_micropolygonmesh_cull_check(const TSFiMicropolygonMesh *mpm, int index, double frustum_z_limit) {
    if (!mpm || index < 0 || index >= mpm->count) return true;
    
    // O(1) constant time culling check against Z boundary limits
    if (mpm->polygons[index].max_z < frustum_z_limit) {
        return true; // Culled (behind near clipping plane boundary)
    }
    return false;
}
