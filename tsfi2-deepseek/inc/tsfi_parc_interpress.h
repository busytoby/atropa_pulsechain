#ifndef TSFI_PARC_INTERPRESS_H
#define TSFI_PARC_INTERPRESS_H

#include <stdint.h>

typedef struct {
    float x;
    float y;
} tsfi_parc_ip_point_t;

typedef struct {
    float matrix[6]; // 2D affine transform matrix [a, b, c, d, tx, ty]
    tsfi_parc_ip_point_t current_point;
} tsfi_parc_interpress_ctx_t;

/* Initialize Interpress PDL graphics context with identity matrix */
int tsfi_parc_interpress_init(tsfi_parc_interpress_ctx_t *ctx);

/* Apply rotation transformation to the context matrix */
int tsfi_parc_interpress_rotate(tsfi_parc_interpress_ctx_t *ctx, float angle_rad);

/* Apply scaling transformation to the context matrix */
int tsfi_parc_interpress_scale(tsfi_parc_interpress_ctx_t *ctx, float sx, float sy);

/* Move the path's current point to transformed (x,y) coordinates */
int tsfi_parc_interpress_moveto(tsfi_parc_interpress_ctx_t *ctx, float x, float y);

/* Draw a vector line from the current point to a transformed target point */
int tsfi_parc_interpress_lineto(tsfi_parc_interpress_ctx_t *ctx, uint32_t *pixels, int w, int h, float x, float y, uint32_t color);

#endif // TSFI_PARC_INTERPRESS_H
