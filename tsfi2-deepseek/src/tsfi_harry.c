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
    if (progress <= 0.0f) {
        memcpy(dst, src_a, w * h * sizeof(uint32_t));
        return 0;
    }
    if (progress >= 1.0f) {
        memcpy(dst, src_b, w * h * sizeof(uint32_t));
        return 0;
    }

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
    if (progress <= 0.0f) {
        memcpy(dst, src_a, w * h * sizeof(uint32_t));
        return 0;
    }
    if (progress >= 1.0f) {
        memcpy(dst, src_b, w * h * sizeof(uint32_t));
        return 0;
    }

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
    if (progress <= 0.0f) {
        memcpy(dst, src_a, w * h * sizeof(uint32_t));
        return 0;
    }
    if (progress >= 1.0f) {
        memcpy(dst, src_b, w * h * sizeof(uint32_t));
        return 0;
    }

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


int tsfi_quantel_harry_time_slice_wipe(const uint32_t *src_a, const uint32_t *src_b, uint32_t *dst, int w, int h, float progress) {
    if (!src_a || !src_b || !dst || w <= 0 || h <= 0) return -1;
    if (progress <= 0.0f) {
        memcpy(dst, src_a, w * h * sizeof(uint32_t));
        return 0;
    }
    if (progress >= 1.0f) {
        memcpy(dst, src_b, w * h * sizeof(uint32_t));
        return 0;
    }

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


int tsfi_quantel_harry_interpolate_keyframe(float t, float start_val, float end_val, float *out_val) {
    if (!out_val) return -1;
    *out_val = start_val * (1.0f - t) + end_val * t;
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


int tsfi_quantel_harry_interlace_fields(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h) {
    if (!field_even || !field_odd || !dst || w <= 0 || h <= 0) return -1;
    for (int y = 0; y < h; y++) {
        const uint32_t *src_row = (y % 2 == 0) ? field_even : field_odd;
        memcpy(dst + y * w, src_row + y * w, w * sizeof(uint32_t));
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


int tsfi_quantel_harry_clock_wipe(const uint32_t *src_a, const uint32_t *src_b, uint32_t *dst, int w, int h, float progress) {
    if (!src_a || !src_b || !dst || w <= 0 || h <= 0) return -1;
    if (progress <= 0.0f) {
        memcpy(dst, src_a, w * h * sizeof(uint32_t));
        return 0;
    }
    if (progress >= 1.0f) {
        memcpy(dst, src_b, w * h * sizeof(uint32_t));
        return 0;
    }

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


int tsfi_quantel_harry_color_film_grain(uint32_t *pixels, int w, int h, float intensity) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    for (int i = 0; i < w * h; i++) {
        uint32_t pix = pixels[i];
        float noise_r = ((float)rand() / RAND_MAX - 0.5f) * intensity * 255.0f;
        float noise_g = ((float)rand() / RAND_MAX - 0.5f) * intensity * 255.0f;
        float noise_b = ((float)rand() / RAND_MAX - 0.5f) * intensity * 255.0f;

        int r = (int)(((pix >> 16) & 0xFF) + noise_r);
        int g = (int)(((pix >> 8) & 0xFF) + noise_g);
        int b = (int)((pix & 0xFF) + noise_b);

        if (r < 0) { r = 0; } if (r > 255) { r = 255; }
        if (g < 0) { g = 0; } if (g > 255) { g = 255; }
        if (b < 0) { b = 0; } if (b > 255) { b = 255; }

        pixels[i] = (0xFF000000) | (r << 16) | (g << 8) | b;
    }
    return 0;
}


int tsfi_quantel_harry_mono_film_grain(uint32_t *pixels, int w, int h, float intensity) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    for (int i = 0; i < w * h; i++) {
        uint32_t pix = pixels[i];
        float noise = ((float)rand() / RAND_MAX - 0.5f) * intensity * 255.0f;

        int r = (int)(((pix >> 16) & 0xFF) + noise);
        int g = (int)(((pix >> 8) & 0xFF) + noise);
        int b = (int)((pix & 0xFF) + noise);

        if (r < 0) { r = 0; } if (r > 255) { r = 255; }
        if (g < 0) { g = 0; } if (g > 255) { g = 255; }
        if (b < 0) { b = 0; } if (b > 255) { b = 255; }

        pixels[i] = (0xFF000000) | (r << 16) | (g << 8) | b;
    }
    return 0;
}


int tsfi_quantel_harry_interpolate_fields(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h) {
    if (!field_even || !field_odd || !dst || w <= 0 || h <= 0) return -1;
    for (int y = 0; y < h; y++) {
        if (y % 2 == 0) {
            memcpy(dst + y * w, field_even + y * w, w * sizeof(uint32_t));
        } else {
            memcpy(dst + y * w, field_odd + y * w, w * sizeof(uint32_t));
        }
    }
    return 0;
}


int tsfi_quantel_harry_field_shift(uint32_t *pixels, int w, int h, int shift_even, int shift_odd) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    uint32_t *temp = (uint32_t *)malloc(w * h * sizeof(uint32_t));
    if (!temp) { return -1; }
    memcpy(temp, pixels, w * h * sizeof(uint32_t));

    for (int y = 0; y < h; y++) {
        int shift = (y % 2 == 0) ? shift_even : shift_odd;
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


int tsfi_quantel_harry_blend_fields(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor) {
    if (!field_even || !field_odd || !dst || w <= 0 || h <= 0) return -1;
    if (blend_factor < 0.0f) { blend_factor = 0.0f; }
    if (blend_factor > 1.0f) { blend_factor = 1.0f; }

    for (int y = 0; y < h; y++) {
        const uint32_t *row_a = field_even + y * w;
        const uint32_t *row_b = field_odd + y * w;
        uint32_t *dst_row = dst + y * w;

        for (int x = 0; x < w; x++) {
            uint32_t ca = row_a[x];
            uint32_t cb = row_b[x];

            uint8_t ra = (ca >> 16) & 0xFF;
            uint8_t ga = (ca >> 8) & 0xFF;
            uint8_t ba = ca & 0xFF;

            uint8_t rb = (cb >> 16) & 0xFF;
            uint8_t gb = (cb >> 8) & 0xFF;
            uint8_t bb = cb & 0xFF;

            uint8_t r = (uint8_t)(ra * (1.0f - blend_factor) + rb * blend_factor);
            uint8_t g = (uint8_t)(ga * (1.0f - blend_factor) + gb * blend_factor);
            uint8_t b = (uint8_t)(ba * (1.0f - blend_factor) + bb * blend_factor);

            dst_row[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    return 0;
}


int tsfi_quantel_harry_scanline_blend(uint32_t *pixels, int w, int h, float blend_factor) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    if (blend_factor < 0.0f) { blend_factor = 0.0f; }
    if (blend_factor > 1.0f) { blend_factor = 1.0f; }

    for (int y = 1; y < h - 1; y += 2) {
        uint32_t *prev = pixels + (y - 1) * w;
        uint32_t *curr = pixels + y * w;
        uint32_t *next = pixels + (y + 1) * w;

        for (int x = 0; x < w; x++) {
            uint32_t c_prev = prev[x];
            uint32_t c_next = next[x];

            uint8_t r_prev = (c_prev >> 16) & 0xFF;
            uint8_t g_prev = (c_prev >> 8) & 0xFF;
            uint8_t b_prev = c_prev & 0xFF;

            uint8_t r_next = (c_next >> 16) & 0xFF;
            uint8_t g_next = (c_next >> 8) & 0xFF;
            uint8_t b_next = c_next & 0xFF;

            uint8_t r_avg = (uint8_t)((r_prev + r_next) / 2);
            uint8_t g_avg = (uint8_t)((g_prev + g_next) / 2);
            uint8_t b_avg = (uint8_t)((b_prev + b_next) / 2);

            uint32_t c_curr = curr[x];
            uint8_t r_curr = (c_curr >> 16) & 0xFF;
            uint8_t g_curr = (c_curr >> 8) & 0xFF;
            uint8_t b_curr = c_curr & 0xFF;

            uint8_t r = (uint8_t)(r_curr * (1.0f - blend_factor) + r_avg * blend_factor);
            uint8_t g = (uint8_t)(g_curr * (1.0f - blend_factor) + g_avg * blend_factor);
            uint8_t b = (uint8_t)(b_curr * (1.0f - blend_factor) + b_avg * blend_factor);

            curr[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    return 0;
}


int tsfi_quantel_harry_blend_fields_offset(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int offset) {
    if (!field_even || !field_odd || !dst || w <= 0 || h <= 0) return -1;
    if (blend_factor < 0.0f) { blend_factor = 0.0f; }
    if (blend_factor > 1.0f) { blend_factor = 1.0f; }

    for (int y = 0; y < h; y++) {
        const uint32_t *row_a = field_even + y * w;
        const uint32_t *row_b = field_odd + y * w;
        uint32_t *dst_row = dst + y * w;

        int shift = (y % 2 == 0) ? offset : -offset;

        for (int x = 0; x < w; x++) {
            int sx_a = (x - shift + w) % w;
            int sx_b = (x + shift + w) % w;

            uint32_t ca = row_a[sx_a];
            uint32_t cb = row_b[sx_b];

            uint8_t ra = (ca >> 16) & 0xFF;
            uint8_t ga = (ca >> 8) & 0xFF;
            uint8_t ba = ca & 0xFF;

            uint8_t rb = (cb >> 16) & 0xFF;
            uint8_t gb = (cb >> 8) & 0xFF;
            uint8_t bb = cb & 0xFF;

            uint8_t r = (uint8_t)(ra * (1.0f - blend_factor) + rb * blend_factor);
            uint8_t g = (uint8_t)(ga * (1.0f - blend_factor) + gb * blend_factor);
            uint8_t b = (uint8_t)(ba * (1.0f - blend_factor) + bb * blend_factor);

            dst_row[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    return 0;
}

int tsfi_quantel_harry_blend_fields_shift(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int shift_even, int shift_odd) {
    if (!field_even || !field_odd || !dst || w <= 0 || h <= 0) return -1;
    if (blend_factor < 0.0f) { blend_factor = 0.0f; }
    if (blend_factor > 1.0f) { blend_factor = 1.0f; }

    for (int y = 0; y < h; y++) {
        const uint32_t *row_a = field_even + y * w;
        const uint32_t *row_b = field_odd + y * w;
        uint32_t *dst_row = dst + y * w;

        int shift = (y % 2 == 0) ? shift_even : shift_odd;

        for (int x = 0; x < w; x++) {
            int sx_a = (x - shift + w) % w;
            int sx_b = (x + shift + w) % w;

            uint32_t ca = row_a[sx_a];
            uint32_t cb = row_b[sx_b];

            uint8_t ra = (ca >> 16) & 0xFF;
            uint8_t ga = (ca >> 8) & 0xFF;
            uint8_t ba = ca & 0xFF;

            uint8_t rb = (cb >> 16) & 0xFF;
            uint8_t gb = (cb >> 8) & 0xFF;
            uint8_t bb = cb & 0xFF;

            uint8_t r = (uint8_t)(ra * (1.0f - blend_factor) + rb * blend_factor);
            uint8_t g = (uint8_t)(ga * (1.0f - blend_factor) + gb * blend_factor);
            uint8_t b = (uint8_t)(ba * (1.0f - blend_factor) + bb * blend_factor);

            dst_row[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    return 0;
}

int tsfi_quantel_harry_split_interpolate(const uint32_t *src, uint32_t *dst_even, uint32_t *dst_odd, int w, int h) {
    if (!src || !dst_even || !dst_odd || w <= 0 || h <= 0) return -1;
    for (int y = 0; y < h; y++) {
        if (y % 2 == 0) {
            memcpy(dst_even + y * w, src + y * w, w * sizeof(uint32_t));
            if (y > 0) {
                for (int x = 0; x < w; x++) {
                    uint32_t c1 = src[(y - 2) * w + x];
                    uint32_t c2 = src[y * w + x];
                    uint8_t r = (((c1 >> 16) & 0xFF) + ((c2 >> 16) & 0xFF)) / 2;
                    uint8_t g = (((c1 >> 8) & 0xFF) + ((c2 >> 8) & 0xFF)) / 2;
                    uint8_t b = ((c1 & 0xFF) + (c2 & 0xFF)) / 2;
                    dst_even[(y - 1) * w + x] = (0xFF000000) | (r << 16) | (g << 8) | b;
                }
            }
        } else {
            memcpy(dst_odd + y * w, src + y * w, w * sizeof(uint32_t));
            if (y > 1) {
                for (int x = 0; x < w; x++) {
                    uint32_t c1 = src[(y - 2) * w + x];
                    uint32_t c2 = src[y * w + x];
                    uint8_t r = (((c1 >> 16) & 0xFF) + ((c2 >> 16) & 0xFF)) / 2;
                    uint8_t g = (((c1 >> 8) & 0xFF) + ((c2 >> 8) & 0xFF)) / 2;
                    uint8_t b = ((c1 & 0xFF) + (c2 & 0xFF)) / 2;
                    dst_odd[(y - 1) * w + x] = (0xFF000000) | (r << 16) | (g << 8) | b;
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_harry_blend_fields_jitter(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, float jitter_amp) {
    if (!field_even || !field_odd || !dst || w <= 0 || h <= 0) return -1;
    if (blend_factor < 0.0f) { blend_factor = 0.0f; }
    if (blend_factor > 1.0f) { blend_factor = 1.0f; }

    for (int y = 0; y < h; y++) {
        const uint32_t *row_a = field_even + y * w;
        const uint32_t *row_b = field_odd + y * w;
        uint32_t *dst_row = dst + y * w;

        float jitter_val = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp;
        int shift = (int)jitter_val;

        for (int x = 0; x < w; x++) {
            int sx_a = (x - shift + w) % w;
            int sx_b = (x + shift + w) % w;

            uint32_t ca = row_a[sx_a];
            uint32_t cb = row_b[sx_b];

            uint8_t ra = (ca >> 16) & 0xFF;
            uint8_t ga = (ca >> 8) & 0xFF;
            uint8_t ba = ca & 0xFF;

            uint8_t rb = (cb >> 16) & 0xFF;
            uint8_t gb = (cb >> 8) & 0xFF;
            uint8_t bb = cb & 0xFF;

            uint8_t r = (uint8_t)(ra * (1.0f - blend_factor) + rb * blend_factor);
            uint8_t g = (uint8_t)(ga * (1.0f - blend_factor) + gb * blend_factor);
            uint8_t b = (uint8_t)(ba * (1.0f - blend_factor) + bb * blend_factor);

            dst_row[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    return 0;
}

int tsfi_quantel_harry_blend_fields_shift_jitter(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int shift_even, int shift_odd, float jitter_amp) {
    if (!field_even || !field_odd || !dst || w <= 0 || h <= 0) return -1;
    if (blend_factor < 0.0f) { blend_factor = 0.0f; }
    if (blend_factor > 1.0f) { blend_factor = 1.0f; }

    for (int y = 0; y < h; y++) {
        const uint32_t *row_a = field_even + y * w;
        const uint32_t *row_b = field_odd + y * w;
        uint32_t *dst_row = dst + y * w;

        float jitter_val = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp;
        int shift = ((y % 2 == 0) ? shift_even : shift_odd) + (int)jitter_val;

        for (int x = 0; x < w; x++) {
            int sx_a = (x - shift + w) % w;
            int sx_b = (x + shift + w) % w;

            uint32_t ca = row_a[sx_a];
            uint32_t cb = row_b[sx_b];

            uint8_t ra = (ca >> 16) & 0xFF;
            uint8_t ga = (ca >> 8) & 0xFF;
            uint8_t ba = ca & 0xFF;

            uint8_t rb = (cb >> 16) & 0xFF;
            uint8_t gb = (cb >> 8) & 0xFF;
            uint8_t bb = cb & 0xFF;

            uint8_t r = (uint8_t)(ra * (1.0f - blend_factor) + rb * blend_factor);
            uint8_t g = (uint8_t)(ga * (1.0f - blend_factor) + gb * blend_factor);
            uint8_t b = (uint8_t)(ba * (1.0f - blend_factor) + bb * blend_factor);

            dst_row[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    return 0;
}

int tsfi_quantel_harry_scanline_offset_jitter(uint32_t *pixels, int w, int h, int offset, float jitter_amp) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    uint32_t *temp = (uint32_t *)malloc(w * h * sizeof(uint32_t));
    if (!temp) { return -1; }
    memcpy(temp, pixels, w * h * sizeof(uint32_t));

    for (int y = 0; y < h; y++) {
        float jitter_val = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp;
        int shift = ((y % 2 == 0) ? offset : -offset) + (int)jitter_val;
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

int tsfi_quantel_harry_blend_fields_offset_jitter(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int offset, float jitter_amp) {
    if (!field_even || !field_odd || !dst || w <= 0 || h <= 0) return -1;
    if (blend_factor < 0.0f) { blend_factor = 0.0f; }
    if (blend_factor > 1.0f) { blend_factor = 1.0f; }

    for (int y = 0; y < h; y++) {
        const uint32_t *row_a = field_even + y * w;
        const uint32_t *row_b = field_odd + y * w;
        uint32_t *dst_row = dst + y * w;

        float jitter_val = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp;
        int shift = ((y % 2 == 0) ? offset : -offset) + (int)jitter_val;

        for (int x = 0; x < w; x++) {
            int sx_a = (x - shift + w) % w;
            int sx_b = (x + shift + w) % w;

            uint32_t ca = row_a[sx_a];
            uint32_t cb = row_b[sx_b];

            uint8_t ra = (ca >> 16) & 0xFF;
            uint8_t ga = (ca >> 8) & 0xFF;
            uint8_t ba = ca & 0xFF;

            uint8_t rb = (cb >> 16) & 0xFF;
            uint8_t gb = (cb >> 8) & 0xFF;
            uint8_t bb = cb & 0xFF;

            uint8_t r = (uint8_t)(ra * (1.0f - blend_factor) + rb * blend_factor);
            uint8_t g = (uint8_t)(ga * (1.0f - blend_factor) + gb * blend_factor);
            uint8_t b = (uint8_t)(ba * (1.0f - blend_factor) + bb * blend_factor);

            dst_row[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    return 0;
}

int tsfi_quantel_harry_blend_fields_jitter_shift(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int shift, float jitter_amp) {
    if (!field_even || !field_odd || !dst || w <= 0 || h <= 0) return -1;
    if (blend_factor < 0.0f) { blend_factor = 0.0f; }
    if (blend_factor > 1.0f) { blend_factor = 1.0f; }

    for (int y = 0; y < h; y++) {
        const uint32_t *row_a = field_even + y * w;
        const uint32_t *row_b = field_odd + y * w;
        uint32_t *dst_row = dst + y * w;

        float jitter_val = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp;
        int active_shift = shift + (int)jitter_val;
        int shift_x = (y % 2 == 0) ? active_shift : -active_shift;

        for (int x = 0; x < w; x++) {
            int sx_a = (x - shift_x + w) % w;
            int sx_b = (x + shift_x + w) % w;

            uint32_t ca = row_a[sx_a];
            uint32_t cb = row_b[sx_b];

            uint8_t ra = (ca >> 16) & 0xFF;
            uint8_t ga = (ca >> 8) & 0xFF;
            uint8_t ba = ca & 0xFF;

            uint8_t rb = (cb >> 16) & 0xFF;
            uint8_t gb = (cb >> 8) & 0xFF;
            uint8_t bb = cb & 0xFF;

            uint8_t r = (uint8_t)(ra * (1.0f - blend_factor) + rb * blend_factor);
            uint8_t g = (uint8_t)(ga * (1.0f - blend_factor) + gb * blend_factor);
            uint8_t b = (uint8_t)(ba * (1.0f - blend_factor) + bb * blend_factor);

            dst_row[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    return 0;
}

int tsfi_quantel_harry_blend_fields_directional_jitter(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, float jitter_x, float jitter_y) {
    if (!field_even || !field_odd || !dst || w <= 0 || h <= 0) return -1;
    if (blend_factor < 0.0f) { blend_factor = 0.0f; }
    if (blend_factor > 1.0f) { blend_factor = 1.0f; }

    for (int y = 0; y < h; y++) {
        float noise_y = ((float)rand() / RAND_MAX - 0.5f) * jitter_y;
        int sy = y + (int)noise_y;
        if (sy < 0) { sy = 0; }
        if (sy >= h) { sy = h - 1; }

        const uint32_t *row_a = field_even + sy * w;
        const uint32_t *row_b = field_odd + sy * w;
        uint32_t *dst_row = dst + y * w;

        float noise_x = ((float)rand() / RAND_MAX - 0.5f) * jitter_x;
        int shift_x = (int)noise_x;

        for (int x = 0; x < w; x++) {
            int sx_a = (x - shift_x + w) % w;
            int sx_b = (x + shift_x + w) % w;

            uint32_t ca = row_a[sx_a];
            uint32_t cb = row_b[sx_b];

            uint8_t ra = (ca >> 16) & 0xFF;
            uint8_t ga = (ca >> 8) & 0xFF;
            uint8_t ba = ca & 0xFF;

            uint8_t rb = (cb >> 16) & 0xFF;
            uint8_t gb = (cb >> 8) & 0xFF;
            uint8_t bb = cb & 0xFF;

            uint8_t r = (uint8_t)(ra * (1.0f - blend_factor) + rb * blend_factor);
            uint8_t g = (uint8_t)(ga * (1.0f - blend_factor) + gb * blend_factor);
            uint8_t b = (uint8_t)(ba * (1.0f - blend_factor) + bb * blend_factor);

            dst_row[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    return 0;
}

int tsfi_quantel_harry_scanline_offset_jitter_width(uint32_t *pixels, int w, int h, int offset, int line_width, float jitter_amp) {
    if (!pixels || w <= 0 || h <= 0 || line_width <= 0) return -1;
    uint32_t *temp = (uint32_t *)malloc(w * h * sizeof(uint32_t));
    if (!temp) { return -1; }
    memcpy(temp, pixels, w * h * sizeof(uint32_t));

    for (int y = 0; y < h; y++) {
        int line_group = y / line_width;
        float jitter_val = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp;
        int shift = ((line_group % 2 == 0) ? offset : -offset) + (int)jitter_val;

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

int tsfi_quantel_harry_blend_fields_directional_jitter_offset(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, float jitter_x, float jitter_y, int offset) {
    if (!field_even || !field_odd || !dst || w <= 0 || h <= 0) return -1;
    if (blend_factor < 0.0f) { blend_factor = 0.0f; }
    if (blend_factor > 1.0f) { blend_factor = 1.0f; }

    for (int y = 0; y < h; y++) {
        float noise_y = ((float)rand() / RAND_MAX - 0.5f) * jitter_y;
        int sy = y + (int)noise_y;
        if (sy < 0) { sy = 0; }
        if (sy >= h) { sy = h - 1; }

        const uint32_t *row_a = field_even + sy * w;
        const uint32_t *row_b = field_odd + sy * w;
        uint32_t *dst_row = dst + y * w;

        float noise_x = ((float)rand() / RAND_MAX - 0.5f) * jitter_x;
        int active_offset = ((y % 2 == 0) ? offset : -offset) + (int)noise_x;

        for (int x = 0; x < w; x++) {
            int sx_a = (x - active_offset + w) % w;
            int sx_b = (x + active_offset + w) % w;

            uint32_t ca = row_a[sx_a];
            uint32_t cb = row_b[sx_b];

            uint8_t ra = (ca >> 16) & 0xFF;
            uint8_t ga = (ca >> 8) & 0xFF;
            uint8_t ba = ca & 0xFF;

            uint8_t rb = (cb >> 16) & 0xFF;
            uint8_t gb = (cb >> 8) & 0xFF;
            uint8_t bb = cb & 0xFF;

            uint8_t r = (uint8_t)(ra * (1.0f - blend_factor) + rb * blend_factor);
            uint8_t g = (uint8_t)(ga * (1.0f - blend_factor) + gb * blend_factor);
            uint8_t b = (uint8_t)(ba * (1.0f - blend_factor) + bb * blend_factor);

            dst_row[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    return 0;
}
