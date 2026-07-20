#define _GNU_SOURCE
#include "tsfi_paint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int tsfi_quantel_paintbox_airbrush(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    
    uint8_t r_src = (color >> 16) & 0xFF;
    uint8_t g_src = (color >> 8) & 0xFF;
    uint8_t b_src = color & 0xFF;
    float sigma = radius / 3.0f;
    float denom = 2.0f * sigma * sigma;

    for (int y = cy - radius; y <= cy + radius; y++) {
        if (y < 0 || y >= h) continue;
        uint32_t *row = pixels + y * w;
        int dy = y - cy;
        for (int x = cx - radius; x <= cx + radius; x++) {
            if (x < 0 || x >= w) continue;
            int dx = x - cx;
            float dist_sq = (float)(dx * dx + dy * dy);
            if (dist_sq <= radius * radius) {
                float intensity = expf(-dist_sq / denom) * pressure;
                if (intensity > 1.0f) intensity = 1.0f;
                if (intensity < 0.0f) intensity = 0.0f;
                
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
    return 0;
}

int tsfi_quantel_paintbox_apply_stencil(uint32_t *pixels, int w, int h, const uint8_t *stencil_mask, uint32_t color) {
    if (!pixels || !stencil_mask || w <= 0 || h <= 0) return -1;
    
    uint8_t r_src = (color >> 16) & 0xFF;
    uint8_t g_src = (color >> 8) & 0xFF;
    uint8_t b_src = color & 0xFF;

    for (int i = 0; i < w * h; i++) {
        float alpha = stencil_mask[i] / 255.0f;
        if (alpha > 0.0f) {
            uint32_t dest = pixels[i];
            uint8_t r_dst = (dest >> 16) & 0xFF;
            uint8_t g_dst = (dest >> 8) & 0xFF;
            uint8_t b_dst = dest & 0xFF;

            uint8_t r_res = (uint8_t)(r_src * alpha + r_dst * (1.0f - alpha));
            uint8_t g_res = (uint8_t)(g_src * alpha + g_dst * (1.0f - alpha));
            uint8_t b_res = (uint8_t)(b_src * alpha + b_dst * (1.0f - alpha));

            pixels[i] = (0xFF000000) | (r_res << 16) | (g_res << 8) | b_res;
        }
    }
    return 0;
}

int tsfi_quantel_paintbox_custom_brush(uint32_t *pixels, int w, int h, int cx, int cy, const uint8_t *brush_tex, int brush_w, int brush_h, float opacity, uint32_t color) {
    if (!pixels || !brush_tex || w <= 0 || h <= 0 || brush_w <= 0 || brush_h <= 0) return -1;

    uint8_t r_src = (color >> 16) & 0xFF;
    uint8_t g_src = (color >> 8) & 0xFF;
    uint8_t b_src = color & 0xFF;

    int half_bw = brush_w / 2;
    int half_bh = brush_h / 2;

    for (int by = 0; by < brush_h; by++) {
        int canvas_y = cy - half_bh + by;
        if (canvas_y < 0 || canvas_y >= h) continue;
        uint32_t *canvas_row = pixels + canvas_y * w;
        const uint8_t *brush_row = brush_tex + by * brush_w;

        for (int bx = 0; bx < brush_w; bx++) {
            int canvas_x = cx - half_bw + bx;
            if (canvas_x < 0 || canvas_x >= w) continue;

            float intensity = (brush_row[bx] / 255.0f) * opacity;
            if (intensity > 0.0f) {
                if (intensity > 1.0f) intensity = 1.0f;
                uint32_t dest = canvas_row[canvas_x];
                uint8_t r_dst = (dest >> 16) & 0xFF;
                uint8_t g_dst = (dest >> 8) & 0xFF;
                uint8_t b_dst = dest & 0xFF;

                uint8_t r_res = (uint8_t)(r_src * intensity + r_dst * (1.0f - intensity));
                uint8_t g_res = (uint8_t)(g_src * intensity + g_dst * (1.0f - intensity));
                uint8_t b_res = (uint8_t)(b_src * intensity + b_dst * (1.0f - intensity));

                canvas_row[canvas_x] = (0xFF000000) | (r_res << 16) | (g_res << 8) | b_res;
            }
        }
    }
    return 0;
}

int tsfi_quantel_paintbox_gradient(uint32_t *pixels, int w, int h, int x0, int y0, int x1, int y1, uint32_t color_start, uint32_t color_end, const char *gradient_type) {
    if (!pixels || w <= 0 || h <= 0) return -1;

    uint8_t r_s = (color_start >> 16) & 0xFF;
    uint8_t g_s = (color_start >> 8) & 0xFF;
    uint8_t b_s = color_start & 0xFF;

    uint8_t r_e = (color_end >> 16) & 0xFF;
    uint8_t g_e = (color_end >> 8) & 0xFF;
    uint8_t b_e = color_end & 0xFF;

    float dx = x1 - x0;
    float dy = y1 - y0;
    float len_sq = dx * dx + dy * dy;
    if (len_sq < 1.0f) len_sq = 1.0f;

    for (int y = 0; y < h; y++) {
        uint32_t *row = pixels + y * w;
        for (int x = 0; x < w; x++) {
            float t = 0.0f;
            if (strcmp(gradient_type, "linear") == 0) {
                t = ((x - x0) * dx + (y - y0) * dy) / len_sq;
            } else if (strcmp(gradient_type, "radial") == 0) {
                float rx = x - x0;
                float ry = y - y0;
                t = sqrtf(rx * rx + ry * ry) / sqrtf(len_sq);
            }

            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;

            uint8_t r = (uint8_t)(r_s * (1.0f - t) + r_e * t);
            uint8_t g = (uint8_t)(g_s * (1.0f - t) + g_e * t);
            uint8_t b = (uint8_t)(b_s * (1.0f - t) + b_e * t);

            row[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    return 0;
}

int tsfi_quantel_paintbox_spray_can(uint32_t *pixels, int w, int h, int cx, int cy, int max_radius, float pressure, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || max_radius <= 0) return -1;

    uint8_t r_src = (color >> 16) & 0xFF;
    uint8_t g_src = (color >> 8) & 0xFF;
    uint8_t b_src = color & 0xFF;

    int dot_count = (int)(pressure * 100.0f);
    if (dot_count < 5) dot_count = 5;

    for (int i = 0; i < dot_count; i++) {
        float r_val = ((float)rand() / RAND_MAX) * max_radius;
        float theta = ((float)rand() / RAND_MAX) * 2.0f * M_PI;

        int dx = (int)(r_val * cosf(theta));
        int dy = (int)(r_val * sinf(theta));

        int px = cx + dx;
        int py = cy + dy;

        if (px >= 0 && px < w && py >= 0 && py < h) {
            int idx = py * w + px;
            uint32_t dest = pixels[idx];

            uint8_t r_dst = (dest >> 16) & 0xFF;
            uint8_t g_dst = (dest >> 8) & 0xFF;
            uint8_t b_dst = dest & 0xFF;

            float intensity = 0.5f * pressure;
            uint8_t r_res = (uint8_t)(r_src * intensity + r_dst * (1.0f - intensity));
            uint8_t g_res = (uint8_t)(g_src * intensity + g_dst * (1.0f - intensity));
            uint8_t b_res = (uint8_t)(b_src * intensity + b_dst * (1.0f - intensity));

            pixels[idx] = (0xFF000000) | (r_res << 16) | (g_res << 8) | b_res;
        }
    }
    return 0;
}

int tsfi_quantel_paintbox_flood_fill(uint32_t *pixels, int w, int h, int start_x, int start_y, uint32_t fill_color, float tolerance) {
    if (!pixels || w <= 0 || h <= 0 || start_x < 0 || start_x >= w || start_y < 0 || start_y >= h) return -1;

    uint32_t start_color = pixels[start_y * w + start_x];
    if (start_color == fill_color) return 0;

    uint8_t target_r = (start_color >> 16) & 0xFF;
    uint8_t target_g = (start_color >> 8) & 0xFF;
    uint8_t target_b = start_color & 0xFF;

    int max_nodes = w * h;
    int *queue_x = malloc(max_nodes * sizeof(int));
    int *queue_y = malloc(max_nodes * sizeof(int));
    uint8_t *visited = calloc(max_nodes, 1);

    if (!queue_x || !queue_y || !visited) {
        free(queue_x); free(queue_y); free(visited);
        return -2;
    }

    int q_start = 0, q_end = 0;
    queue_x[q_end] = start_x;
    queue_y[q_end] = start_y;
    visited[start_y * w + start_x] = 1;
    q_end++;

    int dx[] = { 1, -1, 0, 0 };
    int dy[] = { 0, 0, 1, -1 };

    while (q_start < q_end) {
        int cx = queue_x[q_start];
        int cy = queue_y[q_start];
        q_start++;

        int idx = cy * w + cx;
        uint32_t color = pixels[idx];

        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;

        float diff = sqrtf((r - target_r) * (r - target_r) +
                           (g - target_g) * (g - target_g) +
                           (b - target_b) * (b - target_b));

        if (diff <= tolerance) {
            pixels[idx] = fill_color;

            for (int i = 0; i < 4; i++) {
                int nx = cx + dx[i];
                int ny = cy + dy[i];
                if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                    int nidx = ny * w + nx;
                    if (!visited[nidx]) {
                        queue_x[q_end] = nx;
                        queue_y[q_end] = ny;
                        visited[nidx] = 1;
                        q_end++;
                    }
                }
            }
        }
    }

    free(queue_x); free(queue_y); free(visited);
    return 0;
}

int tsfi_quantel_paintbox_calligraphy_shape(uint32_t *pixels, int w, int h, int start_x, int start_y, int end_x, int end_y, int max_radius, float pressure, float aspect_ratio, float angle, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0) return -1;

    float dx = end_x - start_x;
    float dy = end_y - start_y;
    float dist = sqrtf(dx * dx + dy * dy);

    int steps = (int)(dist + 1.0f);
    for (int i = 0; i <= steps; i++) {
        float t = (float)i / steps;
        int cx = (int)(start_x + dx * t);
        int cy = (int)(start_y + dy * t);
        tsfi_quantel_paintbox_tablet_brush(pixels, w, h, cx, cy, max_radius, pressure, aspect_ratio, angle, color);
    }
    return 0;
}

int tsfi_quantel_paintbox_clone(uint32_t *pixels, int w, int h, int cx, int cy, int src_dx, int src_dy, int radius, float opacity) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;

    uint32_t *temp = malloc(w * h * sizeof(uint32_t));
    if (!temp) return -2;
    memcpy(temp, pixels, w * h * sizeof(uint32_t));

    for (int y = cy - radius; y <= cy + radius; y++) {
        if (y < 0 || y >= h) continue;
        uint32_t *row = pixels + y * w;
        int dy = y - cy;

        for (int x = cx - radius; x <= cx + radius; x++) {
            if (x < 0 || x >= w) continue;
            int dx = x - cx;

            if (dx * dx + dy * dy <= radius * radius) {
                int sy = y + src_dy;
                int sx = x + src_dx;

                if (sx >= 0 && sx < w && sy >= 0 && sy < h) {
                    uint32_t src_color = temp[sy * w + sx];
                    uint8_t r_src = (src_color >> 16) & 0xFF;
                    uint8_t g_src = (src_color >> 8) & 0xFF;
                    uint8_t b_src = src_color & 0xFF;

                    uint32_t dest = row[x];
                    uint8_t r_dst = (dest >> 16) & 0xFF;
                    uint8_t g_dst = (dest >> 8) & 0xFF;
                    uint8_t b_dst = dest & 0xFF;

                    float intensity = (1.0f - (float)(dx * dx + dy * dy) / (radius * radius)) * opacity;
                    if (intensity > 1.0f) intensity = 1.0f;
                    if (intensity < 0.0f) intensity = 0.0f;

                    uint8_t r_res = (uint8_t)(r_src * intensity + r_dst * (1.0f - intensity));
                    uint8_t g_res = (uint8_t)(g_src * intensity + g_dst * (1.0f - intensity));
                    uint8_t b_res = (uint8_t)(b_src * intensity + b_dst * (1.0f - intensity));

                    row[x] = (0xFF000000) | (r_res << 16) | (g_res << 8) | b_res;
                }
            }
        }
    }
    free(temp);
    return 0;
}

int tsfi_quantel_paintbox_spacing_brush(uint32_t *pixels, int w, int h, int prev_x, int prev_y, int curr_x, int curr_y, const uint8_t *brush_tex, int brush_w, int brush_h, float base_spacing, float opacity, uint32_t color) {
    if (!pixels || !brush_tex || w <= 0 || h <= 0 || brush_w <= 0 || brush_h <= 0) return -1;

    float dx = curr_x - prev_x;
    float dy = curr_y - prev_y;
    float dist = sqrtf(dx * dx + dy * dy);

    float speed_factor = dist > 1.0f ? (1.0f + 0.1f * dist) : 1.0f;
    float step = base_spacing * speed_factor;
    if (step < 1.0f) step = 1.0f;

    int stamps = (int)(dist / step) + 1;
    for (int i = 0; i < stamps; i++) {
        float t = stamps > 1 ? ((float)i / (stamps - 1)) : 0.0f;
        int cx = (int)(prev_x + dx * t);
        int cy = (int)(prev_y + dy * t);
        tsfi_quantel_paintbox_custom_brush(pixels, w, h, cx, cy, brush_tex, brush_w, brush_h, opacity, color);
    }
    return 0;
}

int tsfi_quantel_paintbox_velocity_brush(uint32_t *pixels, int w, int h, int prev_x, int prev_y, int curr_x, int curr_y, float max_radius, float pressure, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0) return -1;

    float dx = curr_x - prev_x;
    float dy = curr_y - prev_y;
    float dist = sqrtf(dx * dx + dy * dy);

    float speed_factor = dist > 1.0f ? (1.0f / (1.0f + 0.05f * dist)) : 1.0f;
    float active_radius = max_radius * speed_factor;
    if (active_radius < 1.0f) active_radius = 1.0f;

    int steps = (int)(dist + 1.0f);
    for (int i = 0; i <= steps; i++) {
        float t = (float)i / steps;
        int cx = (int)(prev_x + dx * t);
        int cy = (int)(prev_y + dy * t);
        tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, (int)active_radius, pressure, color);
    }
    return 0;
}

int tsfi_quantel_paintbox_wet_paint(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float smudge_rate, uint32_t *brush_color_in_out) {
    if (!pixels || !brush_color_in_out || w <= 0 || h <= 0 || radius <= 0) return -1;

    double sum_r = 0, sum_g = 0, sum_b = 0;
    int count = 0;

    for (int y = cy - radius; y <= cy + radius; y++) {
        if (y < 0 || y >= h) continue;
        uint32_t *row = pixels + y * w;
        int dy = y - cy;
        for (int x = cx - radius; x <= cx + radius; x++) {
            if (x < 0 || x >= w) continue;
            int dx = x - cx;
            if (dx * dx + dy * dy <= radius * radius) {
                uint32_t pix = row[x];
                sum_r += (pix >> 16) & 0xFF;
                sum_g += (pix >> 8) & 0xFF;
                sum_b += pix & 0xFF;
                count++;
            }
        }
    }

    if (count > 0) {
        uint8_t avg_r = (uint8_t)(sum_r / count);
        uint8_t avg_g = (uint8_t)(sum_g / count);
        uint8_t avg_b = (uint8_t)(sum_b / count);

        uint32_t br_col = *brush_color_in_out;
        uint8_t br_r = (br_col >> 16) & 0xFF;
        uint8_t br_g = (br_col >> 8) & 0xFF;
        uint8_t br_b = br_col & 0xFF;

        uint8_t res_r = (uint8_t)(br_r * (1.0f - smudge_rate) + avg_r * smudge_rate);
        uint8_t res_g = (uint8_t)(br_g * (1.0f - smudge_rate) + avg_g * smudge_rate);
        uint8_t res_b = (uint8_t)(br_b * (1.0f - smudge_rate) + avg_b * smudge_rate);

        *brush_color_in_out = (0xFF000000) | (res_r << 16) | (res_g << 8) | res_b;
    }

    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, radius, 1.0f, *brush_color_in_out);
}

int tsfi_quantel_paintbox_tablet_brush(uint32_t *pixels, int w, int h, int cx, int cy, int max_radius, float pressure, float aspect_ratio, float angle, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || max_radius <= 0) return -1;

    uint8_t r_src = (color >> 16) & 0xFF;
    uint8_t g_src = (color >> 8) & 0xFF;
    uint8_t b_src = color & 0xFF;

    float active_r = max_radius * pressure;
    if (active_r < 1.0f) active_r = 1.0f;
    float cos_a = cosf(angle);
    float sin_a = sinf(angle);

    int bounds = (int)(active_r + 1.0f);
    for (int y = cy - bounds; y <= cy + bounds; y++) {
        if (y < 0 || y >= h) continue;
        uint32_t *row = pixels + y * w;
        int dy = y - cy;

        for (int x = cx - bounds; x <= cx + bounds; x++) {
            if (x < 0 || x >= w) continue;
            int dx = x - cx;

            float rx = dx * cos_a + dy * sin_a;
            float ry = -dx * sin_a + dy * cos_a;

            float term_x = rx / active_r;
            float term_y = ry / (active_r * aspect_ratio);

            float ellipse = term_x * term_x + term_y * term_y;
            if (ellipse <= 1.0f) {
                float intensity = (1.0f - ellipse) * pressure;
                if (intensity > 1.0f) intensity = 1.0f;
                if (intensity < 0.0f) intensity = 0.0f;

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
    return 0;
}

int tsfi_quantel_paintbox_stylus_shear(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float tilt_x, float tilt_y, float rotation, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;

    uint8_t r_src = (color >> 16) & 0xFF;
    uint8_t g_src = (color >> 8) & 0xFF;
    uint8_t b_src = color & 0xFF;

    float cos_r = cosf(rotation);
    float sin_r = sinf(rotation);

    int bounds = radius * 2;
    for (int y = cy - bounds; y <= cy + bounds; y++) {
        if (y < 0 || y >= h) continue;
        uint32_t *row = pixels + y * w;
        int dy = y - cy;

        for (int x = cx - bounds; x <= cx + bounds; x++) {
            if (x < 0 || x >= w) continue;
            int dx = x - cx;

            float sx = dx + tilt_x * dy;
            float sy = dy + tilt_y * dx;

            float rx = sx * cos_r + sy * sin_r;
            float ry = -sx * sin_r + sy * cos_r;

            float dist_sq = rx * rx + ry * ry;
            if (dist_sq <= radius * radius) {
                float intensity = 1.0f - sqrtf(dist_sq) / radius;
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
    return 0;
}

int tsfi_quantel_paintbox_filter_jitter(int raw_x, int raw_y, int prev_x, int prev_y, float alpha, int *smooth_x, int *smooth_y) {
    if (!smooth_x || !smooth_y) return -1;
    *smooth_x = (int)(alpha * raw_x + (1.0f - alpha) * prev_x);
    *smooth_y = (int)(alpha * raw_y + (1.0f - alpha) * prev_y);
    return 0;
}

extern void tsfi_rgb_to_hsl(uint8_t r, uint8_t g, uint8_t b, float *h, float *s, float *l);
extern void tsfi_hsl_to_rgb(float h, float s, float l, uint8_t *r, uint8_t *g, uint8_t *b);

int tsfi_quantel_paintbox_pressure_color_mod(uint32_t base_color, float pressure, float hue_drift, float sat_drift, uint32_t *out_color) {
    if (!out_color) return -1;

    uint8_t r = (base_color >> 16) & 0xFF;
    uint8_t g = (base_color >> 8) & 0xFF;
    uint8_t b = base_color & 0xFF;

    float h, s, l;
    tsfi_rgb_to_hsl(r, g, b, &h, &s, &l);

    h += hue_drift * pressure;
    if (h > 1.0f) h -= 1.0f;
    if (h < 0.0f) h += 1.0f;

    s += sat_drift * pressure;
    if (s > 1.0f) s = 1.0f;
    if (s < 0.0f) s = 0.0f;

    uint8_t out_r, out_g, out_b;
    tsfi_hsl_to_rgb(h, s, l, &out_r, &out_g, &out_b);

    *out_color = (0xFF000000) | (out_r << 16) | (out_g << 8) | out_b;
    return 0;
}

int tsfi_quantel_paintbox_bezier_stroke(uint32_t *pixels, int w, int h, float p0_x, float p0_y, float p1_x, float p1_y, float p2_x, float p2_y, float p3_x, float p3_y, int radius, float pressure, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;

    float d1_x = p1_x - p0_x; float d1_y = p1_y - p0_y;
    float d2_x = p2_x - p1_x; float d2_y = p2_y - p1_y;
    float d3_x = p3_x - p2_x; float d3_y = p3_y - p2_y;
    float total_dist = sqrtf(d1_x * d1_x + d1_y * d1_y) + sqrtf(d2_x * d2_x + d2_y * d2_y) + sqrtf(d3_x * d3_x + d3_y * d3_y);

    int steps = (int)(total_dist / 2.0f);
    if (steps < 4) steps = 4;

    for (int i = 0; i <= steps; i++) {
        float t = (float)i / steps;
        float omt = 1.0f - t;

        float cx = omt * omt * omt * p0_x +
                   3.0f * omt * omt * t * p1_x +
                   3.0f * omt * t * t * p2_x +
                   t * t * t * p3_x;

        float cy = omt * omt * omt * p0_y +
                   3.0f * omt * omt * t * p1_y +
                   3.0f * omt * t * t * p2_y +
                   t * t * t * p3_y;

        tsfi_quantel_paintbox_airbrush(pixels, w, h, (int)cx, (int)cy, radius, pressure, color);
    }
    return 0;
}

int tsfi_quantel_paintbox_texture_reveal(uint32_t *pixels, int w, int h, int cx, int cy, int radius, const uint8_t *paper_tex, int tex_w, int tex_h, float pressure, uint32_t color) {
    if (!pixels || !paper_tex || w <= 0 || h <= 0 || tex_w <= 0 || tex_h <= 0 || radius <= 0) return -1;

    uint8_t r_src = (color >> 16) & 0xFF;
    uint8_t g_src = (color >> 8) & 0xFF;
    uint8_t b_src = color & 0xFF;

    for (int y = cy - radius; y <= cy + radius; y++) {
        if (y < 0 || y >= h) continue;
        uint32_t *row = pixels + y * w;
        int dy = y - cy;

        int tex_y = y % tex_h;
        if (tex_y < 0) tex_y += tex_h;
        const uint8_t *tex_row = paper_tex + tex_y * tex_w;

        for (int x = cx - radius; x <= cx + radius; x++) {
            if (x < 0 || x >= w) continue;
            int dx = x - cx;

            if (dx * dx + dy * dy <= radius * radius) {
                int tex_x = x % tex_w;
                if (tex_x < 0) tex_x += tex_w;

                float texture_grain = tex_row[tex_x] / 255.0f;
                float intensity = (1.0f - pressure) * texture_grain + pressure;
                if (intensity > 1.0f) intensity = 1.0f;
                if (intensity < 0.0f) intensity = 0.0f;

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
    return 0;
}

int tsfi_quantel_paintbox_palette_cycle(uint32_t *pixels, int w, int h, int shift_amount) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    for (int i = 0; i < w * h; i++) {
        uint32_t p = pixels[i];
        if ((p & 0xFFFFFF) == 0) continue;
        uint8_t a = (p >> 24) & 0xFF;
        uint8_t r = (p >> 16) & 0xFF;
        uint8_t g = (p >> 8) & 0xFF;
        uint8_t b = p & 0xFF;
        int total_shift = shift_amount % 3;
        if (total_shift == 1) {
            uint8_t tmp = r; r = g; g = b; b = tmp;
        } else if (total_shift == 2) {
            uint8_t tmp = r; r = b; b = g; g = tmp;
        }
        pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }
    return 0;
}

extern void draw_char(uint32_t *pixels, int w, int h, int rx, int ry, unsigned char c, uint32_t color, int scale);

int tsfi_quantel_paintbox_typographer(uint32_t *pixels, int w, int h, int x, int y, const char *text, uint32_t color, float size) {
    if (!pixels || !text || w <= 0 || h <= 0) return -1;
    int cur_x = x;
    int scale = (int)(size / 8.0f);
    if (scale < 1) scale = 1;
    while (*text) {
        draw_char(pixels, w, h, cur_x, y, (unsigned char)*text, color, scale);
        cur_x += 8 * scale;
        text++;
    }
    return 0;
}

int tsfi_quantel_paintbox_flow_brush(uint32_t *pixels, int w, int h, int prev_x, int prev_y, int curr_x, int curr_y, int radius, float pressure, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0) return -1;
    float dx = (float)(curr_x - prev_x);
    float dy = (float)(curr_y - prev_y);
    float speed = sqrtf(dx*dx + dy*dy);
    float flow = 1.0f / (1.0f + 0.05f * speed);
    float final_opacity = flow * pressure;
    if (final_opacity > 1.0f) final_opacity = 1.0f;
    if (final_opacity < 0.05f) final_opacity = 0.05f;

    int steps = (int)(speed + 1.0f);
    for (int step = 0; step <= steps; step++) {
        float t = (float)step / steps;
        int cx = (int)(prev_x + dx * t);
        int cy = (int)(prev_y + dy * t);

        uint8_t r_src = (color >> 16) & 0xFF;
        uint8_t g_src = (color >> 8) & 0xFF;
        uint8_t b_src = color & 0xFF;

        for (int ry = -radius; ry <= radius; ry++) {
            int py = cy + ry;
            if (py < 0 || py >= h) continue;
            for (int rx = -radius; rx <= radius; rx++) {
                int px = cx + rx;
                if (px < 0 || px >= w) continue;
                if (rx*rx + ry*ry <= radius*radius) {
                    int idx = py * w + px;
                    uint32_t existing = pixels[idx];
                    uint8_t r_dst = (existing >> 16) & 0xFF;
                    uint8_t g_dst = (existing >> 8) & 0xFF;
                    uint8_t b_dst = existing & 0xFF;

                    uint8_t r_res = (uint8_t)(r_src * final_opacity + r_dst * (1.0f - final_opacity));
                    uint8_t g_res = (uint8_t)(g_src * final_opacity + g_dst * (1.0f - final_opacity));
                    uint8_t b_res = (uint8_t)(b_src * final_opacity + b_dst * (1.0f - final_opacity));

                    pixels[idx] = (0xFF000000) | (r_res << 16) | (g_res << 8) | b_res;
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_paintbox_pressure_jitter(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amount, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    float noise_r = ((float)rand() / RAND_MAX - 0.5f) * jitter_amount * radius * (1.0f - pressure);
    float noise_x = ((float)rand() / RAND_MAX - 0.5f) * jitter_amount * radius;
    float noise_y = ((float)rand() / RAND_MAX - 0.5f) * jitter_amount * radius;

    int active_radius = radius + (int)noise_r;
    if (active_radius < 1) active_radius = 1;

    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx + (int)noise_x, cy + (int)noise_y, active_radius, pressure, color);
}

int tsfi_quantel_paintbox_palette_mixer(uint32_t *mixer_pixels, int w, int h, int x, int y, uint32_t brush_color, float mix_rate) {
    if (!mixer_pixels || w <= 0 || h <= 0 || x < 0 || x >= w || y < 0 || y >= h) return -1;
    int idx = y * w + x;
    uint32_t existing = mixer_pixels[idx];

    uint8_t er = (existing >> 16) & 0xFF;
    uint8_t eg = (existing >> 8) & 0xFF;
    uint8_t eb = existing & 0xFF;

    uint8_t br = (brush_color >> 16) & 0xFF;
    uint8_t bg = (brush_color >> 8) & 0xFF;
    uint8_t bb = brush_color & 0xFF;

    uint8_t res_r = (uint8_t)(br * mix_rate + er * (1.0f - mix_rate));
    uint8_t res_g = (uint8_t)(bg * mix_rate + eg * (1.0f - mix_rate));
    uint8_t res_b = (uint8_t)(bb * mix_rate + eb * (1.0f - mix_rate));

    mixer_pixels[idx] = (0xFF000000) | (res_r << 16) | (res_g << 8) | res_b;
    return 0;
}

int tsfi_quantel_paintbox_profile_brush(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float softness, uint32_t color) {
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
                float norm_dist = dist / radius;
                float intensity = 0.0f;
                if (norm_dist > (1.0f - softness)) {
                    float factor = (norm_dist - (1.0f - softness)) / softness;
                    intensity = (1.0f - factor) * pressure;
                } else {
                    intensity = pressure;
                }
                
                if (intensity > 1.0f) intensity = 1.0f;
                if (intensity < 0.0f) intensity = 0.0f;

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
    return 0;
}

int tsfi_quantel_paintbox_shear_jitter(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amount, float base_rotation, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    (void)pressure;
    float jitter_rot = base_rotation + ((float)rand() / RAND_MAX - 0.5f) * jitter_amount;
    float tilt_x = ((float)rand() / RAND_MAX - 0.5f) * jitter_amount;
    float tilt_y = ((float)rand() / RAND_MAX - 0.5f) * jitter_amount;

    return tsfi_quantel_paintbox_stylus_shear(pixels, w, h, cx, cy, radius, tilt_x, tilt_y, jitter_rot, color);
}

int tsfi_quantel_paintbox_saturation_buildup(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float dwell_time, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    float accum_pressure = pressure * (1.0f + 0.1f * dwell_time);
    if (accum_pressure > 1.0f) accum_pressure = 1.0f;
    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, radius, accum_pressure, color);
}

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
        float r_offset = ((float)rand() / RAND_MAX) * radius * 0.7f;
        int bx = cx + (int)(r_offset * cosf(angle));
        int by = cy + (int)(r_offset * sinf(angle));
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

