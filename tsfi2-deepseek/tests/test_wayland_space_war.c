#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include "vulkan/vulkan_main.h"
#include "vulkan/vulkan_render.h"
#include "tsfi_raw.h"
#include "lau_yul_thunk.h"
#include "tsfi_staging.h"
#include "tsfi_font_vectors.h"
#include "tsfi_font_ai.h"

void tsfi_input_set_key_hook(void *hook_func);

// AB4H Pixel Layout
typedef struct {
    uint16_t r, g, b, a;
} AB4HPixel;

static inline uint16_t float_to_half(float f) {
    union { float f; uint32_t u; } u = { f };
    uint32_t sign = (u.u >> 16) & 0x8000;
    int32_t exponent = ((u.u >> 23) & 0xff) - 127;
    uint32_t mantissa = u.u & 0x7fffff;
    if (exponent <= -15) return sign;
    if (exponent >= 16) return sign | 0x7c00;
    return sign | ((exponent + 15) << 10) | (mantissa >> 13);
}

static inline float half_to_float(uint16_t h) {
    uint32_t sign = (h & 0x8000) << 16;
    uint32_t exponent = (h & 0x7c00) >> 10;
    uint32_t mantissa = (h & 0x03ff) << 13;
    if (exponent == 0) {
        if (mantissa == 0) return 0.0f;
        while (!(mantissa & 0x00800000)) { mantissa <<= 1; exponent--; }
        exponent++; exponent += 127 - 15;
    } else if (exponent == 31) exponent = 255;
    else exponent += 127 - 15;
    union { uint32_t u; float f; } u = { sign | (exponent << 23) | mantissa };
    return u.f;
}

static inline AB4HPixel make_ab4h_pixel(float r, float g, float b, float a) {
    AB4HPixel p;
    p.r = float_to_half(r);
    p.g = float_to_half(g);
    p.b = float_to_half(b);
    p.a = float_to_half(a);
    return p;
}

void draw_rect_ab4h(AB4HPixel *pixels, int w, int h, int x, int y, int rw, int rh, AB4HPixel color) {
    float tr = half_to_float(color.r), tg = half_to_float(color.g), tb = half_to_float(color.b), ta = half_to_float(color.a);
    for (int cy = y; cy < y + rh; cy++) {
        for (int cx = x; cx < x + rw; cx++) {
            if (cx >= 0 && cx < w && cy >= 0 && cy < h) {
                if (ta >= 1.0f) {
                    pixels[cy * w + cx] = color;
                } else {
                    AB4HPixel curr = pixels[cy * w + cx];
                    float cr = half_to_float(curr.r), cg = half_to_float(curr.g), cb = half_to_float(curr.b), ca = half_to_float(curr.a);
                    float nr = cr * (1.0f - ta) + tr * ta;
                    float ng = cg * (1.0f - ta) + tg * ta;
                    float nb = cb * (1.0f - ta) + tb * ta;
                    float na = ca * (1.0f - ta) + ta;
                    pixels[cy * w + cx] = make_ab4h_pixel(nr, ng, nb, na);
                }
            }
        }
    }
}

void draw_line_aa(AB4HPixel *pixels, int w, int h, float x0, float y0, float x1, float y1, AB4HPixel color, float thickness) {
    int min_x = fmaxf(0.0f, fminf(x0, x1) - thickness - 1.0f);
    int max_x = fminf(w - 1, fmaxf(x0, x1) + thickness + 1.0f);
    int min_y = fmaxf(0.0f, fminf(y0, y1) - thickness - 1.0f);
    int max_y = fminf(h - 1, fmaxf(y0, y1) + thickness + 1.0f);
    float dx = x1 - x0, dy = y1 - y0;
    float len2 = dx * dx + dy * dy;
    float tr = half_to_float(color.r), tg = half_to_float(color.g), tb = half_to_float(color.b), ta = half_to_float(color.a);
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            float px = (float)x + 0.5f, py = (float)y + 0.5f;
            float t = 0.0f;
            if (len2 > 0.0f) {
                t = ((px - x0) * dx + (py - y0) * dy) / len2;
                t = fmaxf(0.0f, fminf(1.0f, t));
            }
            float proj_x = x0 + t * dx, proj_y = y0 + t * dy;
            float dist = sqrtf((px - proj_x)*(px - proj_x) + (py - proj_y)*(py - proj_y));
            if (dist <= thickness) {
                AB4HPixel curr = pixels[y * w + x];
                float cr = half_to_float(curr.r), cg = half_to_float(curr.g), cb = half_to_float(curr.b), ca = half_to_float(curr.a);
                float nr = cr * (1.0f - ta) + tr * ta;
                float ng = cg * (1.0f - ta) + tg * ta;
                float nb = cb * (1.0f - ta) + tb * ta;
                float na = ca * (1.0f - ta) + ta;
                pixels[y * w + x] = make_ab4h_pixel(nr, ng, nb, na);
            } else if (dist < thickness + 1.0f) {
                float alpha = ta * (1.0f - (dist - thickness));
                AB4HPixel curr = pixels[y * w + x];
                float cr = half_to_float(curr.r), cg = half_to_float(curr.g), cb = half_to_float(curr.b), ca = half_to_float(curr.a);
                float nr = cr * (1.0f - alpha) + tr * alpha;
                float ng = cg * (1.0f - alpha) + tg * alpha;
                float nb = cb * (1.0f - alpha) + tb * alpha;
                float na = ca * (1.0f - alpha) + alpha;
                pixels[y * w + x] = make_ab4h_pixel(nr, ng, nb, na);
            }
        }
    }
}

void draw_char_ab4h(AB4HPixel *pixels, int w, int h, char c, int x, int y, AB4HPixel color) {
    uint32_t temp_buf[32 * 32] = {0};
    StagingBuffer temp_sb = {
        .magic = TSFI_STAGING_MAGIC,
        .data = temp_buf,
        .width = 32,
        .height = 32,
        .stride = 32 * 4,
        .size = 32 * 32 * 4
    };
    draw_debug_codepoint(&temp_sb, 0, 0, (uint32_t)c, 0xFFFFFFFF);
    float tr = half_to_float(color.r);
    float tg = half_to_float(color.g);
    float tb = half_to_float(color.b);
    float ta = half_to_float(color.a);
    for (int j = 0; j < 32; j++) {
        int py = y - 4 + j;
        if (py < 0 || py >= h) continue;
        for (int i = 0; i < 32; i++) {
            int px = x + i;
            if (px < 0 || px >= w) continue;
            uint32_t p = temp_buf[j * 32 + i];
            float alpha = ((p >> 24) & 0xFF) / 255.0f * ta;
            if (alpha > 0.01f) {
                AB4HPixel curr = pixels[py * w + px];
                float cr = half_to_float(curr.r);
                float cg = half_to_float(curr.g);
                float cb = half_to_float(curr.b);
                float ca = half_to_float(curr.a);
                float nr = cr * (1.0f - alpha) + tr * alpha;
                float ng = cg * (1.0f - alpha) + tg * alpha;
                float nb = cb * (1.0f - alpha) + tb * alpha;
                float na = ca * (1.0f - alpha) + alpha;
                pixels[py * w + px] = make_ab4h_pixel(nr, ng, nb, na);
            }
        }
    }
}

void draw_string_ab4h(AB4HPixel *pixels, int w, int h, const char *str, int x, int y, AB4HPixel color) {
    AB4HPixel default_color = color;
    AB4HPixel current_color = color;
    while (*str) {
        if (*str == '^' && *(str + 1) != '\0') {
            char code = *(str + 1);
            if (code == 'B' || code == 'b') {
                float r = half_to_float(default_color.r) * 1.5f;
                float g = half_to_float(default_color.g) * 1.5f;
                float b = half_to_float(default_color.b) * 1.5f;
                float a = half_to_float(default_color.a);
                current_color = make_ab4h_pixel(r, g, b, a);
                str += 2;
                continue;
            } else if (code == 'I' || code == 'i') {
                current_color = make_ab4h_pixel(1.0f, 0.05f, 0.6f, 1.0f); // neon_pink
                str += 2;
                continue;
            } else if (code == 'U' || code == 'u') {
                current_color = make_ab4h_pixel(0.0f, 0.9f, 1.0f, 1.0f); // neon_cyan
                str += 2;
                continue;
            } else if (code == 'C' || code == 'c') {
                current_color = default_color;
                str += 2;
                continue;
            }
        }
        draw_char_ab4h(pixels, w, h, *str, x, y, current_color);
        x += 12;
        str++;
    }
}

void draw_radial_glow(AB4HPixel *pixels, int w, int h, float cx, float cy, float radius, AB4HPixel color) {
    int min_x = fmaxf(0.0f, cx - radius), max_x = fminf(w - 1, cx + radius);
    int min_y = fmaxf(0.0f, cy - radius), max_y = fminf(h - 1, cy + radius);
    float tr = half_to_float(color.r), tg = half_to_float(color.g), tb = half_to_float(color.b), ta = half_to_float(color.a);
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            float dx = (float)x + 0.5f - cx, dy = (float)y + 0.5f - cy;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist < radius) {
                float intensity = (1.0f - (dist / radius));
                intensity = intensity * intensity;
                AB4HPixel curr = pixels[y * w + x];
                float cr = half_to_float(curr.r), cg = half_to_float(curr.g), cb = half_to_float(curr.b), ca = half_to_float(curr.a);
                float final_a = ta * intensity;
                float nr = cr + tr * final_a;
                float ng = cg + tg * final_a;
                float nb = cb + tb * final_a;
                float na = fminf(1.0f, ca + final_a);
                pixels[y * w + x] = make_ab4h_pixel(nr, ng, nb, na);
            }
        }
    }
}

// Sound data structures
struct SoundData {
    char type[32];
};

static void* play_sound_thread(void *arg) {
    struct SoundData *sd = (struct SoundData*)arg;
    snd_pcm_t *pcm_handle;
    if (snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        free(sd); return NULL;
    }
    if (snd_pcm_set_params(pcm_handle, SND_PCM_FORMAT_U8, SND_PCM_ACCESS_RW_INTERLEAVED, 1, 8000, 1, 500000) < 0) {
        snd_pcm_close(pcm_handle); free(sd); return NULL;
    }
    int len = 0;
    uint8_t *buf = NULL;
    if (strcmp(sd->type, "thrust") == 0) {
        len = 800; buf = malloc(len);
        if (buf) {
            for (int i = 0; i < len; i++) {
                buf[i] = 128 + (rand() % 30) - 15;
            }
        }
    } else if (strcmp(sd->type, "shoot") == 0) {
        len = 1600; buf = malloc(len);
        if (buf) {
            for (int i = 0; i < len; i++) {
                float t = (float)i / 8000.0f;
                float freq = 800.0f - 600.0f * (t / 0.2f);
                float phase = freq * t * 2.0f * 3.14159f;
                buf[i] = 128 + (int)((1.0f - t/0.2f) * 60.0f * sinf(phase));
            }
        }
    } else if (strcmp(sd->type, "explode") == 0) {
        len = 4000; buf = malloc(len);
        if (buf) {
            for (int i = 0; i < len; i++) {
                float t = (float)i / 8000.0f;
                buf[i] = 128 + (int)((1.0f - t/0.5f) * 100.0f * ((rand() % 100) - 50.0f) / 50.0f);
            }
        }
    }
    if (buf && len > 0) {
        snd_pcm_sframes_t frames = snd_pcm_writei(pcm_handle, buf, len);
        if (frames < 0) {
            snd_pcm_prepare(pcm_handle);
            snd_pcm_writei(pcm_handle, buf, len);
        }
        free(buf);
    }
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    free(sd);
    return NULL;
}

static void play_synth_sound(const char *type) {
    struct SoundData *sd = malloc(sizeof(struct SoundData));
    if (!sd) return;
    strncpy(sd->type, type, sizeof(sd->type) - 1);
    sd->type[sizeof(sd->type) - 1] = '\0';
    pthread_t thread;
    pthread_create(&thread, NULL, play_sound_thread, sd);
    pthread_detach(thread);
}

// Space War State definitions
typedef struct {
    float x, y;
    float vx, vy;
    float angle;
    // TSFI2 Prophecy: Quaternion orientation for 3D rotation representation (6-DOF Spatial Unity)
    float qw, qx, qy, qz;
    int alive;
    int score;
    int fuel;
} Ship;

typedef struct {
    float x, y;
    float vx, vy;
    int active;
    int life;
} Missile;

#define MAX_MISSILES 8
static Ship needle; // Player 1
static Ship wedge;  // Player 2
static Missile needle_missiles[MAX_MISSILES];
static Missile wedge_missiles[MAX_MISSILES];

static float gravity_well_x = 400.0f;
static float gravity_well_y = 270.0f;
static float G = 4000.0f;

static int exit_requested = 0;
static bool key_p1_left = false;
static bool key_p1_right = false;
static bool key_p1_thrust = false;
static bool key_p2_left = false;
static bool key_p2_right = false;
static bool key_p2_thrust = false;

static void init_game() {
    needle.x = 200.0f;
    needle.y = 270.0f;
    needle.vx = 0.0f;
    needle.vy = -3.5f;
    needle.angle = -3.14159f / 2.0f;
    needle.qw = 1.0f; needle.qx = 0.0f; needle.qy = 0.0f; needle.qz = 0.0f;
    needle.alive = 1;
    needle.fuel = 1000;

    wedge.x = 600.0f;
    wedge.y = 270.0f;
    wedge.vx = 0.0f;
    wedge.vy = 3.5f;
    wedge.angle = 3.14159f / 2.0f;
    wedge.qw = 1.0f; wedge.qx = 0.0f; wedge.qy = 0.0f; wedge.qz = 0.0f;
    wedge.alive = 1;
    wedge.fuel = 1000;

    memset(needle_missiles, 0, sizeof(needle_missiles));
    memset(wedge_missiles, 0, sizeof(wedge_missiles));
}

static void fire_missile(Ship *s, Missile *list) {
    if (!s->alive) return;
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (!list[i].active) {
            list[i].active = 1;
            list[i].life = 120; // 2 seconds
            list[i].x = s->x + cosf(s->angle) * 18.0f;
            list[i].y = s->y + sinf(s->angle) * 18.0f;
            list[i].vx = s->vx + cosf(s->angle) * 6.0f;
            list[i].vy = s->vy + sinf(s->angle) * 6.0f;
            play_synth_sound("shoot");
            break;
        }
    }
}

static void key_hook(void *data, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    (void)data; (void)serial; (void)time;
    bool pressed = (state != 0);

    if (key == 1) { // ESC
        exit_requested = 1;
    } 
    // Player 1 controls (A, D, W, S, E for hyperspace)
    else if (key == 30) { // A
        key_p1_left = pressed;
    } else if (key == 32) { // D
        key_p1_right = pressed;
    } else if (key == 17) { // W (thrust)
        key_p1_thrust = pressed;
        if (pressed && needle.alive && needle.fuel > 0) play_synth_sound("thrust");
    } else if (key == 31 && pressed) { // S (fire)
        fire_missile(&needle, needle_missiles);
    } else if (key == 18 && pressed && needle.alive && needle.fuel >= 150) { // E (Hyperspace)
        needle.x = 50.0f + ((float)rand() / RAND_MAX) * 700.0f;
        needle.y = 50.0f + ((float)rand() / RAND_MAX) * 400.0f;
        needle.vx = (((float)rand() / RAND_MAX) - 0.5f) * 6.0f;
        needle.vy = (((float)rand() / RAND_MAX) - 0.5f) * 6.0f;
        needle.fuel -= 150;
        play_synth_sound("explode");
    }
    // Player 2 controls (Left, Right, Up, Down, Keycode 98 / KP_Slash for Hyperspace)
    else if (key == 105) { // Left arrow
        key_p2_left = pressed;
    } else if (key == 106) { // Right arrow
        key_p2_right = pressed;
    } else if (key == 103) { // Up arrow (thrust)
        key_p2_thrust = pressed;
        if (pressed && wedge.alive && wedge.fuel > 0) play_synth_sound("thrust");
    } else if (key == 108 && pressed) { // Down arrow (fire)
        fire_missile(&wedge, wedge_missiles);
    } else if (key == 98 && pressed && wedge.alive && wedge.fuel >= 150) { // KP_Slash / Hyperspace
        wedge.x = 50.0f + ((float)rand() / RAND_MAX) * 700.0f;
        wedge.y = 50.0f + ((float)rand() / RAND_MAX) * 400.0f;
        wedge.vx = (((float)rand() / RAND_MAX) - 0.5f) * 6.0f;
        wedge.vy = (((float)rand() / RAND_MAX) - 0.5f) * 6.0f;
        wedge.fuel -= 150;
        play_synth_sound("explode");
    } else if (key == 19 && pressed) { // R (restart)
        init_game();
    }
}

static void update_ship(Ship *s, bool left, bool right, bool thrust, float W, float H) {
    if (!s->alive) return;

    if (left) s->angle -= 0.06f;
    if (right) s->angle += 0.06f;

    // TSFI2 Prophecy: Quaternion orientation update reflecting 3D stance (Roll/Pitch/Yaw integration)
    // Rotate about Z axis with s->angle
    float cy = cosf(s->angle * 0.5f);
    float sy = sinf(s->angle * 0.5f);
    // Add small pitch/roll perturbations depending on thrust and current gravity gradient
    float pitch_angle = thrust ? 0.4f * sinf(s->x * 0.02f) : 0.2f * sinf(s->x * 0.01f);
    float cp = cosf(pitch_angle * 0.5f);
    float sp = sinf(pitch_angle * 0.5f);

    s->qw = cy * cp;
    s->qx = cy * sp;
    s->qy = sy * sp;
    s->qz = sy * cp;

    if (thrust && s->fuel > 0) {
        s->vx += cosf(s->angle) * 0.14f;
        s->vy += sinf(s->angle) * 0.14f;
        s->fuel--;
    }

    // Apply gravity well with TSFI2 Relativistic Frame Dragging (Lense-Thirring approximation)
    float dx = gravity_well_x - s->x;
    float dy = gravity_well_y - s->y;
    float dist2 = dx * dx + dy * dy;
    float dist = sqrtf(dist2);
    if (dist < 22.0f) {
        // Crash into the sun
        s->alive = 0;
        play_synth_sound("explode");
        return;
    }

    float gravity = G / (dist2 + 50.0f); // Soften denominator
    s->vx += (dx / dist) * gravity * 0.1f;
    s->vy += (dy / dist) * gravity * 0.1f;

    // TSFI2 Relativistic Solar Wind push (radiation pressure pushing away from gravity well)
    float wind_force = 120.0f / (dist2 + 100.0f);
    s->vx -= (dx / dist) * wind_force * 0.1f;
    s->vy -= (dy / dist) * wind_force * 0.1f;

    // TSFI2 Lense-Thirring Frame Dragging (orthogonal orbit rotation boost)
    float drag_strength = 200.0f / (dist2 + 200.0f);
    s->vx += (-dy / dist) * drag_strength * 0.1f;
    s->vy += (dx / dist) * drag_strength * 0.1f;

    s->x += s->vx;
    s->y += s->vy;

    // Screen wrapping with velocity dampening on warp bounds
    if (s->x < 0) { s->x += W; s->vx *= 0.98f; }
    if (s->x >= W) { s->x -= W; s->vx *= 0.98f; }
    if (s->y < 0) { s->y += H; s->vy *= 0.98f; }
    if (s->y >= H) { s->y -= H; s->vy *= 0.98f; }
}

static void update_missiles(Missile *list, int count, float W, float H) {
    for (int i = 0; i < count; i++) {
        if (!list[i].active) continue;

        // Apply gravity to missiles too
        float dx = gravity_well_x - list[i].x;
        float dy = gravity_well_y - list[i].y;
        float dist2 = dx * dx + dy * dy;
        float dist = sqrtf(dist2);
        if (dist < 15.0f) {
            list[i].active = 0;
            continue;
        }

        float gravity = G / (dist2 + 20.0f);
        list[i].vx += (dx / dist) * gravity * 0.1f;
        list[i].vy += (dy / dist) * gravity * 0.1f;

        list[i].x += list[i].vx;
        list[i].y += list[i].vy;
        list[i].life--;

        if (list[i].life <= 0) {
            list[i].active = 0;
            continue;
        }

        // Wrap around
        if (list[i].x < 0) list[i].x += W;
        if (list[i].x >= W) list[i].x -= W;
        if (list[i].y < 0) list[i].y += H;
        if (list[i].y >= H) list[i].y -= H;
    }
}

static void check_collisions(Ship *s1, Ship *s2, Missile *m_list, int m_count) {
    if (!s1->alive) return;
    for (int i = 0; i < m_count; i++) {
        if (!m_list[i].active) continue;
        float dx = s1->x - m_list[i].x;
        float dy = s1->y - m_list[i].y;
        if (dx * dx + dy * dy < 196.0f) { // Collision radius
            s1->alive = 0;
            s2->score++;
            m_list[i].active = 0;
            play_synth_sound("explode");
            break;
        }
    }
}

// Helper to rotate a 3D point using a Quaternion and project to 2D
static void project_point_3d(Ship *s, float px, float py, float pz, float *rx, float *ry) {
    // Quaternion rotation matrix columns
    float r11 = 1.0f - 2.0f * (s->qy * s->qy + s->qz * s->qz);
    float r12 = 2.0f * (s->qx * s->qy - s->qw * s->qz);
    // float r13 = 2.0f * (s->qx * s->qz + s->qw * s->qy);

    float r21 = 2.0f * (s->qx * s->qy + s->qw * s->qz);
    float r22 = 1.0f - 2.0f * (s->qx * s->qx + s->qz * s->qz);
    // float r23 = 2.0f * (s->qy * s->qz - s->qw * s->qx);

    *rx = s->x + (px * r11 + py * r12);
    *ry = s->y + (px * r21 + py * r22 + pz * 0.3f); // project some Z depth influence
}

// Draw the Needle ship (Player 1)
static void draw_needle(AB4HPixel *pixels, int W, int H, Ship *s, AB4HPixel col) {
    if (!s->alive) return;
    float hx, hy, w1x, w1y, w2x, w2y, cx, cy;
    project_point_3d(s, 16.0f, 0.0f, 0.0f, &hx, &hy);
    project_point_3d(s, -10.0f, 8.0f, -4.0f, &w1x, &w1y);
    project_point_3d(s, -10.0f, -8.0f, -4.0f, &w2x, &w2y);
    project_point_3d(s, -12.0f, 0.0f, 6.0f, &cx, &cy); // Cockpit ridge

    draw_line_aa(pixels, W, H, hx, hy, w1x, w1y, col, 1.5f);
    draw_line_aa(pixels, W, H, hx, hy, w2x, w2y, col, 1.5f);
    draw_line_aa(pixels, W, H, w1x, w1y, s->x, s->y, col, 1.5f);
    draw_line_aa(pixels, W, H, w2x, w2y, s->x, s->y, col, 1.5f);
    draw_line_aa(pixels, W, H, cx, cy, hx, hy, col, 1.0f);
    draw_line_aa(pixels, W, H, cx, cy, w1x, w1y, col, 1.0f);
    draw_line_aa(pixels, W, H, cx, cy, w2x, w2y, col, 1.0f);
}

// Draw the Wedge ship (Player 2)
static void draw_wedge(AB4HPixel *pixels, int W, int H, Ship *s, AB4HPixel col) {
    if (!s->alive) return;
    float hx, hy, w1x, w1y, w2x, w2y, cx, cy;
    project_point_3d(s, 16.0f, 0.0f, 0.0f, &hx, &hy);
    project_point_3d(s, -12.0f, 10.0f, -6.0f, &w1x, &w1y);
    project_point_3d(s, -12.0f, -10.0f, -6.0f, &w2x, &w2y);
    project_point_3d(s, -8.0f, 0.0f, 8.0f, &cx, &cy); // Spine ridge

    draw_line_aa(pixels, W, H, hx, hy, w1x, w1y, col, 1.5f);
    draw_line_aa(pixels, W, H, hx, hy, w2x, w2y, col, 1.5f);
    draw_line_aa(pixels, W, H, w1x, w1y, w2x, w2y, col, 1.5f);
    draw_line_aa(pixels, W, H, cx, cy, hx, hy, col, 1.0f);
    draw_line_aa(pixels, W, H, cx, cy, w1x, w1y, col, 1.0f);
    draw_line_aa(pixels, W, H, cx, cy, w2x, w2y, col, 1.0f);
}

typedef struct {
    AB4HPixel *pixels;
    int width;
    int height;
    int frame_counter;
} EvolveContext;

void space_war_evolve_impl(void *ctx, float intensity) {
    EvolveContext *dec = (EvolveContext*)ctx;
    AB4HPixel *pixels = dec->pixels;
    int W = dec->width;
    int H = dec->height;
    int frame = dec->frame_counter;
    (void)intensity;

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            AB4HPixel p = pixels[y * W + x];
            float r = half_to_float(p.r);
            float g = half_to_float(p.g);
            float b = half_to_float(p.b);
            float a = half_to_float(p.a);
            
            float input_noise = (float)x * 12.9898f + (float)y * 78.233f + (float)(frame % 100) * 0.01f;
            float noise = sinf(input_noise) * 43758.5453f;
            noise = noise - floorf(noise);
            
            if (a > 0.0f) {
                float noise_factor = 1.0f + (noise - 0.5f) * 0.03f;
                r *= noise_factor;
                g *= noise_factor;
                b *= noise_factor;
                pixels[y * W + x] = make_ab4h_pixel(r, g, b, a);
            }
        }
    }
}

int main() {
    printf("[SPACEWAR] Initializing Space War Vulkan/Wayland game runtime...\n");
    signal(SIGPIPE, SIG_IGN);
    setenv("TSFI_AB4H", "1", 1);

    VulkanSystem *s = create_vulkan_system();
    if (!s) return 1;
    lau_unseal_object(s);
    s->disable_ui_overlay = true;

    init_game();
    tsfi_input_set_key_hook((void*)key_hook);

    TSFiFontSystem *ai_fs = tsfi_font_init();
    if (ai_fs) {
        tsfi_font_ai_init(ai_fs);
    }

    AB4HPixel bg_space = make_ab4h_pixel(0.005f, 0.005f, 0.02f, 1.0f);
    AB4HPixel sun_color = make_ab4h_pixel(1.5f, 1.1f, 0.2f, 1.0f);
    AB4HPixel green_needle = make_ab4h_pixel(0.1f, 1.3f, 0.3f, 1.0f);
    AB4HPixel cyan_wedge = make_ab4h_pixel(0.0f, 0.9f, 1.4f, 1.0f);
    AB4HPixel orange_laser = make_ab4h_pixel(1.5f, 0.5f, 0.0f, 1.0f);
    AB4HPixel yellow_hud = make_ab4h_pixel(1.1f, 1.1f, 0.1f, 1.0f);

    int frame_counter = 0;

    while (s->running && !exit_requested) {
        wl_display_roundtrip(s->display);
        
        if (!s->vk->swapchain) {
            init_swapchain(s);
        }

        AB4HPixel *pixels = (AB4HPixel*)s->paint_buffer->data;
        int W = s->paint_buffer->width;
        int H = s->paint_buffer->height;

        // Scale factors for physical positions
        gravity_well_x = (float)W / 2.0f;
        gravity_well_y = (float)(H - 40) / 2.0f;

        // Clear Screen
        draw_rect_ab4h(pixels, W, H, 0, 0, W, H, bg_space);

        // Render Background stars with Auncient twinkling effects
        for (int i = 0; i < 45; i++) {
            int sx = (int)(fmodf((float)i * 187.3f, W));
            int sy = (int)(fmodf((float)i * 91.2f, H - 40));
            float twinkle = 0.3f + 0.7f * sinf(frame_counter * 0.05f + (float)i);
            draw_rect_ab4h(pixels, W, H, sx, sy, 2, 2, make_ab4h_pixel(twinkle, twinkle, twinkle, 0.6f * twinkle));
        }

        // Particle system for explosions and thrust debris
        typedef struct {
            float x, y, vx, vy;
            float life, max_life;
            AB4HPixel color;
            int active;
        } Particle;
        #define MAX_PARTICLES 128
        static Particle particles[MAX_PARTICLES];
        static int particles_init = 0;
        if (!particles_init) {
            memset(particles, 0, sizeof(particles));
            particles_init = 1;
        }

        // Update particles
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (particles[i].active) {
                particles[i].x += particles[i].vx;
                particles[i].y += particles[i].vy;
                particles[i].life -= 1.0f;
                if (particles[i].life <= 0) {
                    particles[i].active = 0;
                } else {
                    float ratio = particles[i].life / particles[i].max_life;
                    AB4HPixel col = particles[i].color;
                    col.a = float_to_half(half_to_float(col.a) * ratio);
                    draw_rect_ab4h(pixels, W, H, (int)particles[i].x, (int)particles[i].y, 2, 2, col);
                }
            }
        }

        // Update ships and missiles
        update_ship(&needle, key_p1_left, key_p1_right, key_p1_thrust, W, H - 40);
        update_ship(&wedge, key_p2_left, key_p2_right, key_p2_thrust, W, H - 40);

        update_missiles(needle_missiles, MAX_MISSILES, W, H - 40);
        update_missiles(wedge_missiles, MAX_MISSILES, W, H - 40);

        // Collisions and spawn particles on explosion
        int needle_was_alive = needle.alive;
        int wedge_was_alive = wedge.alive;
        check_collisions(&needle, &wedge, wedge_missiles, MAX_MISSILES);
        check_collisions(&wedge, &needle, needle_missiles, MAX_MISSILES);

        if (needle_was_alive && !needle.alive) {
            for (int k = 0; k < 30; k++) {
                for (int i = 0; i < MAX_PARTICLES; i++) {
                    if (!particles[i].active) {
                        particles[i].active = 1;
                        particles[i].x = needle.x;
                        particles[i].y = needle.y;
                        float p_angle = ((float)rand() / RAND_MAX) * 2.0f * 3.14159f;
                        float speed = 1.0f + 4.0f * ((float)rand() / RAND_MAX);
                        particles[i].vx = cosf(p_angle) * speed;
                        particles[i].vy = sinf(p_angle) * speed;
                        particles[i].max_life = 30.0f + rand() % 30;
                        particles[i].life = particles[i].max_life;
                        particles[i].color = green_needle;
                        break;
                    }
                }
            }
        }
        if (wedge_was_alive && !wedge.alive) {
            for (int k = 0; k < 30; k++) {
                for (int i = 0; i < MAX_PARTICLES; i++) {
                    if (!particles[i].active) {
                        particles[i].active = 1;
                        particles[i].x = wedge.x;
                        particles[i].y = wedge.y;
                        float p_angle = ((float)rand() / RAND_MAX) * 2.0f * 3.14159f;
                        float speed = 1.0f + 4.0f * ((float)rand() / RAND_MAX);
                        particles[i].vx = cosf(p_angle) * speed;
                        particles[i].vy = sinf(p_angle) * speed;
                        particles[i].max_life = 30.0f + rand() % 30;
                        particles[i].life = particles[i].max_life;
                        particles[i].color = cyan_wedge;
                        break;
                    }
                }
            }
        }

        // Draw Central Star (with dynamic Coronal Flare)
        float flare_r = 45.0f + 10.0f * sinf(frame_counter * 0.15f);
        draw_radial_glow(pixels, W, H, gravity_well_x, gravity_well_y, flare_r, make_ab4h_pixel(1.8f, 0.6f, 0.05f, 0.5f));
        draw_radial_glow(pixels, W, H, gravity_well_x, gravity_well_y, 25.0f, make_ab4h_pixel(2.0f, 1.2f, 0.1f, 0.8f));
        draw_rect_ab4h(pixels, W, H, (int)gravity_well_x - 6, (int)gravity_well_y - 6, 12, 12, sun_color);

        // Thruster trails & Spawn thrust fire particles
        if (key_p1_thrust && needle.alive && needle.fuel > 0) {
            float tx = needle.x - cosf(needle.angle) * 12.0f;
            float ty = needle.y - sinf(needle.angle) * 12.0f;
            draw_radial_glow(pixels, W, H, tx, ty, 15.0f, make_ab4h_pixel(0.2f, 1.8f, 0.4f, 0.8f));
            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (!particles[i].active) {
                    particles[i].active = 1;
                    particles[i].x = tx;
                    particles[i].y = ty;
                    float scatter = needle.angle + 3.14159f + (((float)rand() / RAND_MAX) - 0.5f) * 0.6f;
                    float speed = 1.0f + 2.0f * ((float)rand() / RAND_MAX);
                    particles[i].vx = cosf(scatter) * speed + needle.vx * 0.5f;
                    particles[i].vy = sinf(scatter) * speed + needle.vy * 0.5f;
                    particles[i].max_life = 10.0f + rand() % 10;
                    particles[i].life = particles[i].max_life;
                    particles[i].color = make_ab4h_pixel(0.8f, 1.5f, 0.2f, 0.8f);
                    break;
                }
            }
        }
        if (key_p2_thrust && wedge.alive && wedge.fuel > 0) {
            float tx = wedge.x - cosf(wedge.angle) * 12.0f;
            float ty = wedge.y - sinf(wedge.angle) * 12.0f;
            draw_radial_glow(pixels, W, H, tx, ty, 15.0f, make_ab4h_pixel(0.1f, 0.8f, 1.8f, 0.8f));
            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (!particles[i].active) {
                    particles[i].active = 1;
                    particles[i].x = tx;
                    particles[i].y = ty;
                    float scatter = wedge.angle + 3.14159f + (((float)rand() / RAND_MAX) - 0.5f) * 0.6f;
                    float speed = 1.0f + 2.0f * ((float)rand() / RAND_MAX);
                    particles[i].vx = cosf(scatter) * speed + wedge.vx * 0.5f;
                    particles[i].vy = sinf(scatter) * speed + wedge.vy * 0.5f;
                    particles[i].max_life = 10.0f + rand() % 10;
                    particles[i].life = particles[i].max_life;
                    particles[i].color = make_ab4h_pixel(0.2f, 0.8f, 1.5f, 0.8f);
                    break;
                }
            }
        }

        // Draw Ships and Missiles
        draw_needle(pixels, W, H, &needle, green_needle);
        draw_wedge(pixels, W, H, &wedge, cyan_wedge);

        // Low Fuel Warning HUD alert pulsing
        if (needle.alive && needle.fuel < 200 && (frame_counter % 30 < 15)) {
            draw_radial_glow(pixels, W, H, needle.x, needle.y, 25.0f, make_ab4h_pixel(2.0f, 0.0f, 0.0f, 0.3f));
        }
        if (wedge.alive && wedge.fuel < 200 && (frame_counter % 30 < 15)) {
            draw_radial_glow(pixels, W, H, wedge.x, wedge.y, 25.0f, make_ab4h_pixel(2.0f, 0.0f, 0.0f, 0.3f));
        }

        // Draw Missiles
        for (int i = 0; i < MAX_MISSILES; i++) {
            if (needle_missiles[i].active) {
                draw_radial_glow(pixels, W, H, needle_missiles[i].x, needle_missiles[i].y, 8.0f, orange_laser);
                draw_rect_ab4h(pixels, W, H, (int)needle_missiles[i].x - 2, (int)needle_missiles[i].y - 2, 4, 4, make_ab4h_pixel(2.0f, 2.0f, 1.0f, 1.0f));
            }
            if (wedge_missiles[i].active) {
                draw_radial_glow(pixels, W, H, wedge_missiles[i].x, wedge_missiles[i].y, 8.0f, orange_laser);
                draw_rect_ab4h(pixels, W, H, (int)wedge_missiles[i].x - 2, (int)wedge_missiles[i].y - 2, 4, 4, make_ab4h_pixel(2.0f, 2.0f, 1.0f, 1.0f));
            }
        }

        // HUD Status bar
        char hud_buf[256];
        sprintf(hud_buf, "^B[NEEDLE]^C P1: %d | F: %d  vs  ^B[WEDGE]^C P2: %d | F: %d   ^I[R] Reset^C",
                needle.score, needle.fuel, wedge.score, wedge.fuel);

        draw_rect_ab4h(pixels, W, H, 0, H - 40, W, 40, make_ab4h_pixel(0.01f, 0.01f, 0.05f, 1.0f));
        draw_line_aa(pixels, W, H, 0.0f, (float)(H - 40), (float)W, (float)(H - 40), sun_color, 1.5f);
        draw_string_ab4h(pixels, W, H, hud_buf, 20, H - 30, yellow_hud);

        // post processing
        if (ai_fs) {
            EvolveContext dec = {
                .pixels = pixels,
                .width = W,
                .height = H,
                .frame_counter = frame_counter
            };
            tsfi_font_ai_bind_evolve(ai_fs, space_war_evolve_impl, &dec);
            float current_intensity = 0.5f + 0.5f * sinf(frame_counter * 0.1f);
            tsfi_font_ai_invoke_evolve(ai_fs, current_intensity);
        }

        // Render Frame
        draw_frame(s);
        frame_counter++;
        tsfi_raw_usleep(16000);
    }

    printf("[SPACEWAR] Tearing down space war Vulkan structures...\n");
    lau_unseal_object(s);
    if (ai_fs) {
        tsfi_font_ai_destroy(ai_fs);
        tsfi_font_destroy(ai_fs);
    }
    destroy_vulkan_system(s);
    extern _Atomic int g_teardown_in_progress;
    g_teardown_in_progress = 1;
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_free_all_active(void);
    lau_free_all_active();
    return 0;
}
