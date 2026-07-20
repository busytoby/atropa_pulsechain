// Harry Chronological Frame Interlaced Field Split Jitter Blend Offset Color helper functions part 2
#define _GNU_SOURCE
#include "tsfi_paint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int tsfi_quantel_harry_blend_fields_color_offset(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int offset, uint32_t tint_color) {
    if (!field_even || !field_odd || !dst || w <= 0 || h <= 0) return -1;
    if (blend_factor < 0.0f) { blend_factor = 0.0f; }
    if (blend_factor > 1.0f) { blend_factor = 1.0f; }

    uint8_t tr = (tint_color >> 16) & 0xFF;
    uint8_t tg = (tint_color >> 8) & 0xFF;
    uint8_t tb = tint_color & 0xFF;

    for (int y = 0; y < h; y++) {
        const uint32_t *row_a = field_even + y * w;
        const uint32_t *row_b = field_odd + y * w;
        uint32_t *dst_row = dst + y * w;

        int shift_x = (y % 2 == 0) ? offset : -offset;

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

            if (y % 2 == 0) {
                r = (uint8_t)(r * 0.9f + tr * 0.1f);
                g = (uint8_t)(g * 0.9f + tg * 0.1f);
                b = (uint8_t)(b * 0.9f + tb * 0.1f);
            }

            dst_row[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    return 0;
}

int tsfi_quantel_harry_blend_fields_color_offset_vertical(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int offset, uint32_t tint_color, float vert_displace) {
    if (!field_even || !field_odd || !dst || w <= 0 || h <= 0) return -1;
    if (blend_factor < 0.0f) { blend_factor = 0.0f; }
    if (blend_factor > 1.0f) { blend_factor = 1.0f; }

    uint8_t tr = (tint_color >> 16) & 0xFF;
    uint8_t tg = (tint_color >> 8) & 0xFF;
    uint8_t tb = tint_color & 0xFF;

    for (int y = 0; y < h; y++) {
        float displace_y = (y % 2 == 0) ? vert_displace : -vert_displace;
        int sy = y + (int)displace_y;
        if (sy < 0) { sy = 0; }
        if (sy >= h) { sy = h - 1; }

        const uint32_t *row_a = field_even + sy * w;
        const uint32_t *row_b = field_odd + sy * w;
        uint32_t *dst_row = dst + y * w;

        int shift_x = (y % 2 == 0) ? offset : -offset;

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

            if (y % 2 == 0) {
                r = (uint8_t)(r * 0.9f + tr * 0.1f);
                g = (uint8_t)(g * 0.9f + tg * 0.1f);
                b = (uint8_t)(b * 0.9f + tb * 0.1f);
            }

            dst_row[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    return 0;
}

int tsfi_quantel_harry_blend_fields_color_offset_vertical_scale(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int offset, uint32_t tint_color, float vert_displace, float scale) {
    if (!field_even || !field_odd || !dst || w <= 0 || h <= 0) return -1;
    if (blend_factor < 0.0f) { blend_factor = 0.0f; }
    if (blend_factor > 1.0f) { blend_factor = 1.0f; }
    if (scale <= 0.0f) { scale = 1.0f; }

    uint8_t tr = (tint_color >> 16) & 0xFF;
    uint8_t tg = (tint_color >> 8) & 0xFF;
    uint8_t tb = tint_color & 0xFF;
    float cy = h / 2.0f;
    float cx = w / 2.0f;

    for (int y = 0; y < h; y++) {
        float dy = (y - cy) / scale;
        float displace_y = (y % 2 == 0) ? vert_displace : -vert_displace;
        int sy = (int)(cy + dy + displace_y);
        if (sy < 0) { sy = 0; }
        if (sy >= h) { sy = h - 1; }

        const uint32_t *row_a = field_even + sy * w;
        const uint32_t *row_b = field_odd + sy * w;
        uint32_t *dst_row = dst + y * w;

        int shift_x = (y % 2 == 0) ? offset : -offset;

        for (int x = 0; x < w; x++) {
            float dx = (x - cx) / scale;
            int sx = (int)(cx + dx);
            int sx_a = (sx - shift_x + w) % w;
            int sx_b = (sx + shift_x + w) % w;

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

            if (y % 2 == 0) {
                r = (uint8_t)(r * 0.9f + tr * 0.1f);
                g = (uint8_t)(g * 0.9f + tg * 0.1f);
                b = (uint8_t)(b * 0.9f + tb * 0.1f);
            }

            dst_row[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    return 0;
}

int tsfi_quantel_harry_blend_fields_color_offset_vertical_scale_aspect(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int offset, uint32_t tint_color, float vert_displace, float scale, float aspect) {
    if (!field_even || !field_odd || !dst || w <= 0 || h <= 0) return -1;
    if (blend_factor < 0.0f) { blend_factor = 0.0f; }
    if (blend_factor > 1.0f) { blend_factor = 1.0f; }
    if (scale <= 0.0f) { scale = 1.0f; }
    if (aspect <= 0.0f) { aspect = 1.0f; }

    uint8_t tr = (tint_color >> 16) & 0xFF;
    uint8_t tg = (tint_color >> 8) & 0xFF;
    uint8_t tb = tint_color & 0xFF;
    float cy = h / 2.0f;
    float cx = w / 2.0f;

    for (int y = 0; y < h; y++) {
        float dy = (y - cy) / scale;
        float displace_y = (y % 2 == 0) ? vert_displace : -vert_displace;
        int sy = (int)(cy + dy + displace_y);
        if (sy < 0) { sy = 0; }
        if (sy >= h) { sy = h - 1; }

        const uint32_t *row_a = field_even + sy * w;
        const uint32_t *row_b = field_odd + sy * w;
        uint32_t *dst_row = dst + y * w;

        int shift_x = (y % 2 == 0) ? offset : -offset;

        for (int x = 0; x < w; x++) {
            float dx = (x - cx) / (scale * aspect);
            int sx = (int)(cx + dx);
            int sx_a = (sx - shift_x + w) % w;
            int sx_b = (sx + shift_x + w) % w;

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

            if (y % 2 == 0) {
                r = (uint8_t)(r * 0.9f + tr * 0.1f);
                g = (uint8_t)(g * 0.9f + tg * 0.1f);
                b = (uint8_t)(b * 0.9f + tb * 0.1f);
            }

            dst_row[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    return 0;
}

int tsfi_quantel_harry_blend_fields_color_offset_vertical_scale_aspect_rotation(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int offset, uint32_t tint_color, float vert_displace, float scale, float aspect, float rotation_angle) {
    if (!field_even || !field_odd || !dst || w <= 0 || h <= 0) return -1;
    if (blend_factor < 0.0f) { blend_factor = 0.0f; }
    if (blend_factor > 1.0f) { blend_factor = 1.0f; }
    if (scale <= 0.0f) { scale = 1.0f; }
    if (aspect <= 0.0f) { aspect = 1.0f; }

    uint8_t tr = (tint_color >> 16) & 0xFF;
    uint8_t tg = (tint_color >> 8) & 0xFF;
    uint8_t tb = tint_color & 0xFF;
    float cy = h / 2.0f;
    float cx = w / 2.0f;
    float cos_r = cosf(rotation_angle);
    float sin_r = sinf(rotation_angle);

    for (int y = 0; y < h; y++) {
        float dy = (y - cy) / scale;
        float displace_y = (y % 2 == 0) ? vert_displace : -vert_displace;
        int sy = (int)(cy + dy + displace_y);
        if (sy < 0) { sy = 0; }
        if (sy >= h) { sy = h - 1; }

        const uint32_t *row_a = field_even + sy * w;
        const uint32_t *row_b = field_odd + sy * w;
        uint32_t *dst_row = dst + y * w;

        int shift_x = (y % 2 == 0) ? offset : -offset;

        for (int x = 0; x < w; x++) {
            float dx = (x - cx) / (scale * aspect);
            
            float rx_rot = dx * cos_r - dy * sin_r;
            int sx = (int)(cx + rx_rot);
            int sx_a = (sx - shift_x + w) % w;
            int sx_b = (sx + shift_x + w) % w;

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

            if (y % 2 == 0) {
                r = (uint8_t)(r * 0.9f + tr * 0.1f);
                g = (uint8_t)(g * 0.9f + tg * 0.1f);
                b = (uint8_t)(b * 0.9f + tb * 0.1f);
            }

            dst_row[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    return 0;
}

int tsfi_quantel_harry_blend_fields_color_offset_vertical_scale_aspect_rotation_center(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int offset, uint32_t tint_color, float vert_displace, float scale, float aspect, float rotation_angle, float center_x, float center_y) {
    if (!field_even || !field_odd || !dst || w <= 0 || h <= 0) return -1;
    if (blend_factor < 0.0f) { blend_factor = 0.0f; }
    if (blend_factor > 1.0f) { blend_factor = 1.0f; }
    if (scale <= 0.0f) { scale = 1.0f; }
    if (aspect <= 0.0f) { aspect = 1.0f; }

    uint8_t tr = (tint_color >> 16) & 0xFF;
    uint8_t tg = (tint_color >> 8) & 0xFF;
    uint8_t tb = tint_color & 0xFF;
    float cos_r = cosf(rotation_angle);
    float sin_r = sinf(rotation_angle);

    for (int y = 0; y < h; y++) {
        float dy = (y - center_y) / scale;
        float displace_y = (y % 2 == 0) ? vert_displace : -vert_displace;
        int sy = (int)(center_y + dy + displace_y);
        if (sy < 0) { sy = 0; }
        if (sy >= h) { sy = h - 1; }

        const uint32_t *row_a = field_even + sy * w;
        const uint32_t *row_b = field_odd + sy * w;
        uint32_t *dst_row = dst + y * w;

        int shift_x = (y % 2 == 0) ? offset : -offset;

        for (int x = 0; x < w; x++) {
            float dx = (x - center_x) / (scale * aspect);
            
            float rx_rot = dx * cos_r - dy * sin_r;
            int sx = (int)(center_x + rx_rot);
            int sx_a = (sx - shift_x + w) % w;
            int sx_b = (sx + shift_x + w) % w;

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

            if (y % 2 == 0) {
                r = (uint8_t)(r * 0.9f + tr * 0.1f);
                g = (uint8_t)(g * 0.9f + tg * 0.1f);
                b = (uint8_t)(b * 0.9f + tb * 0.1f);
            }

            dst_row[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    return 0;
}
