#ifndef TSFI_RENDERINDEX_H
#define TSFI_RENDERINDEX_H

#include <stdint.h>
#include <stdbool.h>
#include "tsfi_hydrascene.h"
#include "tsfi_renderdelegate.h"

#define MAX_INDEXED_PRIM_COUNT 16

typedef struct {
    char prim_path[64];
    bool is_dirty;
} RenderIndexPrim;

// Hydra Render Index context
typedef struct {
    RenderIndexPrim primitives[MAX_INDEXED_PRIM_COUNT];
    int prim_count;
    TSFiHydraScene *scene_delegate;
    TSFiRenderDelegate *render_delegate;
} TSFiRenderIndex;

// Initialize the Render Index context
void tsfi_renderindex_init(TSFiRenderIndex *index, TSFiHydraScene *scene, TSFiRenderDelegate *render);

// Registers a primitive path in the render index
bool tsfi_renderindex_register_prim(TSFiRenderIndex *index, const char *path);

// Synchronizes scene delegate changes to the render delegate, clearing dirty states
bool tsfi_renderindex_sync(TSFiRenderIndex *index);

#endif // TSFI_RENDERINDEX_H
