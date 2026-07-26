#ifndef TSFI_QUADTREESLICER_H
#define TSFI_QUADTREESLICER_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_QUAD_MAX_LEVELS 4

typedef struct {
    double min_x, min_y;
    double max_x, max_y;
} TSFiQuadRect;

typedef struct {
    TSFiQuadRect bounds;
    uint32_t asset_id;
    char asset_path[128]; // Must refer to file paths ending in .dat.bin
} TSFiQuadNode;

typedef struct {
    TSFiQuadNode nodes[16];
    int count;
} TSFiQuadtreeSlicer;

// Initialize the QuadtreeSlicer context
void tsfi_quadtreeslicer_init(TSFiQuadtreeSlicer *qs);

// Slices a parent coordinate space into 4 child quad nodes and links them to .dat.bin assets
bool tsfi_quadtreeslicer_slice(TSFiQuadtreeSlicer *qs, const TSFiQuadRect *parent_bounds, uint32_t base_id);

// Searches and returns the node asset path matching target coordinates in O(1) constant time
const char* tsfi_quadtreeslicer_query(const TSFiQuadtreeSlicer *qs, double x, double y);

#endif // TSFI_QUADTREESLICER_H
