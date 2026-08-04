#ifndef TSFI_SCROLLER_GRAPHICS_H
#define TSFI_SCROLLER_GRAPHICS_H

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#define WIDTH 1280
#define HEIGHT 692

extern uint8_t frame_buffer[WIDTH * HEIGHT * 3];

// Custom Vaesen gothic symbols
static const uint8_t symbol_leaf[8] = { 0x08, 0x1c, 0x38, 0x70, 0xe0, 0x60, 0x30, 0x08 };
static const uint8_t symbol_fleur[8] = { 0x18, 0x5a, 0x7e, 0x3c, 0x7e, 0xdb, 0x18, 0x3c };
static const uint8_t symbol_rune[8] = { 0x18, 0x24, 0x42, 0x24, 0x18, 0x24, 0x42, 0x00 };
static const uint8_t symbol_skull[8] = { 0x3c, 0x7e, 0xa5, 0xff, 0x66, 0x3c, 0x2a, 0x00 };

static float bubble_diffuse[16][32][32];
static float bubble_specular[16][32][32];

static void precompute_bubbles() {
    for (int r = 1; r < 16; r++) {
        for (int dy = -r; dy <= r; dy++) {
            for (int dx = -r; dx <= r; dx++) {
                int dist_sq = dx * dx + dy * dy;
                if (dist_sq <= r * r) {
                    float nx = (float)dx / r;
                    float ny = (float)dy / r;
                    float nz = sqrtf(1.0f - nx * nx - ny * ny);

                    float diff = nx * -0.57f + ny * -0.57f + nz * 0.57f;
                    if (diff < 0.0f) diff = 0.0f;
                    bubble_diffuse[r][dy + r][dx + r] = 0.3f + 0.7f * diff;

                    float spec = 0.0f;
                    float hx = nx + 0.35f;
                    float hy = ny + 0.35f;
                    float h_dist = hx * hx + hy * hy;
                    if (h_dist < 0.15f) {
                        spec = powf(1.0f - (h_dist / 0.15f), 6.0f) * 220.0f;
                    }
                    bubble_specular[r][dy + r][dx + r] = spec;
                }
            }
        }
    }
}

static void draw_line(int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    while (1) {
        if (x0 >= 0 && x0 < WIDTH && y0 >= 0 && y0 < HEIGHT) {
            int idx = (y0 * WIDTH + x0) * 3;
            frame_buffer[idx] = (uint8_t)fminf(255, frame_buffer[idx] + r);
            frame_buffer[idx + 1] = (uint8_t)fminf(255, frame_buffer[idx + 1] + g);
            frame_buffer[idx + 2] = (uint8_t)fminf(255, frame_buffer[idx + 2] + b);
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static void draw_panel(int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b) {
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                int idx = (y * WIDTH + x) * 3;
                frame_buffer[idx] = r;
                frame_buffer[idx+1] = g;
                frame_buffer[idx+2] = b;
            }
        }
    }
}

static void draw_glossy_bubble(int cx, int cy, int r_base, uint8_t base_r, uint8_t base_g, uint8_t base_b) {
    int r = r_base;
    if (r >= 16) r = 15;
    if (r < 1) r = 1;
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            int dist_sq = dx * dx + dy * dy;
            if (dist_sq <= r * r) {
                int px = cx + dx;
                int py = cy + dy;
                if (px >= 45 && px < WIDTH && py >= 0 && py < HEIGHT) {
                    float diff_factor = bubble_diffuse[r][dy + r][dx + r];
                    float spec = bubble_specular[r][dy + r][dx + r];

                    int idx = (py * WIDTH + px) * 3;
                    
                    uint8_t pr = (uint8_t)fminf(255, base_r * diff_factor + spec);
                    uint8_t pg = (uint8_t)fminf(255, base_g * diff_factor + spec);
                    uint8_t pb = (uint8_t)fminf(255, base_b * diff_factor + spec);

                    frame_buffer[idx] = (uint8_t)fminf(255, frame_buffer[idx] + pr);
                    frame_buffer[idx + 1] = (uint8_t)fminf(255, frame_buffer[idx + 1] + pg);
                    frame_buffer[idx + 2] = (uint8_t)fminf(255, frame_buffer[idx + 2] + pb);
                }
            }
        }
    }
}

static void draw_gothic_leaf(int bx, int by, float length, float angle_offset) {
    int steps = 18;
    for (int i = 0; i < steps; i++) {
        float t = (float)i / (float)steps;
        float r_len = length * t;
        float angle = angle_offset + 0.15f * sinf(t * 8.0f);
        int lx = bx + (int)(r_len * cosf(angle));
        int ly = by + (int)(r_len * sinf(angle));
        
        uint8_t lf_r = (uint8_t)(34 * (1.0f - t) + 46 * t);
        uint8_t lf_g = (uint8_t)(75 * (1.0f - t) + 165 * t);
        uint8_t lf_b = (uint8_t)(30 * (1.0f - t) + 50 * t);

        float lobe_w = 17.0f * (1.0f - t) * (0.8f + 0.5f * sinf(t * 28.0f));
        for (int lw = 0; lw < (int)lobe_w; lw++) {
            float perp_a = angle + M_PI / 2.0f;
            int l_lx = lx + (int)(lw * cosf(perp_a));
            int l_ly = ly + (int)(lw * sinf(perp_a));
            int r_lx = lx - (int)(lw * cosf(perp_a));
            int r_ly = ly - (int)(lw * sinf(perp_a));
            draw_line(l_lx, l_ly, r_lx, r_ly, lf_r, lf_g, lf_b);
        }

        if (i % 3 == 0) {
            float perp_a = angle + M_PI / 2.0f;
            int v_l_x = lx + (int)((lobe_w * 0.7f) * cosf(perp_a));
            int v_l_y = ly + (int)((lobe_w * 0.7f) * sinf(perp_a));
            int v_r_x = lx - (int)((lobe_w * 0.7f) * cosf(perp_a));
            int v_r_y = ly - (int)((lobe_w * 0.7f) * sinf(perp_a));
            draw_line(lx, ly, v_l_x, v_l_y, (uint8_t)(lf_r * 1.5f), (uint8_t)(lf_g * 1.3f), (uint8_t)(lf_b * 1.4f));
            draw_line(lx, ly, v_r_x, v_r_y, (uint8_t)(lf_r * 1.5f), (uint8_t)(lf_g * 1.3f), (uint8_t)(lf_b * 1.4f));
        }

        draw_line(lx, ly, lx, ly, 100, 190, 105);
    }
}

static void draw_flat_petal(int cx, int cy, float radius, uint8_t r, uint8_t g, uint8_t b, int cx_center, int cy_center, float time_val) {
    int rad_int = (int)radius + 4;
    for (int y = -rad_int; y <= rad_int; y++) {
        for (int x = -rad_int; x <= rad_int; x++) {
            float dist = sqrtf(x*x + y*y);
            if (dist > 0.0f) {
                float angle = atan2f(y, x);
                float ripple = 3.5f * sinf(angle * 5.0f);
                if (dist <= radius + ripple) {
                    int px = cx + x;
                    int py = cy + y;
                    if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                        int idx = (py * WIDTH + px) * 3;
                        float factor = 0.4f + 0.6f * (1.0f - (dist / (radius + ripple)));
                        if (factor < 0.2f) factor = 0.2f;

                        uint8_t pr = (uint8_t)(r * factor);
                        uint8_t pg = (uint8_t)(g * factor);
                        uint8_t pb = (uint8_t)(b * factor);

                        float dist_from_center = sqrtf((px - cx_center)*(px - cx_center) + (py - cy_center)*(py - cy_center));
                        if (dist_from_center < 22.0f) {
                            float blotch_factor = (dist_from_center - 10.0f) / 12.0f;
                            if (blotch_factor < 0.0f) blotch_factor = 0.0f;
                            pr = (uint8_t)(pr * blotch_factor + 12 * (1.0f - blotch_factor));
                            pg = (uint8_t)(pg * blotch_factor + 8 * (1.0f - blotch_factor));
                            pb = (uint8_t)(pb * blotch_factor + 16 * (1.0f - blotch_factor));
                        }

                        frame_buffer[idx] = pr;
                        frame_buffer[idx+1] = pg;
                        frame_buffer[idx+2] = pb;
                    }
                }
            }
        }
    }

    for (int v = 0; v < 12; v++) {
        float v_ang = v * (2.0f * M_PI / 12.0f) + 0.08f * sinf(time_val * 2.0f + v);
        for (float step = 2.0f; step < radius; step += 2.5f) {
            int vx = cx + (int)(step * cosf(v_ang));
            int vy = cy + (int)(step * sinf(v_ang));
            if (vx >= 0 && vx < WIDTH && vy >= 0 && vy < HEIGHT) {
                int idx = (vy * WIDTH + vx) * 3;
                frame_buffer[idx] = (uint8_t)(frame_buffer[idx] * 0.82f + r * 0.13f);
                frame_buffer[idx+1] = (uint8_t)(frame_buffer[idx+1] * 0.82f + g * 0.13f);
                frame_buffer[idx+2] = (uint8_t)(frame_buffer[idx+2] * 0.82f + b * 0.13f);
            }
        }
    }
}

#endif // TSFI_SCROLLER_GRAPHICS_H
