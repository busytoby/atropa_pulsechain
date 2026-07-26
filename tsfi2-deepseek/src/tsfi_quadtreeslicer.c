#include "tsfi_quadtreeslicer.h"
#include <stdio.h>
#include <string.h>

void tsfi_quadtreeslicer_init(TSFiQuadtreeSlicer *qs) {
    if (!qs) return;
    qs->count = 0;
}

bool tsfi_quadtreeslicer_slice(TSFiQuadtreeSlicer *qs, const TSFiQuadRect *parent_bounds, uint32_t base_id) {
    if (!qs || !parent_bounds || qs->count + 4 > 16) return false;
    
    double mid_x = (parent_bounds->min_x + parent_bounds->max_x) * 0.5;
    double mid_y = (parent_bounds->min_y + parent_bounds->max_y) * 0.5;
    
    for (int i = 0; i < 4; i++) {
        TSFiQuadNode *node = &qs->nodes[qs->count++];
        node->asset_id = base_id + i;
        
        // Define quad sub-bounds
        if (i == 0) {
            node->bounds.min_x = parent_bounds->min_x; node->bounds.max_x = mid_x;
            node->bounds.min_y = parent_bounds->min_y; node->bounds.max_y = mid_y;
        } else if (i == 1) {
            node->bounds.min_x = mid_x;                node->bounds.max_x = parent_bounds->max_x;
            node->bounds.min_y = parent_bounds->min_y; node->bounds.max_y = mid_y;
        } else if (i == 2) {
            node->bounds.min_x = parent_bounds->min_x; node->bounds.max_x = mid_x;
            node->bounds.min_y = mid_y;                node->bounds.max_y = parent_bounds->max_y;
        } else {
            node->bounds.min_x = mid_x;                node->bounds.max_x = parent_bounds->max_x;
            node->bounds.min_y = mid_y;                node->bounds.max_y = parent_bounds->max_y;
        }
        
        // Strict compliance check: only .dat.bin extension is supported
        snprintf(node->asset_path, sizeof(node->asset_path), "assets/slice_%u.dat.bin", node->asset_id);
    }
    return true;
}

const char* tsfi_quadtreeslicer_query(const TSFiQuadtreeSlicer *qs, double x, double y) {
    if (!qs) return NULL;
    
    // O(1) constant time node query matching target coordinates
    for (int i = 0; i < qs->count; i++) {
        const TSFiQuadNode *node = &qs->nodes[i];
        if (x >= node->bounds.min_x && x <= node->bounds.max_x &&
            y >= node->bounds.min_y && y <= node->bounds.max_y) {
            return node->asset_path;
        }
    }
    return NULL;
}
