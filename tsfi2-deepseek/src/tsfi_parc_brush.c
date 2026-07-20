#include <math.h>
#include <stdlib.h>
#include "tsfi_parc_brush.h"

int tsfi_parc_brush_draw(uint32_t *pixels, int w, int h, int cx, int cy, int size, SuperPaintBrushType type, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || size <= 0) return -1;

    int r = size / 2;
    for (int dy = -r; dy <= r; dy++) {
        int py = cy + dy;
        if (py < 0 || py >= h) continue;

        for (int dx = -r; dx <= r; dx++) {
            int px = cx + dx;
            if (px < 0 || px >= w) continue;

            int should_draw = 0;
            switch (type) {
                case BRUSH_CIRCLE: {
                    float dist = sqrtf((float)(dx * dx + dy * dy));
                    if (dist <= (float)r) {
                        should_draw = 1;
                    }
                    break;
                }
                case BRUSH_SQUARE:
                    should_draw = 1;
                    break;
                case BRUSH_CHISEL:
                    // diagonal line chisel brush
                    if (abs(dx - dy) <= 1) {
                        should_draw = 1;
                    }
                    break;
                case BRUSH_STAMP:
                    // dotted stamp pattern
                    if ((dx + dy) % 3 == 0) {
                        should_draw = 1;
                    }
                    break;
            }

            if (should_draw) {
                pixels[py * w + px] = color;
            }
        }
    }
    return 0;
}
