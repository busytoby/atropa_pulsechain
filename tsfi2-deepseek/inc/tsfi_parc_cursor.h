#ifndef TSFI_PARC_CURSOR_H
#define TSFI_PARC_CURSOR_H

#include <stdint.h>

typedef enum {
    CURSOR_ARROW = 0,
    CURSOR_IBEAM = 1,
    CURSOR_CROSSHAIR = 2,
    CURSOR_WAIT = 3
} AltoCursorType;

/* Draw a classic Xerox Alto pixel cursor on the frame buffer canvas */
int tsfi_parc_draw_cursor(uint32_t *pixels, int w, int h, int cx, int cy, AltoCursorType type, uint32_t color);

#endif // TSFI_PARC_CURSOR_H
