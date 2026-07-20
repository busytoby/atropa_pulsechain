#ifndef TSFI_PARC_BRUSH_H
#define TSFI_PARC_BRUSH_H

#include <stdint.h>

typedef enum {
    BRUSH_CIRCLE = 0,
    BRUSH_SQUARE = 1,
    BRUSH_CHISEL = 2,
    BRUSH_STAMP  = 3
} SuperPaintBrushType;

/* Draw/Stamp a brush shape onto the frame buffer canvas */
int tsfi_parc_brush_draw(uint32_t *pixels, int w, int h, int cx, int cy, int size, SuperPaintBrushType type, uint32_t color);

#endif // TSFI_PARC_BRUSH_H
