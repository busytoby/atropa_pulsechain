#include "tsfi_renderindex.h"
#include <string.h>

void tsfi_renderindex_init(TSFiRenderIndex *index, TSFiHydraScene *scene, TSFiRenderDelegate *render) {
    if (!index) return;
    index->prim_count = 0;
    index->scene_delegate = scene;
    index->render_delegate = render;
    memset(index->primitives, 0, sizeof(index->primitives));
}

bool tsfi_renderindex_register_prim(TSFiRenderIndex *index, const char *path) {
    if (!index || !path || index->prim_count >= MAX_INDEXED_PRIM_COUNT) return false;
    
    // Check for duplicates
    for (int i = 0; i < index->prim_count; i++) {
        if (strcmp(index->primitives[i].prim_path, path) == 0) {
            index->primitives[i].is_dirty = true;
            return true;
        }
    }
    
    int idx = index->prim_count++;
    strncpy(index->primitives[idx].prim_path, path, sizeof(index->primitives[idx].prim_path) - 1);
    index->primitives[idx].is_dirty = true;
    return true;
}

bool tsfi_renderindex_sync(TSFiRenderIndex *index) {
    if (!index || !index->scene_delegate || !index->render_delegate) return false;
    
    bool did_sync = false;
    float points[8][3];
    if (tsfi_hydrascene_get_points(index->scene_delegate, points)) {
        for (int i = 0; i < index->prim_count; i++) {
            if (index->primitives[i].is_dirty) {
                // Draw primitives using the render delegate
                tsfi_renderdelegate_draw(index->render_delegate, points, 8);
                index->primitives[i].is_dirty = false;
                did_sync = true;
            }
        }
    }
    return did_sync;
}
