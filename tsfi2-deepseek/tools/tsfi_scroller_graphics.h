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

static void draw_gothic_leaf(int bx, int by, float length, float angle_offset, float time_val, float wind_x) {
    int steps = 18;
    float wind_bend = wind_x * 0.45f;
    float l_ctrl_x = bx + length * 0.45f * cosf(angle_offset - 0.4f) + wind_bend;
    float l_ctrl_y = by + length * 0.45f * sinf(angle_offset - 0.4f);
    float tip_flutter = 0.04f * sinf(time_val * 4.5f + angle_offset);
    float l_tip_x = bx + length * cosf(angle_offset + 0.10f * sinf(time_val * 1.5f) + tip_flutter) + wind_bend * 0.7f;
    float l_tip_y = by + length * sinf(angle_offset + 0.10f * sinf(time_val * 1.5f) + tip_flutter);

    for (int i = 0; i < steps; i++) {
        float t = (float)i / (float)steps;
        float omt = 1.0f - t;
        
        // Quadratic Bezier interpolation for the leaf spine
        int lx = (int)(omt * omt * bx + 2.0f * omt * t * l_ctrl_x + t * t * l_tip_x);
        int ly = (int)(omt * omt * by + 2.0f * omt * t * l_ctrl_y + t * t * l_tip_y);

        // Angle of the tangent vector along the spine
        float angle = atan2f((float)(ly - by), (float)(lx - bx));
        
        uint8_t lf_r = (uint8_t)(55 * (1.0f - t) + 80 * t);
        uint8_t lf_g = (uint8_t)(135 * (1.0f - t) + 225 * t);
        uint8_t lf_b = (uint8_t)(45 * (1.0f - t) + 95 * t);

        // Serrated pinnate leaflets projecting forward (at 45 degree angles)
        float lobe_w = 17.0f * (1.0f - t) * (0.8f + 0.5f * sinf(t * 28.0f));
        float leaf_ang_left = angle + M_PI * 0.25f;
        float leaf_ang_right = angle - M_PI * 0.25f;

        // High-frequency serration along the leaflet margins for organic texture
        float serration = 1.0f + 0.08f * sinf(t * 50.0f + time_val * 6.0f);
        
        for (int lw = 0; lw < (int)lobe_w; lw++) {
            int l_lx = lx + (int)(lw * serration * cosf(leaf_ang_left));
            int l_ly = ly + (int)(lw * serration * sinf(leaf_ang_left));
            int r_lx = lx + (int)(lw * serration * cosf(leaf_ang_right));
            int r_ly = ly + (int)(lw * serration * sinf(leaf_ang_right));
            draw_line(lx, ly, l_lx, l_ly, lf_r, lf_g, lf_b);
            draw_line(lx, ly, r_lx, r_ly, lf_r, lf_g, lf_b);
        }

        // Draw bright secondary highlight veins down the center of each leaflet
        int mid_lx = lx + (int)((lobe_w * 0.45f) * cosf(leaf_ang_left));
        int mid_ly = ly + (int)((lobe_w * 0.45f) * sinf(leaf_ang_left));
        int mid_rx = lx + (int)((lobe_w * 0.45f) * cosf(leaf_ang_right));
        int mid_ry = ly + (int)((lobe_w * 0.45f) * sinf(leaf_ang_right));
        draw_line(lx, ly, mid_lx, mid_ly, (uint8_t)fminf(255, lf_r * 1.25f), (uint8_t)fminf(255, lf_g * 1.2f), (uint8_t)fminf(255, lf_b * 1.25f));
        draw_line(lx, ly, mid_rx, mid_ry, (uint8_t)fminf(255, lf_r * 1.25f), (uint8_t)fminf(255, lf_g * 1.2f), (uint8_t)fminf(255, lf_b * 1.25f));

        // Detailed pinnate vein ribs branching out
        if (i % 3 == 0) {
            int v_l_x = lx + (int)((lobe_w * 0.7f) * cosf(leaf_ang_left));
            int v_l_y = ly + (int)((lobe_w * 0.7f) * sinf(leaf_ang_left));
            int v_r_x = lx + (int)((lobe_w * 0.7f) * cosf(leaf_ang_right));
            int v_r_y = ly + (int)((lobe_w * 0.7f) * sinf(leaf_ang_right));
            draw_line(lx, ly, v_l_x, v_l_y, (uint8_t)(lf_r * 1.5f), (uint8_t)(lf_g * 1.3f), (uint8_t)(lf_b * 1.4f));
            draw_line(lx, ly, v_r_x, v_r_y, (uint8_t)(lf_r * 1.5f), (uint8_t)(lf_g * 1.3f), (uint8_t)(lf_b * 1.4f));
        }

        // Glistening dew drops sliding slowly along the leaflet tips (pinnule runoff)
        if (i == 6 || i == 12) {
            float slide_t = fmodf(time_val * 0.12f + (i * 0.08f), 1.0f);
            int drop_x = lx + (int)(lobe_w * slide_t * cosf(leaf_ang_left));
            int drop_y = ly + (int)(lobe_w * slide_t * sinf(leaf_ang_left));
            draw_glossy_bubble(drop_x, drop_y, 2, 210, 255, 220); // Small shiny dew bubble
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
