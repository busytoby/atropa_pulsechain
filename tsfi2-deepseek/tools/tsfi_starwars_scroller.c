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

// Framebuffer allocated as global buffer
uint8_t frame_buffer[WIDTH * HEIGHT * 3];

#include "font8x8.h"
#include "tsfi_scroller_graphics.h"
#include "tsfi_scroller_physics.h"

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

static void draw_vector_teddy_bear(int cx, int cy, float size_scale, float pulse, int color_scheme) {
    (void)cx; (void)cy; (void)size_scale; (void)pulse; (void)color_scheme;
    // Teddy bear visual rendering disabled in favor of organic poppy
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

        draw_string_direct(545, 25, "LORE ARCHIVE SYSTEM //", 1, 100, 150, 180);
        draw_string_direct(765, 23, lore_filename, 2, 255, 215, 0);

        draw_line(545, 45, 875, 45, 100, 150, 180);
        draw_char_direct(890, 41, '~', 2, 255, 215, 0);
        draw_line(910, 45, 1235, 45, 100, 150, 180);

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
