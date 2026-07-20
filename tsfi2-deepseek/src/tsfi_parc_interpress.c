#include <math.h>
#include <stdlib.h>
#include "tsfi_parc_interpress.h"

int tsfi_parc_interpress_init(tsfi_parc_interpress_ctx_t *ctx) {
    if (!ctx) return -1;
    // Set identity matrix: [1, 0, 0, 1, 0, 0]
    ctx->matrix[0] = 1.0f; ctx->matrix[1] = 0.0f;
    ctx->matrix[2] = 0.0f; ctx->matrix[3] = 1.0f;
    ctx->matrix[4] = 0.0f; ctx->matrix[5] = 0.0f;
    ctx->current_point.x = 0.0f;
    ctx->current_point.y = 0.0f;
    return 0;
}

int tsfi_parc_interpress_rotate(tsfi_parc_interpress_ctx_t *ctx, float angle_rad) {
    if (!ctx) return -1;
    float cos_a = cosf(angle_rad);
    float sin_a = sinf(angle_rad);
    float m0 = ctx->matrix[0] * cos_a + ctx->matrix[2] * sin_a;
    float m1 = ctx->matrix[1] * cos_a + ctx->matrix[3] * sin_a;
    float m2 = ctx->matrix[0] * -sin_a + ctx->matrix[2] * cos_a;
    float m3 = ctx->matrix[1] * -sin_a + ctx->matrix[3] * cos_a;
    ctx->matrix[0] = m0; ctx->matrix[1] = m1;
    ctx->matrix[2] = m2; ctx->matrix[3] = m3;
    return 0;
}

int tsfi_parc_interpress_scale(tsfi_parc_interpress_ctx_t *ctx, float sx, float sy) {
    if (!ctx) return -1;
    ctx->matrix[0] *= sx;
    ctx->matrix[1] *= sx;
    ctx->matrix[2] *= sy;
    ctx->matrix[3] *= sy;
    return 0;
}

static void transform_point(const tsfi_parc_interpress_ctx_t *ctx, float x, float y, float *tx, float *ty) {
    *tx = ctx->matrix[0] * x + ctx->matrix[2] * y + ctx->matrix[4];
    *ty = ctx->matrix[1] * x + ctx->matrix[3] * y + ctx->matrix[5];
}

int tsfi_parc_interpress_moveto(tsfi_parc_interpress_ctx_t *ctx, float x, float y) {
    if (!ctx) return -1;
    transform_point(ctx, x, y, &ctx->current_point.x, &ctx->current_point.y);
    return 0;
}

int tsfi_parc_interpress_lineto(tsfi_parc_interpress_ctx_t *ctx, uint32_t *pixels, int w, int h, float x, float y, uint32_t color) {
    if (!ctx || !pixels || w <= 0 || h <= 0) return -1;

    float x1 = ctx->current_point.x;
    float y1 = ctx->current_point.y;
    float x2, y2;
    transform_point(ctx, x, y, &x2, &y2);

    // Update current point
    ctx->current_point.x = x2;
    ctx->current_point.y = y2;

    // Classical Bresenham line drawing algorithm
    int ix1 = (int)x1;
    int iy1 = (int)y1;
    int ix2 = (int)x2;
    int iy2 = (int)y2;

    int dx = abs(ix2 - ix1);
    int dy = abs(iy2 - iy1);
    int sx = (ix1 < ix2) ? 1 : -1;
    int sy = (iy1 < iy2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        if (ix1 >= 0 && ix1 < w && iy1 >= 0 && iy1 < h) {
            pixels[iy1 * w + ix1] = color;
        }
        if (ix1 == ix2 && iy1 == iy2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            ix1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            iy1 += sy;
        }
    }

    return 0;
}
