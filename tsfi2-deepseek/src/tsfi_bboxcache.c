#include "tsfi_bboxcache.h"

void tsfi_bboxcache_init(TSFiBBoxCache *bbc) {
    if (!bbc) return;
    
    for (int i = 0; i < 3; i++) {
        bbc->min_extents[i] = 0.0;
        bbc->max_extents[i] = 0.0;
    }
    bbc->is_valid = false;
}

void tsfi_bboxcache_set(TSFiBBoxCache *bbc, const double min_extents[3], const double max_extents[3]) {
    if (!bbc || !min_extents || !max_extents) return;
    
    for (int i = 0; i < 3; i++) {
        bbc->min_extents[i] = min_extents[i];
        bbc->max_extents[i] = max_extents[i];
    }
    bbc->is_valid = true;
}

bool tsfi_bboxcache_contains(const TSFiBBoxCache *bbc, const double coord[3]) {
    if (!bbc || !coord || !bbc->is_valid) return false;
    
    for (int i = 0; i < 3; i++) {
        if (coord[i] < bbc->min_extents[i] || coord[i] > bbc->max_extents[i]) {
            return false;
        }
    }
    return true;
}

void tsfi_bboxcache_invalidate(TSFiBBoxCache *bbc) {
    if (!bbc) return;
    
    bbc->is_valid = false;
}
