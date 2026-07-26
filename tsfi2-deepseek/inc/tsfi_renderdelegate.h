#ifndef TSFI_RENDERDELEGATE_H
#define TSFI_RENDERDELEGATE_H

#include <stdint.h>
#include <stdbool.h>

// Render Delegate representation
typedef struct {
    char renderer_name[32];
    int drawn_primitive_count;
} TSFiRenderDelegate;

// Initialize the Render Delegate context
void tsfi_renderdelegate_init(TSFiRenderDelegate *render);

// Renders the points supplied by the Scene Delegate using simulated rasterization
bool tsfi_renderdelegate_draw(TSFiRenderDelegate *render, const float (*points)[3], int point_count);

#endif // TSFI_RENDERDELEGATE_H
