#include "tsfi_shadowmap.h"

void tsfi_shadowmap_init(TSFiShadowMap *sm) {
    if (!sm) return;
    for (int y = 0; y < TSFI_SHADOW_SIZE; y++) {
        for (int x = 0; x < TSFI_SHADOW_SIZE; x++) {
            sm->depth_grid[y][x] = 999.0; // Set to large background depth
        }
    }
}

bool tsfi_shadowmap_set_depth(TSFiShadowMap *sm, int x, int y, double depth) {
    if (!sm || x < 0 || x >= TSFI_SHADOW_SIZE || y < 0 || y >= TSFI_SHADOW_SIZE) return false;
    sm->depth_grid[y][x] = depth;
    return true;
}

bool tsfi_shadowmap_is_in_shadow(const TSFiShadowMap *sm, int x, int y, double point_depth) {
    if (!sm || x < 0 || x >= TSFI_SHADOW_SIZE || y < 0 || y >= TSFI_SHADOW_SIZE) return false;
    
    // Constant-time O(1) depth comparison (epsilon factor applied to prevent surface self-shadowing)
    double threshold_epsilon = 0.01;
    if (point_depth > sm->depth_grid[y][x] + threshold_epsilon) {
        return true; // Occluded by closer geometry
    }
    return false; // Illuminated
}
