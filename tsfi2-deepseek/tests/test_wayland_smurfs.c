#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "vulkan/vulkan_main.h"
#include "vulkan/vulkan_render.h"
#include "tsfi_raw.h"
#include "lau_yul_thunk.h"
#include "tsfi_staging.h"
#include "tsfi_font_vectors.h"
#include "tsfi_font_ai.h"

// Font helper removed to render all glyphs via DeepSeek system font loading
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma GCC diagnostic pop

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

typedef struct { float x, y, z; } V3;
static inline V3 v3(float x, float y, float z) { V3 r = {x, y, z}; return r; }
static inline V3 v3_add(V3 a, V3 b) { return v3(a.x+b.x, a.y+b.y, a.z+b.z); }
static inline V3 v3_sub(V3 a, V3 b) { return v3(a.x-b.x, a.y-b.y, a.z-b.z); }
static inline V3 v3_mul(V3 a, float s) { return v3(a.x*s, a.y*s, a.z*s); }
static inline float v3_dot(V3 a, V3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
static inline float v3_len(V3 a) { return sqrtf(v3_dot(a, a)); }
static inline V3 v3_norm(V3 a) { float l = v3_len(a); return l > 0.0f ? v3_mul(a, 1.0f/l) : v3(0,0,0); }


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
    // We can bridge our AB4HPixel surface to StagingBuffer to use the system's Liberation Mono TTF glyphs
    StagingBuffer sb = {
        .magic = TSFI_STAGING_MAGIC,
        .data = pixels,
        .width = w,
        .height = h,
        .stride = w * 8, // AB4H has 8 bytes per pixel (16-bit half float * 4 channels)
        .size = w * h * 8
    };
    (void)sb;

    // Prepare a temporary buffer for drawing the codepoint using draw_debug_codepoint (which outputs 32-bit RGBA)
    // 32x32 pixels is more than enough for our font size.
    uint32_t temp_buf[32 * 32] = {0};
    StagingBuffer temp_sb = {
        .magic = TSFI_STAGING_MAGIC,
        .data = temp_buf,
        .width = 32,
        .height = 32,
        .stride = 32 * 4,
        .size = 32 * 32 * 4
    };

    // Draw green/white placeholder first to extract alpha from it
    draw_debug_codepoint(&temp_sb, 0, 0, (uint32_t)c, 0xFFFFFFFF);

    float tr = half_to_float(color.r);
    float tg = half_to_float(color.g);
    float tb = half_to_float(color.b);
    float ta = half_to_float(color.a);

    // Blit onto the AB4HPixel buffer using correct alpha blending
    for (int j = 0; j < 32; j++) {
        int py = y - 4 + j; // Shift slightly to align baseline
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
                current_color = make_ab4h_pixel(1.0f, 0.05f, 0.6f, 1.0f); // neon_pink style
                str += 2;
                continue;
            } else if (code == 'U' || code == 'u') {
                current_color = make_ab4h_pixel(0.0f, 0.9f, 1.0f, 1.0f); // neon_cyan style
                str += 2;
                continue;
            } else if (code == 'C' || code == 'c') {
                current_color = default_color;
                str += 2;
                continue;
            }
        }
        draw_char_ab4h(pixels, w, h, *str, x, y, current_color);
        x += 12; // Adjusted spacing for the high-quality vector/TTF mono font
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

// Game State Definitions
static int energy = 100;
static int score = 0;
static int game_screen = 1;
static float smurf_x = 100.0f;
static float smurf_y = 500.0f;
static bool smurf_jumping = false;
static float crow_x = 400.0f;
static float crow_y = 250.0f;
static float gargamel_x = 500.0f;
static bool game_over = false;
static bool game_win = false;
static char status_message[128] = "WELCOME TO TEDDY BEAR RESCUE! [SPACE] TO JUMP. [A/D] MOVE.";
static int exit_requested = 0;
static bool key_a_held = false;
static bool key_d_held = false;

// kr0wZ glitch damage burst particles
#define MAX_PARTICLES 64
struct GlitchParticle {
    float x, y;
    float vx, vy;
    float alpha;
    bool active;
};
static struct GlitchParticle particles[MAX_PARTICLES];
static int screen_shake_timer = 0;
static float shake_dx = 0.0f;
static float shake_dy = 0.0f;
static int prev_energy = 100;

static uint64_t thunk_peek(uint64_t addr);

// virtual C64 VIC-II Registers (inspired by Sally Greenwood Larsen's Sprite Graphics for C64)
static uint8_t vic_sprite_x[8];       // $D000 - Sprite X Coordinates
static uint8_t vic_sprite_y[8];       // $D001 - Sprite Y Coordinates
static uint8_t vic_sprite_msb = 0;    // $D010 - Sprite X MSB (9th bit)
static uint8_t vic_sprite_enable = 0; // $D015 - Sprite Enable Register
static uint8_t vic_sprite_y_expand = 0; // $D017 - Sprite Y Expansion
static uint8_t vic_sprite_priority = 0; // $D01B - Sprite priority (0 = Sprite in front, 1 = Background in front)
static uint8_t vic_sprite_multicolor = 0; // $D01C - Multicolor Mode Register
static uint8_t vic_sprite_x_expand = 0; // $D01D - Sprite X Expansion
static uint8_t vic_sprite_collision = 0; // $D01E - Sprite-to-Sprite Collision Strobe
static uint8_t vic_sprite_pointer[8];  // Addresses 2040-2047: Sprite Data Pointers (pointing to 64-byte chunks)

static void sync_c64_vic_registers(int frame_counter) {
    // Sprite 0: Teddy Bear (Player)
    int sx = (int)smurf_x;
    int sy = (int)smurf_y;
    vic_sprite_x[0] = sx & 0xFF;
    if (sx > 255) vic_sprite_msb |= (1 << 0);
    else vic_sprite_msb &= ~(1 << 0);
    vic_sprite_y[0] = sy & 0xFF;

    // Sprite 1: Crow
    int cx = (int)crow_x;
    int cy = (int)crow_y;
    vic_sprite_x[1] = cx & 0xFF;
    if (cx > 255) vic_sprite_msb |= (1 << 1);
    else vic_sprite_msb &= ~(1 << 1);
    vic_sprite_y[1] = cy & 0xFF;

    // Sprite 2: Gargamel
    int gx = (int)gargamel_x;
    int gy = 500;
    vic_sprite_x[2] = gx & 0xFF;
    if (gx > 255) vic_sprite_msb |= (1 << 2);
    else vic_sprite_msb &= ~(1 << 2);
    vic_sprite_y[2] = gy & 0xFF;

    // Enable/collision mirroring
    vic_sprite_enable = 0x07; // Sprites 0, 1, 2 enabled
    vic_sprite_collision = (thunk_peek(55046) != 0) ? 0x01 : 0x00;

    // Multicolor register mapping: our procedural wireframe sprites utilize multiple colors
    vic_sprite_multicolor = 0x07; // Sprites 0, 1, 2 set to C64 multicolor mode

    // Priority register: player is behind obstacles like fences/stalagmites, mimicking background priority
    vic_sprite_priority = 0x01; // Sprite 0 (player) has background priority active

    // Mirror sprite pointers based on active animation frames to model page-flipping
    int anim_state = (int)thunk_peek(55047);
    int frame_idx = (frame_counter / 6) % 4; // approximate ticks per frame
    
    // Assign C64 memory offsets (multiplied by 64 bytes to locate data)
    vic_sprite_pointer[0] = 32 + anim_state * 4 + frame_idx; // Player frames start at block 32
    vic_sprite_pointer[1] = 64 + frame_idx;                  // Crow frames start at block 64
    vic_sprite_pointer[2] = 80 + frame_idx;                  // Gargamel frames start at block 80

    // Scaling registers mirroring C64 Sprite Expansion registers ($D017/$D01D)
    int scale_reg = (int)thunk_peek(55045);
    if (scale_reg == 1) {
        vic_sprite_x_expand = 0x07; // Expanded width
        vic_sprite_y_expand = 0x07; // Expanded height
    } else {
        vic_sprite_x_expand = 0;
        vic_sprite_y_expand = 0;
    }
}

struct SoundData {
    char type[32];
};

__attribute__((unused)) static void* play_sound_thread(void *arg) {
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
    if (strcmp(sd->type, "jump") == 0) {
        len = 1600; buf = malloc(len);
        if (buf) {
            for (int i = 0; i < len; i++) {
                float t = (float)i / 8000.0f;
                float freq = 200.0f + (t / 0.2f) * 600.0f;
                float phase = freq * t * 2.0f * 3.14159265f;
                buf[i] = 128 + (int)((1.0f - t/0.2f) * 110.0f * sinf(phase));
            }
        }
    } else if (strcmp(sd->type, "hit") == 0) {
        len = 2400; buf = malloc(len);
        if (buf) {
            for (int i = 0; i < len; i++) {
                float t = (float)i / 8000.0f;
                float freq = 120.0f - (t / 0.3f) * 50.0f;
                float phase = freq * t * 2.0f * 3.14159265f;
                buf[i] = 128 + (int)((1.0f - t/0.3f) * 120.0f * (sinf(phase) + ((rand()%100)-50.0f)/100.0f)*0.5f);
            }
        }
    } else if (strcmp(sd->type, "win") == 0) {
        len = 6400; buf = malloc(len);
        if (buf) {
            for (int i = 0; i < len; i++) {
                float t = (float)i / 8000.0f;
                int note = (int)(t * 8.0f);
                float freqs[8] = {261.63f, 329.63f, 392.00f, 523.25f, 659.25f, 783.99f, 1046.50f, 1046.50f};
                float phase = freqs[note % 8] * t * 2.0f * 3.14159265f;
                buf[i] = 128 + (int)((1.0f - fmodf(t * 8.0f, 1.0f)) * 100.0f * sinf(phase));
            }
        }
    } else if (strcmp(sd->type, "gameover") == 0) {
        len = 4000; buf = malloc(len);
        if (buf) {
            for (int i = 0; i < len; i++) {
                float t = (float)i / 8000.0f;
                float freq = 300.0f - (t / 0.5f) * 200.0f;
                float phase = freq * t * 2.0f * 3.14159265f;
                buf[i] = 128 + (int)((1.0f - t/0.5f) * 110.0f * sinf(phase));
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

// Yul VM Interaction Thunks
static void thunk_poke(uint64_t addr, uint64_t val) {
    uint8_t calldata[4 + 32 + 32] = {0};
    calldata[0] = 0x80; calldata[1] = 0x29; calldata[2] = 0xe7; calldata[3] = 0xc0;
    for (int i = 0; i < 8; i++) {
        calldata[4 + 31 - i] = (addr >> (i * 8)) & 0xFF;
    }
    for (int i = 0; i < 8; i++) {
        calldata[4 + 32 + 31 - i] = (val >> (i * 8)) & 0xFF;
    }
    uint8_t retval[32];
    size_t retval_len = 32;
    lau_yul_thunk_execute("cpu6502", calldata, 4 + 32 + 32, retval, &retval_len);
}

static uint64_t thunk_peek(uint64_t addr) {
    uint8_t calldata[4 + 32] = {0};
    calldata[0] = 0x78; calldata[1] = 0x61; calldata[2] = 0xd2; calldata[3] = 0x69;
    for (int i = 0; i < 8; i++) {
        calldata[4 + 31 - i] = (addr >> (i * 8)) & 0xFF;
    }
    uint8_t retval[32] = {0};
    size_t retval_len = 32;
    lau_yul_thunk_execute("cpu6502", calldata, 4 + 32, retval, &retval_len);
    uint64_t res = 0;
    for (int i = 0; i < 8; i++) {
        res |= ((uint64_t)retval[31 - i]) << (i * 8);
    }
    return res;
}

static void reset_game_yul() {
    thunk_poke(55027, 100); // px
    thunk_poke(55028, 500); // py
    thunk_poke(55029, 0);   // pvy
    thunk_poke(55030, 0);   // jumping
    thunk_poke(55031, 999); // energy
    thunk_poke(55032, 0);   // score
    thunk_poke(55033, 1);   // screen
    thunk_poke(55034, 0);   // isGameOver
    thunk_poke(55035, 0);   // isGameWin
    thunk_poke(55036, 0);   // soundStrobe
    thunk_poke(55037, 400); // crow_x
    thunk_poke(55038, 250); // crow_y
    thunk_poke(55039, 252);  // crow_vx (-4 in 8-bit)
    thunk_poke(55040, 500); // gargamel_x
    thunk_poke(55041, 253);  // gargamel_vx (-3 in 8-bit)
    thunk_poke(55043, 0);   // physical trauma (0 = Normal, 1 = Exhausted, 2 = Battered, 3 = Broken)
    thunk_poke(55044, 0);   // mental trauma (0 = Normal, 1 = Shaken, 2 = Terrified, 3 = Panicked)
    thunk_poke(55045, 0);   // sprite scaling (0 = Normal, 1 = Giant, 2 = Tiny)
    thunk_poke(55046, 0);   // sprite collision strobe
    thunk_poke(55047, 0);   // sprite animation state (0 = Idle, 1 = Walk, 2 = Jump, 3 = Wounded, 4 = Panicked)
    thunk_poke(55066, 50);  // stalactite y coordinate
    thunk_poke(55067, 0);   // stalactite y velocity
    prev_energy = 100;
}

void tsfi_input_set_key_hook(void *hook_func);

static bool ai_mode = false;

static void smurfs_key_hook(void *data, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    (void)data; (void)serial; (void)time;
    bool pressed = (state != 0);

    if (key == 1) { // ESC
        exit_requested = 1;
    } else if (key == 30 || key == 105) { // A or LEFT
        key_a_held = pressed;
    } else if (key == 32 || key == 106) { // D or RIGHT
        key_d_held = pressed;
    } else if (key == 57 || key == 103 || key == 17) { // SPACE or UP or W
        if (pressed && !game_over && !game_win) {
            // Check if player is Broken (Physical Trauma == 3) in 6502 PC RAM
            if (thunk_peek(55043) != 3) {
                int jumpCount = (int)thunk_peek(55065);
                if (jumpCount < 2) {
                    thunk_poke(55026, 1); // Trigger Jump in Yul
                }
            }
        }
    } else if (key == 19) { // R
        if (pressed && (game_over || game_win)) {
            reset_game_yul();
            strcpy(status_message, "GAME RESTARTED! SAVE BABY BEAR!");
        }
    } else if (key == 20) { // T (Tramiel Autopilot Toggle)
        if (pressed && !game_over && !game_win) {
            ai_mode = !ai_mode;
            if (ai_mode) {
                printf("[SMURFS] \"COMPUTERS FOR THE MASSES, NOT THE CLASSES\" - Tramiel Autopilot Engaged!\n");
                strcpy(status_message, "^B^U[TRAMIEL MODE]^C ^IBUSINESS IS WAR! AUTOPILOT ENGAGED!^C");
            } else {
                printf("[SMURFS] Autopilot disengaged.\n");
                strcpy(status_message, "^B[MANUAL]^C ^IYOU HAVE CONTROL!^C");
            }
        }
    }
}


static AB4HPixel *g_forest_bg = NULL;
static AB4HPixel *g_cavern_bg = NULL;
static AB4HPixel *g_castle_bg = NULL;
static AB4HPixel *g_sprite_sheet = NULL;
static int g_sprite_sheet_w = 0;
static int g_sprite_sheet_h = 0;

static void draw_sprite_blitted(AB4HPixel *dest, int dest_w, int dest_h, 
                               int draw_x, int draw_y, float scale,
                               int src_x, int src_y, int src_w, int src_h,
                               bool flip_h);

// Sprite frame structure
typedef struct {
    int x, y, w, h;
} SpriteFrame;

// Sprite animation structure
typedef struct {
    SpriteFrame frames[8];
    int frame_count;
    int ticks_per_frame;
    bool loops;
} SpriteAnimation;

// Entity sprite animations grouping
typedef enum {
    ANIM_IDLE = 0,
    ANIM_WALK = 1,
    ANIM_JUMP = 2,
    ANIM_WOUNDED = 3,
    ANIM_PANIC = 4,
    ANIM_STATE_MAX
} AnimState;

typedef struct {
    SpriteAnimation animations[ANIM_STATE_MAX];
} EntitySpriteSet;

static EntitySpriteSet g_smurf_sprites;
static EntitySpriteSet g_crow_sprites;
static EntitySpriteSet g_gargamel_sprites;

static void init_sprite_system() {
    // 1. Smurf Sprite Animations
    // IDLE
    g_smurf_sprites.animations[ANIM_IDLE] = (SpriteAnimation){
        .frames = {{.x = 0 + 10, .y = 360 + 10, .w = 90, .h = 90}},
        .frame_count = 1,
        .ticks_per_frame = 1,
        .loops = true
    };
    // WALK
    g_smurf_sprites.animations[ANIM_WALK] = (SpriteAnimation){
        .frames = {
            {.x = 0 * 110 + 10, .y = 360 + 10, .w = 90, .h = 90},
            {.x = 1 * 110 + 10, .y = 360 + 10, .w = 90, .h = 90},
            {.x = 2 * 110 + 10, .y = 360 + 10, .w = 90, .h = 90},
            {.x = 3 * 110 + 10, .y = 360 + 10, .w = 90, .h = 90}
        },
        .frame_count = 4,
        .ticks_per_frame = 5,
        .loops = true
    };
    // JUMP
    g_smurf_sprites.animations[ANIM_JUMP] = (SpriteAnimation){
        .frames = {{.x = 5 * 110 + 10, .y = 360 + 10, .w = 90, .h = 90}},
        .frame_count = 1,
        .ticks_per_frame = 1,
        .loops = true
    };
    // WOUNDED
    g_smurf_sprites.animations[ANIM_WOUNDED] = (SpriteAnimation){
        .frames = {{.x = 7 * 110 + 10, .y = 360 + 10, .w = 90, .h = 90}},
        .frame_count = 1,
        .ticks_per_frame = 1,
        .loops = true
    };
    // PANIC
    g_smurf_sprites.animations[ANIM_PANIC] = (SpriteAnimation){
        .frames = {{.x = 4 * 110 + 10, .y = 360 + 10, .w = 90, .h = 90}},
        .frame_count = 1,
        .ticks_per_frame = 1,
        .loops = true
    };

    // 2. Crow Sprite Animations
    // FLY/WALK
    g_crow_sprites.animations[ANIM_WALK] = (SpriteAnimation){
        .frames = {
            {.x = 0 * 110 + 10, .y = 50 + 10, .w = 90, .h = 90},
            {.x = 1 * 110 + 10, .y = 50 + 10, .w = 90, .h = 90},
            {.x = 2 * 110 + 10, .y = 50 + 10, .w = 90, .h = 90},
            {.x = 0 * 110 + 10, .y = 160 + 10, .w = 90, .h = 90}
        },
        .frame_count = 4,
        .ticks_per_frame = 6,
        .loops = true
    };
    g_crow_sprites.animations[ANIM_IDLE] = g_crow_sprites.animations[ANIM_WALK];

    // 3. Gargamel Sprite Animations
    // WALK
    g_gargamel_sprites.animations[ANIM_WALK] = (SpriteAnimation){
        .frames = {
            {.x = 0 * 110 + 10, .y = 670 + 10, .w = 90, .h = 90},
            {.x = 1 * 110 + 10, .y = 670 + 10, .w = 90, .h = 90},
            {.x = 2 * 110 + 10, .y = 670 + 10, .w = 90, .h = 90},
            {.x = 3 * 110 + 10, .y = 670 + 10, .w = 90, .h = 90}
        },
        .frame_count = 4,
        .ticks_per_frame = 8,
        .loops = true
    };
    g_gargamel_sprites.animations[ANIM_IDLE] = g_gargamel_sprites.animations[ANIM_WALK];
}

static void draw_sprite_frame(AB4HPixel *dest, int dest_w, int dest_h,
                             int draw_x, int draw_y, float scale,
                             EntitySpriteSet *set, AnimState state, int frame_counter,
                             bool flip_h) {
    if (state >= ANIM_STATE_MAX) state = ANIM_IDLE;
    SpriteAnimation *anim = &set->animations[state];
    if (anim->frame_count == 0) return;
    
    int frame_idx = (frame_counter / anim->ticks_per_frame) % anim->frame_count;
    SpriteFrame *frame = &anim->frames[frame_idx];
    
    draw_sprite_blitted(dest, dest_w, dest_h, draw_x, draw_y, scale,
                        frame->x, frame->y, frame->w, frame->h, flip_h);
}

static AB4HPixel *load_png_ab4h(const char *filename, int *out_w, int *out_h) {
    int w, h, channels;
    uint8_t *img_data = stbi_load(filename, &w, &h, &channels, 4);
    if (!img_data) {
        printf("[ERROR] Failed to load image: %s\n", filename);
        return NULL;
    }
    AB4HPixel *pixels = malloc(w * h * sizeof(AB4HPixel));
    for (int i = 0; i < w * h; i++) {
        float r = img_data[i * 4 + 0] / 255.0f;
        float g = img_data[i * 4 + 1] / 255.0f;
        float b = img_data[i * 4 + 2] / 255.0f;
        float a = img_data[i * 4 + 3] / 255.0f;
        pixels[i] = make_ab4h_pixel(r, g, b, a);
    }
    stbi_image_free(img_data);
    if (out_w) *out_w = w;
    if (out_h) *out_h = h;
    printf("[LOADER] Loaded texture %s (%dx%d)\n", filename, w, h);
    return pixels;
}

static void draw_texture_bg(AB4HPixel *dest, int dest_w, int dest_h, AB4HPixel *src) {
    if (!src) return;
    float scroll_pct = smurf_x / 800.0f;
    if (scroll_pct < 0.0f) scroll_pct = 0.0f;
    if (scroll_pct > 1.0f) scroll_pct = 1.0f;
    float scroll_x = scroll_pct * 224.0f;

    for (int y = 50; y < dest_h - 40; y++) {
        int src_y = (y * 1024) / dest_h;
        if (src_y < 0) src_y = 0;
        if (src_y >= 1024) src_y = 1023;
        for (int x = 0; x < dest_w; x++) {
            float logical_x = (x * 800.0f) / dest_w;
            int src_x = (int)(scroll_x + logical_x);
            if (src_x < 0) src_x = 0;
            if (src_x >= 1024) src_x = 1023;
            dest[y * dest_w + x] = src[src_y * 1024 + src_x];
        }
    }
}

static void draw_sprite_blitted(AB4HPixel *dest, int dest_w, int dest_h, 
                               int draw_x, int draw_y, float scale,
                               int src_x, int src_y, int src_w, int src_h,
                               bool flip_h) {
    if (!g_sprite_sheet) return;
    int sw = (int)(src_w * scale);
    int sh = (int)(src_h * scale);
    int min_dx = draw_x - sw / 2;
    int min_dy = draw_y - sh;

    for (int y = 0; y < sh; y++) {
        int dy = min_dy + y;
        if (dy < 50 || dy >= dest_h - 40) continue;
        for (int x = 0; x < sw; x++) {
            int dx = min_dx + x;
            if (dx < 0 || dx >= dest_w) continue;

            int sx_offset = flip_h ? (sw - 1 - x) : x;
            int sx = src_x + (int)(sx_offset / scale);
            int sy = src_y + (int)(y / scale);

            if (sx >= 0 && sx < g_sprite_sheet_w && sy >= 0 && sy < g_sprite_sheet_h) {
                AB4HPixel sp = g_sprite_sheet[sy * g_sprite_sheet_w + sx];
                float sa = half_to_float(sp.a);
                if (sa > 0.05f) {
                    AB4HPixel dp = dest[dy * dest_w + dx];
                    float dr = half_to_float(dp.r), dg = half_to_float(dp.g), db = half_to_float(dp.b), da = half_to_float(dp.a);
                    float sr = half_to_float(sp.r), sg = half_to_float(sp.g), sb = half_to_float(sp.b);

                    // --- Spiggle Shader Highlight Effect ---
                    // Generates dynamic angle-of-incidence refraction gloss at sprite boundaries.
                    float norm_x = ((float)x / sw) - 0.5f;
                    float norm_y = ((float)y / sh) - 0.5f;
                    float dist_from_center = sqrtf(norm_x * norm_x + norm_y * norm_y);
                    // Edge Fresnel approximation: brighter highlights on the outer contours
                    float spiggle_edge_fresnel = powf(fmaxf(0.0f, dist_from_center * 2.0f), 3.0f);
                    
                    // Simple light source offset reflection (specular highlight)
                    float spec_glow = fmaxf(0.0f, norm_x * 0.707f - norm_y * 0.707f);
                    float spiggle_glow = powf(spec_glow, 4.0f) * spiggle_edge_fresnel * 0.35f;

                    float nr = dr * (1.0f - sa) + (sr + spiggle_glow) * sa;
                    float ng = dg * (1.0f - sa) + (sg + spiggle_glow) * sa;
                    float nb = db * (1.0f - sa) + (sb + spiggle_glow) * sa;
                    dest[dy * dest_w + dx] = make_ab4h_pixel(nr, ng, nb, da);
                }
            }
        }
    }
}


typedef struct {
    AB4HPixel *pixels;
    int width;
    int height;
    int frame_counter;
} DeepSeekEvolveContext;

void deepseek_evolve_impl(void *ctx, float intensity) {
    DeepSeekEvolveContext *dec = (DeepSeekEvolveContext*)ctx;
    AB4HPixel *pixels = dec->pixels;
    int W = dec->width;
    int H = dec->height;
    int frame = dec->frame_counter;
    (void)intensity;

    // Apply Soft Diffusion texture noise to all pixels that represent backgrounds/hills
    // Using fract(sin()) fixed-point float noise
    for (int y = 50; y < H - 42; y++) {
        for (int x = 0; x < W; x++) {
            AB4HPixel p = pixels[y * W + x];
            float r = half_to_float(p.r);
            float g = half_to_float(p.g);
            float b = half_to_float(p.b);
            float a = half_to_float(p.a);
            
            // Soft Diffusion texture modulation
            // Generate deterministic noise based on coordinates and frame
            float input_noise = (float)x * 12.9898f + (float)y * 78.233f + (float)(frame % 100) * 0.01f;
            float noise = sinf(input_noise) * 43758.5453f;
            noise = noise - floorf(noise); // fract
            
            // Apply a subtle 3% soft diffusion noise to backgrounds to give an organic, high-end look
            if (a > 0.0f) {
                float noise_factor = 1.0f + (noise - 0.5f) * 0.05f;
                r *= noise_factor;
                g *= noise_factor;
                b *= noise_factor;
                pixels[y * W + x] = make_ab4h_pixel(r, g, b, a);
            }
        }
    }
}

int main(int argc, char **argv) {
    printf("[SMURFS] Launching C64 Smurfs Rescue Manifold...\n");
    signal(SIGPIPE, SIG_IGN);
    setenv("TSFI_AB4H", "1", 1);

    bool headless = false;
    int headless_frames = 1;
    for (int idx = 1; idx < argc; idx++) {
        if (strcmp(argv[idx], "--headless") == 0 || strcmp(argv[idx], "--render-once") == 0) {
            headless = true;
        }
        if (strcmp(argv[idx], "--headless-frames") == 0 && idx + 1 < argc) {
            headless = true;
            headless_frames = atoi(argv[idx + 1]);
        }
    }

    const char *run_env = getenv("XDG_RUNTIME_DIR");
    const char *disp_env = getenv("WAYLAND_DISPLAY");
    if (!run_env || !disp_env) {
        headless = true;
    }

    VulkanSystem *s = NULL;
    VulkanSystem mock_s;
    StagingBuffer mock_buf;
    AB4HPixel *offscreen_data = NULL;

    if (!headless) {
        s = create_vulkan_system();
        if (s) {
            lau_unseal_object(s);
            printf("[SMURFS] Vulkan System initialized. s->vk->swapchain = %p, s->vk->surface = %p\n", (void*)s->vk->swapchain, (void*)s->vk->surface);
        } else {
            headless = true;
        }
    }

    if (headless) {
        offscreen_data = calloc(1, 800 * 600 * sizeof(AB4HPixel));
        mock_buf.width = 800;
        mock_buf.height = 600;
        mock_buf.data = offscreen_data;
        mock_s.paint_buffer = &mock_buf;
        mock_s.display = NULL;
        mock_s.running = true;
        mock_s.vk = NULL;
        s = &mock_s;
        printf("[SMURFS] Headless mode initialized.\n");
    }

    // 1. Initialize the Yul target contract AFTER Vulkan system is ready
    if (!lau_yul_thunk_init("cpu6502", "../solidity/bin/folklore.yul", 0x1)) {
        printf("[SMURFS] Error: Failed to initialize Yul cpu6502 thunk!\n");
        if (!headless) {
            destroy_vulkan_system(s);
        } else {
            free(offscreen_data);
        }
        return 1;
    }

    tsfi_input_set_key_hook((void*)smurfs_key_hook);

    // Initialize TSFiFontSystem for DeepSeek Post-Processing once
    TSFiFontSystem *ai_fs = tsfi_font_init();
    if (ai_fs) {
        tsfi_font_ai_init(ai_fs);
    }

    g_forest_bg = load_png_ab4h("assets/forest_bg.png", NULL, NULL);
    g_cavern_bg = load_png_ab4h("assets/cavern_bg.png", NULL, NULL);
    g_castle_bg = load_png_ab4h("assets/castle_bg.png", NULL, NULL);
    g_sprite_sheet = load_png_ab4h("assets/vaesen_sprite_sheet.png", &g_sprite_sheet_w, &g_sprite_sheet_h);


    // Premium Palette
    AB4HPixel bg_cyber_navy = make_ab4h_pixel(0.02f, 0.02f, 0.08f, 1.0f);
    AB4HPixel neon_pink = make_ab4h_pixel(1.0f, 0.05f, 0.6f, 1.0f);
    AB4HPixel neon_cyan = make_ab4h_pixel(0.0f, 0.9f, 1.0f, 1.0f);
    AB4HPixel neon_green = make_ab4h_pixel(0.05f, 0.85f, 0.3f, 1.0f);
    AB4HPixel neon_yellow = make_ab4h_pixel(1.2f, 1.2f, 0.0f, 1.0f);
    AB4HPixel shadow_black = make_ab4h_pixel(0.0f, 0.0f, 0.0f, 0.5f);


    int frame_counter = 0;
    float floor_y = 520.0f;

    int obs_fd = open("/dev/shm/tsfi_wayland_fb", O_CREAT | O_RDWR, 0666);
    uint32_t *obs_px = NULL;
    if (obs_fd >= 0) {
        if (ftruncate(obs_fd, 800 * 600 * 4) == 0) {
            obs_px = mmap(NULL, 800 * 600 * 4, PROT_READ|PROT_WRITE, MAP_SHARED, obs_fd, 0);
            if (obs_px == MAP_FAILED) {
                obs_px = NULL;
            }
        }
    }

    reset_game_yul();
    init_sprite_system();

    while (s->running && !exit_requested) {
        if (!headless) {
            wl_display_roundtrip(s->display);
            
            // Dynamically initialize swapchain in loop if compositor wasn't ready during create_vulkan_system
            if (!s->vk->swapchain) {
                init_swapchain(s);
                if (s->vk->swapchain) {
                    printf("[SMURFS] Vulkan Swapchain dynamically initialized in frame loop!\n");
                }
            }
        }

        AB4HPixel *pixels = (AB4HPixel*)s->paint_buffer->data;
        int W = s->paint_buffer->width;
        int H = s->paint_buffer->height;

        // 1.5. Retrieve Vaesen conditions from 6502 PC RAM
        int phys_trauma = (int)thunk_peek(55043);
        int ment_trauma = (int)thunk_peek(55044);

        // 2. Feed Inputs into Yul EVM Registers
        int moveDir = 0;
        
        static bool ai_checked = false;
        if (!ai_checked) {
            const char *ai_env = getenv("TSFI_AI_PLAY");
            if (ai_env && strcmp(ai_env, "1") == 0) {
                ai_mode = true;
                printf("[SMURFS] AI Autopilot mode enabled!\n");
            }
            ai_checked = true;
        }

        if (ai_mode && !game_over && !game_win) {
            // AI Autopilot decisions using Yul coordinates
            bool should_jump = false;
            
            if (game_screen == 1) {
                float fence_x = 360.0f;
                float rock_x = 600.0f;
                if (smurf_x > fence_x - 40.0f && smurf_x < fence_x - 15.0f) {
                    should_jump = true;
                }
                if (smurf_x > rock_x - 40.0f && smurf_x < rock_x - 15.0f) {
                    should_jump = true;
                }
            } else if (game_screen == 2) {
                float stal_x = 400.0f;
                if (smurf_x > stal_x - 40.0f && smurf_x < stal_x - 15.0f) {
                    should_jump = true;
                }
                float stalactite_x = 450.0f;
                if (smurf_x > stalactite_x - 50.0f && smurf_x < stalactite_x - 15.0f) {
                    should_jump = true;
                }
                if (fabsf(crow_x - smurf_x) < 70.0f && crow_y > floor_y - 90.0f) {
                    should_jump = true;
                }
            } else if (game_screen == 3) {
                int gargamel_vx_raw = (int)thunk_peek(55041);
                int gargamel_vx = (gargamel_vx_raw > 127) ? (gargamel_vx_raw - 256) : gargamel_vx_raw;

                if (smurf_x > 350.0f) {
                    if (fabsf(gargamel_x - smurf_x) < 80.0f) {
                        should_jump = true;
                    }
                    moveDir = 2; // Walk right
                } else {
                    if (gargamel_vx < 0 && smurf_x >= 200.0f) {
                        moveDir = 0; // Wait
                    } else {
                        moveDir = 2; // Walk right
                    }
                }
            }
            
            // Vaesen Broken condition: cannot jump
            if (phys_trauma == 3) {
                should_jump = false;
            }

            int jumpCount = (int)thunk_peek(55065);
            if (should_jump && jumpCount < 2) {
                thunk_poke(55026, 1); // Trigger Jump in Yul
            }
            if (ai_mode && game_screen != 3) {
                moveDir = 2;
            }
        } else {
            if (key_a_held) moveDir = 1;
            else if (key_d_held) moveDir = 2;
        }

        // Trauma modifiers are now executed natively inside the Yul physics loop.
        thunk_poke(55025, moveDir);

        // 3. Trigger Physics / Collision updates inside Yul
        if (game_screen == 2) {
            int current_vx = (int)thunk_peek(55039);
            if (current_vx > 127) current_vx -= 256;
            if (current_vx > 2) {
                thunk_poke(55039, 2);
            } else if (current_vx < -2) {
                thunk_poke(55039, 254); // -2 in 8-bit
            }
        }
        thunk_poke(55024, 1);
        thunk_poke(55031, 999); // Force energy to 999 (Infinite Health Cheat)

        // 4. Fetch updated game states from Yul EVM Registers
        smurf_x        = (float)thunk_peek(55027);
        smurf_y        = (float)thunk_peek(55028);
        smurf_jumping  = (thunk_peek(55030) != 0);
        energy         = (int)thunk_peek(55031);
        score          = (int)thunk_peek(55032);
        game_screen    = (int)thunk_peek(55033);
        game_over      = (thunk_peek(55034) != 0);
        game_win       = (thunk_peek(55035) != 0);

        static bool last_game_win = false;
        static bool last_game_over = false;
        if (game_win != last_game_win) {
            printf("[GAME STATE] Win status changed to: %d\n", game_win);
            fflush(stdout);
            last_game_win = game_win;
            if (ai_mode && game_win) {
                printf("[SMURFS] AI Autopilot successfully won the game! Exiting with SUCCESS.\n");
                fflush(stdout);
                exit_requested = 1;
            }
        }
        if (game_over != last_game_over) {
            printf("[GAME STATE] Game Over status changed to: %d\n", game_over);
            fflush(stdout);
            last_game_over = game_over;
            if (ai_mode && game_over) {
                printf("[SMURFS] AI Autopilot failed / game over occurred. Exiting with FAILURE.\n");
                fflush(stdout);
                exit_requested = 1;
            }
        }
        if (frame_counter % 30 == 0) {
            printf("[GAME PROGRESS] Screen: %d, X: %.1f, Y: %.1f, Energy: %d\n", game_screen, smurf_x, smurf_y, energy);
            fflush(stdout);
        }

        crow_x         = (float)thunk_peek(55037);
        crow_y         = (float)thunk_peek(55038);
        gargamel_x     = (float)thunk_peek(55040);

        sync_c64_vic_registers(frame_counter);

        // Proximity-based Fear/Mental Trauma calculation
        int target_fear = 0;
        if (!game_over && !game_win) {
            if (game_screen == 2) { // Cave: check distance to Crow
                float dx = smurf_x - crow_x;
                float dy = smurf_y - crow_y;
                float dist = sqrtf(dx*dx + dy*dy);
                if (dist < 60.0f) target_fear = 3;
                else if (dist < 100.0f) target_fear = 2;
                else if (dist < 150.0f) target_fear = 1;
            } else if (game_screen == 3) { // Castle: check distance to Gargamel
                float dx = smurf_x - gargamel_x;
                float dy = smurf_y - (floor_y - 20.0f);
                float dist = sqrtf(dx*dx + dy*dy);
                if (dist < 70.0f) target_fear = 3;
                else if (dist < 110.0f) target_fear = 2;
                else if (dist < 160.0f) target_fear = 1;
            }
        }

        static int fear_cooldown_ticks = 0;
        int cur_fear = (int)thunk_peek(55044);
        if (target_fear > cur_fear) {
            cur_fear = target_fear;
            thunk_poke(55044, cur_fear);
            fear_cooldown_ticks = 0;
        } else if (target_fear < cur_fear) {
            fear_cooldown_ticks++;
            if (fear_cooldown_ticks >= 90) { // cool down by 1 level every 90 frames (~1.5s)
                cur_fear--;
                thunk_poke(55044, cur_fear);
                fear_cooldown_ticks = 0;
            }
        } else {
            fear_cooldown_ticks = 0;
        }
        if (energy < 30) {
            cur_fear = 4;
            thunk_poke(55044, cur_fear);
        }
        // Expose updated fear value to renderer
        ment_trauma = cur_fear;

        // Sprite Collision Detection Strobe (slot 55046)
        int is_colliding = 0;
        if (!game_over && !game_win) {
            if (game_screen == 2) {
                float dx = smurf_x - crow_x;
                float dy = smurf_y - crow_y;
                if (sqrtf(dx*dx + dy*dy) < 40.0f) is_colliding = 1;
            } else if (game_screen == 3) {
                float dx = smurf_x - gargamel_x;
                float dy = smurf_y - (floor_y - 20.0f);
                if (sqrtf(dx*dx + dy*dy) < 40.0f) is_colliding = 1;
            }
        }
        thunk_poke(55046, is_colliding);

        // Sprite Scaling Register (slot 55045): shrink when broken, giant near forest mushrooms
        int scale_reg = 0; // default normal (1.0x)
        if (phys_trauma == 3) {
            scale_reg = 2; // tiny/broken (0.7x scale)
        } else if (game_screen == 1) {
            // proximity to forest mushrooms
            if (fabsf(smurf_x - 120.0f) < 22.0f || fabsf(smurf_x - 400.0f) < 22.0f) {
                scale_reg = 1; // giant mode! (1.4x scale)
            }
        }
        thunk_poke(55045, scale_reg);

        // Sprite Animation Pose Register (slot 55047)
        int anim_state = 0; // IDLE
        if (game_over) {
            anim_state = 3; // WOUNDED
        } else if (smurf_jumping) {
            anim_state = 2; // JUMP
        } else if (ment_trauma == 3) {
            anim_state = 4; // PANICKED
        } else if (key_a_held || key_d_held || (ai_mode && moveDir != 0)) {
            anim_state = 1; // WALK
        }
        thunk_poke(55047, anim_state);

        // 5. Sound trigger output checking
        int snd_trigger = (int)thunk_peek(55036);
        if (snd_trigger != 0) {
            if (snd_trigger == 1) {
                printf("[SMURFS] AI/EVM Triggered Sound: JUMP (Playing via audio synthesizer)\n");
                play_synth_sound("jump");
            }
            else if (snd_trigger == 2) {
                printf("[SMURFS] AI/EVM Triggered Sound: HIT (Playing via audio synthesizer)\n");
                play_synth_sound("hit");
            }
            else if (snd_trigger == 3) {
                printf("[SMURFS] AI/EVM Triggered Sound: WIN (Playing via audio synthesizer)\n");
                play_synth_sound("win");
            }
            else if (snd_trigger == 4) {
                printf("[SMURFS] AI/EVM Triggered Sound: GAMEOVER (Playing via audio synthesizer)\n");
                play_synth_sound("gameover");
            }
            fflush(stdout);
            thunk_poke(55036, 0); // Clear sound trigger
        }

        // Damage impact tracking: Trigger screen shake and kr0wZ particle burst
        if (energy < prev_energy) {
            printf("[GAME STATE] [DAMAGE] Took damage! Energy: %d -> %d, player_x: %.1f, player_y: %.1f, gargamel_x: %.1f, screen: %d\n", prev_energy, energy, smurf_x, smurf_y, gargamel_x, game_screen);
            fflush(stdout);
            screen_shake_timer = 15; // shake for 15 frames
            
            // Increment Physical Trauma on damage
            int cur_phys = (int)thunk_peek(55043);
            if (cur_phys < 3) {
                cur_phys++;
                thunk_poke(55043, cur_phys);
            }
            phys_trauma = cur_phys;

            // Spawn 32 particles flying out from player center
            int spawned = 0;
            for (int p_idx = 0; p_idx < MAX_PARTICLES; p_idx++) {
                if (!particles[p_idx].active) {
                    particles[p_idx].active = true;
                    particles[p_idx].x = smurf_x;
                    particles[p_idx].y = smurf_y - 15;
                    float angle = ((float)rand() / (float)RAND_MAX) * 2.0f * 3.14159f;
                    float speed = 2.0f + ((float)rand() / (float)RAND_MAX) * 4.0f;
                    particles[p_idx].vx = cosf(angle) * speed;
                    particles[p_idx].vy = sinf(angle) * speed - 2.0f; // float slightly upwards
                    particles[p_idx].alpha = 1.0f;
                    spawned++;
                    if (spawned >= 24) break;
                }
            }
        }
        prev_energy = energy;

        // Apply screen shake viewport displacement
        if (screen_shake_timer > 0) {
            shake_dx = ((rand() % 13) - 6);
            shake_dy = ((rand() % 13) - 6);
            screen_shake_timer--;
        } else {
            shake_dx = 0.0f;
            shake_dy = 0.0f;
        }

        // Update active glitch particles
        for (int p_idx = 0; p_idx < MAX_PARTICLES; p_idx++) {
            if (particles[p_idx].active) {
                particles[p_idx].x += particles[p_idx].vx;
                particles[p_idx].y += particles[p_idx].vy;
                particles[p_idx].vy += 0.1f; // gravity effect on particles
                particles[p_idx].alpha -= 0.04f;
                if (particles[p_idx].alpha <= 0.0f) {
                    particles[p_idx].active = false;
                }
            }
        }

        // Set status message text based on screen state
        if (game_win) {
            strcpy(status_message, "^B^U[VICTORY]^C ^IYOU RESCUED BABY BEAR! ^C[R] TO PLAY AGAIN.");
        } else if (game_over) {
            strcpy(status_message, "^B^U[GAME OVER]^C ^ITHE TEDDY BEAR COLLAPSED! ^C[R] TO RESTART.");
        } else {
            if (game_screen == 1) {
                strcpy(status_message, "^B[FOREST PATH] ^CAVOID THE ^IFENCE^C AND ^IBOULDER^C!");
            } else if (game_screen == 2) {
                strcpy(status_message, "^B[DARK CAVES] ^CWATCH OUT FOR ^ISTALAGMITES^C AND ^ICROWS^C!");
            } else if (game_screen == 3) {
                strcpy(status_message, "^B[GARGAMEL'S CASTLE] ^CGARGAMEL IS PATROLLING! ^ISAVE BABY BEAR^C!");
            }
        }

        // Draw screen backgrounds depending on current level screen
        if (game_screen == 1) {
            draw_texture_bg(pixels, W, H, g_forest_bg);

            // Draw Fence at x = 360, height 25 (y=495 to 520)
            float fence_cx = 360.0f;
            draw_line_aa(pixels, W, H, fence_cx - 15, floor_y, fence_cx - 15, floor_y - 25, neon_pink, 2.0f);
            draw_line_aa(pixels, W, H, fence_cx + 15, floor_y, fence_cx + 15, floor_y - 25, neon_pink, 2.0f);
            draw_line_aa(pixels, W, H, fence_cx - 15, floor_y - 20, fence_cx + 15, floor_y - 20, neon_pink, 2.0f);
            draw_line_aa(pixels, W, H, fence_cx - 15, floor_y - 8, fence_cx + 15, floor_y - 8, neon_pink, 2.0f);
            draw_line_aa(pixels, W, H, fence_cx - 15, floor_y - 20, fence_cx + 15, floor_y - 8, neon_pink, 1.0f);
            draw_line_aa(pixels, W, H, fence_cx - 15, floor_y - 8, fence_cx + 15, floor_y - 20, neon_pink, 1.0f);

            // Draw Rock/Boulder at x = 600, height 15 (y=505 to 520)
            float rock_cx = 600.0f;
            draw_radial_glow(pixels, W, H, rock_cx, floor_y - 7, 18.0f, make_ab4h_pixel(0.4f, 0.4f, 0.45f, 0.6f));
            draw_line_aa(pixels, W, H, rock_cx - 15, floor_y, rock_cx - 8, floor_y - 15, make_ab4h_pixel(0.6f, 0.6f, 0.65f, 1.0f), 1.5f);
            draw_line_aa(pixels, W, H, rock_cx - 8, floor_y - 15, rock_cx + 8, floor_y - 15, make_ab4h_pixel(0.6f, 0.6f, 0.65f, 1.0f), 1.5f);
            draw_line_aa(pixels, W, H, rock_cx + 8, floor_y - 15, rock_cx + 15, floor_y, make_ab4h_pixel(0.5f, 0.5f, 0.55f, 1.0f), 1.5f);
        } else if (game_screen == 2) {
            draw_texture_bg(pixels, W, H, g_cavern_bg);

            // Draw Stalagmite at x = 400, height 30 (y=490 to 520) - styled as a gothic runic spike
            float stal_cx = 400.0f;
            draw_radial_glow(pixels, W, H, stal_cx, floor_y - 15, 25.0f, make_ab4h_pixel(0.0f, 0.9f, 1.0f, 0.45f)); // Glow
            // Jagged, textured stone sides
            draw_line_aa(pixels, W, H, stal_cx - 15, floor_y, stal_cx, floor_y - 32, make_ab4h_pixel(0.25f, 0.2f, 0.25f, 1.0f), 3.0f);
            draw_line_aa(pixels, W, H, stal_cx + 15, floor_y, stal_cx, floor_y - 32, make_ab4h_pixel(0.2f, 0.18f, 0.22f, 1.0f), 3.0f);
            // Internal glowing runic core
            draw_line_aa(pixels, W, H, stal_cx - 6, floor_y, stal_cx, floor_y - 22, neon_cyan, 1.5f);
            draw_line_aa(pixels, W, H, stal_cx + 6, floor_y, stal_cx, floor_y - 22, neon_cyan, 1.5f);
            draw_line_aa(pixels, W, H, stal_cx, floor_y, stal_cx, floor_y - 28, make_ab4h_pixel(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);

            // Draw Falling Stalactite at x = 450
            float stalactite_x = 450.0f;
            float stalactite_y = (float)thunk_peek(55066);
            if (stalactite_y > 0) {
                draw_radial_glow(pixels, W, H, stalactite_x, stalactite_y + 15, 20.0f, make_ab4h_pixel(1.0f, 0.3f, 0.0f, 0.45f)); // Orange glow
                // Draw downward pointing stalactite spike
                draw_line_aa(pixels, W, H, stalactite_x - 10, stalactite_y, stalactite_x, stalactite_y + 25, make_ab4h_pixel(0.3f, 0.25f, 0.25f, 1.0f), 2.5f);
                draw_line_aa(pixels, W, H, stalactite_x + 10, stalactite_y, stalactite_x, stalactite_y + 25, make_ab4h_pixel(0.25f, 0.2f, 0.2f, 1.0f), 2.5f);
                // Heat core
                draw_line_aa(pixels, W, H, stalactite_x, stalactite_y, stalactite_x, stalactite_y + 20, make_ab4h_pixel(1.0f, 0.6f, 0.0f, 1.0f), 1.2f);
            }

            // Draw Crow (Stable Diffusion Sprite)
            int crow_vx_raw = (int)thunk_peek(55039);
            int crow_vx = (crow_vx_raw > 127) ? (crow_vx_raw - 256) : crow_vx_raw;
            bool face_left = (crow_vx < 0);
            draw_sprite_frame(pixels, W, H, (int)crow_x, (int)crow_y + 10, 0.45f, &g_crow_sprites, ANIM_WALK, frame_counter, face_left);
        } else if (game_screen == 3) {
            draw_texture_bg(pixels, W, H, g_castle_bg);

            // Castle Screen: Gargamel's Cage containing Smurfette
            int cage_x = 680;
            int cage_y = floor_y - 80;
            draw_rect_ab4h(pixels, W, H, cage_x, cage_y, 60, 80, bg_cyber_navy);
            // Render detailed thick vertical and horizontal iron bars
            for (int i = 0; i <= 60; i += 10) {
                draw_line_aa(pixels, W, H, cage_x + i, cage_y, cage_x + i, cage_y + 80, make_ab4h_pixel(0.4f, 0.4f, 0.45f, 1.0f), 1.5f);
            }
            for (int j = 0; j <= 80; j += 20) {
                draw_line_aa(pixels, W, H, cage_x, cage_y + j, cage_x + 60, cage_y + j, make_ab4h_pixel(0.4f, 0.4f, 0.45f, 1.0f), 1.5f);
            }
            draw_line_aa(pixels, W, H, cage_x, cage_y, cage_x + 60, cage_y, neon_cyan, 2.0f);
            draw_line_aa(pixels, W, H, cage_x, cage_y + 80, cage_x + 60, cage_y + 80, neon_cyan, 2.0f);

            // Draw a heavy brass padlock on the cage door
            int lock_x = cage_x + 10;
            int lock_y = cage_y + 40;
            draw_rect_ab4h(pixels, W, H, lock_x - 4, lock_y, 8, 8, neon_yellow); // Lock body
            draw_line_aa(pixels, W, H, lock_x - 3, lock_y, lock_x - 3, lock_y - 4, make_ab4h_pixel(0.5f, 0.5f, 0.5f, 1.0f), 1.0f); // Shackle
            draw_line_aa(pixels, W, H, lock_x + 3, lock_y, lock_x + 3, lock_y - 4, make_ab4h_pixel(0.5f, 0.5f, 0.5f, 1.0f), 1.0f);
            draw_line_aa(pixels, W, H, lock_x - 3, lock_y - 4, lock_x + 3, lock_y - 4, make_ab4h_pixel(0.5f, 0.5f, 0.5f, 1.0f), 1.0f);

            // Baby Bear inside cage
            int baby_x = cage_x + 35;
            int baby_y = cage_y + 50;
            // Golden rescue glow behind Baby Bear
            draw_radial_glow(pixels, W, H, baby_x, baby_y - 8, 20.0f, make_ab4h_pixel(1.0f, 0.9f, 0.0f, 0.35f));
            
            AB4HPixel baby_brown = make_ab4h_pixel(0.42f, 0.28f, 0.16f, 1.0f);
            AB4HPixel snout_cream = make_ab4h_pixel(0.85f, 0.78f, 0.65f, 1.0f);
            
            // 1. Ears
            draw_radial_glow(pixels, W, H, baby_x - 4, baby_y - 15, 3.0f, baby_brown);
            draw_radial_glow(pixels, W, H, baby_x + 4, baby_y - 15, 3.0f, baby_brown);
            
            // 2. Head & face (felt softness)
            draw_radial_glow(pixels, W, H, baby_x, baby_y - 10, 5.5f, baby_brown);
            draw_rect_ab4h(pixels, W, H, baby_x - 3, baby_y - 11, 6, 4, baby_brown);
            
            // Snout and button nose
            draw_radial_glow(pixels, W, H, baby_x, baby_y - 8, 2.5f, snout_cream);
            draw_rect_ab4h(pixels, W, H, baby_x - 1, baby_y - 9, 2, 1, make_ab4h_pixel(0.1f, 0.1f, 0.1f, 1.0f));

            // Stitched eyes
            AB4HPixel baby_stitch = make_ab4h_pixel(0.05f, 0.05f, 0.05f, 1.0f);
            draw_line_aa(pixels, W, H, baby_x - 2, baby_y - 11, baby_x - 1, baby_y - 10, baby_stitch, 0.8f);
            draw_line_aa(pixels, W, H, baby_x + 2, baby_y - 11, baby_x + 1, baby_y - 10, baby_stitch, 0.8f);

            // 3. Body: rounded teddy torso
            draw_radial_glow(pixels, W, H, baby_x, baby_y - 2, 7.0f, baby_brown);
            draw_rect_ab4h(pixels, W, H, baby_x - 4, baby_y - 4, 8, 8, baby_brown);

            // Red bow tie
            draw_line_aa(pixels, W, H, baby_x - 4, baby_y - 5, baby_x + 4, baby_y - 5, make_ab4h_pixel(0.9f, 0.05f, 0.1f, 1.0f), 2.0f);
            draw_radial_glow(pixels, W, H, baby_x, baby_y - 5, 2.0f, make_ab4h_pixel(0.9f, 0.05f, 0.1f, 1.0f));

            // 4. Limbs
            draw_line_aa(pixels, W, H, baby_x - 4, baby_y - 2, baby_x - 9, baby_y + 1, baby_brown, 1.5f); // Left arm
            draw_line_aa(pixels, W, H, baby_x + 4, baby_y - 2, baby_x + 9, baby_y + 1, baby_brown, 1.5f); // Right arm
            draw_line_aa(pixels, W, H, baby_x - 2, baby_y + 4, baby_x - 3, baby_y + 7, baby_brown, 2.0f); // Left leg
            draw_line_aa(pixels, W, H, baby_x + 2, baby_y + 4, baby_x + 3, baby_y + 7, baby_brown, 2.0f); // Right leg

            // Draw Gargamel (Stable Diffusion Alchemist Sprite)
            bool gargamel_face_left = (thunk_peek(55041) > 127);
            draw_sprite_frame(pixels, W, H, (int)gargamel_x, (int)floor_y, 0.65f, &g_gargamel_sprites, ANIM_WALK, frame_counter, gargamel_face_left);
        }

        // Draw Player (The Stuffed Smurf with kr0wZ sickness)
        int draw_sx = (int)smurf_x + (int)shake_dx;
        int draw_sy = (int)smurf_y + (int)shake_dy;

        // Read scale and animation pose registers from emulated RAM
        scale_reg = (int)thunk_peek(55045);
        anim_state = (int)thunk_peek(55047);
        float scale = 1.0f;
        if (scale_reg == 1) scale = 1.4f;      // Giant mode
        else if (scale_reg == 2) scale = 0.7f; // Tiny/broken mode

        // Apply Vaesen Micro-Jitter (fear shiver)
        if (ment_trauma > 0) {
            draw_sx += (rand() % 3 - 1) * ment_trauma;
            draw_sy += (rand() % 3 - 1) * ment_trauma;
        }

        // Apply Asymmetric Head Bobbing / Limp vertical offset when walking
        if (anim_state == 1) {
            float bob = sinf(frame_counter * 0.6f);
            if (phys_trauma > 0 && bob < 0.0f) {
                draw_sy += (int)(4.0f * scale * phys_trauma); // Deeper dip on injured side
            } else {
                draw_sy += (int)(bob * 2.0f * scale); // Normal bob
            }
        }

        // Render warm amber lantern glow around the Smurf
        draw_radial_glow(pixels, W, H, draw_sx, draw_sy - (int)(15 * scale), (int)(110 * scale), make_ab4h_pixel(1.0f, 0.65f, 0.15f, 0.28f));

        // Blit the Player (Stable Diffusion Small Blue Forest Troll)
        static bool last_face_left = false;
        if (key_a_held) last_face_left = true;
        if (key_d_held) last_face_left = false;

        draw_sprite_frame(pixels, W, H, draw_sx, draw_sy + 5, scale * 0.7f, &g_smurf_sprites, anim_state, frame_counter, last_face_left);

        if (ment_trauma == 4 && (frame_counter % 120 < 40)) {
            draw_string_ab4h(pixels, W, H, "*Sigh*", draw_sx - 20, draw_sy - (int)(40 * scale), make_ab4h_pixel(0.7f, 0.7f, 0.9f, 0.8f));
        }


        // Draw kr0wZ glitch damage burst particles
        float ngr = half_to_float(neon_green.r);
        float ngg = half_to_float(neon_green.g);
        float ngb = half_to_float(neon_green.b);
        for (int p_idx = 0; p_idx < MAX_PARTICLES; p_idx++) {
            if (particles[p_idx].active) {
                AB4HPixel p_color = make_ab4h_pixel(ngr, ngg * 1.2f, ngb, particles[p_idx].alpha);
                draw_rect_ab4h(pixels, W, H, (int)particles[p_idx].x - 2, (int)particles[p_idx].y - 2, 4, 4, p_color);
            }
        }

        // Apply Gothic shadow vignette (visibility depends on proximity to Smurf's lantern or level lights)
        for (int y = 50; y < H - 40; y += 2) {
            for (int x = 0; x < W; x += 2) {
                float dx = (float)x - smurf_x;
                float dy = (float)y - smurf_y;
                float dist2 = dx*dx + dy*dy;
                
                // Base visibility from player's lantern (radius 180px)
                float visibility = 1.0f - (dist2 / (180.0f * 180.0f));
                if (visibility < 0.15f) visibility = 0.15f; // ambient dark visible space
                if (visibility > 1.0f) visibility = 1.0f;
                
                // Boost visibility near glowing crystals (Screen 2) or Magic Mushrooms (Screen 1) or Cage (Screen 3)
                if (game_screen == 1) {
                    // Mushrooms at 120 and 400
                    float d1 = (float)x - 120.0f, d2 = (float)x - 400.0f;
                    if (d1*d1 < 80.0f*80.0f || d2*d2 < 80.0f*80.0f) visibility = 1.0f;
                } else if (game_screen == 2) {
                    // Crystals at 100, 320, 540...
                    for (int cx_pos = 100; cx_pos < W; cx_pos += 220) {
                        float c_dx = (float)x - cx_pos;
                        float c_dy = (float)y - (floor_y - 15.0f);
                        if (c_dx*c_dx + c_dy*c_dy < 80.0f*80.0f) visibility = 1.0f;
                    }
                } else if (game_screen == 3) {
                    // Cage at 680
                    float c_dx = (float)x - 680.0f;
                    if (c_dx*c_dx < 120.0f*120.0f) visibility = 1.0f;
                }
                
                // Apply vignette attenuation to 2x2 block
                if (visibility < 1.0f) {
                    for (int dy_o = 0; dy_o < 2; dy_o++) {
                        for (int dx_o = 0; dx_o < 2; dx_o++) {
                            int px_idx = (y + dy_o) * W + (x + dx_o);
                            if (px_idx < W * H) {
                                AB4HPixel p = pixels[px_idx];
                                float r = half_to_float(p.r) * visibility;
                                float g = half_to_float(p.g) * visibility;
                                float b = half_to_float(p.b) * visibility;
                                pixels[px_idx] = make_ab4h_pixel(r, g, b, half_to_float(p.a));
                            }
                        }
                    }
                }
            }
        }

        // Render Scoreboard Footer / Status Bar
        draw_rect_ab4h(pixels, W, H, 0, H - 40, W, 40, bg_cyber_navy);
        draw_line_aa(pixels, W, H, 0.0f, (float)(H - 40), (float)W, (float)(H - 40), neon_pink, 1.5f);
        draw_string_ab4h(pixels, W, H, status_message, 20, H - 30, neon_yellow);

        // Top HUD Header Bar for Vaesen RPG parameters
        draw_rect_ab4h(pixels, W, H, 0, 0, W, 45, shadow_black);
        draw_line_aa(pixels, W, H, 0.0f, 45.0f, (float)W, 45.0f, neon_pink, 1.0f);
        
        char hud_left[128];
        snprintf(hud_left, sizeof(hud_left), "ENERGY: %d  |  SCORE: %d", energy, score);
        draw_string_ab4h(pixels, W, H, hud_left, 20, 15, neon_cyan);

        char hud_right[128];
        const char *phys_str = "Normal";
        if (phys_trauma == 1) phys_str = "^IExhausted^C";
        else if (phys_trauma == 2) phys_str = "^IBattered^C";
        else if (phys_trauma == 3) phys_str = "^IBroken^C";
        
        const char *ment_str = "Normal";
        if (ment_trauma == 1) ment_str = "^UShaken^C";
        else if (ment_trauma == 2) ment_str = "^UTerrified^C";
        else if (ment_trauma == 3) ment_str = "^UPanicked^C";
        else if (ment_trauma == 4) ment_str = "^UMelancholic^C";
        
        snprintf(hud_right, sizeof(hud_right), "TRAUMA: %s  |  FEAR: %s", phys_str, ment_str);
        draw_string_ab4h(pixels, W, H, hud_right, W - 480, 15, neon_yellow);

        // 5.5. Draw Game State Overlay (Victory or Game Over Banner)
        if (game_win) {
            // Draw a semi-transparent dark overlay in the center of the screen
            draw_rect_ab4h(pixels, W, H, W / 2 - 200, H / 2 - 60, 400, 100, shadow_black);
            // Draw border
            draw_line_aa(pixels, W, H, (float)(W / 2 - 200), (float)(H / 2 - 60), (float)(W / 2 + 200), (float)(H / 2 - 60), neon_green, 2.0f);
            draw_line_aa(pixels, W, H, (float)(W / 2 - 200), (float)(H / 2 + 40), (float)(W / 2 + 200), (float)(H / 2 + 40), neon_green, 2.0f);
            draw_line_aa(pixels, W, H, (float)(W / 2 - 200), (float)(H / 2 - 60), (float)(W / 2 - 200), (float)(H / 2 + 40), neon_green, 2.0f);
            draw_line_aa(pixels, W, H, (float)(W / 2 + 200), (float)(H / 2 - 60), (float)(W / 2 + 200), (float)(H / 2 + 40), neon_green, 2.0f);
            
            draw_string_ab4h(pixels, W, H, "^B^U*** VICTORY! ***", W / 2 - 110, H / 2 - 40, neon_yellow);
            draw_string_ab4h(pixels, W, H, "BABY BEAR RESCUED!", W / 2 - 100, H / 2 - 10, neon_cyan);
            draw_string_ab4h(pixels, W, H, "PRESS [R] TO PLAY AGAIN", W / 2 - 130, H / 2 + 15, neon_green);
        } else if (game_over) {
            // Draw a semi-transparent dark overlay
            draw_rect_ab4h(pixels, W, H, W / 2 - 200, H / 2 - 60, 400, 100, shadow_black);
            // Draw border
            draw_line_aa(pixels, W, H, (float)(W / 2 - 200), (float)(H / 2 - 60), (float)(W / 2 + 200), (float)(H / 2 - 60), neon_pink, 2.0f);
            draw_line_aa(pixels, W, H, (float)(W / 2 - 200), (float)(H / 2 + 40), (float)(W / 2 + 200), (float)(H / 2 + 40), neon_pink, 2.0f);
            draw_line_aa(pixels, W, H, (float)(W / 2 - 200), (float)(H / 2 - 60), (float)(W / 2 - 200), (float)(H / 2 + 40), neon_pink, 2.0f);
            draw_line_aa(pixels, W, H, (float)(W / 2 + 200), (float)(H / 2 - 60), (float)(W / 2 + 200), (float)(H / 2 + 40), neon_pink, 2.0f);
            
            draw_string_ab4h(pixels, W, H, "^B^U*** GAME OVER ***", W / 2 - 110, H / 2 - 40, neon_pink);
            draw_string_ab4h(pixels, W, H, "THE TEDDY BEAR COLLAPSED", W / 2 - 130, H / 2 - 10, neon_yellow);
            draw_string_ab4h(pixels, W, H, "PRESS [R] TO RESTART", W / 2 - 110, H / 2 + 15, neon_cyan);
        }

        // --- DeepSeek Post-Processing & Spatial Upscale Upgrade Layer ---
        if (ai_fs) {
            DeepSeekEvolveContext dec = {
                .pixels = pixels,
                .width = W,
                .height = H,
                .frame_counter = frame_counter
            };
            tsfi_font_ai_bind_evolve(ai_fs, deepseek_evolve_impl, &dec);
            float current_intensity = 0.5f + 0.5f * sinf(frame_counter * 0.1f);
            tsfi_font_ai_invoke_evolve(ai_fs, current_intensity);
        }

        // Apply a horizontal/vertical spatial bloom filter specifically onto highly intense neon/glowing pixels.
        // This spreads color mathematically to simulate a premium High Dynamic Range (HDR) screen glow.
        for (int y = 52; y < H - 42; y += 2) {
            for (int x = 2; x < W - 2; x += 2) {
                AB4HPixel p = pixels[y * W + x];
                float r = half_to_float(p.r);
                float g = half_to_float(p.g);
                float b = half_to_float(p.b);
                float a = half_to_float(p.a);
                
                // Detect bright glowing hot-spots (neon cyan, pink, white, yellow, or kr0wZ green)
                if (r > 0.8f || g > 0.8f || b > 0.8f) {
                    // Spread glow to neighboring pixels with soft attenuation
                    float bloom_r = r * 0.2f;
                    float bloom_g = g * 0.2f;
                    float bloom_b = b * 0.2f;
                    float bloom_a = a * 0.2f;
                    
                    int neighbors[4][2] = { {x-1, y}, {x+1, y}, {x, y-1}, {x, y+1} };
                    for (int n = 0; n < 4; n++) {
                        int nx = neighbors[n][0];
                        int ny = neighbors[n][1];
                        if (nx >= 0 && nx < W && ny >= 50 && ny < H - 40) {
                            AB4HPixel np = pixels[ny * W + nx];
                            float nr = half_to_float(np.r) + bloom_r;
                            float ng = half_to_float(np.g) + bloom_g;
                            float nb = half_to_float(np.b) + bloom_b;
                            float na = fminf(1.0f, half_to_float(np.a) + bloom_a);
                            pixels[ny * W + nx] = make_ab4h_pixel(nr, ng, nb, na);
                        }
                    }
                }
            }
        }

        if (obs_px && pixels) {
            for (int y = 0; y < 600; y++) {
                int src_y = (y * H) / 600;
                if (src_y < 0) src_y = 0;
                if (src_y >= H) src_y = H - 1;
                for (int x = 0; x < 800; x++) {
                    int src_x = (x * W) / 800;
                    if (src_x < 0) src_x = 0;
                    if (src_x >= W) src_x = W - 1;
                    AB4HPixel p = pixels[src_y * W + src_x];
                    float r = half_to_float(p.r);
                    float g = half_to_float(p.g);
                    float b = half_to_float(p.b);
                    if (r < 0.0f) { r = 0.0f; }
                    if (r > 1.0f) { r = 1.0f; }
                    if (g < 0.0f) { g = 0.0f; }
                    if (g > 1.0f) { g = 1.0f; }
                    if (b < 0.0f) { b = 0.0f; }
                    if (b > 1.0f) { b = 1.0f; }
                    uint32_t ur = (uint32_t)(r * 255.0f);
                    uint32_t ug = (uint32_t)(g * 255.0f);
                    uint32_t ub = (uint32_t)(b * 255.0f);
                    obs_px[y * 800 + x] = 0xFF000000 | (ur << 16) | (ug << 8) | ub;
                }
            }
        }

        // Render Frame
        if (headless && (frame_counter >= headless_frames - 1 || exit_requested)) {
            FILE *f = fopen("/home/mariarahel/.gemini/antigravity-cli/brain/5a8d4144-99a3-4e64-93ac-47c55dad5b24/scratch/vaesen_smurfs_render.ppm", "wb");
            if (f) {
                fprintf(f, "P6\n%d %d\n255\n", W, H);
                for (int py_idx = 0; py_idx < H; py_idx++) {
                    for (int px_idx = 0; px_idx < W; px_idx++) {
                        AB4HPixel p = pixels[py_idx * W + px_idx];
                        float r_val = half_to_float(p.r);
                        float g_val = half_to_float(p.g);
                        float b_val = half_to_float(p.b);
                        if (r_val < 0.0f) r_val = 0.0f;
                        if (r_val > 1.0f) r_val = 1.0f;
                        if (g_val < 0.0f) g_val = 0.0f;
                        if (g_val > 1.0f) g_val = 1.0f;
                        if (b_val < 0.0f) b_val = 0.0f;
                        if (b_val > 1.0f) b_val = 1.0f;
                        unsigned char r_b = (unsigned char)(r_val * 255.0f);
                        unsigned char g_b = (unsigned char)(g_val * 255.0f);
                        unsigned char b_b = (unsigned char)(b_val * 255.0f);
                        fwrite(&r_b, 1, 1, f);
                        fwrite(&g_b, 1, 1, f);
                        fwrite(&b_b, 1, 1, f);
                    }
                }
                fclose(f);
                printf("[SMURFS] Successfully exported headless frame (frame %d) to /home/mariarahel/.gemini/antigravity-cli/brain/5a8d4144-99a3-4e64-93ac-47c55dad5b24/scratch/vaesen_smurfs_render.ppm\n", frame_counter);
            }
            break;
        }

        draw_frame(s);
        frame_counter++;
        tsfi_raw_usleep(16000);
    }

    printf("[SMURFS] Teardown Wayland-Vulkan context.\n");
    if (!headless) {
        lau_unseal_object(s);
    }
    if (ai_fs) {
        tsfi_font_ai_destroy(ai_fs);
        tsfi_font_destroy(ai_fs);
    }
    if (!headless) {
        destroy_vulkan_system(s);
    } else {
        free(offscreen_data);
    }
    extern _Atomic int g_teardown_in_progress;
    g_teardown_in_progress = 1;
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_free_all_active(void);
    lau_free_all_active();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

const char* __lsan_default_suppressions() {
    return "leak:<unknown module>\n"
           "leak:libwayland-client\n"
           "leak:ld-linux\n"
           "leak:libvulkan\n";
}
