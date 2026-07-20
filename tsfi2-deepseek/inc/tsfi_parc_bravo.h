#ifndef TSFI_PARC_BRAVO_H
#define TSFI_PARC_BRAVO_H

#include <stdint.h>

typedef struct {
    int margin_left;
    int margin_right;
    int margin_top;
    int margin_bottom;
    int current_column;
} tsfi_parc_bravo_layout_t;

/* Initialize Bravo layout parameters */
int tsfi_parc_bravo_init(tsfi_parc_bravo_layout_t *layout, int ml, int mr, int mt, int mb);

/* Render a dynamic page header annotation */
int tsfi_parc_bravo_draw_header(uint32_t *pixels, int w, int h, tsfi_parc_bravo_layout_t *layout, const char *title, uint32_t color);

/* Draw formatting layout guides (dashed margins) onto the canvas overlay */
int tsfi_parc_bravo_draw_guides(uint32_t *pixels, int w, int h, tsfi_parc_bravo_layout_t *layout, uint32_t guide_color);

#endif // TSFI_PARC_BRAVO_H
