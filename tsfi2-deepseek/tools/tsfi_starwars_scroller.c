#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define WIDTH 1280
#define HEIGHT 692
#define FPS 24

static uint8_t frame_buffer[WIDTH * HEIGHT * 3];

#include "font8x8.h"

// Custom Vaesen gothic symbols
static const uint8_t symbol_leaf[8] = { 0x08, 0x1c, 0x38, 0x70, 0xe0, 0x60, 0x30, 0x08 };
static const uint8_t symbol_fleur[8] = { 0x18, 0x5a, 0x7e, 0x3c, 0x7e, 0xdb, 0x18, 0x3c };
static const uint8_t symbol_rune[8] = { 0x18, 0x24, 0x42, 0x24, 0x18, 0x24, 0x42, 0x00 };
static const uint8_t symbol_skull[8] = { 0x3c, 0x7e, 0xa5, 0xff, 0x66, 0x3c, 0x2a, 0x00 };

static void draw_char_direct(int x, int y, char c, int scale, uint8_t r, uint8_t g, uint8_t b);

typedef struct {
    char helix_text[16];
    int color_scheme;
    int shape_type;
    float speed_scale;
    int bear_count;
} TSFiScrollerLiveConfig;

static char live_helix_text[16] = "TSFi/2";
static float live_speed_scale = 1.0f;
static int live_bear_count = 3;

typedef struct {
    float x, y, z;
    float speed;
} Star;

#define MAX_STARS 120
static Star stars[MAX_STARS];


typedef struct {
    float x, y;
    float speed;
    float size_scale;
} TeddyBear;

#define MAX_BEARS 100
static TeddyBear bears[MAX_BEARS];


// Biotika Spore Particle structure
typedef struct {
    float x, y, z;
    float vx, vy, vz;
    float age;
    float max_age;
} BioSpore;

#define MAX_SPORES 16
static BioSpore spores[MAX_SPORES];
static bool spores_initialized = false;

static void init_stars_and_bears() {
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].x = (rand() % 500) - 250;
        stars[i].y = (rand() % HEIGHT) - HEIGHT / 2;
        stars[i].z = (rand() % 1000) + 1;
        stars[i].speed = (rand() % 4) + 1.5f;
    }

    for (int i = 0; i < MAX_BEARS; i++) {
        bears[i].x = 60 + rand() % 380;
        bears[i].y = HEIGHT + 100 + i * 90;
        bears[i].speed = 0.6f + (rand() % 100) / 120.0f;
        bears[i].size_scale = 0.3f + (rand() % 100) / 300.0f;
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

static void update_and_draw_stars(bool hyperspace) {
    for (int i = 0; i < MAX_STARS; i++) {
        float speed = stars[i].speed * live_speed_scale * (hyperspace ? 20.0f : 1.0f);

        float prev_z = stars[i].z;
        stars[i].z -= speed;
        if (stars[i].z <= 0) {
            stars[i].x = (rand() % 500) - 250;
            stars[i].y = (rand() % HEIGHT) - HEIGHT / 2;
            stars[i].z = 1000;
            prev_z = 1000;
        }

        float k = 350.0f / stars[i].z;
        int px = 240 + (int)(stars[i].x * k);
        int py = (HEIGHT / 2) + (int)(stars[i].y * k);

        if (px >= 45 && px < 490) {
            if (hyperspace) {
                float pk = 350.0f / prev_z;
                int ppx = 240 + (int)(stars[i].x * pk);
                int ppy = (HEIGHT / 2) + (int)(stars[i].y * pk);
                if (ppx >= 45 && ppx < 490) {
                    uint8_t brightness = (uint8_t)((1.0f - (stars[i].z / 1000.0f)) * 255);
                    draw_line(ppx, ppy, px, py, brightness, brightness, brightness);
                }
            } else {
                if (py >= 0 && py < HEIGHT) {
                    int idx = (py * WIDTH + px) * 3;
                    uint8_t val = (uint8_t)((1.0f - (stars[i].z / 1000.0f)) * 200);
                    frame_buffer[idx] = (uint8_t)fminf(255, frame_buffer[idx] + val);
                    frame_buffer[idx + 1] = (uint8_t)fminf(255, frame_buffer[idx + 1] + val);
                    frame_buffer[idx + 2] = (uint8_t)fminf(255, frame_buffer[idx + 2] + val);
                }
            }
        }
    }
}

static float get_beat_pulse(float t_sec, float tempo_bpm) {
    float step_duration = 60.0f / tempo_bpm / 4.0f;
    float pattern_duration = step_duration * 16.0f;

    int pattern_idx = (int)(t_sec / pattern_duration) % 8;
    int step_idx = (int)(fmodf(t_sec, pattern_duration) / step_duration) % 16;
    float step_age = fmodf(t_sec, step_duration);
    
    int pattern_type = 0;
    if (pattern_idx == 0) pattern_type = 0;
    else if (pattern_idx == 1 || pattern_idx == 4) pattern_type = 1;
    else if (pattern_idx == 2 || pattern_idx == 5) pattern_type = 2;
    else if (pattern_idx == 3 || pattern_idx == 6) pattern_type = 3;
    else pattern_type = 4;
    
    static const int kick_patterns[5][16] = {
        {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
        {1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0},
        {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    
    if (kick_patterns[pattern_type][step_idx] == 1) {
        return expf(-step_age / 0.08f);
    }
    return 0.0f;
}

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

static void draw_vector_teddy_bear(int cx, int cy, float size_scale, float pulse, int color_scheme) {
    float final_scale = size_scale * (1.0f + pulse * 0.16f);
    
    uint8_t r_col = 250, g_col = 250, b_col = 250;
    switch (color_scheme) {
        case 1: r_col = 240; g_col = 150; b_col = 240; break;
        case 2: r_col = 150; g_col = 250; b_col = 150; break;
        case 3: r_col = 250; g_col = 220; b_col = 150; break;
        case 4: r_col = 250; g_col = 100; b_col = 100; break;
        case 0:
        default: r_col = 150; g_col = 230; b_col = 250; break;
    }

    float r_head = 30.0f * final_scale;
    for (float a = 0; a < 2.0f * M_PI; a += 0.25f) {
        int bx = cx + (int)(r_head * cosf(a));
        int by = cy + (int)(r_head * sinf(a));
        draw_glossy_bubble(bx, by, 3, r_col, g_col, b_col);
    }

    float r_ear = 11.0f * final_scale;
    int ear_offset = (int)(24.0f * final_scale);
    for (float a = 0; a < 2.0f * M_PI; a += 0.4f) {
        int bx = cx - ear_offset + (int)(r_ear * cosf(a));
        int by = cy - ear_offset + (int)(r_ear * sinf(a));
        draw_glossy_bubble(bx, by, 2, r_col, g_col, b_col);
    }

    for (float a = 0; a < 2.0f * M_PI; a += 0.4f) {
        int bx = cx + ear_offset + (int)(r_ear * cosf(a));
        int by = cy - ear_offset + (int)(r_ear * sinf(a));
        draw_glossy_bubble(bx, by, 2, r_col, g_col, b_col);
    }

    int eye_x = (int)(10.0f * final_scale);
    int eye_y = (int)(5.0f * final_scale);
    draw_glossy_bubble(cx - eye_x, cy - eye_y, 2, 255, 255, 255);
    draw_glossy_bubble(cx + eye_x, cy - eye_y, 2, 255, 255, 255);

    float r_snout = 8.0f * final_scale;
    int snout_y = (int)(9.0f * final_scale);
    for (float a = 0; a < 2.0f * M_PI; a += 0.5f) {
        int bx = cx + (int)(r_snout * cosf(a));
        int by = cy + snout_y + (int)(r_snout * sinf(a));
        draw_glossy_bubble(bx, by, 2, (uint8_t)fmaxf(0, r_col - 40), (uint8_t)fmaxf(0, g_col - 40), b_col);
    }

    draw_glossy_bubble(cx, cy + snout_y - 1, 2, 255, 255, 255);
}

// 3D USD Spline Ribbon: Renders entwined Vaesen thorny/leafy vines, warped dynamically by SFT swap volume!
typedef struct {
    float x, y, z;
    float px, py, pz;
} ScrollerPoppyJoint;

static ScrollerPoppyJoint poppy_joints[7];
static bool poppy_initialized = false;

#define MAX_POLLEN 16
typedef struct {
    float x, y;
    float vx, vy;
    float life;
} PollenParticle;

static PollenParticle pollen[MAX_POLLEN];
static bool pollen_initialized = false;

static void draw_gothic_leaf(int bx, int by, float length, float angle_offset) {
    int steps = 18;
    for (int i = 0; i < steps; i++) {
        float t = (float)i / (float)steps;
        float r_len = length * t;
        float angle = angle_offset + 0.15f * sinf(t * 8.0f);
        int lx = bx + (int)(r_len * cosf(angle));
        int ly = by + (int)(r_len * sinf(angle));
        
        // Base connection shading (t=0 is brownish-green, t=1 is vibrant fresh green)
        uint8_t lf_r = (uint8_t)(34 * (1.0f - t) + 46 * t);
        uint8_t lf_g = (uint8_t)(75 * (1.0f - t) + 165 * t);
        uint8_t lf_b = (uint8_t)(30 * (1.0f - t) + 50 * t);

        // Expanded lobe width and increased frequency for jagged serrations
        float lobe_w = 17.0f * (1.0f - t) * (0.8f + 0.5f * sinf(t * 28.0f));
        for (int lw = 0; lw < (int)lobe_w; lw++) {
            float perp_a = angle + M_PI / 2.0f;
            int l_lx = lx + (int)(lw * cosf(perp_a));
            int l_ly = ly + (int)(lw * sinf(perp_a));
            int r_lx = lx - (int)(lw * cosf(perp_a));
            int r_ly = ly - (int)(lw * sinf(perp_a));
            draw_line(l_lx, l_ly, r_lx, r_ly, lf_r, lf_g, lf_b);
        }

        // Side veins radiating from spine to lobe margins for realism
        if (i % 3 == 0) {
            float perp_a = angle + M_PI / 2.0f;
            int v_l_x = lx + (int)((lobe_w * 0.7f) * cosf(perp_a));
            int v_l_y = ly + (int)((lobe_w * 0.7f) * sinf(perp_a));
            int v_r_x = lx - (int)((lobe_w * 0.7f) * cosf(perp_a));
            int v_r_y = ly - (int)((lobe_w * 0.7f) * sinf(perp_a));
            draw_line(lx, ly, v_l_x, v_l_y, (uint8_t)(lf_r * 1.5f), (uint8_t)(lf_g * 1.3f), (uint8_t)(lf_b * 1.4f));
            draw_line(lx, ly, v_r_x, v_r_y, (uint8_t)(lf_r * 1.5f), (uint8_t)(lf_g * 1.3f), (uint8_t)(lf_b * 1.4f));
        }

        // Draw light green center spine vein
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
                // Ruffled petal edges using sin ripple
                float ripple = 3.5f * sinf(angle * 5.0f);
                if (dist <= radius + ripple) {
                    int px = cx + x;
                    int py = cy + y;
                    if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                        int idx = (py * WIDTH + px) * 3;
                        // Velvet lighting: darker color towards the ruffled margins
                        float factor = 0.4f + 0.6f * (1.0f - (dist / (radius + ripple)));
                        if (factor < 0.2f) factor = 0.2f;

                        uint8_t pr = (uint8_t)(r * factor);
                        uint8_t pg = (uint8_t)(g * factor);
                        uint8_t pb = (uint8_t)(b * factor);

                        // Black/dark blotch near the botanical base of the petal
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

    // Draw 12 delicate crinkled radial vein lines radiating outwards from petal center
    for (int v = 0; v < 12; v++) {
        float v_ang = v * (2.0f * M_PI / 12.0f) + 0.08f * sinf(time_val * 2.0f + v);
        for (float step = 2.0f; step < radius; step += 2.5f) {
            int vx = cx + (int)(step * cosf(v_ang));
            int vy = cy + (int)(step * sinf(v_ang));
            if (vx >= 0 && vx < WIDTH && vy >= 0 && vy < HEIGHT) {
                // Subtle overlay shading
                int idx = (vy * WIDTH + vx) * 3;
                frame_buffer[idx] = (uint8_t)(frame_buffer[idx] * 0.82f + r * 0.13f);
                frame_buffer[idx+1] = (uint8_t)(frame_buffer[idx+1] * 0.82f + g * 0.13f);
                frame_buffer[idx+2] = (uint8_t)(frame_buffer[idx+2] * 0.82f + b * 0.13f);
            }
        }
    }
}

static void draw_usd_spline_ribbon(float time_val, float pulse, double usd_value, int color_scheme) {
    (void)color_scheme;

    // 1. Initialize Verlet joints: 1 central pod (0) + 6 radiating petals (1..6)
    if (!poppy_initialized) {
        poppy_joints[0] = (ScrollerPoppyJoint){ .x = 640.0f, .y = 300.0f, .z = 50.0f, .px = 640.0f, .py = 300.0f, .pz = 50.0f }; // central pod
        for (int i = 1; i <= 6; i++) {
            float ang = (i - 1) * (2.0f * M_PI / 6.0f);
            float px = 640.0f + 22.0f * cosf(ang);
            float py = 300.0f + 22.0f * sinf(ang);
            poppy_joints[i] = (ScrollerPoppyJoint){ .x = px, .y = py, .z = 50.0f, .px = px, .py = py, .pz = 50.0f };
        }
        poppy_initialized = true;
    }

    // 2. Sway the Central Pod gently and slowly in the wind (low frequency 0.6f)
    float stem_base_x = 640.0f;
    float stem_base_y = (float)HEIGHT - 80.0f;
    float wind_boost = (float)log10(usd_value + 1.0) * 0.8f;
    float wind_x = (5.0f + wind_boost) * sinf(time_val * 0.6f);

    poppy_joints[0].x = stem_base_x + wind_x * 2.5f;
    poppy_joints[0].y = stem_base_y - 250.0f; // Lift higher up on the screen

    // Petal rest length (bloom expansion) based on USD telemetry, beat pulse, and breathing cycle
    float breathe = 3.5f * sinf(time_val * 1.2f);
    float petal_rest_len = 24.0f + (float)fminf(18.0f, (float)log10(usd_value + 1.0) * 3.0f) + pulse * 4.0f + breathe;
    // Set high damping decay (0.55f) to prevent runaway gravity drift
    float decay = 0.55f;

    // 3. Update Petals using Verlet and pull tightly to their target radial angles around the Pod
    for (int i = 1; i <= 6; i++) {
        float tx = poppy_joints[i].x;
        float ty = poppy_joints[i].y;

        // Slow, gentle fluttering force
        float petal_wind_x = 1.5f * sinf(time_val * 1.5f + i) + wind_x * 0.05f;
        float petal_gravity_y = 0.15f;

        poppy_joints[i].x += (poppy_joints[i].x - poppy_joints[i].px) * decay + petal_wind_x * 0.05f;
        poppy_joints[i].y += (poppy_joints[i].y - poppy_joints[i].py) * decay + petal_gravity_y * 0.05f;

        poppy_joints[i].px = tx;
        poppy_joints[i].py = ty;

        // Angular targets: evenly spaced at 60 degree intervals, tilted based on stem deflection velocity
        float target_angle = (i - 1) * (2.0f * M_PI / 6.0f) + wind_x * 0.035f;
        float target_x = poppy_joints[0].x + petal_rest_len * cosf(target_angle);
        float target_y = poppy_joints[0].y + petal_rest_len * sinf(target_angle);

        // Strong stiffness (0.85f) to prevent petals from drifting away from the pod
        poppy_joints[i].x += (target_x - poppy_joints[i].x) * 0.85f;
        poppy_joints[i].y += (target_y - poppy_joints[i].y) * 0.85f;
    }

    // 4. Render the Verlet Poppy Flower components
    // Swaying feathery leaves
    float leaf_sway = 0.04f * sinf(time_val * 1.5f);
    draw_gothic_leaf((int)stem_base_x, (int)stem_base_y, 80.0f, -M_PI * 0.8f + leaf_sway);
    draw_gothic_leaf((int)stem_base_x, (int)stem_base_y, 80.0f, -M_PI * 0.2f - leaf_sway);

    // Curved Bezier Stem (forest green with darker shading)
    int segments = 24;
    int prev_x = (int)stem_base_x;
    int prev_y = (int)stem_base_y;
    float ctrl_x = (stem_base_x + poppy_joints[0].x) * 0.5f - wind_x * 2.0f;
    float ctrl_y = (stem_base_y + poppy_joints[0].y) * 0.5f;

    for (int j = 1; j <= segments; j++) {
        float t = (float)j / (float)segments;
        float omt = 1.0f - t;
        int sx = (int)(omt * omt * stem_base_x + 2.0f * omt * t * ctrl_x + t * t * poppy_joints[0].x);
        int sy = (int)(omt * omt * stem_base_y + 2.0f * omt * t * ctrl_y + t * t * poppy_joints[0].y);

        // Organic color shading: brown near the base to fresh green at the top
        uint8_t r_col = (uint8_t)(80 * (1.0f - t) + 34 * t);
        uint8_t g_col = (uint8_t)(60 * (1.0f - t) + 139 * t);
        uint8_t b_col = (uint8_t)(30 * (1.0f - t) + 34 * t);

        draw_line(prev_x, prev_y, sx, sy, r_col, g_col, b_col);
        draw_line(prev_x + 1, prev_y, sx + 1, sy, (uint8_t)(r_col * 0.7f), (uint8_t)(g_col * 0.7f), (uint8_t)(b_col * 0.7f)); // shadow line

        // Tiny structural bristles/hairs projecting perpendicular to the stem
        if (j % 2 == 0) {
            float perp_x = -(float)(sy - prev_y);
            float perp_y = (float)(sx - prev_x);
            float len = sqrtf(perp_x*perp_x + perp_y*perp_y);
            if (len > 0.0f) {
                perp_x /= len;
                perp_y /= len;
                int hx = sx + (int)(perp_x * 3.5f);
                int hy = sy + (int)(perp_y * 3.5f);
                draw_line(sx, sy, hx, hy, 80, 150, 90); // Muted green hair bristle
            }
        }

        prev_x = sx;
        prev_y = sy;
    }

    // Calyx / Receptacle (forest green bud cup at the bottom base of the flower head)
    draw_glossy_bubble((int)poppy_joints[0].x, (int)poppy_joints[0].y + 12, 10, 46, 125, 50);


    // Multilayered Petals: Outer layer (1, 3, 5) is larger & darker; Inner layer (2, 4, 6) is smaller & brighter
    for (int i = 1; i <= 6; i += 2) {
        draw_flat_petal((int)poppy_joints[i].x, (int)poppy_joints[i].y, 38.0f, 195, 15, 50, (int)poppy_joints[0].x, (int)poppy_joints[0].y, time_val); // Outer
    }
    for (int i = 2; i <= 6; i += 2) {
        draw_flat_petal((int)poppy_joints[i].x, (int)poppy_joints[i].y, 32.0f, 235, 30, 70, (int)poppy_joints[0].x, (int)poppy_joints[0].y, time_val); // Inner
    }


    // Draw stamen filaments radiating from the center pod (black stems with gold tips)
    float stamen_count = 16.0f;
    for (int s = 0; s < (int)stamen_count; s++) {
        float angle = (s / stamen_count) * 2.0f * M_PI;
        float stamen_len = 16.0f + 2.0f * sinf(time_val * 6.0f + s);
        int sx = (int)(poppy_joints[0].x + stamen_len * cosf(angle));
        int sy = (int)(poppy_joints[0].y + stamen_len * sinf(angle));
        draw_line((int)poppy_joints[0].x, (int)poppy_joints[0].y, sx, sy, 30, 30, 30);
        draw_glossy_bubble(sx, sy, 2, 255, 215, 0); // Gold tips
    }

    // Central Pod (Deep Black/Gold core, drawn on top)
    draw_glossy_bubble((int)poppy_joints[0].x, (int)poppy_joints[0].y, 14, 20, 20, 20);
    draw_glossy_bubble((int)poppy_joints[0].x - 2, (int)poppy_joints[0].y - 2, 4, 255, 215, 0);

    // Star-shaped Poppy seed crown (6 dark spikes on top of central pod)
    for (int c = 0; c < 6; c++) {
        float c_ang = c * (2.0f * M_PI / 6.0f) + time_val * 0.1f;
        int csx = (int)(poppy_joints[0].x + 7.0f * cosf(c_ang));
        int csy = (int)(poppy_joints[0].y + 7.0f * sinf(c_ang));
        draw_line((int)poppy_joints[0].x, (int)poppy_joints[0].y, csx, csy, 5, 5, 5);
    }

    // 5. Update and render 16 dynamic gold pollen particles drifting in the wind
    if (!pollen_initialized) {
        for (int p = 0; p < MAX_POLLEN; p++) {
            pollen[p].life = 0.0f;
        }
        pollen_initialized = true;
    }

    for (int p = 0; p < MAX_POLLEN; p++) {
        pollen[p].life -= 0.012f;
        if (pollen[p].life <= 0.0f) {
            pollen[p].x = poppy_joints[0].x + (float)(rand() % 14 - 7);
            pollen[p].y = poppy_joints[0].y + (float)(rand() % 14 - 7);
            pollen[p].vx = (wind_x * 0.15f) + (float)(rand() % 100 - 50) / 120.0f;
            pollen[p].vy = 0.4f + (float)(rand() % 100) / 250.0f;
            pollen[p].life = 0.4f + (float)(rand() % 100) / 160.0f; // randomized lifetime
        }

        // Adding miniature swirling eddies turbulence
        float turbulence = 1.4f * cosf(time_val * 5.0f + p);
        pollen[p].x += pollen[p].vx + turbulence * 0.05f;
        pollen[p].y += pollen[p].vy;

        // Draw glowing gold particle tip
        if (pollen[p].x >= 0 && pollen[p].x < WIDTH && pollen[p].y >= 0 && pollen[p].y < HEIGHT) {
            uint8_t alpha = (uint8_t)(255.0f * pollen[p].life);
            // Blend gold color with current background pixels
            int p_idx = ((int)pollen[p].y * WIDTH + (int)pollen[p].x) * 3;
            frame_buffer[p_idx] = (uint8_t)fminf(255, frame_buffer[p_idx] + (255 * alpha / 255));
            frame_buffer[p_idx + 1] = (uint8_t)fminf(255, frame_buffer[p_idx + 1] + (215 * alpha / 255));
            frame_buffer[p_idx + 2] = (uint8_t)fminf(255, frame_buffer[p_idx + 2] + (0 * alpha / 255));
        }
}
}


static void draw_char_direct(int x, int y, char c, int scale, uint8_t r, uint8_t g, uint8_t b) {
    const uint8_t *bitmap = NULL;
    
    if (c == '~') bitmap = symbol_leaf;
    else if (c == '@') bitmap = symbol_fleur;
    else if (c == '^') bitmap = symbol_rune;
    else if (c == '&') bitmap = symbol_skull;
    
    if (bitmap) {
        for (int row = 0; row < 8; row++) {
            uint8_t byte = bitmap[row];
            for (int col = 0; col < 8; col++) {
                if (byte & (1 << (7 - col))) {
                    for (int sy = 0; sy < scale; sy++) {
                        for (int sx = 0; sx < scale; sx++) {
                            int px = x + col * scale + sx;
                            int py = y + row * scale + sy;
                            if (px >= 45 && px < WIDTH && py >= 0 && py < HEIGHT) {
                                int idx = (py * WIDTH + px) * 3;
                                frame_buffer[idx] = (uint8_t)fminf(255, frame_buffer[idx] + r);
                                frame_buffer[idx + 1] = (uint8_t)fminf(255, frame_buffer[idx + 1] + g);
                                frame_buffer[idx + 2] = (uint8_t)fminf(255, frame_buffer[idx + 2] + b);
                            }
                        }
                    }
                }
            }
        }
        return;
    }

    if (c < 32 || c > 126) return;
    int char_idx = c - 32;
    for (int row = 0; row < 8; row++) {
        uint8_t byte = font8x8_basic[char_idx][row];
        for (int col = 0; col < 8; col++) {
            if (byte & (1 << (7 - col))) {
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        int px = x + col * scale + sx;
                        int py = y + row * scale + sy;
                        if (px >= 45 && px < WIDTH && py >= 0 && py < HEIGHT) {
                            int idx = (py * WIDTH + px) * 3;
                            frame_buffer[idx] = (uint8_t)fminf(255, frame_buffer[idx] + r);
                            frame_buffer[idx + 1] = (uint8_t)fminf(255, frame_buffer[idx + 1] + g);
                            frame_buffer[idx + 2] = (uint8_t)fminf(255, frame_buffer[idx + 2] + b);
                        }
                    }
                }
            }
        }
    }
}

static void draw_string_direct(int x, int y, const char *str, int scale, uint8_t r, uint8_t g, uint8_t b) {
    int curr_x = x;
    while (*str) {
        draw_char_direct(curr_x, y, *str, scale, r, g, b);
        curr_x += 8 * scale;
        str++;
    }
}

static void draw_panel(int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b) {
    for (int y = y0; y <= y1; y++) {
        if (y < 0 || y >= HEIGHT) continue;
        for (int x = x0; x <= x1; x++) {
            if (x < 45 || x >= WIDTH) continue;
            int idx = (y * WIDTH + x) * 3;
            frame_buffer[idx] = (uint8_t)(frame_buffer[idx] * 0.5f + r * 0.5f);
            frame_buffer[idx + 1] = (uint8_t)(frame_buffer[idx + 1] * 0.5f + g * 0.5f);
            frame_buffer[idx + 2] = (uint8_t)(frame_buffer[idx + 2] * 0.5f + b * 0.5f);
        }
    }
}

static void draw_gothic_frame(uint8_t r, uint8_t g, uint8_t b) {
    int x0 = 515, y0 = 10, x1 = 1270, y1 = HEIGHT - 10;
    
    draw_line(x0, y0, x1, y0, r, g, b);
    draw_line(x0, y0 + 3, x1, y0 + 3, r, g, b);
    
    draw_line(x0, y1, x1, y1, r, g, b);
    draw_line(x0, y1 - 3, x1, y1 - 3, r, g, b);
    
    draw_line(x0, y0, x0, y1, r, g, b);
    draw_line(x0 + 3, y0, x0 + 3, y1, r, g, b);
    
    draw_line(x1, y0, x1, y1, r, g, b);
    draw_line(x1 - 3, y0, x1 - 3, y1, r, g, b);

    draw_char_direct(x0 + 6, y0 + 6, '@', 2, r, g, b);
    draw_char_direct(x1 - 20, y0 + 6, '@', 2, r, g, b);
    draw_char_direct(x0 + 6, y1 - 20, '@', 2, r, g, b);
    draw_char_direct(x1 - 20, y1 - 20, '@', 2, r, g, b);
}

static void draw_super8_sprockets() {
    draw_panel(0, 0, 45, HEIGHT, 10, 10, 10);
    draw_line(45, 0, 45, HEIGHT, 60, 60, 60);

    for (int y = 20; y < HEIGHT; y += 90) {
        draw_panel(15, y, 35, y + 24, 0, 0, 0);
        draw_line(15, y, 17, y, 10, 10, 10);
        draw_line(15, y + 24, 17, y + 24, 10, 10, 10);
        draw_line(33, y, 35, y, 10, 10, 10);
        draw_line(33, y + 24, 35, y + 24, 10, 10, 10);
    }
}

int main(int argc, char **argv) {
    precompute_bubbles();

    if (argc < 3) {
        fprintf(stderr, "Usage: tsfi_starwars_scroller <markdown_filepath> <total_frames> [lore_filename] [start_time_sec] [color_scheme] [shape_type] [tempo_bpm] [usd_val] [token_sym] [tx_hash]\n");
        return 1;
    }

    const char *filepath = argv[1];
    int total_frames = atoi(argv[2]);
    if (total_frames <= 0) total_frames = 200;

    const char *lore_filename = (argc >= 4) ? argv[3] : "Auncient Document";
    float start_time_sec = (argc >= 5) ? atof(argv[4]) : 0.0f;
    int color_scheme = (argc >= 6) ? atoi(argv[5]) : 0;
    int shape_type = (argc >= 7) ? atoi(argv[6]) : 0;
    float tempo_bpm = (argc >= 8) ? atof(argv[7]) : 96.0f;

    double usd_value = (argc >= 9) ? atof(argv[8]) : 15000.0;
    const char *token_sym = (argc >= 10) ? argv[9] : "DAI";
    const char *tx_hash = (argc >= 11) ? argv[10] : "0000000000000000000000000000000000000000000000000000000000000000";

    FILE *file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Error opening file: %s\n", filepath);
        return 1;
    }

    char block_text[8192];
    memset(block_text, 0, sizeof(block_text));
    size_t bytes_read = fread(block_text, 1, sizeof(block_text) - 1, file);
    fclose(file);

    init_stars_and_bears();

    float time_val = 0.0f;
    for (int frame = 0; frame < total_frames; frame++) {
        // Load live configuration parameters from binary state file
        FILE *cfg_f = fopen("/tmp/scroller_live_config.bin", "rb");
        if (cfg_f) {
            TSFiScrollerLiveConfig live_cfg;
            if (fread(&live_cfg, sizeof(TSFiScrollerLiveConfig), 1, cfg_f) == 1) {
                strncpy(live_helix_text, live_cfg.helix_text, 15);
                live_helix_text[15] = '\0';
                if (live_cfg.color_scheme >= 0) color_scheme = live_cfg.color_scheme;
                if (live_cfg.shape_type >= 0) shape_type = live_cfg.shape_type;
                if (live_cfg.speed_scale > 0.0f) live_speed_scale = live_cfg.speed_scale;
                if (live_cfg.bear_count >= 0) {
                    live_bear_count = live_cfg.bear_count;
                    if (live_bear_count > MAX_BEARS) live_bear_count = MAX_BEARS;
                }
            }
            fclose(cfg_f);
        }

        time_val += 1.0f / FPS;
        float current_time_sec = start_time_sec + (float)frame / (float)FPS;


        float bulb_flicker = 1.0f + ((rand() % 100) - 50) / 1000.0f;

        for (int y = 0; y < HEIGHT; y++) {
            float gradient = (float)y / HEIGHT;
            uint8_t left_r = 0, left_g = 0, left_b = 0;
            uint8_t right_r = 0, right_g = 0, right_b = 0;

            switch (color_scheme) {
                case 1:
                    left_r = (uint8_t)(15 * (1.0f - gradient) * bulb_flicker);
                    left_g = (uint8_t)(3 * (1.0f - gradient) * bulb_flicker);
                    left_b = (uint8_t)(15 * (1.0f - gradient) * bulb_flicker);
                    right_r = (uint8_t)((20 + 10 * gradient) * bulb_flicker);
                    right_g = (uint8_t)((10 + 10 * gradient) * bulb_flicker);
                    right_b = (uint8_t)((25 + 15 * gradient) * bulb_flicker);
                    break;
                case 2:
                    left_r = (uint8_t)(2 * (1.0f - gradient) * bulb_flicker);
                    left_g = (uint8_t)(18 * (1.0f - gradient) * bulb_flicker);
                    left_b = (uint8_t)(4 * (1.0f - gradient) * bulb_flicker);
                    right_r = (uint8_t)((15 + 10 * gradient) * bulb_flicker);
                    right_g = (uint8_t)((20 + 12 * gradient) * bulb_flicker);
                    right_b = (uint8_t)((20 + 15 * gradient) * bulb_flicker);
                    break;
                case 3:
                    left_r = (uint8_t)(22 * (1.0f - gradient) * bulb_flicker);
                    left_g = (uint8_t)(14 * (1.0f - gradient) * bulb_flicker);
                    left_b = (uint8_t)(2 * (1.0f - gradient) * bulb_flicker);
                    right_r = (uint8_t)((25 + 15 * gradient) * bulb_flicker);
                    right_g = (uint8_t)((20 + 10 * gradient) * bulb_flicker);
                    right_b = (uint8_t)((10 + 5 * gradient) * bulb_flicker);
                    break;
                case 4:
                    left_r = (uint8_t)(25 * (1.0f - gradient) * bulb_flicker);
                    left_g = (uint8_t)(2 * (1.0f - gradient) * bulb_flicker);
                    left_b = (uint8_t)(2 * (1.0f - gradient) * bulb_flicker);
                    right_r = (uint8_t)((5 + 8 * gradient) * bulb_flicker);
                    right_g = (uint8_t)(2 * bulb_flicker);
                    right_b = (uint8_t)(3 * bulb_flicker);
                    break;
                case 0:
                default:
                    left_r = (uint8_t)(6 * (1.0f - gradient) * bulb_flicker);
                    left_g = (uint8_t)(4 * (1.0f - gradient) * bulb_flicker);
                    left_b = (uint8_t)(16 * (1.0f - gradient) * bulb_flicker);
                    right_r = (uint8_t)((10 + 15 * gradient) * bulb_flicker);
                    right_g = (uint8_t)((15 + 20 * gradient) * bulb_flicker);
                    right_b = (uint8_t)((25 + 35 * gradient) * bulb_flicker);
                    break;
            }

            for (int x = 0; x < WIDTH; x++) {
                int idx = (y * WIDTH + x) * 3;
                if (x < 500) {
                    frame_buffer[idx] = left_r;
                    frame_buffer[idx + 1] = left_g;
                    frame_buffer[idx + 2] = left_b;
                } else {
                    frame_buffer[idx] = right_r;
                    frame_buffer[idx + 1] = right_g;
                    frame_buffer[idx + 2] = right_b;
                }
            }
        }

        bool in_hyperspace = (frame < 36);
        update_and_draw_stars(in_hyperspace);
        float pulse = get_beat_pulse(current_time_sec, tempo_bpm);

        // Render the 3D USD Spline Ribbon (USD/SFT integration!)
        draw_usd_spline_ribbon(time_val, pulse, usd_value, color_scheme);

        if (!in_hyperspace) {
            for (int i = 0; i < live_bear_count; i++) {
                bears[i].y -= bears[i].speed;
                if (bears[i].y < -80) {
                    bears[i].y = HEIGHT + 80;
                    bears[i].x = 100 + rand() % 280;
                    bears[i].speed = 0.6f + (rand() % 100) / 120.0f;
                    bears[i].size_scale = 0.3f + (rand() % 100) / 300.0f;
                }
                draw_vector_teddy_bear((int)bears[i].x, (int)bears[i].y, bears[i].size_scale, pulse, color_scheme);
            }

        }

        draw_gothic_frame(100, 150, 180);

        // Header Title
        draw_string_direct(545, 25, "LORE ARCHIVE SYSTEM //", 1, 100, 150, 180);
        draw_string_direct(765, 23, lore_filename, 2, 255, 215, 0);

        draw_line(545, 45, 875, 45, 100, 150, 180);
        draw_char_direct(890, 41, '~', 2, 255, 215, 0);
        draw_line(910, 45, 1235, 45, 100, 150, 180);

        // Swap Flow Telemetry (SFT) overlay (bottom left terminal display)
        char sft_title[64], sft_hash[64], sft_vol[64];
        sprintf(sft_title, "SFT DATA ACTIVE [%s]", token_sym);
        sprintf(sft_hash, "TX: %.12s...", tx_hash);
        if (usd_value > 1.0e12) {
            sprintf(sft_vol, "VOL: $%.2e USD", usd_value);
        } else {
            sprintf(sft_vol, "VOL: $%.2f USD", usd_value);
        }
        draw_string_direct(55, HEIGHT - 85, sft_title, 1, 0, 220, 255);
        draw_string_direct(55, HEIGHT - 70, sft_hash, 1, 180, 180, 180);
        draw_string_direct(55, HEIGHT - 55, sft_vol, 1, 0, 255, 120);

        if (in_hyperspace) {
            // No decoding text
        } else {
            // No text layout
        }


        draw_super8_sprockets();

        int scratch_mod = 4;
        int max_dust = 3;
        if (color_scheme == 1) {
            scratch_mod = 2;
            max_dust = 6;
        } else if (color_scheme == 4 || color_scheme == 2) {
            scratch_mod = 8;
            max_dust = 1;
        }

        if (rand() % scratch_mod == 0) {
            int scratch_x = 45 + rand() % (WIDTH - 55);
            int scratch_h = 100 + rand() % (HEIGHT - 200);
            int scratch_y = rand() % (HEIGHT - scratch_h);
            for (int sy = scratch_y; sy < scratch_y + scratch_h; sy++) {
                int idx = (sy * WIDTH + scratch_x) * 3;
                frame_buffer[idx] = (uint8_t)(frame_buffer[idx] * 0.15f);
                frame_buffer[idx + 1] = (uint8_t)(frame_buffer[idx + 1] * 0.15f);
                frame_buffer[idx + 2] = (uint8_t)(frame_buffer[idx + 2] * 0.15f);
            }
        }

        for (int d = 0; d < max_dust; d++) {
            if (rand() % 2 == 0) {
                int dust_x = 55 + rand() % (WIDTH - 70);
                int dust_y = 15 + rand() % (HEIGHT - 30);
                int dust_r = 1 + rand() % 3;
                for (int dy = -dust_r; dy <= dust_r; dy++) {
                    for (int dx = -dust_r; dx <= dust_r; dx++) {
                        if (dx*dx + dy*dy <= dust_r*dust_r) {
                            int px = dust_x + dx;
                            int py = dust_y + dy;
                            if (px >= 45 && px < WIDTH && py >= 0 && py < HEIGHT) {
                                int idx = (py * WIDTH + px) * 3;
                                frame_buffer[idx] = 12;
                                frame_buffer[idx + 1] = 12;
                                frame_buffer[idx + 2] = 12;
                            }
                        }
                    }
                }
            }
        }

        int cx = WIDTH / 2;
        int cy = HEIGHT / 2;
        float max_dist = sqrtf(cx*cx + cy*cy);

        float burn_progress = 0.0f;
        if (frame < 12) {
            burn_progress = 1.0f - ((float)frame / 12.0f);
        } else if (frame > total_frames - 12) {
            burn_progress = ((float)(frame - (total_frames - 12)) / 12.0f);
        }

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                int idx = (y * WIDTH + x) * 3;
                int grain = (rand() % 16) - 8;

                float dx = x - cx;
                float dy = y - cy;
                float dist = sqrtf(dx*dx + dy*dy);
                float vignette = 1.0f;
                if (dist > max_dist * 0.6f) {
                    vignette = 1.0f - ((dist - max_dist * 0.6f) / (max_dist * 0.4f)) * 0.45f;
                }

                float r = (frame_buffer[idx] + grain) * 1.08f * vignette;
                float g = (frame_buffer[idx + 1] + grain) * 0.95f * vignette;
                float b = (frame_buffer[idx + 2] + grain) * 0.80f * vignette;

                if (burn_progress > 0.0f) {
                    r += burn_progress * 220.0f;
                    g += burn_progress * 130.0f;
                    b += burn_progress * 30.0f;
                }

                frame_buffer[idx] = (uint8_t)fminf(255, fmaxf(0, r));
                frame_buffer[idx + 1] = (uint8_t)fminf(255, fmaxf(0, g));
                frame_buffer[idx + 2] = (uint8_t)fminf(255, fmaxf(0, b));
            }
        }

        int jitter_x = 0;
        int jitter_y = 0;
        if (pulse > 0.05f) {
            jitter_x = (int)((rand() % 9 - 4) * pulse);
            jitter_y = (int)((rand() % 9 - 4) * pulse);
        }

        if (jitter_x == 0 && jitter_y == 0) {
            fwrite(frame_buffer, 1, sizeof(frame_buffer), stdout);
        } else {
            static uint8_t jitter_buffer[WIDTH * HEIGHT * 3];
            memset(jitter_buffer, 0, sizeof(jitter_buffer));
            for (int y = 0; y < HEIGHT; y++) {
                int target_y = y + jitter_y;
                if (target_y < 0 || target_y >= HEIGHT) continue;
                for (int x = 0; x < WIDTH; x++) {
                    int target_x = x + jitter_x;
                    if (target_x < 0 || target_x >= WIDTH) continue;
                    int src_idx = (y * WIDTH + x) * 3;
                    int dst_idx = (target_y * WIDTH + target_x) * 3;
                    jitter_buffer[dst_idx] = frame_buffer[src_idx];
                    jitter_buffer[dst_idx + 1] = frame_buffer[src_idx + 1];
                    jitter_buffer[dst_idx + 2] = frame_buffer[src_idx + 2];
                }
            }
            fwrite(jitter_buffer, 1, sizeof(jitter_buffer), stdout);
        }
    }

    return 0;
}

