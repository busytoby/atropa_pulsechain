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

int tsfi_quantel_storyboard_border_highlights_offset_width(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, uint32_t highlight_color) {
    if (!pixels || w <= 0 || h <= 0 || offset_w < 0 || border_w <= 0) return -1;
    for (int offset = 0; offset < border_w; offset++) {
        int tx = cell_x + offset_w + offset;
        int ty = cell_y + offset_w + offset;
        int tw = cell_w - 2 * (offset_w + offset);
        int th = cell_h - 2 * (offset_w + offset);
        if (tw <= 0 || th <= 0) { continue; }
        tsfi_quantel_storyboard_border_highlights(pixels, w, h, tx, ty, tw, th, highlight_color);
    }
    return 0;
}

int tsfi_quantel_storyboard_crosshairs(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    int mid_x = cell_x + cell_w / 2;
    int mid_y = cell_y + cell_h / 2;

    for (int x = cell_x; x < cell_x + cell_w; x++) {
        if (x >= 0 && x < w && mid_y >= 0 && mid_y < h) {
            pixels[mid_y * w + x] = color;
        }
    }
    for (int y = cell_y; y < cell_y + cell_h; y++) {
        if (y >= 0 && y < h && mid_x >= 0 && mid_x < w) {
            pixels[y * w + mid_x] = color;
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_outer_borders(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, uint32_t border_color) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    for (int y = cell_y; y < cell_y + cell_h; y++) {
        if (y < 0 || y >= h) continue;
        for (int x = cell_x; x < cell_x + cell_w; x++) {
            if (x < 0 || x >= w) continue;
            if (y == cell_y || y == cell_y + cell_h - 1 || x == cell_x || x == cell_x + cell_w - 1) {
                pixels[y * w + x] = border_color;
            }
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_inner_borders(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, uint32_t border_color) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    for (int y = cell_y + 1; y < cell_y + cell_h - 1; y++) {
        if (y < 0 || y >= h) continue;
        for (int x = cell_x + 1; x < cell_x + cell_w - 1; x++) {
            if (x < 0 || x >= w) continue;
            if (y == cell_y + 1 || y == cell_y + cell_h - 2 || x == cell_x + 1 || x == cell_x + cell_w - 2) {
                pixels[y * w + x] = border_color;
            }
        }
    }
    return 0;
}

extern void draw_text(uint32_t *pixels, int w, int h, int rx, int ry, const char *text, uint32_t color, int scale);

int tsfi_quantel_storyboard_grid(const uint32_t **frames, int frame_count, int frame_w, int frame_h, uint32_t *dst_sheet, int sheet_w, int sheet_h, int rows, int cols) {
    if (!frames || !dst_sheet || frame_w <= 0 || frame_h <= 0 || sheet_w <= 0 || sheet_h <= 0 || rows <= 0 || cols <= 0) return -1;

    memset(dst_sheet, 0x1F, sheet_w * sheet_h * sizeof(uint32_t));

    int cell_w = sheet_w / cols;
    int cell_h = sheet_h / rows;

    for (int i = 0; i < frame_count && i < (rows * cols); i++) {
        const uint32_t *src_frame = frames[i];
        if (!src_frame) continue;

        int grid_y = i / cols;
        int grid_x = i % cols;

        int start_x = grid_x * cell_w + (cell_w - frame_w) / 2;
        int start_y = grid_y * cell_h + (cell_h - frame_h) / 2;

        for (int y = 0; y < frame_h; y++) {
            int cy = start_y + y;
            if (cy < 0 || cy >= sheet_h) continue;
            uint32_t *dst_row = dst_sheet + cy * sheet_w;
            const uint32_t *src_row = src_frame + y * frame_w;

            for (int x = 0; x < frame_w; x++) {
                int cx = start_x + x;
                if (cx >= 0 && cx < sheet_w) {
                    dst_row[cx] = src_row[x];
                }
            }
        }

        char label[64];
        snprintf(label, sizeof(label), "PANEL %02d", i + 1);
        draw_text(dst_sheet, sheet_w, sheet_h, grid_x * cell_w + 10, (grid_y + 1) * cell_h - 20, label, 0xFF00FF00, 1);
        tsfi_quantel_storyboard_aspect_guides(dst_sheet, sheet_w, sheet_h, grid_x * cell_w, grid_y * cell_h, cell_w, cell_h, "1.85:1", 0xFFFFD700);
    }
    return 0;
}

int tsfi_quantel_storyboard_timecode_burn(uint32_t *pixels, int w, int h, int frame_number, float fps, uint32_t text_color) {
    if (!pixels || w <= 0 || h <= 0 || fps <= 0.0f) return -1;

    int total_secs = (int)(frame_number / fps);
    int frames = frame_number % (int)fps;
    int hours = total_secs / 3600;
    int mins = (total_secs % 3600) / 60;
    int secs = total_secs % 60;

    char tc_str[64];
    snprintf(tc_str, sizeof(tc_str), "TCR %02d:%02d:%02d:%02d", hours, mins, secs, frames);

    int rx = w - (int)(strlen(tc_str) * 8 * 2) - 20;
    int ry = h - 30;

    for (int y = ry - 4; y < ry + 20; y++) {
        if (y >= 0 && y < h) {
            uint32_t *row = pixels + y * w;
            for (int x = rx - 4; x < w - 10; x++) {
                if (x >= 0 && x < w) {
                    row[x] = 0xFF000000;
                }
            }
        }
    }

    draw_text(pixels, w, h, rx, ry, tc_str, text_color, 2);
    return 0;
}

int tsfi_quantel_storyboard_onion_skin(const uint32_t *prev_frame, const uint32_t *next_frame, uint32_t *active_canvas, int w, int h, float opacity_prev, float opacity_next) {
    if (!active_canvas || w <= 0 || h <= 0) return -1;

    for (int i = 0; i < w * h; i++) {
        uint32_t curr_pixel = active_canvas[i];
        uint8_t r_c = (curr_pixel >> 16) & 0xFF;
        uint8_t g_c = (curr_pixel >> 8) & 0xFF;
        uint8_t b_c = curr_pixel & 0xFF;

        float r_accum = r_c * (1.0f - opacity_prev - opacity_next);
        float g_accum = g_c * (1.0f - opacity_prev - opacity_next);
        float b_accum = b_c * (1.0f - opacity_prev - opacity_next);

        if (prev_frame) {
            uint32_t p_pixel = prev_frame[i];
            r_accum += ((p_pixel >> 16) & 0xFF) * opacity_prev;
            g_accum += ((p_pixel >> 8) & 0xFF) * opacity_prev;
            b_accum += (p_pixel & 0xFF) * opacity_prev;
        }

        if (next_frame) {
            uint32_t n_pixel = next_frame[i];
            r_accum += ((n_pixel >> 16) & 0xFF) * opacity_next;
            g_accum += ((n_pixel >> 8) & 0xFF) * opacity_next;
            b_accum += (n_pixel & 0xFF) * opacity_next;
        }

        int r = (int)r_accum;
        int g = (int)g_accum;
        int b = (int)b_accum;
        if (r < 0) { r = 0; }
        if (r > 255) { r = 255; }
        if (g < 0) { g = 0; }
        if (g > 255) { g = 255; }
        if (b < 0) { b = 0; }
        if (b > 255) { b = 255; }

        active_canvas[i] = (0xFF000000) | (r << 16) | (g << 8) | b;
    }
    return 0;
}

int tsfi_quantel_storyboard_export_sheet(const uint32_t *sheet_pixels, int w, int h, const char *output_path) {
    if (!sheet_pixels || !output_path || w <= 0 || h <= 0) return -1;
    FILE *f = fopen(output_path, "wb");
    if (!f) return -2;

    fprintf(f, "P6\n%d %d\n255\n", w, h);
    for (int i = 0; i < w * h; i++) {
        uint32_t pix = sheet_pixels[i];
        uint8_t rgb[3];
        rgb[0] = (pix >> 16) & 0xFF;
        rgb[1] = (pix >> 8) & 0xFF;
        rgb[2] = pix & 0xFF;
        size_t written = fwrite(rgb, 1, 3, f);
        (void)written;
    }
    fclose(f);
    return 0;
}

int tsfi_quantel_storyboard_burn_captions(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, const char *scene, const char *take, const char *desc, uint32_t text_color) {
    (void)cell_w;
    if (!pixels || w <= 0 || h <= 0) return -1;
    char meta[128];
    snprintf(meta, sizeof(meta), "SCENE: %s  TAKE: %s", scene, take);
    draw_text(pixels, w, h, cell_x + 10, cell_y + cell_h - 32, meta, text_color, 1);
    draw_text(pixels, w, h, cell_x + 10, cell_y + cell_h - 18, desc, text_color, 1);
    return 0;
}

int tsfi_quantel_storyboard_delta_overlay(const uint32_t *prev_frame, const uint32_t *next_frame, uint32_t *dst, int w, int h) {
    if (!prev_frame || !next_frame || !dst || w <= 0 || h <= 0) return -1;
    for (int i = 0; i < w * h; i++) {
        uint32_t p = prev_frame[i];
        uint32_t n = next_frame[i];

        int dr = abs((int)((p >> 16) & 0xFF) - (int)((n >> 16) & 0xFF));
        int dg = abs((int)((p >> 8) & 0xFF) - (int)((n >> 8) & 0xFF));
        int db = abs((int)(p & 0xFF) - (int)(n & 0xFF));

        dst[i] = (0xFF000000) | (dr << 16) | (dg << 8) | db;
    }
    return 0;
}

int tsfi_quantel_storyboard_drop_frame_timecode(int frame_number, float fps, char *timecode_out, int max_len) {
    if (!timecode_out || max_len <= 0) return -1;
    double fps_d = (double)fps;
    bool is_drop_frame = (fabs(fps_d - 29.97) < 0.01) || (fabs(fps_d - 59.94) < 0.01);
    
    int drop_frames = 2;
    if (fabs(fps_d - 59.94) < 0.01) drop_frames = 4;

    int total_minutes = frame_number / (int)(fps_d * 60.0);
    if (is_drop_frame) {
        int drop_count = total_minutes - total_minutes / 10;
        frame_number += drop_frames * drop_count;
    }

    int frames_per_sec = (int)round(fps_d);
    int total_secs = frame_number / frames_per_sec;
    int frames = frame_number % frames_per_sec;
    int hours = total_secs / 3600;
    int mins = (total_secs % 3600) / 60;
    int secs = total_secs % 60;

    snprintf(timecode_out, max_len, "%02d:%02d:%02d%c%02d", 
             hours, mins, secs, is_drop_frame ? ';' : ':', frames);
    return 0;
}

int tsfi_quantel_storyboard_aspect_guides(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, const char *ratio_str, uint32_t color) {
    (void)ratio_str;
    if (!pixels || w <= 0 || h <= 0) return -1;
    float aspect = 1.85f;

    int crop_w = cell_w - 30;
    int crop_h = (int)(crop_w / aspect);
    if (crop_h > cell_h) {
        crop_h = cell_h;
        crop_w = (int)(cell_h * aspect);
    }

    int ox = cell_x + 30 + (cell_w - 30 - crop_w) / 2;
    int oy = cell_y + (cell_h - crop_h) / 2;

    for (int y = oy; y < oy + crop_h; y++) {
        for (int x = cell_x; x < ox; x++) {
            int y_mod = y % 20;
            if (y_mod >= 6 && y_mod <= 14 && x >= cell_x + 8 && x <= cell_x + 18) {
                pixels[y * w + x] = 0xFF2A2820;
            } else {
                pixels[y * w + x] = 0xFF0D0D0D;
            }
        }
    }

    for (int x = ox; x < ox + crop_w; x++) {
        if (x >= 0 && x < w) {
            if (oy >= 0 && oy < h) pixels[oy * w + x] = color;
            if (oy + crop_h - 1 >= 0 && oy + crop_h - 1 < h) pixels[(oy + crop_h - 1) * w + x] = color;
        }
    }
    for (int y = oy; y < oy + crop_h; y++) {
        if (y >= 0 && y < h) {
            if (ox >= 0 && ox < w) pixels[y * w + ox] = color;
            if (ox + crop_w - 1 >= 0 && ox + crop_w - 1 < w) pixels[y * w + (ox + crop_w - 1)] = color;
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_burn_index(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int frame_idx, float fps, uint32_t text_color) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    char timecode[64];
    tsfi_quantel_storyboard_drop_frame_timecode(frame_idx, fps, timecode, sizeof(timecode));

    char index_str[128];
    snprintf(index_str, sizeof(index_str), "F: %05d  TC: %s", frame_idx, timecode);
    draw_text(pixels, w, h, cell_x + cell_w - 180, cell_y + cell_h - 18, index_str, text_color, 1);
    return 0;
}

int tsfi_quantel_storyboard_production_slate(uint32_t *pixels, int w, int h, const char *director, const char *project, const char *date) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    for (int y = 0; y < 45; y++) {
        for (int x = 0; x < w; x++) {
            pixels[y * w + x] = 0xFF1C1A17;
        }
    }

    char slate_info[256];
    snprintf(slate_info, sizeof(slate_info), "PROJECT: %s | DIR: %s | DATE: %s", project, director, date);
    draw_text(pixels, w, h, 20, 16, slate_info, 0xFFFFD700, 1);
    return 0;
}

int tsfi_quantel_storyboard_film_borders(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, uint32_t border_color) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    for (int y = cell_y; y < cell_y + cell_h; y++) {
        if (y < 0 || y >= h) continue;
        for (int x = cell_x; x < cell_x + cell_w; x++) {
            if (x < 0 || x >= w) continue;
            if (y < cell_y + 4 || y >= cell_y + cell_h - 4 || x < cell_x + 4 || x >= cell_x + cell_w - 4) {
                pixels[y * w + x] = border_color;
            }
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_page_divider(uint32_t *pixels, int w, int h, int y_coord, uint32_t line_color) {
    if (!pixels || w <= 0 || h <= 0 || y_coord < 0 || y_coord >= h) return -1;
    for (int x = 0; x < w; x++) {
        pixels[y_coord * w + x] = line_color;
    }
    return 0;
}

int tsfi_quantel_storyboard_thumbnail_shadows(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    for (int y = cell_y + 4; y < cell_y + cell_h + 4; y++) {
        if (y < 0 || y >= h) continue;
        uint32_t *row = pixels + y * w;
        for (int x = cell_x + 4; x < cell_x + cell_w + 4; x++) {
            if (x < 0 || x >= w) continue;
            if (y >= cell_y + cell_h || x >= cell_x + cell_w) {
                uint32_t pix = row[x];
                uint8_t r = (uint8_t)(((pix >> 16) & 0xFF) * 0.3f);
                uint8_t g = (uint8_t)(((pix >> 8) & 0xFF) * 0.3f);
                uint8_t b = (uint8_t)((pix & 0xFF) * 0.3f);
                row[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
            }
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_annotate_title(uint32_t *pixels, int w, int h, int x, int y, const char *title, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || !title) return -1;
    draw_text(pixels, w, h, x, y, title, color, 1);
    return 0;
}

int tsfi_quantel_storyboard_outline_cell(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, uint32_t outline_color) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    for (int y = cell_y; y < cell_y + cell_h; y++) {
        if (y < 0 || y >= h) continue;
        for (int x = cell_x; x < cell_x + cell_w; x++) {
            if (x < 0 || x >= w) continue;
            if (y < cell_y + 2 || y >= cell_y + cell_h - 2 || x < cell_x + 2 || x >= cell_x + cell_w - 2) {
                pixels[y * w + x] = outline_color;
            }
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_burn_label(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, const char *label, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || !label) return -1;
    (void)cell_w;
    draw_text(pixels, w, h, cell_x + 10, cell_y + cell_h - 38, label, color, 1);
    return 0;
}

int tsfi_quantel_storyboard_grid_spacers(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, uint32_t bg_color) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    for (int y = cell_y; y < cell_y + cell_h; y++) {
        if (y < 0 || y >= h) continue;
        for (int x = cell_x; x < cell_x + cell_w; x++) {
            if (x < 0 || x >= w) continue;
            if (y < cell_y + 10 || y >= cell_y + cell_h - 10 || x < cell_x + 10 || x >= cell_x + cell_w - 10) {
                pixels[y * w + x] = bg_color;
            }
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_cell_overlay(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, uint32_t border_color, float alpha) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    if (alpha < 0.0f) { alpha = 0.0f; }
    if (alpha > 1.0f) { alpha = 1.0f; }

    uint8_t r_src = (border_color >> 16) & 0xFF;
    uint8_t g_src = (border_color >> 8) & 0xFF;
    uint8_t b_src = border_color & 0xFF;

    for (int y = cell_y; y < cell_y + cell_h; y++) {
        if (y < 0 || y >= h) continue;
        uint32_t *row = pixels + y * w;
        for (int x = cell_x; x < cell_x + cell_w; x++) {
            if (x < 0 || x >= w) continue;
            uint32_t dest = row[x];
            uint8_t r_dst = (dest >> 16) & 0xFF;
            uint8_t g_dst = (dest >> 8) & 0xFF;
            uint8_t b_dst = dest & 0xFF;

            uint8_t r_res = (uint8_t)(r_src * alpha + r_dst * (1.0f - alpha));
            uint8_t g_res = (uint8_t)(g_src * alpha + g_dst * (1.0f - alpha));
            uint8_t b_res = (uint8_t)(b_src * alpha + b_dst * (1.0f - alpha));

            row[x] = (0xFF000000) | (r_res << 16) | (g_res << 8) | b_res;
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_border_margins(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int margin_w, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || margin_w <= 0) return -1;
    for (int y = cell_y; y < cell_y + cell_h; y++) {
        if (y < 0 || y >= h) continue;
        for (int x = cell_x; x < cell_x + cell_w; x++) {
            if (x < 0 || x >= w) continue;
            if (y < cell_y + margin_w || y >= cell_y + cell_h - margin_w || x < cell_x + margin_w || x >= cell_x + cell_w - margin_w) {
                pixels[y * w + x] = color;
            }
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_corner_marks(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int mark_len, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || mark_len <= 0) return -1;

    for (int i = 0; i < mark_len; i++) {
        if (cell_y >= 0 && cell_y < h && cell_x + i >= 0 && cell_x + i < w) { pixels[cell_y * w + cell_x + i] = color; }
        if (cell_y + i >= 0 && cell_y + i < h && cell_x >= 0 && cell_x < w) { pixels[(cell_y + i) * w + cell_x] = color; }

        if (cell_y >= 0 && cell_y < h && cell_x + cell_w - 1 - i >= 0 && cell_x + cell_w - 1 - i < w) { pixels[cell_y * w + (cell_x + cell_w - 1 - i)] = color; }
        if (cell_y + i >= 0 && cell_y + i < h && cell_x + cell_w - 1 >= 0 && cell_x + cell_w - 1 < w) { pixels[(cell_y + i) * w + (cell_x + cell_w - 1)] = color; }

        if (cell_y + cell_h - 1 >= 0 && cell_y + cell_h - 1 < h && cell_x + i >= 0 && cell_x + i < w) { pixels[(cell_y + cell_h - 1) * w + cell_x + i] = color; }
        if (cell_y + cell_h - 1 - i >= 0 && cell_y + cell_h - 1 - i < h && cell_x >= 0 && cell_x < w) { pixels[(cell_y + cell_h - 1 - i) * w + cell_x] = color; }

        if (cell_y + cell_h - 1 >= 0 && cell_y + cell_h - 1 < h && cell_x + cell_w - 1 - i >= 0 && cell_x + cell_w - 1 - i < w) { pixels[(cell_y + cell_h - 1) * w + (cell_x + cell_w - 1 - i)] = color; }
        if (cell_y + cell_h - 1 - i >= 0 && cell_y + cell_h - 1 - i < h && cell_x + cell_w - 1 >= 0 && cell_x + cell_w - 1 < w) { pixels[(cell_y + cell_h - 1 - i) * w + (cell_x + cell_w - 1)] = color; }
    }
    return 0;
}

int tsfi_quantel_storyboard_outer_highlights_offset(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || offset_w < 0) return -1;
    int tx = cell_x - offset_w;
    int ty = cell_y - offset_w;
    int tw = cell_w + 2 * offset_w;
    int th = cell_h + 2 * offset_w;
    if (tx < 0 || ty < 0 || tx + tw > w || ty + th > h) return -1;
    return tsfi_quantel_storyboard_border_highlights(pixels, w, h, tx, ty, tw, th, color);
}

int tsfi_quantel_storyboard_double_borders_offset_color(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, uint32_t color1, uint32_t color2) {
    if (!pixels || w <= 0 || h <= 0 || offset_w < 0) return -1;
    int tx1 = cell_x + offset_w;
    int ty1 = cell_y + offset_w;
    int tw1 = cell_w - 2 * offset_w;
    int th1 = cell_h - 2 * offset_w;
    if (tw1 > 0 && th1 > 0) {
        tsfi_quantel_storyboard_outer_borders(pixels, w, h, tx1, ty1, tw1, th1, color1);
        tsfi_quantel_storyboard_inner_borders(pixels, w, h, tx1, ty1, tw1, th1, color2);
    }
    return 0;
}

int tsfi_quantel_storyboard_border_highlights_offset_width_color(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || offset_w < 0 || border_w <= 0) return -1;
    for (int offset = 0; offset < border_w; offset++) {
        int tx = cell_x + offset_w + offset;
        int ty = cell_y + offset_w + offset;
        int tw = cell_w - 2 * (offset_w + offset);
        int th = cell_h - 2 * (offset_w + offset);
        if (tw <= 0 || th <= 0) { continue; }
        tsfi_quantel_storyboard_border_highlights(pixels, w, h, tx, ty, tw, th, color);
    }
    return 0;
}

int tsfi_quantel_storyboard_border_highlights_offset_width_color_outer(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || offset_w < 0 || border_w <= 0) return -1;
    for (int offset = 0; offset < border_w; offset++) {
        int tx = cell_x - offset_w - offset;
        int ty = cell_y - offset_w - offset;
        int tw = cell_w + 2 * (offset_w + offset);
        int th = cell_h + 2 * (offset_w + offset);
        if (tx < 0 || ty < 0 || tx + tw > w || ty + th > h) { continue; }
        tsfi_quantel_storyboard_border_highlights(pixels, w, h, tx, ty, tw, th, color);
    }
    return 0;
}

int tsfi_quantel_storyboard_border_highlights_offset_width_color_outer_double(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, uint32_t color1, uint32_t color2) {
    if (!pixels || w <= 0 || h <= 0 || offset_w < 0 || border_w <= 0) return -1;
    for (int offset = 0; offset < border_w; offset++) {
        int tx = cell_x - offset_w - offset;
        int ty = cell_y - offset_w - offset;
        int tw = cell_w + 2 * (offset_w + offset);
        int th = cell_h + 2 * (offset_w + offset);
        if (tx < 0 || ty < 0 || tx + tw > w || ty + th > h) { continue; }
        uint32_t color = (offset % 2 == 0) ? color1 : color2;
        tsfi_quantel_storyboard_border_highlights(pixels, w, h, tx, ty, tw, th, color);
    }
    return 0;
}

int tsfi_quantel_storyboard_border_highlights_concentric(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, int count, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || offset_w < 0 || border_w <= 0 || count <= 0) return -1;
    for (int c = 0; c < count; c++) {
        int current_offset = offset_w + c * border_w * 2;
        for (int offset = 0; offset < border_w; offset++) {
            int tx = cell_x - current_offset - offset;
            int ty = cell_y - current_offset - offset;
            int tw = cell_w + 2 * (current_offset + offset);
            int th = cell_h + 2 * (current_offset + offset);
            if (tx < 0 || ty < 0 || tx + tw > w || ty + th > h) { continue; }
            tsfi_quantel_storyboard_border_highlights(pixels, w, h, tx, ty, tw, th, color);
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_border_highlights_concentric_double(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, int count, uint32_t color1, uint32_t color2) {
    if (!pixels || w <= 0 || h <= 0 || offset_w < 0 || border_w <= 0 || count <= 0) return -1;
    for (int c = 0; c < count; c++) {
        int current_offset = offset_w + c * border_w * 2;
        uint32_t active_color = (c % 2 == 0) ? color1 : color2;
        for (int offset = 0; offset < border_w; offset++) {
            int tx = cell_x - current_offset - offset;
            int ty = cell_y - current_offset - offset;
            int tw = cell_w + 2 * (current_offset + offset);
            int th = cell_h + 2 * (current_offset + offset);
            if (tx < 0 || ty < 0 || tx + tw > w || ty + th > h) { continue; }
            tsfi_quantel_storyboard_border_highlights(pixels, w, h, tx, ty, tw, th, active_color);
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_border_highlights_concentric_double_outer(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, int count, uint32_t color1, uint32_t color2, int outer_margin) {
    if (!pixels || w <= 0 || h <= 0 || offset_w < 0 || border_w <= 0 || count <= 0) return -1;
    for (int c = 0; c < count; c++) {
        int current_offset = offset_w + c * border_w * 2;
        uint32_t active_color = (c % 2 == 0) ? color1 : color2;
        for (int offset = 0; offset < border_w; offset++) {
            int tx = cell_x - current_offset - offset;
            int ty = cell_y - current_offset - offset;
            int tw = cell_w + 2 * (current_offset + offset);
            int th = cell_h + 2 * (current_offset + offset);
            if (tx < outer_margin || ty < outer_margin || tx + tw > w - outer_margin || ty + th > h - outer_margin) { continue; }
            tsfi_quantel_storyboard_border_highlights(pixels, w, h, tx, ty, tw, th, active_color);
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_border_highlights_concentric_double_outer_width(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, int count, uint32_t color1, uint32_t color2, int outer_margin, int highlight_thickness) {
    if (!pixels || w <= 0 || h <= 0 || offset_w < 0 || border_w <= 0 || count <= 0 || highlight_thickness <= 0) return -1;
    for (int c = 0; c < count; c++) {
        int current_offset = offset_w + c * border_w * 2;
        uint32_t active_color = (c % 2 == 0) ? color1 : color2;
        for (int offset = 0; offset < border_w; offset++) {
            int tx = cell_x - current_offset - offset;
            int ty = cell_y - current_offset - offset;
            int tw = cell_w + 2 * (current_offset + offset);
            int th = cell_h + 2 * (current_offset + offset);
            if (tx < outer_margin || ty < outer_margin || tx + tw > w - outer_margin || ty + th > h - outer_margin) { continue; }
            for (int t = 0; t < highlight_thickness; t++) {
                tsfi_quantel_storyboard_border_highlights(pixels, w, h, tx - t, ty - t, tw + 2 * t, th + 2 * t, active_color);
            }
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_border_highlights_concentric_double_outer_width_offset(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, int count, uint32_t color1, uint32_t color2, int outer_margin, int highlight_thickness, int highlight_offset_x, int highlight_offset_y) {
    if (!pixels || w <= 0 || h <= 0 || offset_w < 0 || border_w <= 0 || count <= 0 || highlight_thickness <= 0) return -1;
    for (int c = 0; c < count; c++) {
        int current_offset = offset_w + c * border_w * 2;
        uint32_t active_color = (c % 2 == 0) ? color1 : color2;
        for (int offset = 0; offset < border_w; offset++) {
            int tx = cell_x - current_offset - offset + highlight_offset_x;
            int ty = cell_y - current_offset - offset + highlight_offset_y;
            int tw = cell_w + 2 * (current_offset + offset);
            int th = cell_h + 2 * (current_offset + offset);
            if (tx < outer_margin || ty < outer_margin || tx + tw > w - outer_margin || ty + th > h - outer_margin) { continue; }
            for (int t = 0; t < highlight_thickness; t++) {
                tsfi_quantel_storyboard_border_highlights(pixels, w, h, tx - t, ty - t, tw + 2 * t, th + 2 * t, active_color);
            }
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_border_highlights_concentric_double_outer_width_offset_color(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, int count, uint32_t color1, uint32_t color2, int outer_margin, int highlight_thickness, int highlight_offset_x, int highlight_offset_y, uint32_t shadow_color) {
    if (!pixels || w <= 0 || h <= 0 || offset_w < 0 || border_w <= 0 || count <= 0 || highlight_thickness <= 0) return -1;
    for (int c = 0; c < count; c++) {
        int current_offset = offset_w + c * border_w * 2;
        for (int offset = 0; offset < border_w; offset++) {
            int tx = cell_x - current_offset - offset + highlight_offset_x + 3;
            int ty = cell_y - current_offset - offset + highlight_offset_y + 3;
            int tw = cell_w + 2 * (current_offset + offset);
            int th = cell_h + 2 * (current_offset + offset);
            if (tx < outer_margin || ty < outer_margin || tx + tw > w - outer_margin || ty + th > h - outer_margin) { continue; }
            for (int t = 0; t < highlight_thickness; t++) {
                tsfi_quantel_storyboard_border_highlights(pixels, w, h, tx - t, ty - t, tw + 2 * t, th + 2 * t, shadow_color);
            }
        }
    }
    for (int c = 0; c < count; c++) {
        int current_offset = offset_w + c * border_w * 2;
        uint32_t active_color = (c % 2 == 0) ? color1 : color2;
        for (int offset = 0; offset < border_w; offset++) {
            int tx = cell_x - current_offset - offset + highlight_offset_x;
            int ty = cell_y - current_offset - offset + highlight_offset_y;
            int tw = cell_w + 2 * (current_offset + offset);
            int th = cell_h + 2 * (current_offset + offset);
            if (tx < outer_margin || ty < outer_margin || tx + tw > w - outer_margin || ty + th > h - outer_margin) { continue; }
            for (int t = 0; t < highlight_thickness; t++) {
                tsfi_quantel_storyboard_border_highlights(pixels, w, h, tx - t, ty - t, tw + 2 * t, th + 2 * t, active_color);
            }
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_border_highlights_concentric_double_outer_width_offset_color_texture(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, int count, uint32_t color1, uint32_t color2, int outer_margin, int highlight_thickness, int highlight_offset_x, int highlight_offset_y, uint32_t shadow_color, float texture_intensity) {
    if (!pixels || w <= 0 || h <= 0 || offset_w < 0 || border_w <= 0 || count <= 0 || highlight_thickness <= 0) return -1;
    for (int c = 0; c < count; c++) {
        int current_offset = offset_w + c * border_w * 2;
        for (int offset = 0; offset < border_w; offset++) {
            int tx = cell_x - current_offset - offset + highlight_offset_x + 3;
            int ty = cell_y - current_offset - offset + highlight_offset_y + 3;
            int tw = cell_w + 2 * (current_offset + offset);
            int th = cell_h + 2 * (current_offset + offset);
            if (tx < outer_margin || ty < outer_margin || tx + tw > w - outer_margin || ty + th > h - outer_margin) { continue; }
            for (int t = 0; t < highlight_thickness; t++) {
                tsfi_quantel_storyboard_border_highlights(pixels, w, h, tx - t, ty - t, tw + 2 * t, th + 2 * t, shadow_color);
            }
        }
    }
    for (int c = 0; c < count; c++) {
        int current_offset = offset_w + c * border_w * 2;
        uint32_t active_color = (c % 2 == 0) ? color1 : color2;
        for (int offset = 0; offset < border_w; offset++) {
            int tx = cell_x - current_offset - offset + highlight_offset_x;
            int ty = cell_y - current_offset - offset + highlight_offset_y;
            int tw = cell_w + 2 * (current_offset + offset);
            int th = cell_h + 2 * (current_offset + offset);
            if (tx < outer_margin || ty < outer_margin || tx + tw > w - outer_margin || ty + th > h - outer_margin) { continue; }
            for (int t = 0; t < highlight_thickness; t++) {
                float noise = (float)rand() / RAND_MAX * texture_intensity;
                uint8_t tr = (active_color >> 16) & 0xFF;
                uint8_t tg = (active_color >> 8) & 0xFF;
                uint8_t tb = active_color & 0xFF;
                tr = (uint8_t)(tr * (1.0f - noise));
                tg = (uint8_t)(tg * (1.0f - noise));
                tb = (uint8_t)(tb * (1.0f - noise));
                uint32_t textured_color = (0xFF000000) | (tr << 16) | (tg << 8) | tb;

                tsfi_quantel_storyboard_border_highlights(pixels, w, h, tx - t, ty - t, tw + 2 * t, th + 2 * t, textured_color);
            }
        }
    }
    return 0;
}

int tsfi_quantel_storyboard_border_highlights_concentric_double_outer_width_offset_color_texture_bevel(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, int count, uint32_t color1, uint32_t color2, int outer_margin, int highlight_thickness, int highlight_offset_x, int highlight_offset_y, uint32_t shadow_color, float texture_intensity, int bevel_thickness) {
    if (!pixels || w <= 0 || h <= 0 || offset_w < 0 || border_w <= 0 || count <= 0 || highlight_thickness <= 0) return -1;
    tsfi_quantel_storyboard_border_highlights_concentric_double_outer_width_offset_color_texture(pixels, w, h, cell_x, cell_y, cell_w, cell_h, offset_w, border_w, count, color1, color2, outer_margin, highlight_thickness, highlight_offset_x, highlight_offset_y, shadow_color, texture_intensity);

    for (int t = 0; t < bevel_thickness; t++) {
        int bx = cell_x + t + 1;
        int by = cell_y + t + 1;
        int bw = cell_w - 2 * (t + 1);
        int bh = cell_h - 2 * (t + 1);
        if (bx >= 0 && by >= 0 && bx + bw <= w && by + bh <= h) {
            for (int x = bx; x < bx + bw; x++) {
                pixels[by * w + x] = color1;
                pixels[(by + bh - 1) * w + x] = color2;
            }
            for (int y = by; y < by + bh; y++) {
                pixels[y * w + bx] = color1;
                pixels[y * w + (bx + bw - 1)] = color2;
            }
        }
    }
    return 0;
}
