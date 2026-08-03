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
} TSFiScrollerLiveConfig;

static char live_helix_text[16] = "TSFi/2";

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

static TeddyBear bears[3];

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

    for (int i = 0; i < 3; i++) {
        bears[i].x = 100 + rand() % 280;
        bears[i].y = HEIGHT + 100 + i * 220;
        bears[i].speed = 0.6f + (rand() % 100) / 120.0f;
        bears[i].size_scale = 0.5f + (rand() % 100) / 200.0f;
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
        float speed = stars[i].speed * (hyperspace ? 20.0f : 1.0f);
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
static void draw_usd_spline_ribbon(float time_val, float pulse, double usd_value, int color_scheme) {
    double px[5] = { -160.0, -80.0, 20.0, 90.0, 160.0 };
    double py[5] = { -110.0, -40.0, 60.0, 110.0, -70.0 };
    double pz[5] = { -40.0, 30.0, -20.0, 50.0, 10.0 };

    double sft_scale = log10(usd_value + 1.0) * 16.0;
    if (sft_scale > 150.0) sft_scale = 150.0;
    if (sft_scale < 5.0) sft_scale = 5.0;

    for (int i = 0; i < 5; i++) {
        px[i] += sin(time_val * 2.0 + i) * sft_scale * 0.4;
        py[i] += cos(time_val * 1.7 + i) * sft_scale * 0.4;
        pz[i] += sin(time_val * 1.3 + i) * sft_scale * 0.3;
    }

    double cz = 320.0;
    int vx_center = 240;
    int vy_center = HEIGHT / 2;

    float pulse_scale = 1.0f + pulse * 0.45f;
    
    // Biotika Heartbeat Dilation cycle (simulating cellular expand/contract respiration)
    float heartbeat = 1.0f + 0.22f * sin(time_val * 4.2f) + pulse * 0.25f;

    int prev_px1 = -1, prev_py1 = -1;
    int prev_px2 = -1, prev_py2 = -1;

    // Render entwined double-strand Vaesen rose/ivy vines
    int idx_count = 0;
    for (double t = 0.0; t <= 1.0; t += 0.006) {
        idx_count++;
        double omt = 1.0 - t;
        double omt2 = omt * omt;
        double omt3 = omt2 * omt;
        double t2 = t * t;
        double t3 = t2 * t;

        // Base spline path
        double bx = omt3 * px[0] + 3.0 * omt2 * t * px[1] + 3.0 * omt * t2 * px[2] + t3 * px[3];
        double by = omt3 * py[0] + 3.0 * omt2 * t * py[1] + 3.0 * omt * t2 * py[2] + t3 * py[3];
        double bz = omt3 * pz[0] + 3.0 * omt2 * t * pz[1] + 3.0 * omt * t2 * pz[2] + t3 * pz[3];

        // DNA-like fiber thickness modulation along the path
        double thick_mod = 0.8 + 0.35 * sin(t * 38.0);
        int bubble_size = (int)(5 * pulse_scale * heartbeat * thick_mod);
        if (bubble_size < 3) bubble_size = 3;

        // 3D helical rotation offset to weave two vine strands
        double helix_ang = t * 32.0 + time_val * 6.0;
        double ox = 14.0 * sin(helix_ang) * heartbeat;
        double oy = 14.0 * cos(helix_ang) * heartbeat;
        double oz = 8.0 * sin(helix_ang * 0.5) * heartbeat;

        // Strand 1: Ivy green vine
        double x1 = bx + ox;
        double y1 = by + oy;
        double z1 = bz + oz;

        // Strand 2: Thorny wood-brown vine
        double x2 = bx - ox;
        double y2 = by - oy;
        double z2 = bz - oz;

        // Project Strand 1
        double dist1 = cz - z1;
        if (dist1 > 10.0) {
            double scale1 = 300.0 / dist1;
            int sx1 = vx_center + (int)(x1 * scale1);
            int sy1 = vy_center + (int)(y1 * scale1);

            if (sx1 >= 45 && sx1 < 490 && sy1 >= 0 && sy1 < HEIGHT) {
                if (prev_px1 != -1 && prev_py1 != -1) {
                    draw_line(prev_px1, prev_py1, sx1, sy1, 35, 155, 65); // Leafy Ivy Green
                }
                if (idx_count % 8 == 0) {
                    // Bio-luminescent green node
                    draw_glossy_bubble(sx1, sy1, bubble_size, 45, 220, 110);
                }
                prev_px1 = sx1;
                prev_py1 = sy1;
            } else {
                prev_px1 = -1;
                prev_py1 = -1;
            }
        }

        // Project Strand 2
        double dist2 = cz - z2;
        if (dist2 > 10.0) {
            double scale2 = 300.0 / dist2;
            int sx2 = vx_center + (int)(x2 * scale2);
            int sy2 = vy_center + (int)(y2 * scale2);

            if (sx2 >= 45 && sx2 < 490 && sy2 >= 0 && sy2 < HEIGHT) {
                if (prev_px2 != -1 && prev_py2 != -1) {
                    draw_line(prev_px2, prev_py2, sx2, sy2, 145, 95, 35); // Thorny Wood Brown
                }
                if (idx_count % 10 == 0) {
                    // Bio-luminescent ruby red/orange node
                    draw_glossy_bubble(sx2, sy2, bubble_size - 1, 255, 70, 50);
                }
                prev_px2 = sx2;
                prev_py2 = sy2;
            } else {
                prev_px2 = -1;
                prev_py2 = -1;
            }
        }

        // Draw dynamic sliding gold letters from live_helix_text on the base spline path
        if (idx_count % 35 == 0) {
            double r_dist = cz - bz;
            if (r_dist > 10.0) {
                double r_scale = 300.0 / r_dist;
                int rx = vx_center + (int)(bx * r_scale);
                int ry = vy_center + (int)(by * r_scale);
                if (rx >= 45 && rx < 480 && ry >= 10 && ry < HEIGHT - 20) {
                    int len = strlen(live_helix_text);
                    if (len > 0) {
                        int char_idx = ((idx_count / 35) - 1) % len;
                        char sym = live_helix_text[char_idx];
                        draw_char_direct(rx, ry, sym, 2, 255, 215, 0); // Gold
                    }
                }
            }
        }



        // Biotika Spore particles initialization & emission
        if (!spores_initialized) {
            for (int s = 0; s < MAX_SPORES; s++) {
                spores[s].age = 0.0f;
            }
            spores_initialized = true;
        }

        // Trigger spore launch at random locations along the spline
        if (idx_count % 20 == 0 && rand() % 5 == 0) {
            for (int s = 0; s < MAX_SPORES; s++) {
                if (spores[s].age <= 0.0f) {
                    spores[s].x = bx;
                    spores[s].y = by;
                    spores[s].z = bz;
                    spores[s].vx = ((rand() % 100 - 50) / 100.0f) * 1.5f;
                    spores[s].vy = -((rand() % 100) / 100.0f) * 2.0f - 0.5f; // Drifts upward
                    spores[s].vz = ((rand() % 100 - 50) / 100.0f) * 1.0f;
                    spores[s].max_age = 1.0f + (rand() % 100) / 50.0f;
                    spores[s].age = spores[s].max_age;
                    break;
                }
            }
        }
    }

    // Update and draw floating bio-luminescent spores
    for (int s = 0; s < MAX_SPORES; s++) {
        if (spores[s].age > 0.0f) {
            spores[s].x += spores[s].vx;
            spores[s].y += spores[s].vy;
            spores[s].z += spores[s].vz;
            spores[s].age -= 1.0f / FPS;

            double sp_dist = cz - spores[s].z;
            if (sp_dist > 10.0) {
                double sp_scale = 300.0 / sp_dist;
                int sp_sx = vx_center + (int)(spores[s].x * sp_scale);
                int sp_sy = vy_center + (int)(spores[s].y * sp_scale);

                if (sp_sx >= 45 && sp_sx < 490 && sp_sy >= 0 && sp_sy < HEIGHT) {
                    float fade = spores[s].age / spores[s].max_age;
                    int sp_size = (int)(4 * fade + 1);
                    uint8_t sp_r = (uint8_t)(255 * fade);
                    uint8_t sp_g = (uint8_t)(200 * fade);
                    uint8_t sp_b = (uint8_t)(50 * fade); // Warm glowing amber spore
                    draw_glossy_bubble(sp_sx, sp_sy, sp_size, sp_r, sp_g, sp_b);
                }
            }
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
            for (int i = 0; i < 3; i++) {
                bears[i].y -= bears[i].speed;
                if (bears[i].y < -80) {
                    bears[i].y = HEIGHT + 80;
                    bears[i].x = 100 + rand() % 280;
                    bears[i].speed = 0.6f + (rand() % 100) / 120.0f;
                    bears[i].size_scale = 0.5f + (rand() % 100) / 200.0f;
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
            draw_string_direct(700, 320, "DECODING NEURAL LORE MEMORY CARD...", 2, 0, 200, 255);
        } else {
            int start_y = 70;
            char *line_start = block_text;
            
            while (*line_start && start_y < HEIGHT - 65) {
                char *line_end = strchr(line_start, '\n');
                char line[512];
                memset(line, 0, sizeof(line));
                if (line_end) {
                    size_t len = line_end - line_start;
                    if (len > sizeof(line) - 1) len = sizeof(line) - 1;
                    strncpy(line, line_start, len);
                    line_start = line_end + 1;
                } else {
                    strncpy(line, line_start, sizeof(line) - 1);
                    line_start = "";
                }

                char *trimmed = line;
                while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
                if (strlen(trimmed) == 0) {
                    start_y += 12;
                    continue;
                }

                if (trimmed[0] == '#') {
                    int lvl = 0;
                    while (trimmed[lvl] == '#') lvl++;
                    char *h_text = trimmed + lvl;
                    while (*h_text == ' ') h_text++;
                    int text_scale = (lvl == 1) ? 3 : 2;
                    draw_string_direct(545, start_y + 4, h_text, text_scale, 255, 215, 0);
                    start_y += (text_scale == 3) ? 36 : 28;
                } else if (trimmed[0] == '-' || trimmed[0] == '*') {
                    char *item = trimmed + 1;
                    while (*item == ' ') item++;
                    draw_char_direct(545, start_y, '~', 2, 255, 215, 0);
                    
                    int len = strlen(item);
                    int chars_per_line = 42;
                    for (int offset = 0; offset < len && start_y < HEIGHT - 65; offset += chars_per_line) {
                        char chunk[64];
                        int to_copy = len - offset < chars_per_line ? len - offset : chars_per_line;
                        strncpy(chunk, item + offset, to_copy);
                        chunk[to_copy] = '\0';
                        draw_string_direct(565, start_y, chunk, 2, 240, 240, 250);
                        start_y += 22;
                    }
                } else if (strstr(trimmed, "[CHART:") == trimmed) {
                    draw_string_direct(545, start_y, "PERFORMANCE TELEMETRY DIAGRAM", 1, 0, 200, 255);
                    start_y += 18;
                    
                    draw_panel(545, start_y, 1220, start_y + 190, 10, 20, 30);
                    draw_line(585, start_y + 10, 585, start_y + 160, 150, 150, 150);
                    draw_line(585, start_y + 160, 1200, start_y + 160, 150, 150, 150);
                    
                    int chart_values[4] = { 120, 70, 140, 100 };
                    const char *labels[4] = { "ZMM CORE", "Winchester", "WinchesterMQ", "Lissajous" };
                    for (int col = 0; col < 4; col++) {
                        int bar_h = chart_values[col];
                        int bar_x = 640 + col * 130;
                        int bar_y = start_y + 160 - bar_h;
                        
                        draw_panel(bar_x, bar_y, bar_x + 50, start_y + 159, 0, 160 + col * 20, 220);
                        
                        char val_str[16];
                        sprintf(val_str, "%d", bar_h);
                        draw_string_direct(bar_x + 10, bar_y - 12, val_str, 1, 255, 255, 255);
                        draw_string_direct(bar_x - 10, start_y + 170, labels[col], 1, 180, 180, 180);
                    }
                    start_y += 210;
                } else if (trimmed[0] == '|' && trimmed[1] == '-') {
                    continue;
                } else if (trimmed[0] == '|') {
                    draw_panel(545, start_y - 2, 1220, start_y + 22, 20, 30, 45);
                    char row_copy[512];
                    strcpy(row_copy, trimmed + 1);
                    char *cell = strtok(row_copy, "|");
                    int col_x = 555;
                    while (cell && col_x < 1200) {
                        char *tc = cell;
                        while (*tc == ' ') tc++;
                        size_t l = strlen(tc);
                        while (l > 0 && tc[l - 1] == ' ') { tc[l - 1] = '\0'; l--; }
                        draw_string_direct(col_x, start_y, tc, 1, 240, 240, 240);
                        col_x += 160;
                        cell = strtok(NULL, "|");
                    }
                    start_y += 26;
                } else {
                    int len = strlen(trimmed);
                    int chars_per_line = 46;
                    
                    char first_c = trimmed[0];
                    int text_start_offset = 0;
                    if ((first_c >= 'a' && first_c <= 'z') || (first_c >= 'A' && first_c <= 'Z')) {
                        draw_char_direct(545, start_y, first_c, 3, 255, 215, 0);
                        text_start_offset = 1;
                    }
                    
                    for (int offset = 0; offset < len && start_y < HEIGHT - 65; offset += chars_per_line) {
                        char chunk[64];
                        int chunk_start = offset + (offset == 0 ? text_start_offset : 0);
                        int to_copy = len - chunk_start < chars_per_line ? len - chunk_start : chars_per_line;
                        if (to_copy <= 0) break;
                        strncpy(chunk, trimmed + chunk_start, to_copy);
                        chunk[to_copy] = '\0';
                        
                        int text_x = (offset == 0 && text_start_offset > 0) ? 570 : 545;
                        draw_string_direct(text_x, start_y + (offset == 0 && text_start_offset > 0 ? 2 : 0), chunk, 2, 240, 240, 250);
                        start_y += 22;
                    }
                }
            }

            // Playback progress bar
            float progress = (float)frame / (float)total_frames;
            int progress_w = (int)(690 * progress);
            draw_string_direct(545, HEIGHT - 35, "PLAYBACK:", 1, 150, 150, 150);
            draw_panel(625, HEIGHT - 33, 1220, HEIGHT - 27, 20, 30, 40);
            draw_panel(625, HEIGHT - 33, 625 + progress_w, HEIGHT - 27, 0, 220, 120);
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

