// Mirage 3D Spherical Page Curl helper functions part 2
#define _GNU_SOURCE
#include "tsfi_paint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int tsfi_quantel_mirage_page_curl_perspective_rotation_shear_center_light_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float curl_radius, float angle, float zoom, float perspective, float rotation_angle, float shear_x, float shear_y, float center_x, float center_y, float light_x, float light_y) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0 || curl_radius <= 0.0f) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cos_a = cosf(angle);
    float sin_a = sinf(angle);
    float cos_r = cosf(rotation_angle);
    float sin_r = sinf(rotation_angle);

    for (int y = 0; y < dst_h; y++) {
        for (int x = 0; x < dst_w; x++) {
            float dy = y - center_y;
            float p_scale = 1.0f + dy * perspective / dst_h;
            float px = center_x + (x - center_x) / (p_scale * zoom);
            float py = center_y + (y - center_y) / (p_scale * zoom);

            float rx_rot = (px - center_x) * cos_r - (py - center_y) * sin_r + center_x;
            float ry_rot = (px - center_x) * sin_r + (py - center_y) * cos_r + center_y;

            float rx = rx_rot + shear_x * (ry_rot - center_y);
            float ry = ry_rot + shear_y * (rx_rot - center_x);

            float d = rx * cos_a + ry * sin_a;
            if (d > 0.0f) {
                float theta = d / curl_radius;
                float nx = rx - curl_radius * sinf(theta) * cos_a;
                float ny = ry - curl_radius * sinf(theta) * sin_a;

                int sx = (int)(nx * src_w / dst_w);
                int sy = (int)(ny * src_h / dst_h);

                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    uint32_t pix = src[sy * src_w + sx];
                    float dot_light = cos_a * light_x + sin_a * light_y;
                    float shadow = 0.7f + 0.3f * fmaxf(0.0f, dot_light * sinf(theta));
                    uint8_t r = (uint8_t)(((pix >> 16) & 0xFF) * shadow);
                    uint8_t g = (uint8_t)(((pix >> 8) & 0xFF) * shadow);
                    uint8_t b = (uint8_t)((pix & 0xFF) * shadow);
                    dst[y * dst_w + x] = (0xFF000000) | (r << 16) | (g << 8) | b;
                }
            } else {
                int sx = (int)(rx * src_w / dst_w);
                int sy = (int)(ry * src_h / dst_h);
                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    dst[y * dst_w + x] = src[sy * src_w + sx];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_page_curl_perspective_rotation_shear_center_light_depth_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float curl_radius, float angle, float zoom, float perspective, float rotation_angle, float shear_x, float shear_y, float center_x, float center_y, float light_x, float light_y, float depth_offset) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0 || curl_radius <= 0.0f) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cos_a = cosf(angle);
    float sin_a = sinf(angle);
    float cos_r = cosf(rotation_angle);
    float sin_r = sinf(rotation_angle);

    for (int y = 0; y < dst_h; y++) {
        for (int x = 0; x < dst_w; x++) {
            float dy = y - center_y;
            float p_scale = 1.0f + dy * perspective / dst_h + depth_offset;
            float px = center_x + (x - center_x) / (p_scale * zoom);
            float py = center_y + (y - center_y) / (p_scale * zoom);

            float rx_rot = (px - center_x) * cos_r - (py - center_y) * sin_r + center_x;
            float ry_rot = (px - center_x) * sin_r + (py - center_y) * cos_r + center_y;

            float rx = rx_rot + shear_x * (ry_rot - center_y);
            float ry = ry_rot + shear_y * (rx_rot - center_x);

            float d = rx * cos_a + ry * sin_a;
            if (d > 0.0f) {
                float theta = d / curl_radius;
                float nx = rx - curl_radius * sinf(theta) * cos_a;
                float ny = ry - curl_radius * sinf(theta) * sin_a;

                int sx = (int)(nx * src_w / dst_w);
                int sy = (int)(ny * src_h / dst_h);

                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    uint32_t pix = src[sy * src_w + sx];
                    float dot_light = cos_a * light_x + sin_a * light_y;
                    float depth_fade = 1.0f - fminf(1.0f, fmaxf(0.0f, depth_offset * 0.5f));
                    float shadow = (0.7f + 0.3f * fmaxf(0.0f, dot_light * sinf(theta))) * depth_fade;
                    uint8_t r = (uint8_t)(((pix >> 16) & 0xFF) * shadow);
                    uint8_t g = (uint8_t)(((pix >> 8) & 0xFF) * shadow);
                    uint8_t b = (uint8_t)((pix & 0xFF) * shadow);
                    dst[y * dst_w + x] = (0xFF000000) | (r << 16) | (g << 8) | b;
                }
            } else {
                int sx = (int)(rx * src_w / dst_w);
                int sy = (int)(ry * src_h / dst_h);
                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    dst[y * dst_w + x] = src[sy * src_w + sx];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_page_curl_perspective_rotation_shear_center_light_depth_displacement_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float curl_radius, float angle, float zoom, float perspective, float rotation_angle, float shear_x, float shear_y, float center_x, float center_y, float light_x, float light_y, float depth_offset, float disp_amplitude, float disp_frequency) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0 || curl_radius <= 0.0f) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cos_a = cosf(angle);
    float sin_a = sinf(angle);
    float cos_r = cosf(rotation_angle);
    float sin_r = sinf(rotation_angle);

    for (int y = 0; y < dst_h; y++) {
        for (int x = 0; x < dst_w; x++) {
            float dy = y - center_y;
            float p_scale = 1.0f + dy * perspective / dst_h + depth_offset;
            float px = center_x + (x - center_x) / (p_scale * zoom);
            float py = center_y + (y - center_y) / (p_scale * zoom);

            float rx_rot = (px - center_x) * cos_r - (py - center_y) * sin_r + center_x;
            float ry_rot = (px - center_x) * sin_r + (py - center_y) * cos_r + center_y;

            float rx = rx_rot + shear_x * (ry_rot - center_y);
            float ry = ry_rot + shear_y * (rx_rot - center_x);

            float disp = disp_amplitude * sinf((rx + ry) * disp_frequency);
            float rx_disp = rx + disp * cos_a;
            float ry_disp = ry + disp * sin_a;

            float d = rx_disp * cos_a + ry_disp * sin_a;
            if (d > 0.0f) {
                float theta = d / curl_radius;
                float nx = rx_disp - curl_radius * sinf(theta) * cos_a;
                float ny = ry_disp - curl_radius * sinf(theta) * sin_a;

                int sx = (int)(nx * src_w / dst_w);
                int sy = (int)(ny * src_h / dst_h);

                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    uint32_t pix = src[sy * src_w + sx];
                    float dot_light = cos_a * light_x + sin_a * light_y;
                    float depth_fade = 1.0f - fminf(1.0f, fmaxf(0.0f, depth_offset * 0.5f));
                    float shadow = (0.7f + 0.3f * fmaxf(0.0f, dot_light * sinf(theta))) * depth_fade;
                    uint8_t r = (uint8_t)(((pix >> 16) & 0xFF) * shadow);
                    uint8_t g = (uint8_t)(((pix >> 8) & 0xFF) * shadow);
                    uint8_t b = (uint8_t)((pix & 0xFF) * shadow);
                    dst[y * dst_w + x] = (0xFF000000) | (r << 16) | (g << 8) | b;
                }
            } else {
                int sx = (int)(rx_disp * src_w / dst_w);
                int sy = (int)(ry_disp * src_h / dst_h);
                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    dst[y * dst_w + x] = src[sy * src_w + sx];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_page_curl_perspective_rotation_shear_center_light_depth_displacement_phong_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float curl_radius, float angle, float zoom, float perspective, float rotation_angle, float shear_x, float shear_y, float center_x, float center_y, float light_x, float light_y, float depth_offset, float disp_amplitude, float disp_frequency, float phong_specular) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0 || curl_radius <= 0.0f) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cos_a = cosf(angle);
    float sin_a = sinf(angle);
    float cos_r = cosf(rotation_angle);
    float sin_r = sinf(rotation_angle);

    for (int y = 0; y < dst_h; y++) {
        for (int x = 0; x < dst_w; x++) {
            float dy = y - center_y;
            float p_scale = 1.0f + dy * perspective / dst_h + depth_offset;
            float px = center_x + (x - center_x) / (p_scale * zoom);
            float py = center_y + (y - center_y) / (p_scale * zoom);

            float rx_rot = (px - center_x) * cos_r - (py - center_y) * sin_r + center_x;
            float ry_rot = (px - center_x) * sin_r + (py - center_y) * cos_r + center_y;

            float rx = rx_rot + shear_x * (ry_rot - center_y);
            float ry = ry_rot + shear_y * (rx_rot - center_x);

            float disp = disp_amplitude * sinf((rx + ry) * disp_frequency);
            float rx_disp = rx + disp * cos_a;
            float ry_disp = ry + disp * sin_a;

            float d = rx_disp * cos_a + ry_disp * sin_a;
            if (d > 0.0f) {
                float theta = d / curl_radius;
                float nx = rx_disp - curl_radius * sinf(theta) * cos_a;
                float ny = ry_disp - curl_radius * sinf(theta) * sin_a;

                int sx = (int)(nx * src_w / dst_w);
                int sy = (int)(ny * src_h / dst_h);

                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    uint32_t pix = src[sy * src_w + sx];
                    float dot_light = cos_a * light_x + sin_a * light_y;
                    float depth_fade = 1.0f - fminf(1.0f, fmaxf(0.0f, depth_offset * 0.5f));
                    float shadow = (0.7f + 0.3f * fmaxf(0.0f, dot_light * sinf(theta))) * depth_fade;

                    float nx_norm = cos_a * cosf(theta);
                    float ny_norm = sin_a * cosf(theta);
                    float nz_norm = sinf(theta);
                    float dot_spec = nx_norm * light_x + ny_norm * light_y + nz_norm * 0.5f;
                    float phong = 0.0f;
                    if (dot_spec > 0.0f) {
                        phong = powf(dot_spec, 16.0f) * phong_specular;
                    }

                    int r = (int)(((pix >> 16) & 0xFF) * shadow + phong * 255.0f);
                    int g = (int)(((pix >> 8) & 0xFF) * shadow + phong * 255.0f);
                    int b = (int)((pix & 0xFF) * shadow + phong * 255.0f);

                    if (r > 255) r = 255;
                    if (g > 255) g = 255;
                    if (b > 255) b = 255;

                    dst[y * dst_w + x] = (0xFF000000) | (r << 16) | (g << 8) | b;
                }
            } else {
                int sx = (int)(rx_disp * src_w / dst_w);
                int sy = (int)(ry_disp * src_h / dst_h);
                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    dst[y * dst_w + x] = src[sy * src_w + sx];
                }
            }
        }
    }
    return 0;
}
