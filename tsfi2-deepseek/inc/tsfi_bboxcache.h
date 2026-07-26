#ifndef TSFI_BBOXCACHE_H
#define TSFI_BBOXCACHE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    double min_extents[3];
    double max_extents[3];
    bool is_valid;
} TSFiBBoxCache;

// Initialize the BBoxCache context
void tsfi_bboxcache_init(TSFiBBoxCache *bbc);

// Sets the bounding box extents for a geometry block
void tsfi_bboxcache_set(TSFiBBoxCache *bbc, const double min_extents[3], const double max_extents[3]);

// Checks if a target coordinate falls within the cached boundaries
bool tsfi_bboxcache_contains(const TSFiBBoxCache *bbc, const double coord[3]);

// Invalidates the cache status
void tsfi_bboxcache_invalidate(TSFiBBoxCache *bbc);

#endif // TSFI_BBOXCACHE_H
