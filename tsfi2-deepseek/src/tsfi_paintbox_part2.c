#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include "tsfi_paint.h"

int tsfi_quantel_paintbox_pressure_jitter_opacity_saturation_value_width_angle_shape_texture_path(
    uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp,
    uint32_t color, float angle, float shape_aspect, float texture_scale, float path_vel_x, float path_vel_y)
{
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;

    float offset_cx = cx + path_vel_x * pressure;
    float offset_cy = cy + path_vel_y * pressure;

    float effective_radius_x = radius * pressure * shape_aspect;
    float effective_radius_y = radius * pressure / shape_aspect;

    if (effective_radius_x < 1.0f) effective_radius_x = 1.0f;
    if (effective_radius_y < 1.0f) effective_radius_y = 1.0f;

    uint8_t r_src = (color >> 16) & 0xFF;
    uint8_t g_src = (color >> 8) & 0xFF;
    uint8_t b_src = color & 0xFF;

    float jitter = ((float)rand() / RAND_MAX - 0.5f) * jitter_amp;
    float sat_scale = 1.0f + jitter * pressure;
    float val_scale = 1.0f - fabs(jitter) * (1.0f - pressure);

    int r_j = (int)(r_src * sat_scale * val_scale);
    int g_j = (int)(g_src * sat_scale * val_scale);
    int b_j = (int)(b_src * sat_scale * val_scale);

    if (r_j < 0) { r_j = 0; }
    if (r_j > 255) { r_j = 255; }
    if (g_j < 0) { g_j = 0; }
    if (g_j > 255) { g_j = 255; }
    if (b_j < 0) { b_j = 0; }
    if (b_j > 255) { b_j = 255; }

    float cos_a = cosf(angle);
    float sin_a = sinf(angle);

    int bound = (int)(effective_radius_x > effective_radius_y ? effective_radius_x : effective_radius_y) + 2;

    for (int y = (int)offset_cy - bound; y <= (int)offset_cy + bound; y++) {
        if (y < 0 || y >= h) continue;
        uint32_t *row = pixels + y * w;
        float dy = y - offset_cy;

        for (int x = (int)offset_cx - bound; x <= (int)offset_cx + bound; x++) {
            if (x < 0 || x >= w) continue;
            float dx = x - offset_cx;

            float rx = dx * cos_a + dy * sin_a;
            float ry = -dx * sin_a + dy * cos_a;

            float ellipse_val = (rx * rx) / (effective_radius_x * effective_radius_x) +
                                (ry * ry) / (effective_radius_y * effective_radius_y);

            if (ellipse_val <= 1.0f) {
                float tex_val = sinf(x * texture_scale) * cosf(y * texture_scale);
                float grain = 0.8f + 0.2f * tex_val;

                float opacity = (1.0f - ellipse_val) * pressure * grain;
                if (opacity > 1.0f) opacity = 1.0f;
                if (opacity < 0.0f) opacity = 0.0f;

                uint32_t dest = row[x];
                uint8_t r_dst = (dest >> 16) & 0xFF;
                uint8_t g_dst = (dest >> 8) & 0xFF;
                uint8_t b_dst = dest & 0xFF;

                uint8_t r_out = (uint8_t)(r_j * opacity + r_dst * (1.0f - opacity));
                uint8_t g_out = (uint8_t)(g_j * opacity + g_dst * (1.0f - opacity));
                uint8_t b_out = (uint8_t)(b_j * opacity + b_dst * (1.0f - opacity));

                row[x] = 0xFF000000 | (r_out << 16) | (g_out << 8) | b_out;
            }
        }
    }

    return 0;
}
