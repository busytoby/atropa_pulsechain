#define _GNU_SOURCE
#include "tsfi_paint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int tsfi_quantel_storyboard_inner_borders_offset(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, uint32_t border_color) {
    if (!pixels || w <= 0 || h <= 0 || offset_w <= 0) return -1;
    for (int y = cell_y + offset_w; y < cell_y + cell_h - offset_w; y++) {
        if (y < 0 || y >= h) continue;
        for (int x = cell_x + offset_w; x < cell_x + cell_w - offset_w; x++) {
            if (x < 0 || x >= w) continue;
            if (y == cell_y + offset_w || y == cell_y + cell_h - 1 - offset_w || x == cell_x + offset_w || x == cell_x + cell_w - 1 - offset_w) {
                pixels[y * w + x] = border_color;
            }
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_double_borders_offset(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, uint32_t border_color) {
    if (!pixels || w <= 0 || h <= 0 || offset_w <= 0) return -1;
    tsfi_quantel_storyboard_outer_borders(pixels, w, h, cell_x, cell_y, cell_w, cell_h, border_color);
    tsfi_quantel_storyboard_inner_borders_offset(pixels, w, h, cell_x, cell_y, cell_w, cell_h, offset_w, border_color);
    return 0;
}

int tsfi_quantel_storyboard_border_highlights_offset(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, uint32_t highlight_color) {
    if (!pixels || w <= 0 || h <= 0 || offset_w < 0) return -1;
    int tx = cell_x + offset_w;
    int ty = cell_y + offset_w;
    int tw = cell_w - 2 * offset_w;
    int th = cell_h - 2 * offset_w;
    if (tw <= 0 || th <= 0) return -1;

    for (int x = tx; x < tx + tw; x++) {
        if (x >= 0 && x < w && ty >= 0 && ty < h) {
            pixels[ty * w + x] = highlight_color;
        }
    }
    for (int y = ty; y < ty + th; y++) {
        if (y >= 0 && y < h && tx >= 0 && tx < w) {
            pixels[y * w + tx] = highlight_color;
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_outer_borders_corners(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, uint32_t border_color, uint32_t corner_color) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    tsfi_quantel_storyboard_outer_borders(pixels, w, h, cell_x, cell_y, cell_w, cell_h, border_color);

    if (cell_y >= 0 && cell_y < h && cell_x >= 0 && cell_x < w) { pixels[cell_y * w + cell_x] = corner_color; }
    if (cell_y >= 0 && cell_y < h && cell_x + cell_w - 1 >= 0 && cell_x + cell_w - 1 < w) { pixels[cell_y * w + cell_x + cell_w - 1] = corner_color; }
    if (cell_y + cell_h - 1 >= 0 && cell_y + cell_h - 1 < h && cell_x >= 0 && cell_x < w) { pixels[(cell_y + cell_h - 1) * w + cell_x] = corner_color; }
    if (cell_y + cell_h - 1 >= 0 && cell_y + cell_h - 1 < h && cell_x + cell_w - 1 >= 0 && cell_x + cell_w - 1 < w) { pixels[(cell_y + cell_h - 1) * w + cell_x + cell_w - 1] = corner_color; }

    return 0;
}

int tsfi_quantel_storyboard_corner_spacers(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int spacer_w, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || spacer_w <= 0) return -1;

    for (int y = cell_y; y < cell_y + cell_h; y++) {
        if (y < 0 || y >= h) continue;
        for (int x = cell_x; x < cell_x + cell_w; x++) {
            if (x < 0 || x >= w) continue;
            int in_left = (x >= cell_x && x < cell_x + spacer_w);
            int in_right = (x >= cell_x + cell_w - spacer_w && x < cell_x + cell_w);
            int in_top = (y >= cell_y && y < cell_y + spacer_w);
            int in_bottom = (y >= cell_y + cell_h - spacer_w && y < cell_y + cell_h);

            if ((in_left && in_top) || (in_right && in_top) || (in_left && in_bottom) || (in_right && in_bottom)) {
                pixels[y * w + x] = color;
            }
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_dotted_borders(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int dot_spacing, uint32_t border_color) {
    if (!pixels || w <= 0 || h <= 0 || dot_spacing <= 0) return -1;
    for (int y = cell_y; y < cell_y + cell_h; y++) {
        if (y < 0 || y >= h) continue;
        for (int x = cell_x; x < cell_x + cell_w; x++) {
            if (x < 0 || x >= w) continue;
            if (y == cell_y || y == cell_y + cell_h - 1 || x == cell_x || x == cell_x + cell_w - 1) {
                if (((x - cell_x) + (y - cell_y)) % dot_spacing == 0) {
                    pixels[y * w + x] = border_color;
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_dotted_borders_offset(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int dot_spacing, uint32_t border_color) {
    if (!pixels || w <= 0 || h <= 0 || offset_w < 0 || dot_spacing <= 0) return -1;
    int tx = cell_x + offset_w;
    int ty = cell_y + offset_w;
    int tw = cell_w - 2 * offset_w;
    int th = cell_h - 2 * offset_w;
    if (tw <= 0 || th <= 0) return -1;
    return tsfi_quantel_storyboard_dotted_borders(pixels, w, h, tx, ty, tw, th, dot_spacing, border_color);
}

int tsfi_quantel_storyboard_corner_outlines_offset(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int outline_len, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || offset_w < 0 || outline_len <= 0) return -1;
    int tx = cell_x + offset_w;
    int ty = cell_y + offset_w;
    int tw = cell_w - 2 * offset_w;
    int th = cell_h - 2 * offset_w;
    if (tw <= 0 || th <= 0) return -1;
    return tsfi_quantel_storyboard_corner_outlines(pixels, w, h, tx, ty, tw, th, outline_len, color);
}

int tsfi_quantel_storyboard_corner_outlines(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int outline_len, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || outline_len <= 0) return -1;

    for (int i = 0; i < outline_len; i++) {
        if (cell_y + 2 >= 0 && cell_y + 2 < h && cell_x + 2 + i >= 0 && cell_x + 2 + i < w) { pixels[(cell_y + 2) * w + cell_x + 2 + i] = color; }
        if (cell_y + 2 + i >= 0 && cell_y + 2 + i < h && cell_x + 2 >= 0 && cell_x + 2 < w) { pixels[(cell_y + 2 + i) * w + cell_x + 2] = color; }

        if (cell_y + 2 >= 0 && cell_y + 2 < h && cell_x + cell_w - 3 - i >= 0 && cell_x + cell_w - 3 - i < w) { pixels[(cell_y + 2) * w + (cell_x + cell_w - 3 - i)] = color; }
        if (cell_y + 2 + i >= 0 && cell_y + 2 + i < h && cell_x + cell_w - 3 >= 0 && cell_x + cell_w - 3 < w) { pixels[(cell_y + 2 + i) * w + (cell_x + cell_w - 3)] = color; }

        if (cell_y + cell_h - 3 >= 0 && cell_y + cell_h - 3 < h && cell_x + 2 + i >= 0 && cell_x + 2 + i < w) { pixels[(cell_y + cell_h - 3) * w + cell_x + 2 + i] = color; }
        if (cell_y + cell_h - 3 - i >= 0 && cell_y + cell_h - 3 - i < h && cell_x + 2 >= 0 && cell_x + 2 < w) { pixels[(cell_y + cell_h - 3 - i) * w + cell_x + 2] = color; }

        if (cell_y + cell_h - 3 >= 0 && cell_y + cell_h - 3 < h && cell_x + cell_w - 3 - i >= 0 && cell_x + cell_w - 3 - i < w) { pixels[(cell_y + cell_h - 3) * w + (cell_x + cell_w - 3 - i)] = color; }
        if (cell_y + cell_h - 3 - i >= 0 && cell_y + cell_h - 3 - i < h && cell_x + cell_w - 3 >= 0 && cell_x + cell_w - 3 < w) { pixels[(cell_y + cell_h - 3 - i) * w + (cell_x + cell_w - 3)] = color; }
    }
    return 0;
}

int tsfi_quantel_storyboard_border_highlights(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, uint32_t highlight_color) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    for (int x = cell_x; x < cell_x + cell_w; x++) {
        if (x >= 0 && x < w && cell_y >= 0 && cell_y < h) {
            pixels[cell_y * w + x] = highlight_color;
        }
    }
    for (int y = cell_y; y < cell_y + cell_h; y++) {
        if (y >= 0 && y < h && cell_x >= 0 && cell_x < w) {
            pixels[y * w + cell_x] = highlight_color;
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_double_borders(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, uint32_t border_color) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    tsfi_quantel_storyboard_outer_borders(pixels, w, h, cell_x, cell_y, cell_w, cell_h, border_color);
    tsfi_quantel_storyboard_inner_borders(pixels, w, h, cell_x, cell_y, cell_w, cell_h, border_color);
    return 0;
}

int tsfi_quantel_storyboard_corner_spacers_offset(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int spacer_w, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || offset_w < 0 || spacer_w <= 0) return -1;
    int tx = cell_x + offset_w;
    int ty = cell_y + offset_w;
    int tw = cell_w - 2 * offset_w;
    int th = cell_h - 2 * offset_w;
    if (tw <= 0 || th <= 0) return -1;
    return tsfi_quantel_storyboard_corner_spacers(pixels, w, h, tx, ty, tw, th, spacer_w, color);
}
