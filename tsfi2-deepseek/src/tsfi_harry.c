#define _GNU_SOURCE
#include "tsfi_paint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int tsfi_quantel_harry_chroma_key(const uint32_t *src_pixels, int w, int h, uint8_t *out_mask, uint32_t key_color, float tolerance) {
    if (!src_pixels || !out_mask || w <= 0 || h <= 0) return -1;

    uint8_t kr = (key_color >> 16) & 0xFF;
    uint8_t kg = (key_color >> 8) & 0xFF;
    uint8_t kb = key_color & 0xFF;

    for (int i = 0; i < w * h; i++) {
        uint32_t pix = src_pixels[i];
        uint8_t r = (pix >> 16) & 0xFF;
        uint8_t g = (pix >> 8) & 0xFF;
        uint8_t b = pix & 0xFF;

        float dr = r - kr;
        float dg = g - kg;
        float db = b - kb;
        float dist = sqrtf(dr * dr + dg * dg + db * db);

        if (dist < tolerance) {
            out_mask[i] = 0;
        } else {
            out_mask[i] = 255;
        }
    }
    return 0;
}

int tsfi_quantel_harry_blend(const uint32_t *fg_pixels, const uint32_t *bg_pixels, uint32_t *dst_pixels, int w, int h, const uint8_t *mask, const char *blend_mode) {
    if (!fg_pixels || !bg_pixels || !dst_pixels || w <= 0 || h <= 0) return -1;

    for (int i = 0; i < w * h; i++) {
        float alpha = mask ? (mask[i] / 255.0f) : 1.0f;
        uint32_t fg = fg_pixels[i];
        uint32_t bg = bg_pixels[i];

        uint8_t r_f = (fg >> 16) & 0xFF;
        uint8_t g_f = (fg >> 8) & 0xFF;
        uint8_t b_f = fg & 0xFF;

        uint8_t r_b = (bg >> 16) & 0xFF;
        uint8_t g_b = (bg >> 8) & 0xFF;
        uint8_t b_b = bg & 0xFF;

        uint8_t r_blended = r_f;
        uint8_t g_blended = g_f;
        uint8_t b_blended = b_f;

        if (strcmp(blend_mode, "multiply") == 0) {
            r_blended = (uint8_t)((r_f * r_b) / 255);
            g_blended = (uint8_t)((g_f * g_b) / 255);
            b_blended = (uint8_t)((b_f * b_b) / 255);
        } else if (strcmp(blend_mode, "screen") == 0) {
            r_blended = (uint8_t)(255 - ((255 - r_f) * (255 - r_b)) / 255);
            g_blended = (uint8_t)(255 - ((255 - g_f) * (255 - g_b)) / 255);
            b_blended = (uint8_t)(255 - ((255 - b_f) * (255 - b_b)) / 255);
        }

        uint8_t r_res = (uint8_t)(r_blended * alpha + r_b * (1.0f - alpha));
        uint8_t g_res = (uint8_t)(g_blended * alpha + g_b * (1.0f - alpha));
        uint8_t b_res = (uint8_t)(b_blended * alpha + b_b * (1.0f - alpha));

        dst_pixels[i] = (0xFF000000) | (r_res << 16) | (g_res << 8) | b_res;
    }
    return 0;
}

int tsfi_quantel_harry_wipe(const uint32_t *src_a, const uint32_t *src_b, uint32_t *dst, int w, int h, float progress, const char *wipe_type) {
    if (!src_a || !src_b || !dst || w <= 0 || h <= 0) return -1;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = y * w + x;
            bool show_b = false;

            if (strcmp(wipe_type, "radial") == 0) {
                float dx = x - (w / 2.0f);
                float dy = y - (h / 2.0f);
                float angle = atan2f(dy, dx) + M_PI;
                float normalized_angle = angle / (2.0f * M_PI);
                show_b = (normalized_angle < progress);
            } else {
                float px = (float)x / w;
                show_b = (px < progress);
            }

            dst[idx] = show_b ? src_b[idx] : src_a[idx];
        }
    }
    return 0;
}

int tsfi_quantel_harry_invert(uint32_t *pixels, int w, int h) {
    if (!pixels || w <= 0 || h <= 0) return -1;

    for (int i = 0; i < w * h; i++) {
        uint32_t pix = pixels[i];
        uint8_t r = 255 - ((pix >> 16) & 0xFF);
        uint8_t g = 255 - ((pix >> 8) & 0xFF);
        uint8_t b = 255 - (pix & 0xFF);
        pixels[i] = (0xFF000000) | (r << 16) | (g << 8) | b;
    }
    return 0;
}

int tsfi_quantel_harry_contrast_adjust(uint32_t *pixels, int w, int h, float brightness, float contrast) {
    if (!pixels || w <= 0 || h <= 0) return -1;

    for (int i = 0; i < w * h; i++) {
        uint32_t pix = pixels[i];
        float r = (pix >> 16) & 0xFF;
        float g = (pix >> 8) & 0xFF;
        float b = pix & 0xFF;

        r = (r - 128.0f) * contrast + 128.0f + brightness;
        g = (g - 128.0f) * contrast + 128.0f + brightness;
        b = (b - 128.0f) * contrast + 128.0f + brightness;

        int ir = (int)r;
        int ig = (int)g;
        int ib = (int)b;
        if (ir < 0) { ir = 0; }
        if (ir > 255) { ir = 255; }
        if (ig < 0) { ig = 0; }
        if (ig > 255) { ig = 255; }
        if (ib < 0) { ib = 0; }
        if (ib > 255) { ib = 255; }

        pixels[i] = (0xFF000000) | (ir << 16) | (ig << 8) | ib;
    }
    return 0;
}

int tsfi_quantel_harry_posterize(uint32_t *pixels, int w, int h, int levels) {
    if (!pixels || w <= 0 || h <= 0 || levels < 2) return -1;

    float scale = 255.0f / (levels - 1);

    for (int i = 0; i < w * h; i++) {
        uint32_t pix = pixels[i];
        uint8_t r = (pix >> 16) & 0xFF;
        uint8_t g = (pix >> 8) & 0xFF;
        uint8_t b = pix & 0xFF;

        r = (uint8_t)(roundf(r / scale) * scale);
        g = (uint8_t)(roundf(g / scale) * scale);
        b = (uint8_t)(roundf(b / scale) * scale);

        pixels[i] = (0xFF000000) | (r << 16) | (g << 8) | b;
    }
    return 0;
}

int tsfi_quantel_harry_solarize(uint32_t *pixels, int w, int h, float threshold) {
    if (!pixels || w <= 0 || h <= 0) return -1;

    for (int i = 0; i < w * h; i++) {
        uint32_t pix = pixels[i];
        uint8_t r = (pix >> 16) & 0xFF;
        uint8_t g = (pix >> 8) & 0xFF;
        uint8_t b = pix & 0xFF;

        float luma = 0.299f * r + 0.587f * g + 0.114f * b;
        if (luma > threshold) {
            r = 255 - r;
            g = 255 - g;
            b = 255 - b;
        }

        pixels[i] = (0xFF000000) | (r << 16) | (g << 8) | b;
    }
    return 0;
}

int tsfi_quantel_harry_spill_suppress(uint32_t *pixels, int w, int h, const char *suppress_type, float amount) {
    if (!pixels || w <= 0 || h <= 0) return -1;

    for (int i = 0; i < w * h; i++) {
        uint32_t pix = pixels[i];
        uint8_t r = (pix >> 16) & 0xFF;
        uint8_t g = (pix >> 8) & 0xFF;
        uint8_t b = pix & 0xFF;

        if (strcmp(suppress_type, "green") == 0) {
            uint8_t max_rb = r > b ? r : b;
            if (g > max_rb) {
                g = (uint8_t)(g - amount * (g - max_rb));
            }
        } else if (strcmp(suppress_type, "blue") == 0) {
            uint8_t max_rg = r > g ? r : g;
            if (b > max_rg) {
                b = (uint8_t)(b - amount * (b - max_rg));
            }
        }

        pixels[i] = (0xFF000000) | (r << 16) | (g << 8) | b;
    }
    return 0;
}

int tsfi_quantel_harry_temporal_blend(const uint32_t *frame_a, const uint32_t *frame_b, uint32_t *dst, int w, int h, float blend_factor) {
    if (!frame_a || !frame_b || !dst || w <= 0 || h <= 0) return -1;

    for (int i = 0; i < w * h; i++) {
        uint32_t pix_a = frame_a[i];
        uint32_t pix_b = frame_b[i];

        uint8_t ra = (pix_a >> 16) & 0xFF;
        uint8_t ga = (pix_a >> 8) & 0xFF;
        uint8_t ba = pix_a & 0xFF;

        uint8_t rb = (pix_b >> 16) & 0xFF;
        uint8_t gb = (pix_b >> 8) & 0xFF;
        uint8_t bb = pix_b & 0xFF;

        uint8_t r = (uint8_t)(ra * (1.0f - blend_factor) + rb * blend_factor);
        uint8_t g = (uint8_t)(ga * (1.0f - blend_factor) + gb * blend_factor);
        uint8_t b = (uint8_t)(ba * (1.0f - blend_factor) + bb * blend_factor);

        dst[i] = (0xFF000000) | (r << 16) | (g << 8) | b;
    }
    return 0;
}

int tsfi_quantel_harry_displacement_wipe(const uint32_t *src_a, const uint32_t *src_b, uint32_t *dst, int w, int h, float progress, float wave_amplitude, float wave_frequency) {
    if (!src_a || !src_b || !dst || w <= 0 || h <= 0) return -1;

    for (int y = 0; y < h; y++) {
        float displacement = wave_amplitude * sinf(2.0f * M_PI * wave_frequency * ((float)y / h));
        float limit_x = progress * w + displacement;

        for (int x = 0; x < w; x++) {
            int idx = y * w + x;
            if ((float)x < limit_x) {
                dst[idx] = src_b[idx];
            } else {
                dst[idx] = src_a[idx];
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

int tsfi_quantel_harry_luma_key(const uint32_t *src_pixels, int w, int h, uint8_t *out_mask, uint8_t low_threshold, uint8_t high_threshold) {
    if (!src_pixels || !out_mask || w <= 0 || h <= 0) return -1;

    for (int i = 0; i < w * h; i++) {
        uint32_t pix = src_pixels[i];
        uint8_t r = (pix >> 16) & 0xFF;
        uint8_t g = (pix >> 8) & 0xFF;
        uint8_t b = pix & 0xFF;
        uint8_t luma = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);

        if (luma >= low_threshold && luma <= high_threshold) {
            out_mask[i] = 255;
        } else {
            out_mask[i] = 0;
        }
    }
    return 0;
}

int tsfi_quantel_harry_filter(const uint32_t *src, uint32_t *dst, int w, int h, const float kernel[9]) {
    if (!src || !dst || w <= 0 || h <= 0) return -1;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float sum_r = 0.0f, sum_g = 0.0f, sum_b = 0.0f;
            for (int ky = -1; ky <= 1; ky++) {
                int py = y + ky;
                if (py < 0) py = 0;
                if (py >= h) py = h - 1;
                const uint32_t *row = src + py * w;

                for (int kx = -1; kx <= 1; kx++) {
                    int px = x + kx;
                    if (px < 0) px = 0;
                    if (px >= w) px = w - 1;

                    uint32_t pix = row[px];
                    float k_val = kernel[(ky + 1) * 3 + (kx + 1)];
                    sum_r += ((pix >> 16) & 0xFF) * k_val;
                    sum_g += ((pix >> 8) & 0xFF) * k_val;
                    sum_b += (pix & 0xFF) * k_val;
                }
            }

            int r = (int)sum_r;
            int g = (int)sum_g;
            int b = (int)sum_b;
            if (r < 0) { r = 0; }
            if (r > 255) { r = 255; }
            if (g < 0) { g = 0; }
            if (g > 255) { g = 255; }
            if (b < 0) { b = 0; }
            if (b > 255) { b = 255; }

            dst[y * w + x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    return 0;
}

void tsfi_rgb_to_hsl(uint8_t r, uint8_t g, uint8_t b, float *h, float *s, float *l) {
    float fr = r / 255.0f;
    float fg = g / 255.0f;
    float fb = b / 255.0f;
    float max_c = fr > fg ? (fr > fb ? fr : fb) : (fg > fb ? fg : fb);
    float min_c = fr < fg ? (fr < fb ? fr : fb) : (fg < fb ? fg : fb);
    *l = (max_c + min_c) / 2.0f;
    if (max_c == min_c) {
        *h = 0.0f;
        *s = 0.0f;
    } else {
        float d = max_c - min_c;
        *s = (*l > 0.5f) ? d / (2.0f - max_c - min_c) : d / (max_c + min_c);
        if (max_c == fr) {
            *h = (fg - fb) / d + (fg < fb ? 6.0f : 0.0f);
        } else if (max_c == fg) {
            *h = (fb - fr) / d + 2.0f;
        } else {
            *h = (fr - fg) / d + 4.0f;
        }
        *h /= 6.0f;
    }
}

float tsfi_hue_to_rgb(float p, float q, float t) {
    if (t < 0.0f) t += 1.0f;
    if (t > 1.0f) t -= 1.0f;
    if (t < 1.0f/6.0f) return p + (q - p) * 6.0f * t;
    if (t < 1.0f/2.0f) return q;
    if (t < 2.0f/3.0f) return p + (q - p) * (2.0f/3.0f - t) * 6.0f;
    return p;
}

void tsfi_hsl_to_rgb(float h, float s, float l, uint8_t *r, uint8_t *g, uint8_t *b) {
    if (s == 0.0f) {
        *r = *g = *b = (uint8_t)(l * 255.0f);
    } else {
        float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
        float p = 2.0f * l - q;
        *r = (uint8_t)(tsfi_hue_to_rgb(p, q, h + 1.0f/3.0f) * 255.0f);
        *g = (uint8_t)(tsfi_hue_to_rgb(p, q, h) * 255.0f);
        *b = (uint8_t)(tsfi_hue_to_rgb(p, q, h - 1.0f/3.0f) * 255.0f);
    }
}

int tsfi_quantel_harry_color_adjust(const uint32_t *src, uint32_t *dst, int w, int h, float hue_shift, float sat_scale, uint32_t tint_color, float tint_amount) {
    if (!src || !dst || w <= 0 || h <= 0) return -1;

    uint8_t tr = (tint_color >> 16) & 0xFF;
    uint8_t tg = (tint_color >> 8) & 0xFF;
    uint8_t tb = tint_color & 0xFF;

    for (int i = 0; i < w * h; i++) {
        uint32_t pix = src[i];
        uint8_t r = (pix >> 16) & 0xFF;
        uint8_t g = (pix >> 8) & 0xFF;
        uint8_t b = pix & 0xFF;

        float hue, sat, luma;
        tsfi_rgb_to_hsl(r, g, b, &hue, &sat, &luma);

        hue += hue_shift;
        if (hue > 1.0f) hue -= 1.0f;
        if (hue < 0.0f) hue += 1.0f;
        sat *= sat_scale;
        if (sat > 1.0f) sat = 1.0f;
        if (sat < 0.0f) sat = 0.0f;

        uint8_t r_adj, g_adj, b_adj;
        tsfi_hsl_to_rgb(hue, sat, luma, &r_adj, &g_adj, &b_adj);

        uint8_t r_res = (uint8_t)(r_adj * (1.0f - tint_amount) + tr * tint_amount);
        uint8_t g_res = (uint8_t)(g_adj * (1.0f - tint_amount) + tg * tint_amount);
        uint8_t b_res = (uint8_t)(b_adj * (1.0f - tint_amount) + tb * tint_amount);

        dst[i] = (0xFF000000) | (r_res << 16) | (g_res << 8) | b_res;
    }
    return 0;
}

int tsfi_quantel_harry_color_difference_key(const uint32_t *src, int w, int h, uint8_t *out_mask, float k_factor, float b_factor, float feather_radius) {
    if (!src || !out_mask || w <= 0 || h <= 0) return -1;

    for (int i = 0; i < w * h; i++) {
        uint32_t pix = src[i];
        float r = (pix >> 16) & 0xFF;
        float g = (pix >> 8) & 0xFF;
        float b = pix & 0xFF;

        float max_rb = r > b ? r : b;
        float val = g - b_factor * max_rb;
        float alpha = 1.0f - k_factor * val;

        if (alpha < 0.0f) alpha = 0.0f;
        if (alpha > 1.0f) alpha = 1.0f;

        out_mask[i] = (uint8_t)(alpha * 255.0f);
    }

    if (feather_radius > 0.5f) {
        uint8_t *temp = malloc(w * h);
        if (temp) {
            memcpy(temp, out_mask, w * h);
            int rad = (int)feather_radius;
            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    int sum = 0, count = 0;
                    for (int ky = -rad; ky <= rad; ky++) {
                        int py = y + ky;
                        if (py >= 0 && py < h) {
                            for (int kx = -rad; kx <= rad; kx++) {
                                int px = x + kx;
                                if (px >= 0 && px < w) {
                                    sum += temp[py * w + px];
                                    count++;
                                }
                            }
                        }
                    }
                    out_mask[y * w + x] = sum / count;
                }
            }
            free(temp);
        }
    }
    return 0;
}

int tsfi_quantel_harry_keyframe_transform(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float progress, float start_scale, float end_scale, float start_rot, float end_rot, float start_alpha, float end_alpha) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;

    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float scale = start_scale * (1.0f - progress) + end_scale * progress;
    float rot = start_rot * (1.0f - progress) + end_rot * progress;
    float alpha = start_alpha * (1.0f - progress) + end_alpha * progress;

    if (scale <= 0.001f) return 0;

    float cos_r = cosf(rot);
    float sin_r = sinf(rot);

    float cx_s = src_w / 2.0f;
    float cy_s = src_h / 2.0f;
    float cx_d = dst_w / 2.0f;
    float cy_d = dst_h / 2.0f;

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy_d;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx_d;

            float rx = (dx * cos_r + dy * sin_r) / scale;
            float ry = (-dx * sin_r + dy * cos_r) / scale;

            int sx = (int)(rx + cx_s);
            int sy = (int)(ry + cy_s);

            if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                uint32_t pixel = src[sy * src_w + sx];
                uint8_t r = (pixel >> 16) & 0xFF;
                uint8_t g = (pixel >> 8) & 0xFF;
                uint8_t b = pixel & 0xFF;

                uint8_t r_res = (uint8_t)(r * alpha);
                uint8_t g_res = (uint8_t)(g * alpha);
                uint8_t b_res = (uint8_t)(b * alpha);

                dst[y * dst_w + x] = (0xFF000000) | (r_res << 16) | (g_res << 8) | b_res;
            }
        }
    }
    return 0;
}

int tsfi_quantel_harry_matrix_wipe(const uint32_t *src_a, const uint32_t *src_b, uint32_t *dst, int w, int h, float progress, int grid_m, int grid_n) {
    if (!src_a || !src_b || !dst || w <= 0 || h <= 0 || grid_m <= 0 || grid_n <= 0) return -1;

    int cell_w = w / grid_m;
    int cell_h = h / grid_n;

    for (int y = 0; y < h; y++) {
        int cell_y = y / cell_h;
        if (cell_y >= grid_n) cell_y = grid_n - 1;

        for (int x = 0; x < w; x++) {
            int cell_x = x / cell_w;
            if (cell_x >= grid_m) cell_x = grid_m - 1;

            float threshold = (float)(cell_x + cell_y * grid_m) / (grid_m * grid_n);
            int idx = y * w + x;
            if (threshold < progress) {
                dst[idx] = src_b[idx];
            } else {
                dst[idx] = src_a[idx];
            }
        }
    }
    return 0;
}

extern int tsfi_quantel_paintbox_airbrush(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, uint32_t color);

int tsfi_quantel_harry_bezier_animate(const uint32_t *fg, int fg_w, int fg_h, uint32_t *bg, int w, int h, float t, float p0_x, float p0_y, float p1_x, float p1_y, float p2_x, float p2_y) {
    if (!fg || !bg || w <= 0 || h <= 0 || fg_w <= 0 || fg_h <= 0) return -1;

    float omt = 1.0f - t;
    float bx = omt * omt * p0_x + 2.0f * omt * t * p1_x + t * t * p2_x;
    float by = omt * omt * p0_y + 2.0f * omt * t * p1_y + t * t * p2_y;

    int ox = (int)(bx - fg_w / 2.0f);
    int oy = (int)(by - fg_h / 2.0f);

    for (int y = 0; y < fg_h; y++) {
        int cy = oy + y;
        if (cy < 0 || cy >= h) continue;
        uint32_t *bg_row = bg + cy * w;
        const uint32_t *fg_row = fg + y * fg_w;

        for (int x = 0; x < fg_w; x++) {
            int cx = ox + x;
            if (cx < 0 || cx >= w) continue;

            uint32_t fg_pixel = fg_row[x];
            uint8_t alpha = (fg_pixel >> 24) & 0xFF;
            if (alpha > 0) {
                float a = alpha / 255.0f;
                uint8_t rf = (fg_pixel >> 16) & 0xFF;
                uint8_t gf = (fg_pixel >> 8) & 0xFF;
                uint8_t bf = fg_pixel & 0xFF;

                uint32_t bg_pixel = bg_row[cx];
                uint8_t rb = (bg_pixel >> 16) & 0xFF;
                uint8_t gb = (bg_pixel >> 8) & 0xFF;
                uint8_t bb = bg_pixel & 0xFF;

                uint8_t r_res = (uint8_t)(rf * a + rb * (1.0f - a));
                uint8_t g_res = (uint8_t)(gf * a + gb * (1.0f - a));
                uint8_t b_res = (uint8_t)(bf * a + bb * (1.0f - a));

                bg_row[cx] = (0xFF000000) | (r_res << 16) | (g_res << 8) | b_res;
            }
        }
    }
    return 0;
}

int tsfi_quantel_harry_lift_gamma_gain(uint32_t *pixels, int w, int h, float lift[3], float gamma[3], float gain[3]) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    for (int i = 0; i < w * h; i++) {
        uint32_t pix = pixels[i];
        float r = ((pix >> 16) & 0xFF) / 255.0f;
        float g = ((pix >> 8) & 0xFF) / 255.0f;
        float b = (pix & 0xFF) / 255.0f;

        r = r * gain[0] + lift[0];
        g = g * gain[1] + lift[1];
        b = b * gain[2] + lift[2];

        if (r < 0.0f) { r = 0.0f; }
        if (r > 1.0f) { r = 1.0f; }
        if (g < 0.0f) { g = 0.0f; }
        if (g > 1.0f) { g = 1.0f; }
        if (b < 0.0f) { b = 0.0f; }
        if (b > 1.0f) { b = 1.0f; }

        if (gamma[0] > 0.001f) r = powf(r, 1.0f / gamma[0]);
        if (gamma[1] > 0.001f) g = powf(g, 1.0f / gamma[1]);
        if (gamma[2] > 0.001f) b = powf(b, 1.0f / gamma[2]);

        if (r < 0.0f) { r = 0.0f; }
        if (r > 1.0f) { r = 1.0f; }
        if (g < 0.0f) { g = 0.0f; }
        if (g > 1.0f) { g = 1.0f; }
        if (b < 0.0f) { b = 0.0f; }
        if (b > 1.0f) { b = 1.0f; }

        pixels[i] = (0xFF000000) | ((int)(r * 255.0f) << 16) | ((int)(g * 255.0f) << 8) | (int)(b * 255.0f);
    }
    return 0;
}

int tsfi_quantel_harry_rotoscope_blend(const uint32_t *canvas, const uint32_t *reference_frame, uint32_t *dst, int w, int h, float reference_opacity) {
    if (!canvas || !reference_frame || !dst || w <= 0 || h <= 0) return -1;
    for (int i = 0; i < w * h; i++) {
        uint32_t c_pix = canvas[i];
        uint32_t r_pix = reference_frame[i];

        uint8_t cr = (c_pix >> 16) & 0xFF;
        uint8_t cg = (c_pix >> 8) & 0xFF;
        uint8_t cb = c_pix & 0xFF;

        uint8_t rr = (r_pix >> 16) & 0xFF;
        uint8_t rg = (r_pix >> 8) & 0xFF;
        uint8_t rb = r_pix & 0xFF;

        uint8_t out_r = (uint8_t)(cr * (1.0f - reference_opacity) + rr * reference_opacity);
        uint8_t out_g = (uint8_t)(cg * (1.0f - reference_opacity) + rg * reference_opacity);
        uint8_t out_b = (uint8_t)(cb * (1.0f - reference_opacity) + rb * reference_opacity);

        dst[i] = (0xFF000000) | (out_r << 16) | (out_g << 8) | out_b;
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

int tsfi_quantel_harry_multitrack_dissolve(const uint32_t *src_a, const uint32_t *src_b, const uint32_t *src_c, uint32_t *dst, int w, int h, const float weights[3]) {
    if (!src_a || !src_b || !src_c || !dst || w <= 0 || h <= 0) return -1;
    for (int i = 0; i < w * h; i++) {
        uint32_t pa = src_a[i];
        uint32_t pb = src_b[i];
        uint32_t pc = src_c[i];

        float r = ((pa >> 16) & 0xFF) * weights[0] + ((pb >> 16) & 0xFF) * weights[1] + ((pc >> 16) & 0xFF) * weights[2];
        float g = ((pa >> 8) & 0xFF) * weights[0] + ((pb >> 8) & 0xFF) * weights[1] + ((pc >> 8) & 0xFF) * weights[2];
        float b = (pa & 0xFF) * weights[0] + (pb & 0xFF) * weights[1] + (pc & 0xFF) * weights[2];

        int ir = (int)r; if (ir < 0) ir = 0; if (ir > 255) ir = 255;
        int ig = (int)g; if (ig < 0) ig = 0; if (ig > 255) ig = 255;
        int ib = (int)b; if (ib < 0) ib = 0; if (ib > 255) ib = 255;

        dst[i] = (0xFF000000) | (ir << 16) | (ig << 8) | ib;
    }
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

int tsfi_quantel_harry_advanced_difference_key(const uint32_t *src, int w, int h, uint8_t *out_mask, uint32_t target_color, float range_min, float range_max) {
    if (!src || !out_mask || w <= 0 || h <= 0) return -1;

    uint8_t tr = (target_color >> 16) & 0xFF;
    uint8_t tg = (target_color >> 8) & 0xFF;
    uint8_t tb = target_color & 0xFF;

    for (int i = 0; i < w * h; i++) {
        uint32_t pix = src[i];
        uint8_t r = (pix >> 16) & 0xFF;
        uint8_t g = (pix >> 8) & 0xFF;
        uint8_t b = pix & 0xFF;

        float dist = sqrtf((r - tr) * (r - tr) + (g - tg) * (g - tg) + (b - tb) * (b - tb));
        if (dist >= range_min && dist <= range_max) {
            out_mask[i] = 0;
        } else {
            out_mask[i] = 255;
        }
    }
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

int tsfi_quantel_harry_erode_dilate(const uint8_t *src_mask, uint8_t *dst_mask, int w, int h, int radius, int erode) {
    if (!src_mask || !dst_mask || w <= 0 || h <= 0 || radius <= 0) return -1;
    memcpy(dst_mask, src_mask, w * h);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            uint8_t target = erode ? 255 : 0;
            bool trigger = false;
            for (int ky = -radius; ky <= radius; ky++) {
                int py = y + ky;
                if (py >= 0 && py < h) {
                    for (int kx = -radius; kx <= radius; kx++) {
                        int px = x + kx;
                        if (px >= 0 && px < w) {
                            if (src_mask[py * w + px] == target) {
                                trigger = true;
                                break;
                            }
                        }
                    }
                }
                if (trigger) break;
            }
            if (trigger) {
                dst_mask[y * w + x] = erode ? 0 : 255;
            }
        }
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

int tsfi_quantel_harry_hsl_despill(uint32_t *pixels, int w, int h, float threshold_hue, float suppression_amount) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    for (int i = 0; i < w * h; i++) {
        uint32_t pix = pixels[i];
        uint8_t r = (pix >> 16) & 0xFF;
        uint8_t g = (pix >> 8) & 0xFF;
        uint8_t b = pix & 0xFF;

        float h_val, s_val, l_val;
        tsfi_rgb_to_hsl(r, g, b, &h_val, &s_val, &l_val);

        float delta_h = fabsf(h_val - threshold_hue);
        if (delta_h > 0.5f) delta_h = 1.0f - delta_h;

        if (delta_h < 0.1f) {
            float damp = (0.1f - delta_h) / 0.1f * suppression_amount;
            s_val *= (1.0f - damp);
            uint8_t nr, ng, nb;
            tsfi_hsl_to_rgb(h_val, s_val, l_val, &nr, &ng, &nb);
            pixels[i] = (0xFF000000) | (nr << 16) | (ng << 8) | nb;
        }
    }
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

int tsfi_quantel_harry_time_slice_wipe(const uint32_t *src_a, const uint32_t *src_b, uint32_t *dst, int w, int h, float progress) {
    if (!src_a || !src_b || !dst || w <= 0 || h <= 0) return -1;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float slice_thresh = (float)x / w;
            int idx = y * w + x;
            if (slice_thresh < progress) {
                dst[idx] = src_b[idx];
            } else {
                dst[idx] = src_a[idx];
            }
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

int tsfi_quantel_harry_split_matte_preview(const uint32_t *composite, const uint8_t *matte, uint32_t *dst, int w, int h, float split_x) {
    if (!composite || !matte || !dst || w <= 0 || h <= 0) return -1;
    int border_x = (int)(split_x * w);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = y * w + x;
            if (x < border_x) {
                dst[idx] = composite[idx];
            } else {
                uint8_t m = matte[idx];
                dst[idx] = (0xFF000000) | (m << 16) | (m << 8) | m;
            }
        }
    }
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

int tsfi_quantel_harry_interpolate_keyframe(float t, float start_val, float end_val, float *out_val) {
    if (!out_val) return -1;
    *out_val = start_val * (1.0f - t) + end_val * t;
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

int tsfi_quantel_harry_smptec_to_rec709(uint32_t *pixels, int w, int h) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    for (int i = 0; i < w * h; i++) {
        uint32_t pix = pixels[i];
        float r = ((pix >> 16) & 0xFF) / 255.0f;
        float g = ((pix >> 8) & 0xFF) / 255.0f;
        float b = (pix & 0xFF) / 255.0f;

        float nr = 1.112f * r - 0.100f * g - 0.012f * b;
        float ng = -0.021f * r + 1.037f * g - 0.016f * b;
        float nb = -0.041f * r - 0.063f * g + 1.104f * b;

        if (nr < 0.0f) { nr = 0.0f; }
        if (nr > 1.0f) { nr = 1.0f; }
        if (ng < 0.0f) { ng = 0.0f; }
        if (ng > 1.0f) { ng = 1.0f; }
        if (nb < 0.0f) { nb = 0.0f; }
        if (nb > 1.0f) { nb = 1.0f; }

        pixels[i] = (0xFF000000) | ((int)(nr * 255.0f) << 16) | ((int)(ng * 255.0f) << 8) | (int)(nb * 255.0f);
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

int tsfi_quantel_harry_color_balance_sweep(uint32_t *pixels, int w, int h, float red_bal, float green_bal, float blue_bal) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    for (int i = 0; i < w * h; i++) {
        uint32_t pix = pixels[i];
        int r = (int)(((pix >> 16) & 0xFF) * red_bal);
        int g = (int)(((pix >> 8) & 0xFF) * green_bal);
        int b = (int)((pix & 0xFF) * blue_bal);

        if (r < 0) { r = 0; }
        if (r > 255) { r = 255; }
        if (g < 0) { g = 0; }
        if (g > 255) { g = 255; }
        if (b < 0) { b = 0; }
        if (b > 255) { b = 255; }

        pixels[i] = (0xFF000000) | (r << 16) | (g << 8) | b;
    }
    return 0;
}

int tsfi_quantel_storyboard_annotate_title(uint32_t *pixels, int w, int h, int x, int y, const char *title, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || !title) return -1;
    draw_text(pixels, w, h, x, y, title, color, 1);
    return 0;
}

int tsfi_quantel_harry_subpixel_shift(const uint32_t *src, uint32_t *dst, int w, int h, float dx, float dy) {
    if (!src || !dst || w <= 0 || h <= 0) return -1;
    memset(dst, 0, w * h * sizeof(uint32_t));

    int shift_x = (int)floorf(dx);
    int shift_y = (int)floorf(dy);
    float fx = dx - shift_x;
    float fy = dy - shift_y;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int x0 = x - shift_x;
            int y0 = y - shift_y;
            int x1 = x0 - 1;
            int y1 = y0 - 1;

            if (x0 >= 0 && x0 < w && y0 >= 0 && y0 < h &&
                x1 >= 0 && x1 < w && y1 >= 0 && y1 < h) {
                uint32_t c00 = src[y0 * w + x0];
                uint32_t c10 = src[y0 * w + x1];
                uint32_t c01 = src[y1 * w + x0];
                uint32_t c11 = src[y1 * w + x1];

                uint8_t r = (uint8_t)(
                    ((c00 >> 16) & 0xFF) * (1.0f - fx) * (1.0f - fy) +
                    ((c10 >> 16) & 0xFF) * fx * (1.0f - fy) +
                    ((c01 >> 16) & 0xFF) * (1.0f - fx) * fy +
                    ((c11 >> 16) & 0xFF) * fx * fy
                );
                uint8_t g = (uint8_t)(
                    ((c00 >> 8) & 0xFF) * (1.0f - fx) * (1.0f - fy) +
                    ((c10 >> 8) & 0xFF) * fx * (1.0f - fy) +
                    ((c01 >> 8) & 0xFF) * (1.0f - fx) * fy +
                    ((c11 >> 8) & 0xFF) * fx * fy
                );
                uint8_t b = (uint8_t)(
                    (c00 & 0xFF) * (1.0f - fx) * (1.0f - fy) +
                    (c10 & 0xFF) * fx * (1.0f - fy) +
                    (c01 & 0xFF) * (1.0f - fx) * fy +
                    (c11 & 0xFF) * fx * fy
                );

                dst[y * w + x] = (0xFF000000) | (r << 16) | (g << 8) | b;
            }
        }
    }
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

int tsfi_quantel_harry_chroma_gain(uint32_t *pixels, int w, int h, float gain) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    for (int i = 0; i < w * h; i++) {
        uint32_t pix = pixels[i];
        float r = ((pix >> 16) & 0xFF) / 255.0f;
        float g = ((pix >> 8) & 0xFF) / 255.0f;
        float b = (pix & 0xFF) / 255.0f;

        float y_val = 0.299f * r + 0.587f * g + 0.114f * b;
        float i_val = (0.596f * r - 0.274f * g - 0.322f * b) * gain;
        float q_val = (0.211f * r - 0.523f * g + 0.312f * b) * gain;

        float nr = y_val + 0.956f * i_val + 0.621f * q_val;
        float ng = y_val - 0.272f * i_val - 0.647f * q_val;
        float nb = y_val - 1.106f * i_val + 1.703f * q_val;

        if (nr < 0.0f) { nr = 0.0f; } if (nr > 1.0f) { nr = 1.0f; }
        if (ng < 0.0f) { ng = 0.0f; } if (ng > 1.0f) { ng = 1.0f; }
        if (nb < 0.0f) { nb = 0.0f; } if (nb > 1.0f) { nb = 1.0f; }

        pixels[i] = (0xFF000000) | ((int)(nr * 255.0f) << 16) | ((int)(ng * 255.0f) << 8) | (int)(nb * 255.0f);
    }
    return 0;
}

int tsfi_quantel_storyboard_burn_label(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, const char *label, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || !label) return -1;
    (void)cell_w;
    draw_text(pixels, w, h, cell_x + 10, cell_y + cell_h - 38, label, color, 1);
    return 0;
}

int tsfi_quantel_harry_interlace_fields(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h) {
    if (!field_even || !field_odd || !dst || w <= 0 || h <= 0) return -1;
    for (int y = 0; y < h; y++) {
        const uint32_t *src_row = (y % 2 == 0) ? field_even : field_odd;
        memcpy(dst + y * w, src_row + y * w, w * sizeof(uint32_t));
    }
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

int tsfi_quantel_harry_dissolve(const uint32_t *src_a, const uint32_t *src_b, uint32_t *dst, int w, int h, float progress) {
    if (!src_a || !src_b || !dst || w <= 0 || h <= 0) return -1;
    if (progress < 0.0f) { progress = 0.0f; }
    if (progress > 1.0f) { progress = 1.0f; }

    for (int i = 0; i < w * h; i++) {
        uint32_t ca = src_a[i];
        uint32_t cb = src_b[i];

        uint8_t ra = (ca >> 16) & 0xFF;
        uint8_t ga = (ca >> 8) & 0xFF;
        uint8_t ba = ca & 0xFF;

        uint8_t rb = (cb >> 16) & 0xFF;
        uint8_t gb = (cb >> 8) & 0xFF;
        uint8_t bb = cb & 0xFF;

        uint8_t r = (uint8_t)(ra * (1.0f - progress) + rb * progress);
        uint8_t g = (uint8_t)(ga * (1.0f - progress) + gb * progress);
        uint8_t b = (uint8_t)(ba * (1.0f - progress) + bb * progress);

        dst[i] = (0xFF000000) | (r << 16) | (g << 8) | b;
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

int tsfi_quantel_harry_film_grain(uint32_t *pixels, int w, int h, float intensity) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    for (int i = 0; i < w * h; i++) {
        uint32_t pix = pixels[i];
        float noise = ((float)rand() / RAND_MAX - 0.5f) * intensity * 255.0f;

        int r = (int)(((pix >> 16) & 0xFF) + noise);
        int g = (int)(((pix >> 8) & 0xFF) + noise);
        int b = (int)((pix & 0xFF) + noise);

        if (r < 0) { r = 0; }
        if (r > 255) { r = 255; }
        if (g < 0) { g = 0; }
        if (g > 255) { g = 255; }
        if (b < 0) { b = 0; }
        if (b > 255) { b = 255; }

        pixels[i] = (0xFF000000) | (r << 16) | (g << 8) | b;
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

int tsfi_quantel_harry_matte_choke(const uint8_t *src_mask, uint8_t *dst_mask, int w, int h, int choke_radius) {
    if (!src_mask || !dst_mask || w <= 0 || h <= 0) return -1;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            uint8_t min_val = 255;
            for (int cy = y - choke_radius; cy <= y + choke_radius; cy++) {
                if (cy < 0 || cy >= h) continue;
                for (int cx = x - choke_radius; cx <= x + choke_radius; cx++) {
                    if (cx < 0 || cx >= w) continue;
                    uint8_t val = src_mask[cy * w + cx];
                    if (val < min_val) { min_val = val; }
                }
            }
            dst_mask[y * w + x] = min_val;
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

int tsfi_quantel_harry_clock_wipe(const uint32_t *src_a, const uint32_t *src_b, uint32_t *dst, int w, int h, float progress) {
    if (!src_a || !src_b || !dst || w <= 0 || h <= 0) return -1;
    float cx = w / 2.0f;
    float cy = h / 2.0f;
    float target_angle = progress * 2.0f * (float)M_PI - (float)M_PI / 2.0f;

    for (int y = 0; y < h; y++) {
        float dy = y - cy;
        for (int x = 0; x < w; x++) {
            float dx = x - cx;
            float angle = atan2f(dy, dx);
            if (angle < -(float)M_PI / 2.0f) {
                angle += 2.0f * (float)M_PI;
            }

            int idx = y * w + x;
            if (angle <= target_angle + 0.001f) {
                dst[idx] = src_b[idx];
            } else {
                dst[idx] = src_a[idx];
            }
        }
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

int tsfi_quantel_harry_split_fields(const uint32_t *src, uint32_t *field_even, uint32_t *field_odd, int w, int h) {
    if (!src || !field_even || !field_odd || w <= 0 || h <= 0) return -1;
    for (int y = 0; y < h; y++) {
        uint32_t *dst_row = (y % 2 == 0) ? field_even : field_odd;
        memcpy(dst_row + y * w, src + y * w, w * sizeof(uint32_t));
        
        uint32_t *unused_row = (y % 2 == 0) ? field_odd : field_even;
        memset(unused_row + y * w, 0, w * sizeof(uint32_t));
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

int tsfi_quantel_harry_scanline_offset(uint32_t *pixels, int w, int h, int offset) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    uint32_t *temp = (uint32_t *)malloc(w * h * sizeof(uint32_t));
    if (!temp) { return -1; }
    memcpy(temp, pixels, w * h * sizeof(uint32_t));

    for (int y = 0; y < h; y++) {
        int shift = (y % 2 == 0) ? offset : -offset;
        uint32_t *dst_row = pixels + y * w;
        uint32_t *src_row = temp + y * w;
        for (int x = 0; x < w; x++) {
            int sx = (x - shift + w) % w;
            dst_row[x] = src_row[sx];
        }
    }
    free(temp);
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
