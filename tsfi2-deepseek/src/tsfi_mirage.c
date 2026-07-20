#define _GNU_SOURCE
#include "tsfi_paint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int tsfi_quantel_mirage_page_curl(const uint32_t *src_pixels, int src_w, int src_h, uint32_t *dst_pixels, int dst_w, int dst_h, float curl_radius, float roll_percent) {
    if (!src_pixels || !dst_pixels || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;

    memset(dst_pixels, 0, dst_w * dst_h * sizeof(uint32_t));
    float roll_line = roll_percent * src_w;

    for (int y = 0; y < src_h; y++) {
        float v = (float)y / src_h;
        int dy = (int)(v * dst_h);
        if (dy < 0 || dy >= dst_h) continue;

        for (int x = 0; x < src_w; x++) {
            float u = (float)x / src_w;
            float px = (float)x;

            if (px < roll_line) {
                float dist_to_roll = roll_line - px;
                float theta = dist_to_roll / curl_radius;
                if (theta <= M_PI) {
                    float warp_x = roll_line - curl_radius * sinf(theta);
                    int dx = (int)(warp_x * dst_w / src_w);
                    if (dx >= 0 && dx < dst_w) {
                        dst_pixels[dy * dst_w + dx] = src_pixels[y * src_w + x];
                    }
                }
            } else {
                int dx = (int)(u * dst_w);
                if (dx >= 0 && dx < dst_w) {
                    dst_pixels[dy * dst_w + dx] = src_pixels[y * src_w + x];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_sphere_wrap(const uint32_t *src_pixels, int src_w, int src_h, uint32_t *dst_pixels, int dst_w, int dst_h, float sphere_radius) {
    if (!src_pixels || !dst_pixels || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0 || sphere_radius <= 0.0f) return -1;

    memset(dst_pixels, 0, dst_w * dst_h * sizeof(uint32_t));
    float cx = dst_w / 2.0f;
    float cy = dst_h / 2.0f;

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx;
            float r = sqrtf(dx * dx + dy * dy);
            if (r < sphere_radius) {
                float z = sqrtf(sphere_radius * sphere_radius - r * r);
                float lon = atan2f(dx, z);
                float lat = asinf(dy / sphere_radius);

                float u = (lon + M_PI) / (2.0f * M_PI);
                float v = (lat + M_PI_2) / M_PI;

                int sx = (int)(u * src_w);
                int sy = (int)(v * src_h);

                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    dst_pixels[y * dst_w + x] = src_pixels[sy * src_w + sx];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_flag_ripple(const uint32_t *src_pixels, int src_w, int src_h, uint32_t *dst_pixels, int dst_w, int dst_h, float amplitude, float frequency, float phase) {
    if (!src_pixels || !dst_pixels || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;

    memset(dst_pixels, 0, dst_w * dst_h * sizeof(uint32_t));

    for (int y = 0; y < dst_h; y++) {
        float v = (float)y / dst_h;
        int sy = (int)(v * src_h);
        if (sy < 0 || sy >= src_h) continue;

        for (int x = 0; x < dst_w; x++) {
            float u = (float)x / dst_w;
            float ripple = amplitude * sinf(2.0f * M_PI * frequency * v + phase);
            int sx = (int)((u * src_w) + ripple);

            if (sx >= 0 && sx < src_w) {
                dst_pixels[y * dst_w + x] = src_pixels[sy * src_w + sx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_pyramid_wrap(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float height, float base_size) {
    (void)height;
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0 || base_size <= 0.0f) return -1;

    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));
    float cx = dst_w / 2.0f;
    float cy = dst_h / 2.0f;
    float half_base = base_size / 2.0f;

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx;

            if (fabsf(dx) <= half_base && fabsf(dy) <= half_base) {
                float u = 0.0f, v = 0.0f;
                if (fabsf(dx) > fabsf(dy)) {
                    if (dx > 0) {
                        u = (half_base - dx) / half_base;
                        v = (dy + half_base) / base_size;
                    } else {
                        u = (dx + half_base) / half_base;
                        v = (dy + half_base) / base_size;
                    }
                } else {
                    if (dy > 0) {
                        u = (dx + half_base) / base_size;
                        v = (half_base - dy) / half_base;
                    } else {
                        u = (dx + half_base) / base_size;
                        v = (dy + half_base) / half_base;
                    }
                }

                int sx = (int)(u * src_w);
                int sy = (int)(v * src_h);
                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    dst[y * dst_w + x] = src[sy * src_w + sx];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_cylinder_wrap(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float cylinder_radius) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0 || cylinder_radius <= 0.0f) return -1;

    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));
    float cx = dst_w / 2.0f;

    for (int y = 0; y < dst_h; y++) {
        float v = (float)y / dst_h;
        int sy = (int)(v * src_h);
        if (sy < 0 || sy >= src_h) continue;

        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx;
            if (fabsf(dx) <= cylinder_radius) {
                float theta = asinf(dx / cylinder_radius);
                float u = 0.5f + theta / M_PI;

                int sx = (int)(u * src_w);
                if (sx >= 0 && sx < src_w) {
                    dst[y * dst_w + x] = src[sy * src_w + sx];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_cone_wrap(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float cone_height, float cone_radius) {
    (void)cone_height;
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0 || cone_radius <= 0.0f) return -1;

    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));
    float cx = dst_w / 2.0f;
    float cy = dst_h / 2.0f;

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx;
            float r_proj = sqrtf(dx * dx + dy * dy);

            if (r_proj <= cone_radius) {
                float theta = atan2f(dy, dx);
                if (theta < 0) theta += 2.0f * M_PI;

                float u = theta / (2.0f * M_PI);
                float v = r_proj / cone_radius;

                int sx = (int)(u * src_w);
                int sy = (int)(v * src_h);

                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    dst[y * dst_w + x] = src[sy * src_w + sx];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_torus_wrap(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float inner_r, float outer_r) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0 || inner_r <= 0.0f || outer_r <= inner_r) return -1;

    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));
    float cx = dst_w / 2.0f;
    float cy = dst_h / 2.0f;
    float R = (outer_r + inner_r) / 2.0f;
    float r_minor = (outer_r - inner_r) / 2.0f;

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx;
            float r_proj = sqrtf(dx * dx + dy * dy);

            if (r_proj >= inner_r && r_proj <= outer_r) {
                float theta = atan2f(dy, dx);
                if (theta < 0) theta += 2.0f * M_PI;

                float dist_major = r_proj - R;
                float phi = acosf(dist_major / r_minor);

                float u = theta / (2.0f * M_PI);
                float v = phi / M_PI;

                int sx = (int)(u * src_w);
                int sy = (int)(v * src_h);

                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    dst[y * dst_w + x] = src[sy * src_w + sx];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_mesh_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, const float *grid_points_x, const float *grid_points_y, int grid_cols, int grid_rows) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0 || !grid_points_x || !grid_points_y || grid_cols <= 1 || grid_rows <= 1) return -1;

    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    for (int y = 0; y < dst_h; y++) {
        float norm_y = (float)y / (dst_h - 1) * (grid_rows - 1);
        int r_idx = (int)norm_y;
        if (r_idx >= grid_rows - 1) r_idx = grid_rows - 2;
        float v = norm_y - r_idx;

        for (int x = 0; x < dst_w; x++) {
            float norm_x = (float)x / (dst_w - 1) * (grid_cols - 1);
            int c_idx = (int)norm_x;
            if (c_idx >= grid_cols - 1) c_idx = grid_cols - 2;
            float u = norm_x - c_idx;

            int idx00 = r_idx * grid_cols + c_idx;
            int idx10 = idx00 + 1;
            int idx01 = (r_idx + 1) * grid_cols + c_idx;
            int idx11 = idx01 + 1;

            float x_int = (1.0f - u) * (1.0f - v) * grid_points_x[idx00] +
                          u * (1.0f - v) * grid_points_x[idx10] +
                          (1.0f - u) * v * grid_points_x[idx01] +
                          u * v * grid_points_x[idx11];

            float y_int = (1.0f - u) * (1.0f - v) * grid_points_y[idx00] +
                          u * (1.0f - v) * grid_points_y[idx10] +
                          (1.0f - u) * v * grid_points_y[idx01] +
                          u * v * grid_points_y[idx11];

            int sx = (int)(x_int * src_w / dst_w);
            int sy = (int)(y_int * src_h / dst_h);

            if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                dst[y * dst_w + x] = src[sy * src_w + sx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_dual_sided_page_curl(const uint32_t *front_pixels, const uint32_t *back_pixels, int src_w, int src_h, uint32_t *dst_pixels, int dst_w, int dst_h, float curl_radius, float roll_percent) {
    if (!front_pixels || !back_pixels || !dst_pixels || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;

    memset(dst_pixels, 0, dst_w * dst_h * sizeof(uint32_t));
    float roll_line = roll_percent * src_w;

    for (int y = 0; y < src_h; y++) {
        float v = (float)y / src_h;
        int dy = (int)(v * dst_h);
        if (dy < 0 || dy >= dst_h) continue;

        for (int x = 0; x < src_w; x++) {
            float u = (float)x / src_w;
            float px = (float)x;

            if (px < roll_line) {
                float dist_to_roll = roll_line - px;
                float theta = dist_to_roll / curl_radius;
                if (theta <= M_PI) {
                    float warp_x = roll_line - curl_radius * sinf(theta);
                    int dx = (int)(warp_x * dst_w / src_w);
                    if (dx >= 0 && dx < dst_w) {
                        if (theta > M_PI_2) {
                            dst_pixels[dy * dst_w + dx] = back_pixels[y * src_w + (src_w - 1 - x)];
                        } else {
                            dst_pixels[dy * dst_w + dx] = front_pixels[y * src_w + x];
                        }
                    }
                }
            } else {
                int dx = (int)(u * dst_w);
                if (dx >= 0 && dx < dst_w) {
                    dst_pixels[dy * dst_w + dx] = front_pixels[y * src_w + x];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_page_peel_transition(const uint32_t *src_a, const uint32_t *src_b, uint32_t *dst, int w, int h, float progress, float peel_radius) {
    if (!src_a || !src_b || !dst || w <= 0 || h <= 0) return -1;

    float roll_line = progress * w;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = y * w + x;
            float px = (float)x;

            if (px < roll_line) {
                float dist_to_roll = roll_line - px;
                float theta = dist_to_roll / peel_radius;
                if (theta <= M_PI) {
                    float warp_x = roll_line - peel_radius * sinf(theta);
                    int sx = (int)warp_x;
                    if (sx >= 0 && sx < w) {
                        dst[idx] = src_a[y * w + sx];
                    } else {
                        dst[idx] = src_b[idx];
                    }
                } else {
                    dst[idx] = src_b[idx];
                }
            } else {
                dst[idx] = src_a[idx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_perspective_rotate(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float rot_x, float rot_y, float rot_z, float fov) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;

    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cos_x = cosf(rot_x); float sin_x = sinf(rot_x);
    float cos_y = cosf(rot_y); float sin_y = sinf(rot_y);
    float cos_z = cosf(rot_z); float sin_z = sinf(rot_z);

    float cx_s = src_w / 2.0f;
    float cy_s = src_h / 2.0f;
    float cx_d = dst_w / 2.0f;
    float cy_d = dst_h / 2.0f;

    for (int y = 0; y < src_h; y++) {
        float sy = y - cy_s;
        for (int x = 0; x < src_w; x++) {
            float sx = x - cx_s;

            float x1 = sx * cos_z - sy * sin_z;
            float y1 = sx * sin_z + sy * cos_z;
            float z1 = 0;

            float x2 = x1 * cos_y + z1 * sin_y;
            float y2 = y1;
            float z2 = -x1 * sin_y + z1 * cos_y;

            float x3 = x2;
            float y3 = y2 * cos_x - z2 * sin_x;
            float z3 = y2 * sin_x + z2 * cos_x;

            float z_offset = z3 + fov;
            if (z_offset > 1.0f) {
                float scale = fov / z_offset;
                int dx = (int)(x3 * scale + cx_d);
                int dy = (int)(y3 * scale + cy_d);

                if (dx >= 0 && dx < dst_w && dy >= 0 && dy < dst_h) {
                    dst[dy * dst_w + dx] = src[y * src_w + x];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_height_map_extrude(const uint32_t *src, int w, int h, uint32_t *dst, float max_height, float scale) {
    if (!src || !dst || w <= 0 || h <= 0) return -1;
    memset(dst, 0, w * h * sizeof(uint32_t));
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            uint32_t pix = src[y * w + x];
            uint8_t r = (pix >> 16) & 0xFF;
            uint8_t g = (pix >> 8) & 0xFF;
            uint8_t b = pix & 0xFF;
            float luma = (0.299f * r + 0.587f * g + 0.114f * b) / 255.0f;
            float displace = luma * max_height * scale;
            int new_y = y - (int)displace;
            if (new_y >= 0 && new_y < h) {
                dst[new_y * w + x] = pix;
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_ribbon_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float amplitude, float frequency, float speed, float t) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));
    for (int y = 0; y < dst_h; y++) {
        float norm_y = (float)y / dst_h;
        float wave = amplitude * sinf(2.0f * M_PI * frequency * norm_y + speed * t);
        for (int x = 0; x < dst_w; x++) {
            int sx = x - (int)wave;
            if (sx >= 0 && sx < src_w) {
                int sy = (int)(norm_y * src_h);
                if (sy >= 0 && sy < src_h) {
                    dst[y * dst_w + x] = src[sy * src_w + sx];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_cube_map(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float rot_x, float rot_y, float size) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cos_x = cosf(rot_x); float sin_x = sinf(rot_x);
    float cos_y = cosf(rot_y); float sin_y = sinf(rot_y);
    (void)cos_x; (void)sin_x;

    float cx_d = dst_w / 2.0f;
    float cy_d = dst_h / 2.0f;

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy_d;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx_d;

            float rx = dx * cos_y - dy * sin_y;
            float ry = dx * sin_y * cos_x + dy * cos_y * cos_x;

            if (fabsf(rx) < size && fabsf(ry) < size) {
                float u = (rx + size) / (2.0f * size);
                float v = (ry + size) / (2.0f * size);
                int sx = (int)(u * src_w);
                int sy = (int)(v * src_h);
                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    dst[y * dst_w + x] = src[sy * src_w + sx];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_concentric_ripple(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float amplitude, float wavelength, float speed, float t, float cx, float cy) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx;
            float dist = sqrtf(dx*dx + dy*dy);
            
            float displacement = 0.0f;
            if (dist > 0.001f) {
                displacement = amplitude * sinf(2.0f * M_PI * dist / wavelength - speed * t);
            }
            float factor = dist > 0.001f ? (dist + displacement) / dist : 1.0f;
            
            float norm_x = cx + dx * factor;
            float norm_y = cy + dy * factor;

            int sx = (int)(norm_x * src_w / dst_w);
            int sy = (int)(norm_y * src_h / dst_h);

            if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                dst[y * dst_w + x] = src[sy * src_w + sx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_twirl(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float angle, float radius, float cx, float cy) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx;
            float r = sqrtf(dx * dx + dy * dy);
            if (r < radius) {
                float theta = atan2f(dy, dx) + angle * (1.0f - r / radius);
                float tx = cx + r * cosf(theta);
                float ty = cy + r * sinf(theta);

                int sx = (int)(tx * src_w / dst_w);
                int sy = (int)(ty * src_h / dst_h);

                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    dst[y * dst_w + x] = src[sy * src_w + sx];
                }
            } else {
                int sx = (int)(x * src_w / dst_w);
                int sy = (int)(y * src_h / dst_h);
                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    dst[y * dst_w + x] = src[sy * src_w + sx];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_accordion_fold(const uint32_t *src_a, const uint32_t *src_b, uint32_t *dst, int w, int h, float progress, int folds) {
    if (!src_a || !src_b || !dst || w <= 0 || h <= 0 || folds <= 0) return -1;
    memset(dst, 0, w * h * sizeof(uint32_t));

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float norm_x = (float)x / w;
            float cycle = sinf(norm_x * folds * M_PI);
            float fold_offset = cycle * progress * 0.1f * w;

            int sx = x - (int)fold_offset;
            if (sx >= 0 && sx < w) {
                int idx = y * w + sx;
                if (norm_x < progress) {
                    dst[y * w + x] = src_b[idx];
                } else {
                    dst[y * w + x] = src_a[idx];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_sphere_map(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float rot_x, float rot_y, float radius) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cx_d = dst_w / 2.0f;
    float cy_d = dst_h / 2.0f;

    (void)rot_x;
    (void)rot_y;

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy_d;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx_d;
            float dist_sq = dx * dx + dy * dy;
            if (dist_sq < radius * radius) {
                float z = sqrtf(radius * radius - dist_sq);
                float lon = atan2f(dx, z);
                float lat = asinf(dy / radius);

                float u = (lon + (float)M_PI) / (2.0f * (float)M_PI);
                float v = (lat + (float)M_PI / 2.0f) / (float)M_PI;

                int sx = (int)(u * src_w);
                int sy = (int)(v * src_h);

                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    dst[y * dst_w + x] = src[sy * src_w + sx];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_peel_shadow(uint32_t *pixels, int w, int h, float progress, float shadow_intensity) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    int shadow_boundary = (int)(progress * w);

    for (int y = 0; y < h; y++) {
        uint32_t *row = pixels + y * w;
        for (int x = 0; x < w; x++) {
            if (x >= shadow_boundary - 40 && x < shadow_boundary) {
                float dist = (float)(shadow_boundary - x) / 40.0f;
                float shadow_factor = 1.0f - (1.0f - dist) * shadow_intensity;

                uint32_t pix = row[x];
                uint8_t r = (pix >> 16) & 0xFF;
                uint8_t g = (pix >> 8) & 0xFF;
                uint8_t b = pix & 0xFF;

                r = (uint8_t)(r * shadow_factor);
                g = (uint8_t)(g * shadow_factor);
                b = (uint8_t)(b * shadow_factor);

                row[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_cylindrical_wrap(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float rot_y, float radius) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cx_d = dst_w / 2.0f;
    (void)rot_y;

    for (int y = 0; y < dst_h; y++) {
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx_d;
            if (fabsf(dx) < radius) {
                float z = sqrtf(radius * radius - dx * dx);
                float theta = atan2f(dx, z);

                float u = (theta + (float)M_PI / 2.0f) / (float)M_PI;
                float v = (float)y / dst_h;

                int sx = (int)(u * src_w);
                int sy = (int)(v * src_h);

                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    dst[y * dst_w + x] = src[sy * src_w + sx];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_plane_tilt(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float pitch, float yaw) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cos_p = cosf(pitch); float sin_p = sinf(pitch);
    float cos_y = cosf(yaw); float sin_y = sinf(yaw);

    float cx_d = dst_w / 2.0f;
    float cy_d = dst_h / 2.0f;

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy_d;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx_d;

            float rx = dx * cos_y - dy * sin_y;
            float ry = dx * sin_y * cos_p + dy * cos_y * cos_p;
            float rz = -dx * sin_y * sin_p - dy * cos_y * sin_p + 200.0f;

            if (rz > 1.0f) {
                float px_proj = rx * 200.0f / rz + cx_d;
                float py_proj = ry * 200.0f / rz + cy_d;

                int sx = (int)(px_proj * src_w / dst_w);
                int sy = (int)(py_proj * src_h / dst_h);

                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    dst[y * dst_w + x] = src[sy * src_w + sx];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_flag_wave(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float amplitude, float frequency, float speed, float t) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    for (int y = 0; y < dst_h; y++) {
        float norm_y = (float)y / dst_h;
        for (int x = 0; x < dst_w; x++) {
            float norm_x = (float)x / dst_w;
            float wave = amplitude * norm_x * sinf(2.0f * M_PI * frequency * norm_x + speed * t);
            
            int sy = (int)(norm_y * src_h - wave);
            int sx = (int)(norm_x * src_w);

            if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                dst[y * dst_w + x] = src[sy * src_w + sx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_coordinate_shear(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float shear_x, float shear_y) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cx_d = dst_w / 2.0f;
    float cy_d = dst_h / 2.0f;

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy_d;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx_d;

            float sx_coord = dx + shear_x * dy + cx_d;
            float sy_coord = dy + shear_y * dx + cy_d;

            int sx = (int)(sx_coord * src_w / dst_w);
            int sy = (int)(sy_coord * src_h / dst_h);

            if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                dst[y * dst_w + x] = src[sy * src_w + sx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_wave_grid(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float phase) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    for (int y = 0; y < dst_h; y++) {
        float norm_y = (float)y / dst_h;
        for (int x = 0; x < dst_w; x++) {
            float norm_x = (float)x / dst_w;
            float offset_x = 0.05f * sinf(2.0f * M_PI * norm_y * 4.0f + phase);
            float offset_y = 0.05f * cosf(2.0f * M_PI * norm_x * 4.0f + phase);

            int sx = (int)((norm_x + offset_x) * src_w);
            int sy = (int)((norm_y + offset_y) * src_h);

            if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                dst[y * dst_w + x] = src[sy * src_w + sx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_spherize(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float amount) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cx_d = dst_w / 2.0f;
    float cy_d = dst_h / 2.0f;
    float max_r = cx_d < cy_d ? cx_d : cy_d;
    if (max_r < 1.0f) { max_r = 1.0f; }

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy_d;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx_d;
            float r = sqrtf(dx * dx + dy * dy);
            if (r < max_r) {
                float factor = sinf((float)M_PI * 0.5f * r / max_r);
                float nx = cx_d + dx * (1.0f + (factor - 1.0f) * amount);
                float ny = cy_d + dy * (1.0f + (factor - 1.0f) * amount);

                int sx = (int)(nx * src_w / dst_w);
                int sy = (int)(ny * src_h / dst_h);

                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    dst[y * dst_w + x] = src[sy * src_w + sx];
                }
            } else {
                int sx = (int)((float)x * src_w / dst_w);
                int sy = (int)((float)y * src_h / dst_h);
                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    dst[y * dst_w + x] = src[sy * src_w + sx];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_twirl_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float angle, float radius) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cx_d = dst_w / 2.0f;
    float cy_d = dst_h / 2.0f;

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy_d;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx_d;
            float r = sqrtf(dx * dx + dy * dy);
            if (r < radius) {
                float factor = (radius - r) / radius;
                float theta = factor * factor * angle;
                float cos_t = cosf(theta);
                float sin_t = sinf(theta);

                float nx = cx_d + (dx * cos_t - dy * sin_t);
                float ny = cy_d + (dx * sin_t + dy * cos_t);

                int sx = (int)(nx * src_w / dst_w);
                int sy = (int)(ny * src_h / dst_h);

                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    dst[y * dst_w + x] = src[sy * src_w + sx];
                }
            } else {
                int sx = (int)((float)x * src_w / dst_w);
                int sy = (int)((float)y * src_h / dst_h);
                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    dst[y * dst_w + x] = src[sy * src_w + sx];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_fisheye_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float strength) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cx_d = dst_w / 2.0f;
    float cy_d = dst_h / 2.0f;
    float max_r = cx_d < cy_d ? cx_d : cy_d;
    if (max_r < 1.0f) { max_r = 1.0f; }

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy_d;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx_d;
            float r = sqrtf(dx * dx + dy * dy);
            float factor = 1.0f + strength * (r * r) / (max_r * max_r);

            float nx = cx_d + dx * factor;
            float ny = cy_d + dy * factor;

            int sx = (int)(nx * src_w / dst_w);
            int sy = (int)(ny * src_h / dst_h);

            if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                dst[y * dst_w + x] = src[sy * src_w + sx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_ripple_deform(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float amplitude, float frequency, float phase) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cx_d = dst_w / 2.0f;
    float cy_d = dst_h / 2.0f;

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy_d;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx_d;
            float r = sqrtf(dx * dx + dy * dy);
            float offset = amplitude * sinf(2.0f * M_PI * frequency * r + phase);
            float factor = r > 0.001f ? (r + offset) / r : 1.0f;

            float nx = cx_d + dx * factor;
            float ny = cy_d + dy * factor;

            int sx = (int)(nx * src_w / dst_w);
            int sy = (int)(ny * src_h / dst_h);

            if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                dst[y * dst_w + x] = src[sy * src_w + sx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_pinch_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float factor) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cx_d = dst_w / 2.0f;
    float cy_d = dst_h / 2.0f;
    float max_r = cx_d < cy_d ? cx_d : cy_d;
    if (max_r < 1.0f) { max_r = 1.0f; }

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy_d;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx_d;
            float r = sqrtf(dx * dx + dy * dy);
            float zoom = powf(r / max_r, factor);
            float nx = cx_d + dx * zoom;
            float ny = cy_d + dy * zoom;

            int sx = (int)(nx * src_w / dst_w);
            int sy = (int)(ny * src_h / dst_h);

            if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                dst[y * dst_w + x] = src[sy * src_w + sx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_twirl_zoom(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float angle, float zoom_factor) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cx_d = dst_w / 2.0f;
    float cy_d = dst_h / 2.0f;
    float max_r = cx_d < cy_d ? cx_d : cy_d;
    if (max_r < 1.0f) { max_r = 1.0f; }

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy_d;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx_d;
            float r = sqrtf(dx * dx + dy * dy);
            
            float factor = (max_r - r) / max_r;
            if (factor < 0.0f) { factor = 0.0f; }
            float theta = factor * factor * angle;
            float cos_t = cosf(theta);
            float sin_t = sinf(theta);

            float eff_zoom = 1.0f + (zoom_factor - 1.0f) * factor;
            float nx = cx_d + (dx * cos_t - dy * sin_t) * eff_zoom;
            float ny = cy_d + (dx * sin_t + dy * cos_t) * eff_zoom;

            int sx = (int)(nx * src_w / dst_w);
            int sy = (int)(ny * src_h / dst_h);

            if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                dst[y * dst_w + x] = src[sy * src_w + sx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_double_twirl(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float angle, float radius) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cx_d = dst_w / 2.0f;
    float cy_d = dst_h / 2.0f;

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy_d;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx_d;
            float r = sqrtf(dx * dx + dy * dy);
            
            float factor = (radius - r) / radius;
            if (factor < 0.0f) { factor = 0.0f; }
            float theta = factor * factor * angle * (dx * dy > 0.0f ? 1.0f : -1.0f);
            float cos_t = cosf(theta);
            float sin_t = sinf(theta);

            float nx = cx_d + (dx * cos_t - dy * sin_t);
            float ny = cy_d + (dx * sin_t + dy * cos_t);

            int sx = (int)(nx * src_w / dst_w);
            int sy = (int)(ny * src_h / dst_h);

            if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                dst[y * dst_w + x] = src[sy * src_w + sx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_wave_grid_shift(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float phase, float shift_amount) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    for (int y = 0; y < dst_h; y++) {
        float norm_y = (float)y / dst_h;
        for (int x = 0; x < dst_w; x++) {
            float norm_x = (float)x / dst_w;
            float offset_x = shift_amount * sinf(2.0f * M_PI * norm_y * 4.0f + phase);
            float offset_y = shift_amount * cosf(2.0f * M_PI * norm_x * 4.0f + phase);

            int sx = (int)((norm_x + offset_x) * src_w);
            int sy = (int)((norm_y + offset_y) * src_h);

            if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                dst[y * dst_w + x] = src[sy * src_w + sx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_polar_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float radius) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cx_d = dst_w / 2.0f;
    float cy_d = dst_h / 2.0f;
    if (radius < 1.0f) { radius = 1.0f; }

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy_d;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx_d;
            float r = sqrtf(dx * dx + dy * dy);
            float theta = atan2f(dy, dx);
            if (theta < 0.0f) { theta += 2.0f * (float)M_PI; }

            float u = theta / (2.0f * (float)M_PI);
            float v = r / radius;

            int sx = (int)(u * src_w);
            int sy = (int)(v * src_h);

            if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                dst[y * dst_w + x] = src[sy * src_w + sx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_polar_inverse_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float radius) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cx_d = dst_w / 2.0f;
    float cy_d = dst_h / 2.0f;

    for (int y = 0; y < dst_h; y++) {
        float v = (float)y / dst_h;
        for (int x = 0; x < dst_w; x++) {
            float u = (float)x / dst_w;
            float theta = u * 2.0f * (float)M_PI;
            float r = v * radius;

            float nx = cx_d + r * cosf(theta);
            float ny = cy_d + r * sinf(theta);

            int sx = (int)(nx * src_w / dst_w);
            int sy = (int)(ny * src_h / dst_h);

            if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                dst[y * dst_w + x] = src[sy * src_w + sx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_polar_swirl(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float radius, float swirl_angle) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cx_d = dst_w / 2.0f;
    float cy_d = dst_h / 2.0f;

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy_d;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx_d;
            float r = sqrtf(dx * dx + dy * dy);
            float theta = atan2f(dy, dx);
            if (theta < 0.0f) { theta += 2.0f * (float)M_PI; }

            float factor = (radius - r) / radius;
            if (factor < 0.0f) { factor = 0.0f; }
            float swirl = theta + factor * factor * swirl_angle;

            float u = swirl / (2.0f * (float)M_PI);
            float v = r / radius;

            int sx = (int)(u * src_w);
            int sy = (int)(v * src_h);

            if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                dst[y * dst_w + x] = src[sy * src_w + sx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_plane_rotation(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float pitch, float yaw, float roll) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cx_d = dst_w / 2.0f;
    float cy_d = dst_h / 2.0f;

    float cos_p = cosf(pitch), sin_p = sinf(pitch);
    float cos_y = cosf(yaw), sin_y = sinf(yaw);
    float cos_r = cosf(roll), sin_r = sinf(roll);

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy_d;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx_d;

            float rx = dx * cos_r - dy * sin_r;
            float ry = dx * sin_r + dy * cos_r;

            float rz = ry * sin_p;
            ry = ry * cos_p;

            float nx = rx * cos_y - rz * sin_y;
            float nz = rx * sin_y + rz * cos_y;

            float scale = 1.0f / (1.0f - nz * 0.001f);
            float px = cx_d + nx * scale;
            float py = cy_d + ry * scale;

            int sx = (int)(px * src_w / dst_w);
            int sy = (int)(py * src_h / dst_h);

            if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                dst[y * dst_w + x] = src[sy * src_w + sx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_twirl_inverse_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float angle, float radius) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cx_d = dst_w / 2.0f;
    float cy_d = dst_h / 2.0f;

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy_d;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx_d;
            float r = sqrtf(dx * dx + dy * dy);
            
            float factor = (radius - r) / radius;
            if (factor < 0.0f) { factor = 0.0f; }
            float theta = -factor * factor * angle;
            float cos_t = cosf(theta);
            float sin_t = sinf(theta);

            float nx = cx_d + (dx * cos_t - dy * sin_t);
            float ny = cy_d + (dx * sin_t + dy * cos_t);

            int sx = (int)(nx * src_w / dst_w);
            int sy = (int)(ny * src_h / dst_h);

            if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                dst[y * dst_w + x] = src[sy * src_w + sx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_flag_wave_shift(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float phase, float amplitude) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    for (int y = 0; y < dst_h; y++) {
        float norm_y = (float)y / dst_h;
        for (int x = 0; x < dst_w; x++) {
            float norm_x = (float)x / dst_w;
            float offset = amplitude * sinf(2.0f * M_PI * norm_x * 2.0f + phase);

            int sx = (int)(norm_x * src_w);
            int sy = (int)((norm_y + offset) * src_h);

            if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                dst[y * dst_w + x] = src[sy * src_w + sx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_wave_grid_phase(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float phase_x, float phase_y) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;
    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    for (int y = 0; y < dst_h; y++) {
        float norm_y = (float)y / dst_h;
        for (int x = 0; x < dst_w; x++) {
            float norm_x = (float)x / dst_w;
            float offset_x = 0.05f * sinf(2.0f * M_PI * norm_y * 3.0f + phase_x);
            float offset_y = 0.05f * cosf(2.0f * M_PI * norm_x * 3.0f + phase_y);

            int sx = (int)((norm_x + offset_x) * src_w);
            int sy = (int)((norm_y + offset_y) * src_h);

            if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                dst[y * dst_w + x] = src[sy * src_w + sx];
            }
        }
    }
    return 0;
}
