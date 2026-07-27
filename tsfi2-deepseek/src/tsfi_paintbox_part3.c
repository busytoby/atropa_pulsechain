#define _GNU_SOURCE
#include "tsfi_paint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int tsfi_quantel_paintbox_multistop_gradient(uint32_t *pixels, int w, int h, int x0, int y0, int x1, int y1, const uint32_t *colors, const float *stops, int stop_count) {
    if (!pixels || w <= 0 || h <= 0 || stop_count < 2 || !colors || !stops) return -1;
    float dx = (float)(x1 - x0);
    float dy = (float)(y1 - y0);
    float len_sq = dx*dx + dy*dy;
    if (len_sq < 0.001f) len_sq = 0.001f;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float px_x = (float)(x - x0);
            float px_y = (float)(y - y0);
            float t = (px_x * dx + px_y * dy) / len_sq;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;

            int lower_idx = 0;
            int upper_idx = 1;
            for (int i = 0; i < stop_count - 1; i++) {
                if (t >= stops[i] && t <= stops[i+1]) {
                    lower_idx = i;
                    upper_idx = i + 1;
                    break;
                }
            }

            float range = stops[upper_idx] - stops[lower_idx];
            float factor = range > 0.0001f ? (t - stops[lower_idx]) / range : 0.0f;

            uint32_t c0 = colors[lower_idx];
            uint32_t c1 = colors[upper_idx];

            uint8_t r0 = (c0 >> 16) & 0xFF;
            uint8_t g0 = (c0 >> 8) & 0xFF;
            uint8_t b0 = c0 & 0xFF;

            uint8_t r1 = (c1 >> 16) & 0xFF;
            uint8_t g1 = (c1 >> 8) & 0xFF;
            uint8_t b1 = c1 & 0xFF;

            uint8_t r = (uint8_t)(r0 * (1.0f - factor) + r1 * factor);
            uint8_t g = (uint8_t)(g0 * (1.0f - factor) + g1 * factor);
            uint8_t b = (uint8_t)(b0 * (1.0f - factor) + b1 * factor);

            pixels[y * w + x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    return 0;
}

int tsfi_quantel_paintbox_bristle_brush(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, int bristle_count, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0 || bristle_count <= 0) return -1;
    for (int i = 0; i < bristle_count; i++) {
        float angle = ((float)i / bristle_count) * 2.0f * (float)M_PI;
        float r_displacement = ((float)rand() / RAND_MAX) * radius * 0.7f;
        int bx = cx + (int)(r_displacement * cosf(angle));
        int by = cy + (int)(r_displacement * sinf(angle));
        int bristle_r = radius / 5;
        if (bristle_r < 1) bristle_r = 1;

        tsfi_quantel_paintbox_airbrush(pixels, w, h, bx, by, bristle_r, pressure * 0.4f, color);
    }
    return 0;
}

int tsfi_quantel_paintbox_chalk_brush(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    
    uint8_t r_src = (color >> 16) & 0xFF;
    uint8_t g_src = (color >> 8) & 0xFF;
    uint8_t b_src = color & 0xFF;

    for (int y = cy - radius; y <= cy + radius; y++) {
        if (y < 0 || y >= h) continue;
        uint32_t *row = pixels + y * w;
        int dy = y - cy;
        for (int x = cx - radius; x <= cx + radius; x++) {
            if (x < 0 || x >= w) continue;
            int dx = x - cx;
            if (dx * dx + dy * dy <= radius * radius) {
                float noise = (float)rand() / RAND_MAX;
                if (noise > 0.4f) {
                    float intensity = pressure * noise;
                    uint32_t dest = row[x];
                    uint8_t r_dst = (dest >> 16) & 0xFF;
                    uint8_t g_dst = (dest >> 8) & 0xFF;
                    uint8_t b_dst = dest & 0xFF;

                    uint8_t r_res = (uint8_t)(r_src * intensity + r_dst * (1.0f - intensity));
                    uint8_t g_res = (uint8_t)(g_src * intensity + g_dst * (1.0f - intensity));
                    uint8_t b_res = (uint8_t)(b_src * intensity + b_dst * (1.0f - intensity));

                    row[x] = (0xFF000000) | (r_res << 16) | (g_res << 8) | b_res;
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_paintbox_dynamic_smudge(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float smudge_dryness, uint32_t *brush_color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0 || !brush_color) return -1;
    float effective_smudge = pressure * (1.0f - smudge_dryness);
    return tsfi_quantel_paintbox_wet_paint(pixels, w, h, cx, cy, radius, effective_smudge, brush_color);
}

int tsfi_quantel_paintbox_velocity_spray(uint32_t *pixels, int w, int h, int prev_x, int prev_y, int curr_x, int curr_y, int radius, float pressure, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    float dx = (float)(curr_x - prev_x);
    float dy = (float)(curr_y - prev_y);
    float speed = sqrtf(dx*dx + dy*dy);
    
    float scatter_radius = radius * (1.0f + 0.1f * speed);
    float density = pressure / (1.0f + 0.2f * speed);
    if (density < 0.05f) density = 0.05f;

    return tsfi_quantel_paintbox_spray_can(pixels, w, h, curr_x, curr_y, (int)scatter_radius, density, color);
}

int tsfi_quantel_paintbox_sort_palette(uint32_t *palette, int count, int sort_by_luma) {
    if (!palette || count <= 1) return -1;
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            uint32_t c0 = palette[i];
            uint32_t c1 = palette[j];

            float v0, v1;
            if (sort_by_luma) {
                v0 = 0.299f * ((c0 >> 16) & 0xFF) + 0.587f * ((c0 >> 8) & 0xFF) + 0.114f * (c0 & 0xFF);
                v1 = 0.299f * ((c1 >> 16) & 0xFF) + 0.587f * ((c1 >> 8) & 0xFF) + 0.114f * (c1 & 0xFF);
            } else {
                v0 = (float)((c0 >> 16) & 0xFF);
                v1 = (float)((c1 >> 16) & 0xFF);
            }

            if (v0 > v1) {
                palette[i] = c1;
                palette[j] = c0;
            }
        }
    }
    return 0;
}

int tsfi_quantel_paintbox_blend_mixer(uint32_t *pixels, int w, int h, const uint32_t *src_a, const uint32_t *src_b, float mix_factor) {
    if (!pixels || !src_a || !src_b || w <= 0 || h <= 0) return -1;
    if (mix_factor < 0.0f) { mix_factor = 0.0f; }
    if (mix_factor > 1.0f) { mix_factor = 1.0f; }

    for (int i = 0; i < w * h; i++) {
        uint32_t ca = src_a[i];
        uint32_t cb = src_b[i];

        uint8_t ra = (ca >> 16) & 0xFF;
        uint8_t ga = (ca >> 8) & 0xFF;
        uint8_t ba = ca & 0xFF;

        uint8_t rb = (cb >> 16) & 0xFF;
        uint8_t gb = (cb >> 8) & 0xFF;
        uint8_t bb = cb & 0xFF;

        uint8_t r = (uint8_t)(ra * (1.0f - mix_factor) + rb * mix_factor);
        uint8_t g = (uint8_t)(ga * (1.0f - mix_factor) + gb * mix_factor);
        uint8_t b = (uint8_t)(ba * (1.0f - mix_factor) + bb * mix_factor);

        pixels[i] = (0xFF000000) | (r << 16) | (g << 8) | b;
    }
    return 0;
}

int tsfi_quantel_paintbox_interpolate_palette(const uint32_t *palette_a, const uint32_t *palette_b, uint32_t *dst_palette, int count, float t) {
    if (!palette_a || !palette_b || !dst_palette || count <= 0) return -1;
    if (t < 0.0f) { t = 0.0f; }
    if (t > 1.0f) { t = 1.0f; }

    for (int i = 0; i < count; i++) {
        uint32_t ca = palette_a[i];
        uint32_t cb = palette_b[i];

        uint8_t ra = (ca >> 16) & 0xFF;
        uint8_t ga = (ca >> 8) & 0xFF;
        uint8_t ba = ca & 0xFF;

        uint8_t rb = (cb >> 16) & 0xFF;
        uint8_t gb = (cb >> 8) & 0xFF;
        uint8_t bb = cb & 0xFF;

        uint8_t r = (uint8_t)(ra * (1.0f - t) + rb * t);
        uint8_t g = (uint8_t)(ga * (1.0f - t) + gb * t);
        uint8_t b = (uint8_t)(ba * (1.0f - t) + bb * t);

        dst_palette[i] = (0xFF000000) | (r << 16) | (g << 8) | b;
    }
    return 0;
}

int tsfi_quantel_paintbox_dynamic_flow(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float speed, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    float flow = pressure / (1.0f + 0.1f * speed);
    if (flow < 0.01f) { flow = 0.01f; }
    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, radius, flow, color);
}

int tsfi_quantel_paintbox_angle_lock(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float angle_lock, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    return tsfi_quantel_paintbox_tablet_brush(pixels, w, h, cx, cy, radius, 1.0f, 0.3f, angle_lock, color);
}

int tsfi_quantel_paintbox_complementary_color(uint32_t color, uint32_t *out_color) {
    if (!out_color) return -1;
    uint8_t r = 255 - ((color >> 16) & 0xFF);
    uint8_t g = 255 - ((color >> 8) & 0xFF);
    uint8_t b = 255 - (color & 0xFF);
    *out_color = (0xFF000000) | (r << 16) | (g << 8) | b;
    return 0;
}

int tsfi_quantel_paintbox_warm_cool_filter(uint32_t *pixels, int w, int h, float temp_adjust) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    for (int i = 0; i < w * h; i++) {
        uint32_t pix = pixels[i];
        int r = (pix >> 16) & 0xFF;
        int g = (pix >> 8) & 0xFF;
        int b = pix & 0xFF;

        r = (int)(r + temp_adjust * 20.0f);
        b = (int)(b - temp_adjust * 20.0f);

        if (r < 0) { r = 0; }
        if (r > 255) { r = 255; }
        if (b < 0) { b = 0; }
        if (b > 255) { b = 255; }

        pixels[i] = (0xFF000000) | (r << 16) | (g << 8) | b;
    }
    return 0;
}

int tsfi_quantel_paintbox_saturation_sweep(uint32_t *pixels, int w, int h, float saturation_adjust) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    for (int i = 0; i < w * h; i++) {
        uint32_t pix = pixels[i];
        float r = ((pix >> 16) & 0xFF) / 255.0f;
        float g = ((pix >> 8) & 0xFF) / 255.0f;
        float b = (pix & 0xFF) / 255.0f;

        float luma = 0.299f * r + 0.587f * g + 0.114f * b;
        float nr = luma + (r - luma) * saturation_adjust;
        float ng = luma + (g - luma) * saturation_adjust;
        float nb = luma + (b - luma) * saturation_adjust;

        if (nr < 0.0f) { nr = 0.0f; } if (nr > 1.0f) { nr = 1.0f; }
        if (ng < 0.0f) { ng = 0.0f; } if (ng > 1.0f) { ng = 1.0f; }
        if (nb < 0.0f) { nb = 0.0f; } if (nb > 1.0f) { nb = 1.0f; }

        pixels[i] = (0xFF000000) | ((int)(nr * 255.0f) << 16) | ((int)(ng * 255.0f) << 8) | (int)(nb * 255.0f);
    }
    return 0;
}

int tsfi_quantel_paintbox_contrast_adjust(uint32_t *pixels, int w, int h, float contrast) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    for (int i = 0; i < w * h; i++) {
        uint32_t pix = pixels[i];
        float r = ((pix >> 16) & 0xFF) / 255.0f;
        float g = ((pix >> 8) & 0xFF) / 255.0f;
        float b = (pix & 0xFF) / 255.0f;

        float nr = 0.5f + (r - 0.5f) * contrast;
        float ng = 0.5f + (g - 0.5f) * contrast;
        float nb = 0.5f + (b - 0.5f) * contrast;

        if (nr < 0.0f) { nr = 0.0f; } if (nr > 1.0f) { nr = 1.0f; }
        if (ng < 0.0f) { ng = 0.0f; } if (ng > 1.0f) { ng = 1.0f; }
        if (nb < 0.0f) { nb = 0.0f; } if (nb > 1.0f) { nb = 1.0f; }

        pixels[i] = (0xFF000000) | ((int)(nr * 255.0f) << 16) | ((int)(ng * 255.0f) << 8) | (int)(nb * 255.0f);
    }
    return 0;
}

int tsfi_quantel_paintbox_pressure_radius(uint32_t *pixels, int w, int h, int cx, int cy, int max_radius, float pressure, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || max_radius <= 0) return -1;
    int r = (int)(max_radius * pressure);
    if (r < 1) { r = 1; }
    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, r, 1.0f, color);
}

int tsfi_quantel_paintbox_velocity_radius(uint32_t *pixels, int w, int h, int cx, int cy, int max_radius, float speed, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || max_radius <= 0) return -1;
    float speed_factor = 1.0f / (1.0f + 0.05f * speed);
    int r = (int)(max_radius * speed_factor);
    if (r < 1) { r = 1; }
    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, r, 1.0f, color);
}

int tsfi_quantel_paintbox_velocity_flow(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float speed, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    float flow = 1.0f / (1.0f + 0.1f * speed);
    if (flow < 0.01f) { flow = 0.01f; }
    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, radius, flow, color);
}

int tsfi_quantel_paintbox_pressure_flow(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    float flow = pressure;
    if (flow < 0.01f) { flow = 0.01f; }
    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, radius, flow, color);
}

int tsfi_quantel_paintbox_sort_palette_green(uint32_t *palette, int size) {
    if (!palette || size <= 0) return -1;
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            uint8_t g1 = (palette[j] >> 8) & 0xFF;
            uint8_t g2 = (palette[j+1] >> 8) & 0xFF;
            if (g1 > g2) {
                uint32_t temp = palette[j];
                palette[j] = palette[j+1];
                palette[j+1] = temp;
            }
        }
    }
    return 0;
}

int tsfi_quantel_paintbox_pressure_jitter_amp(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    float jitter = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp;
    float flow = pressure + jitter;
    if (flow < 0.01f) { flow = 0.01f; }
    if (flow > 1.0f) { flow = 1.0f; }
    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, radius, flow, color);
}

int tsfi_quantel_paintbox_pressure_saturation(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float saturation, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    float sat_mod = saturation * pressure;
    uint32_t mod_color = color;
    
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;

    float luma = 0.299f * r + 0.587f * g + 0.114f * b;
    float nr = luma + (r - luma) * sat_mod;
    float ng = luma + (g - luma) * sat_mod;
    float nb = luma + (b - luma) * sat_mod;

    if (nr < 0.0f) { nr = 0.0f; } if (nr > 1.0f) { nr = 1.0f; }
    if (ng < 0.0f) { ng = 0.0f; } if (ng > 1.0f) { ng = 1.0f; }
    if (nb < 0.0f) { nb = 0.0f; } if (nb > 1.0f) { nb = 1.0f; }

    mod_color = (0xFF000000) | ((int)(nr * 255.0f) << 16) | ((int)(ng * 255.0f) << 8) | (int)(nb * 255.0f);

    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, radius, 1.0f, mod_color);
}

int tsfi_quantel_paintbox_velocity_saturation(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float speed, float saturation, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    float speed_factor = 1.0f / (1.0f + 0.05f * speed);
    float sat_mod = saturation * speed_factor;
    uint32_t mod_color = color;
    
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;

    float luma = 0.299f * r + 0.587f * g + 0.114f * b;
    float nr = luma + (r - luma) * sat_mod;
    float ng = luma + (g - luma) * sat_mod;
    float nb = luma + (b - luma) * sat_mod;

    if (nr < 0.0f) { nr = 0.0f; } if (nr > 1.0f) { nr = 1.0f; }
    if (ng < 0.0f) { ng = 0.0f; } if (ng > 1.0f) { ng = 1.0f; }
    if (nb < 0.0f) { nb = 0.0f; } if (nb > 1.0f) { nb = 1.0f; }

    mod_color = (0xFF000000) | ((int)(nr * 255.0f) << 16) | ((int)(ng * 255.0f) << 8) | (int)(nb * 255.0f);

    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, radius, 1.0f, mod_color);
}

int tsfi_quantel_paintbox_pressure_jitter_flow(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    float jitter = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp;
    float flow = pressure + jitter;
    if (flow < 0.01f) { flow = 0.01f; }
    if (flow > 1.0f) { flow = 1.0f; }
    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, radius, flow, color);
}

int tsfi_quantel_paintbox_velocity_jitter_flow(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float speed, float jitter_amp, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    float speed_factor = 1.0f / (1.0f + 0.05f * speed);
    float jitter = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp;
    float flow = speed_factor + jitter;
    if (flow < 0.01f) { flow = 0.01f; }
    if (flow > 1.0f) { flow = 1.0f; }
    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, radius, flow, color);
}

int tsfi_quantel_paintbox_pressure_jitter_opacity(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    float jitter = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp;
    float opacity = pressure + jitter;
    if (opacity < 0.01f) { opacity = 0.01f; }
    if (opacity > 1.0f) { opacity = 1.0f; }
    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, radius, opacity, color);
}

int tsfi_quantel_paintbox_velocity_jitter_opacity(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float speed, float jitter_amp, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    float speed_factor = 1.0f / (1.0f + 0.05f * speed);
    float jitter = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp;
    float opacity = speed_factor + jitter;
    if (opacity < 0.01f) { opacity = 0.01f; }
    if (opacity > 1.0f) { opacity = 1.0f; }
    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, radius, opacity, color);
}

int tsfi_quantel_paintbox_pressure_jitter_size(uint32_t *pixels, int w, int h, int cx, int cy, int base_radius, float pressure, float jitter_amp, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || base_radius <= 0) return -1;
    float pressure_factor = pressure;
    float jitter = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp;
    float final_radius_f = base_radius * (pressure_factor + jitter);
    int final_radius = (int)final_radius_f;
    if (final_radius < 1) final_radius = 1;
    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, final_radius, 1.0f, color);
}

int tsfi_quantel_paintbox_pressure_jitter_hue(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    float h_val, s_val, l_val;
    tsfi_rgb_to_hsl(r, g, b, &h_val, &s_val, &l_val);

    float jitter = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * pressure;
    h_val += jitter;
    if (h_val < 0.0f) { h_val += 1.0f; }
    if (h_val > 1.0f) { h_val -= 1.0f; }

    uint8_t out_r, out_g, out_b;
    tsfi_hsl_to_rgb(h_val, s_val, l_val, &out_r, &out_g, &out_b);
    uint32_t final_color = (0xFF000000) | (out_r << 16) | (out_g << 8) | out_b;

    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, radius, 1.0f, final_color);
}

int tsfi_quantel_paintbox_pressure_jitter_saturation(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    float h_val, s_val, l_val;
    tsfi_rgb_to_hsl(r, g, b, &h_val, &s_val, &l_val);

    float jitter = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * pressure;
    s_val += jitter;
    if (s_val < 0.0f) { s_val = 0.0f; }
    if (s_val > 1.0f) { s_val = 1.0f; }

    uint8_t out_r, out_g, out_b;
    tsfi_hsl_to_rgb(h_val, s_val, l_val, &out_r, &out_g, &out_b);
    uint32_t final_color = (0xFF000000) | (out_r << 16) | (out_g << 8) | out_b;

    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, radius, 1.0f, final_color);
}

int tsfi_quantel_paintbox_pressure_jitter_value(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    float h_val, s_val, l_val;
    tsfi_rgb_to_hsl(r, g, b, &h_val, &s_val, &l_val);

    float jitter = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * pressure;
    l_val += jitter;
    if (l_val < 0.0f) { l_val = 0.0f; }
    if (l_val > 1.0f) { l_val = 1.0f; }

    uint8_t out_r, out_g, out_b;
    tsfi_hsl_to_rgb(h_val, s_val, l_val, &out_r, &out_g, &out_b);
    uint32_t final_color = (0xFF000000) | (out_r << 16) | (out_g << 8) | out_b;

    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, radius, 1.0f, final_color);
}

int tsfi_quantel_paintbox_pressure_jitter_opacity_value(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    float h_val, s_val, l_val;
    tsfi_rgb_to_hsl(r, g, b, &h_val, &s_val, &l_val);

    float jitter_l = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * pressure;
    l_val += jitter_l;
    if (l_val < 0.0f) { l_val = 0.0f; }
    if (l_val > 1.0f) { l_val = 1.0f; }

    float jitter_a = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp;
    float opacity = pressure + jitter_a;
    if (opacity < 0.01f) { opacity = 0.01f; }
    if (opacity > 1.0f) { opacity = 1.0f; }

    uint8_t out_r, out_g, out_b;
    tsfi_hsl_to_rgb(h_val, s_val, l_val, &out_r, &out_g, &out_b);
    uint32_t final_color = (0xFF000000) | (out_r << 16) | (out_g << 8) | out_b;

    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, radius, opacity, final_color);
}

extern bool blue_box_generate_tone(uint32_t freq1, uint32_t freq2, float *samples_out, size_t count);
extern void blue_box_accumulate_state(uint64_t input_signal);

int tsfi_quantel_orchestrator_paintbox_synth_link(uint32_t *pixels, int w, int h, int cx, int cy, float pressure, float speed, float *audio_buf, int audio_count) {
    if (!pixels || w <= 0 || h <= 0) return -1;

    uint32_t freq1 = 200 + (uint32_t)(cx * 800.0f / w);
    uint32_t freq2 = 300 + (uint32_t)(cy * 1200.0f / h);

    if (audio_buf && audio_count > 0) {
        blue_box_generate_tone(freq1, freq2, audio_buf, (size_t)audio_count);
    }

    uint64_t accumulated_signal = (uint64_t)(pressure * 1000.0f) + (uint64_t)(speed * 500.0f);
    blue_box_accumulate_state(accumulated_signal);

    uint32_t color = (0xFF000000) | ((freq1 & 0xFF) << 16) | ((freq2 & 0xFF) << 8) | ((int)(pressure * 255.0f) & 0xFF);
    int brush_radius = 5 + (int)(speed * 4.0f);
    tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, brush_radius, pressure, color);

    return 0;
}

int tsfi_quantel_paintbox_chalk_pressure_texture(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float texture_scale, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    uint8_t r_src = (color >> 16) & 0xFF;
    uint8_t g_src = (color >> 8) & 0xFF;
    uint8_t b_src = color & 0xFF;

    for (int y = cy - radius; y <= cy + radius; y++) {
        if (y < 0 || y >= h) continue;
        uint32_t *row = pixels + y * w;
        int dy = y - cy;
        for (int x = cx - radius; x <= cx + radius; x++) {
            if (x < 0 || x >= w) continue;
            int dx = x - cx;
            float dist = sqrtf((float)(dx * dx + dy * dy));
            if (dist <= radius) {
                float noise = (float)rand() / RAND_MAX;
                float pattern = sinf(x * texture_scale) * cosf(y * texture_scale);
                float density = (1.0f - (dist / radius)) * pressure * pattern;
                
                if (noise < density) {
                    uint32_t pix = row[x];
                    uint8_t r_dst = (pix >> 16) & 0xFF;
                    uint8_t g_dst = (pix >> 8) & 0xFF;
                    uint8_t b_dst = pix & 0xFF;

                    float alpha = density * 0.8f;
                    uint8_t r = (uint8_t)(r_src * alpha + r_dst * (1.0f - alpha));
                    uint8_t g = (uint8_t)(g_src * alpha + g_dst * (1.0f - alpha));
                    uint8_t b = (uint8_t)(b_src * alpha + b_dst * (1.0f - alpha));

                    row[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_paintbox_pressure_jitter_opacity_saturation(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    float h_val, s_val, l_val;
    tsfi_rgb_to_hsl(r, g, b, &h_val, &s_val, &l_val);

    float jitter_s = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * pressure;
    s_val += jitter_s;
    if (s_val < 0.0f) { s_val = 0.0f; }
    if (s_val > 1.0f) { s_val = 1.0f; }

    float jitter_a = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp;
    float opacity = pressure + jitter_a;
    if (opacity < 0.01f) { opacity = 0.01f; }
    if (opacity > 1.0f) { opacity = 1.0f; }

    uint8_t out_r, out_g, out_b;
    tsfi_hsl_to_rgb(h_val, s_val, l_val, &out_r, &out_g, &out_b);
    uint32_t final_color = (0xFF000000) | (out_r << 16) | (out_g << 8) | out_b;

    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, radius, opacity, final_color);
}

int tsfi_quantel_paintbox_pressure_jitter_opacity_saturation_value(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    float h_val, s_val, l_val;
    tsfi_rgb_to_hsl(r, g, b, &h_val, &s_val, &l_val);

    float jitter_s = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * pressure;
    s_val += jitter_s;
    if (s_val < 0.0f) { s_val = 0.0f; }
    if (s_val > 1.0f) { s_val = 1.0f; }

    float jitter_v = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * pressure;
    l_val += jitter_v;
    if (l_val < 0.0f) { l_val = 0.0f; }
    if (l_val > 1.0f) { l_val = 1.0f; }

    float jitter_a = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp;
    float opacity = pressure + jitter_a;
    if (opacity < 0.01f) { opacity = 0.01f; }
    if (opacity > 1.0f) { opacity = 1.0f; }

    uint8_t out_r, out_g, out_b;
    tsfi_hsl_to_rgb(h_val, s_val, l_val, &out_r, &out_g, &out_b);
    uint32_t final_color = (0xFF000000) | (out_r << 16) | (out_g << 8) | out_b;

    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, radius, opacity, final_color);
}

int tsfi_quantel_paintbox_pressure_jitter_opacity_saturation_value_width(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    float h_val, s_val, l_val;
    tsfi_rgb_to_hsl(r, g, b, &h_val, &s_val, &l_val);

    float jitter_s = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * pressure;
    s_val += jitter_s;
    if (s_val < 0.0f) { s_val = 0.0f; }
    if (s_val > 1.0f) { s_val = 1.0f; }

    float jitter_v = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * pressure;
    l_val += jitter_v;
    if (l_val < 0.0f) { l_val = 0.0f; }
    if (l_val > 1.0f) { l_val = 1.0f; }

    float jitter_a = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp;
    float opacity = pressure + jitter_a;
    if (opacity < 0.01f) { opacity = 0.01f; }
    if (opacity > 1.0f) { opacity = 1.0f; }

    float jitter_w = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * radius;
    int target_radius = radius + (int)jitter_w;
    if (target_radius < 1) { target_radius = 1; }

    uint8_t out_r, out_g, out_b;
    tsfi_hsl_to_rgb(h_val, s_val, l_val, &out_r, &out_g, &out_b);
    uint32_t final_color = (0xFF000000) | (out_r << 16) | (out_g << 8) | out_b;

    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, target_radius, opacity, final_color);
}

int tsfi_quantel_paintbox_pressure_jitter_opacity_saturation_value_width_angle(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color, float angle) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    float h_val, s_val, l_val;
    tsfi_rgb_to_hsl(r, g, b, &h_val, &s_val, &l_val);

    float jitter_s = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * pressure;
    s_val += jitter_s;
    if (s_val < 0.0f) { s_val = 0.0f; }
    if (s_val > 1.0f) { s_val = 1.0f; }

    float jitter_v = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * pressure;
    l_val += jitter_v;
    if (l_val < 0.0f) { l_val = 0.0f; }
    if (l_val > 1.0f) { l_val = 1.0f; }

    float jitter_a = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp;
    float opacity = pressure + jitter_a;
    if (opacity < 0.01f) { opacity = 0.01f; }
    if (opacity > 1.0f) { opacity = 1.0f; }

    float jitter_w = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * radius;
    int target_radius = radius + (int)jitter_w;
    if (target_radius < 1) { target_radius = 1; }

    float jitter_ang = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * (float)M_PI;
    float target_angle = angle + jitter_ang;

    int target_cx = cx + (int)(cosf(target_angle) * pressure * 10.0f);
    int target_cy = cy + (int)(sinf(target_angle) * pressure * 10.0f);

    uint8_t out_r, out_g, out_b;
    tsfi_hsl_to_rgb(h_val, s_val, l_val, &out_r, &out_g, &out_b);
    uint32_t final_color = (0xFF000000) | (out_r << 16) | (out_g << 8) | out_b;

    return tsfi_quantel_paintbox_airbrush(pixels, w, h, target_cx, target_cy, target_radius, opacity, final_color);
}

int tsfi_quantel_paintbox_pressure_jitter_opacity_saturation_value_width_angle_shape(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color, float angle, float shape_aspect) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    float h_val, s_val, l_val;
    tsfi_rgb_to_hsl(r, g, b, &h_val, &s_val, &l_val);

    float jitter_s = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * pressure;
    s_val += jitter_s;
    if (s_val < 0.0f) { s_val = 0.0f; }
    if (s_val > 1.0f) { s_val = 1.0f; }

    float jitter_v = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * pressure;
    l_val += jitter_v;
    if (l_val < 0.0f) { l_val = 0.0f; }
    if (l_val > 1.0f) { l_val = 1.0f; }

    float jitter_a = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp;
    float opacity = pressure + jitter_a;
    if (opacity < 0.01f) { opacity = 0.01f; }
    if (opacity > 1.0f) { opacity = 1.0f; }

    float jitter_w = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * radius;
    int target_radius = radius + (int)jitter_w;
    if (target_radius < 1) { target_radius = 1; }

    float jitter_ang = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * (float)M_PI;
    float target_angle = angle + jitter_ang;

    float jitter_aspect = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * shape_aspect;
    float active_aspect = shape_aspect + jitter_aspect;
    if (active_aspect < 0.1f) { active_aspect = 0.1f; }

    int target_cx = cx + (int)(cosf(target_angle) * pressure * 10.0f);
    int target_cy = cy + (int)(sinf(target_angle) * pressure * 10.0f);

    uint8_t out_r, out_g, out_b;
    tsfi_hsl_to_rgb(h_val, s_val, l_val, &out_r, &out_g, &out_b);
    uint32_t final_color = (0xFF000000) | (out_r << 16) | (out_g << 8) | out_b;

    for (int y = target_cy - target_radius; y <= target_cy + target_radius; y++) {
        if (y < 0 || y >= h) continue;
        int dy = y - target_cy;
        for (int x = target_cx - target_radius; x <= target_cx + target_radius; x++) {
            if (x < 0 || x >= w) continue;
            int dx = x - target_cx;
            float dist = sqrtf((float)(dx * dx) + (float)(dy * dy) / (active_aspect * active_aspect));
            if (dist <= target_radius) {
                pixels[y * w + x] = final_color;
            }
        }
    }
    return 0;
}

int tsfi_quantel_paintbox_pressure_jitter_opacity_saturation_value_width_angle_shape_texture(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color, float angle, float shape_aspect, float texture_scale) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    float h_val, s_val, l_val;
    tsfi_rgb_to_hsl(r, g, b, &h_val, &s_val, &l_val);

    float jitter_s = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * pressure;
    s_val += jitter_s;
    if (s_val < 0.0f) { s_val = 0.0f; }
    if (s_val > 1.0f) { s_val = 1.0f; }

    float jitter_v = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * pressure;
    l_val += jitter_v;
    if (l_val < 0.0f) { l_val = 0.0f; }
    if (l_val > 1.0f) { l_val = 1.0f; }

    float jitter_a = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp;
    float opacity = pressure + jitter_a;
    if (opacity < 0.01f) { opacity = 0.01f; }
    if (opacity > 1.0f) { opacity = 1.0f; }

    float jitter_w = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * radius;
    int target_radius = radius + (int)jitter_w;
    if (target_radius < 1) { target_radius = 1; }

    float jitter_ang = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * (float)M_PI;
    float target_angle = angle + jitter_ang;

    float jitter_aspect = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp * shape_aspect;
    float active_aspect = shape_aspect + jitter_aspect;
    if (active_aspect < 0.1f) { active_aspect = 0.1f; }

    int target_cx = cx + (int)(cosf(target_angle) * pressure * 10.0f);
    int target_cy = cy + (int)(sinf(target_angle) * pressure * 10.0f);

    uint8_t out_r, out_g, out_b;
    tsfi_hsl_to_rgb(h_val, s_val, l_val, &out_r, &out_g, &out_b);
    uint32_t final_color = (0xFF000000) | (out_r << 16) | (out_g << 8) | out_b;

    for (int y = target_cy - target_radius; y <= target_cy + target_radius; y++) {
        if (y < 0 || y >= h) continue;
        int dy = y - target_cy;
        for (int x = target_cx - target_radius; x <= target_cx + target_radius; x++) {
            if (x < 0 || x >= w) continue;
            int dx = x - target_cx;
            float dist = sqrtf((float)(dx * dx) + (float)(dy * dy) / (active_aspect * active_aspect));
            if (dist <= target_radius) {
                float noise = (float)rand() / RAND_MAX;
                if (noise * texture_scale > 0.1f) {
                    pixels[y * w + x] = final_color;
                }
            }
        }
    }
    return 0;
}
