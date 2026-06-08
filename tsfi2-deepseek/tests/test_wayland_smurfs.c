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

// Font helper removed to render all glyphs via DeepSeek system font loading

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
static char status_message[128] = "WELCOME TO SMURF RESCUE! [SPACE] TO JUMP. [A/D] MOVE.";
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
    thunk_poke(55031, 100); // energy
    thunk_poke(55032, 0);   // score
    thunk_poke(55033, 1);   // screen
    thunk_poke(55034, 0);   // isGameOver
    thunk_poke(55035, 0);   // isGameWin
    thunk_poke(55036, 0);   // soundStrobe
    thunk_poke(55037, 400); // crow_x
    thunk_poke(55038, 250); // crow_y
    thunk_poke(55039, -4);  // crow_vx
    thunk_poke(55040, 500); // gargamel_x
    thunk_poke(55041, -3);  // gargamel_vx
    thunk_poke(55043, 0);   // physical trauma (0 = Normal, 1 = Exhausted, 2 = Battered, 3 = Broken)
    thunk_poke(55044, 0);   // mental trauma (0 = Normal, 1 = Shaken, 2 = Terrified, 3 = Panicked)
    thunk_poke(55045, 0);   // sprite scaling (0 = Normal, 1 = Giant, 2 = Tiny)
    thunk_poke(55046, 0);   // sprite collision strobe
    thunk_poke(55047, 0);   // sprite animation state (0 = Idle, 1 = Walk, 2 = Jump, 3 = Wounded, 4 = Panicked)
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
        if (pressed && !smurf_jumping && !game_over && !game_win) {
            // Check if player is Broken (Physical Trauma == 3) in 6502 PC RAM
            if (thunk_peek(55043) != 3) {
                thunk_poke(55026, 1); // Trigger Jump in Yul
            }
        }
    } else if (key == 19) { // R
        if (pressed && (game_over || game_win)) {
            reset_game_yul();
            strcpy(status_message, "GAME RESTARTED! SAVE SMURFETTE!");
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

int main() {
    printf("[SMURFS] Launching C64 Smurfs Rescue Manifold...\n");
    signal(SIGPIPE, SIG_IGN);
    setenv("TSFI_AB4H", "1", 1);

    VulkanSystem *s = create_vulkan_system();
    if (!s) return 1;
    lau_unseal_object(s);
    printf("[SMURFS] Vulkan System initialized. s->vk->swapchain = %p, s->vk->surface = %p\n", (void*)s->vk->swapchain, (void*)s->vk->surface);

    // 1. Initialize the Yul target contract AFTER Vulkan system is ready
    if (!lau_yul_thunk_init("cpu6502", "../solidity/bin/cpu6502.yul", 0x1)) {
        printf("[SMURFS] Error: Failed to initialize Yul cpu6502 thunk!\n");
        destroy_vulkan_system(s);
        return 1;
    }

    tsfi_input_set_key_hook((void*)smurfs_key_hook);

    // Initialize TSFiFontSystem for DeepSeek Post-Processing once
    TSFiFontSystem *ai_fs = tsfi_font_init();
    if (ai_fs) {
        tsfi_font_ai_init(ai_fs);
    }

    // Premium Palette
    AB4HPixel bg_cyber_navy = make_ab4h_pixel(0.02f, 0.02f, 0.08f, 1.0f);
    AB4HPixel neon_pink = make_ab4h_pixel(1.0f, 0.05f, 0.6f, 1.0f);
    AB4HPixel neon_cyan = make_ab4h_pixel(0.0f, 0.9f, 1.0f, 1.0f);
    AB4HPixel neon_green = make_ab4h_pixel(0.05f, 0.85f, 0.3f, 1.0f);
    AB4HPixel neon_white = make_ab4h_pixel(1.0f, 1.0f, 1.0f, 1.0f);
    AB4HPixel neon_yellow = make_ab4h_pixel(1.2f, 1.2f, 0.0f, 1.0f);
    AB4HPixel smurf_blue = make_ab4h_pixel(0.0f, 0.6f, 1.0f, 1.0f);
    AB4HPixel shadow_black = make_ab4h_pixel(0.0f, 0.0f, 0.0f, 0.5f);
    AB4HPixel gargamel_grey = make_ab4h_pixel(0.4f, 0.4f, 0.4f, 1.0f);

    int frame_counter = 0;
    float floor_y = 520.0f;

    while (s->running && !exit_requested) {
        wl_display_roundtrip(s->display);
        
        // Dynamically initialize swapchain in loop if compositor wasn't ready during create_vulkan_system
        if (!s->vk->swapchain) {
            init_swapchain(s);
            if (s->vk->swapchain) {
                printf("[SMURFS] Vulkan Swapchain dynamically initialized in frame loop!\n");
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
                if (smurf_x > fence_x - 80.0f && smurf_x < fence_x - 10.0f) {
                    should_jump = true;
                }
                if (smurf_x > rock_x - 80.0f && smurf_x < rock_x - 10.0f) {
                    should_jump = true;
                }
            } else if (game_screen == 2) {
                float stal_x = 400.0f;
                if (smurf_x > stal_x - 80.0f && smurf_x < stal_x - 10.0f) {
                    should_jump = true;
                }
                if (fabsf(crow_x - smurf_x) < 90.0f && crow_y > floor_y - 90.0f) {
                    should_jump = true;
                }
            } else if (game_screen == 3) {
                // If Gargamel is close, jump over him
                if (fabsf(gargamel_x - smurf_x) < 100.0f) {
                    should_jump = true;
                }
            }
            
            // Vaesen Broken condition: cannot jump
            if (phys_trauma == 3) {
                should_jump = false;
            }

            if (should_jump && !smurf_jumping) {
                thunk_poke(55026, 1); // Trigger Jump in Yul
            }
            moveDir = 2; // Walk right
        } else {
            if (key_a_held) moveDir = 1;
            else if (key_d_held) moveDir = 2;
        }

        // Apply Vaesen Mental condition effects on movement inputs
        if (ment_trauma == 2) { // Terrified: reversed controls
            if (moveDir == 1) moveDir = 2;
            else if (moveDir == 2) moveDir = 1;
        } else if (ment_trauma == 3) { // Panicked: freeze periodically
            if (frame_counter % 20 < 8) {
                moveDir = 0;
            }
        }

        // Apply Vaesen Physical condition effects on movement inputs (speed penalty)
        if (phys_trauma == 1) { // Exhausted: 33% slower (skip move every 3rd frame)
            if (frame_counter % 3 == 0) {
                moveDir = 0;
            }
        } else if (phys_trauma == 2) { // Battered: 50% slower (skip move every 2nd frame)
            if (frame_counter % 2 == 0) {
                moveDir = 0;
            }
        } else if (phys_trauma == 3) { // Broken: 75% slower (only move every 4th frame)
            if (frame_counter % 4 != 0) {
                moveDir = 0;
            }
            thunk_poke(55026, 0); // Override and clear jump trigger
        }

        thunk_poke(55025, moveDir);

        // 3. Trigger Physics / Collision updates inside Yul
        thunk_poke(55024, 1);

        // 4. Fetch updated game states from Yul EVM Registers
        smurf_x        = (float)thunk_peek(55027);
        smurf_y        = (float)thunk_peek(55028);
        smurf_jumping  = (thunk_peek(55030) != 0);
        energy         = (int)thunk_peek(55031);
        score          = (int)thunk_peek(55032);
        game_screen    = (int)thunk_peek(55033);
        game_over      = (thunk_peek(55034) != 0);
        game_win       = (thunk_peek(55035) != 0);

        crow_x         = (float)thunk_peek(55037);
        crow_y         = (float)thunk_peek(55038);
        gargamel_x     = (float)thunk_peek(55040);

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
            if (snd_trigger == 1) play_synth_sound("jump");
            else if (snd_trigger == 2) play_synth_sound("hit");
            else if (snd_trigger == 3) play_synth_sound("win");
            else if (snd_trigger == 4) play_synth_sound("gameover");
            thunk_poke(55036, 0); // Clear sound trigger
        }

        // Damage impact tracking: Trigger screen shake and kr0wZ particle burst
        if (energy < prev_energy && !game_over) {
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
            strcpy(status_message, "^B^U[VICTORY]^C ^IYOU RESCUED SMURFETTE! ^C[R] TO PLAY AGAIN.");
        } else if (game_over) {
            strcpy(status_message, "^B^U[GAME OVER]^C ^ITHE SMURF COLLAPSED! ^C[R] TO RESTART.");
        } else {
            if (game_screen == 1) {
                strcpy(status_message, "^B[FOREST PATH] ^CAVOID THE ^IFENCE^C AND ^IBOULDER^C!");
            } else if (game_screen == 2) {
                strcpy(status_message, "^B[DARK CAVES] ^CWATCH OUT FOR ^ISTALAGMITES^C AND ^ICROWS^C!");
            } else if (game_screen == 3) {
                strcpy(status_message, "^B[GARGAMEL'S CASTLE] ^CGARGAMEL IS PATROLLING! ^ISAVE SMURFETTE^C!");
            }
        }

        // Draw screen backgrounds depending on current level screen
        if (game_screen == 1) {
            // Screen 1: Forest Gradient with scrolling stars/dust
            for (int y = 0; y < H; y++) {
                float t = (float)y / H;
                float gr = 0.01f * (1.0f - t) + 0.02f * t;
                float gg = 0.10f * (1.0f - t) + 0.22f * t;
                float gb = 0.04f * (1.0f - t) + 0.06f * t;
                draw_rect_ab4h(pixels, W, H, 0, y, W, 1, make_ab4h_pixel(gr, gg, gb, 1.0f));
            }

            // Parallax scrolling clouds in the upper sky
            for (int i = 0; i < 3; i++) {
                float cloud_x = fmodf(50.0f + i * 300.0f - frame_counter * (0.2f + i * 0.1f), W + 200.0f) - 100.0f;
                draw_radial_glow(pixels, W, H, cloud_x, 80.0f + i * 15.0f, 60.0f, make_ab4h_pixel(1.0f, 1.0f, 1.0f, 0.08f));
            }

            // Distant rolling mountain silhouette (rendered as overlapping hills)
            for (int hill_x = 0; hill_x < W; hill_x += 16) {
                float hill_y1 = floor_y - 100.0f + sinf(hill_x * 0.006f) * 40.0f + cosf(hill_x * 0.012f) * 10.0f;
                draw_line_aa(pixels, W, H, hill_x, floor_y, hill_x, hill_y1, make_ab4h_pixel(0.01f, 0.08f, 0.04f, 1.0f), 8.0f);
            }

            // Draw forest trees
            draw_radial_glow(pixels, W, H, W * 0.3f, floor_y - 120, 110.0f, make_ab4h_pixel(0.0f, 0.4f, 0.15f, 0.35f));
            draw_rect_ab4h(pixels, W, H, W * 0.3f - 8, floor_y - 120, 16, 120, make_ab4h_pixel(0.18f, 0.08f, 0.0f, 1.0f));
            draw_radial_glow(pixels, W, H, W * 0.7f, floor_y - 140, 130.0f, make_ab4h_pixel(0.0f, 0.4f, 0.15f, 0.35f));
            draw_rect_ab4h(pixels, W, H, W * 0.7f - 12, floor_y - 150, 24, 150, make_ab4h_pixel(0.18f, 0.08f, 0.0f, 1.0f));

            // Glowing Magic Mushrooms on the floor path
            for (int mush_x = 120; mush_x < W; mush_x += 280) {
                draw_radial_glow(pixels, W, H, mush_x, floor_y - 10, 25.0f, make_ab4h_pixel(1.0f, 0.3f, 0.0f, 0.35f)); // neon orange mushroom glow
                draw_rect_ab4h(pixels, W, H, mush_x - 3, floor_y - 12, 6, 12, make_ab4h_pixel(1.0f, 1.0f, 1.0f, 1.0f)); // stem
                draw_radial_glow(pixels, W, H, mush_x, floor_y - 12, 10.0f, make_ab4h_pixel(1.0f, 0.2f, 0.0f, 1.0f)); // cap
            }

            // Animated falling glowing forest leaves/wind particles
            for (int i = 0; i < 5; i++) {
                float leaf_x = fmodf(100.0f + i * 180.0f - frame_counter * (1.2f + i * 0.3f), W);
                if (leaf_x < 0) leaf_x += W;
                float leaf_y = 100.0f + sinf(frame_counter * 0.05f + i) * 30.0f + i * 40.0f;
                draw_radial_glow(pixels, W, H, leaf_x, leaf_y, 8.0f, make_ab4h_pixel(0.1f, 0.8f, 0.2f, 0.3f));
            }

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
            // Screen 2: Cave System (darker theme with blue/purple ambient shadows)
            for (int y = 0; y < H; y++) {
                float t = (float)y / H;
                float cr = 0.02f * (1.0f - t) + 0.06f * t;
                float cg = 0.02f * (1.0f - t) + 0.05f * t;
                float cb = 0.05f * (1.0f - t) + 0.12f * t;
                draw_rect_ab4h(pixels, W, H, 0, y, W, 1, make_ab4h_pixel(cr, cg, cb, 1.0f));
            }
            // Stalactites hanging from ceiling
            for (int i = 0; i < W; i += 60) {
                int len = 40 + (i % 7) * 8;
                draw_line_aa(pixels, W, H, i, 50, i + 15, 50 + len, make_ab4h_pixel(0.2f, 0.18f, 0.22f, 1.0f), 2.0f);
                draw_line_aa(pixels, W, H, i + 30, 50, i + 15, 50 + len, make_ab4h_pixel(0.2f, 0.18f, 0.22f, 1.0f), 2.0f);
            }
            // Glowing neon crystals on cave floor
            for (int x_pos = 100; x_pos < W; x_pos += 220) {
                draw_radial_glow(pixels, W, H, x_pos, floor_y - 15, 30.0f, make_ab4h_pixel(0.0f, 0.8f, 1.0f, 0.3f)); // Crystal glow
                draw_line_aa(pixels, W, H, x_pos, floor_y, x_pos - 8, floor_y - 20, make_ab4h_pixel(0.0f, 0.9f, 1.0f, 1.0f), 1.5f);
                draw_line_aa(pixels, W, H, x_pos, floor_y, x_pos + 8, floor_y - 25, make_ab4h_pixel(0.2f, 0.7f, 1.0f, 1.0f), 1.5f);
            }

            // Draw Stalagmite at x = 400, height 30 (y=490 to 520)
            float stal_cx = 400.0f;
            draw_radial_glow(pixels, W, H, stal_cx, floor_y - 15, 20.0f, make_ab4h_pixel(0.0f, 0.9f, 1.0f, 0.4f)); // Glow
            draw_line_aa(pixels, W, H, stal_cx - 12, floor_y, stal_cx, floor_y - 30, neon_cyan, 2.0f);
            draw_line_aa(pixels, W, H, stal_cx + 12, floor_y, stal_cx, floor_y - 30, neon_cyan, 2.0f);
            draw_line_aa(pixels, W, H, stal_cx - 6, floor_y, stal_cx, floor_y - 20, make_ab4h_pixel(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);
            draw_line_aa(pixels, W, H, stal_cx + 6, floor_y, stal_cx, floor_y - 20, make_ab4h_pixel(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);

            // Draw Crow at crow_x, crow_y
            float wing_sweep = sinf(frame_counter * 0.3f) * 12.0f;
            draw_radial_glow(pixels, W, H, crow_x, crow_y, 14.0f, make_ab4h_pixel(0.05f, 0.05f, 0.15f, 0.25f)); // Crow ambient shadow glow
            
            // Get Crow horizontal velocity direction from EVM
            int crow_vx_raw = (int)thunk_peek(55039);
            int crow_vx = (crow_vx_raw > 127) ? (crow_vx_raw - 256) : crow_vx_raw;
            bool face_left = (crow_vx < 0);

            // 1. Black felt Torso
            draw_radial_glow(pixels, W, H, crow_x, crow_y, 7.0f, make_ab4h_pixel(0.08f, 0.08f, 0.1f, 1.0f));
            draw_rect_ab4h(pixels, W, H, (int)crow_x - 4, (int)crow_y - 2, 8, 6, make_ab4h_pixel(0.05f, 0.05f, 0.07f, 1.0f));

            // 2. Black head
            draw_radial_glow(pixels, W, H, crow_x, crow_y - 5, 5.0f, make_ab4h_pixel(0.08f, 0.08f, 0.1f, 1.0f));

            // 3. Sharp yellow beak pointing in movement direction
            if (face_left) {
                draw_line_aa(pixels, W, H, crow_x - 2, crow_y - 4, crow_x - 10, crow_y - 2, neon_yellow, 1.5f);
                draw_line_aa(pixels, W, H, crow_x - 2, crow_y - 3, crow_x - 10, crow_y - 2, neon_yellow, 1.0f);
            } else {
                draw_line_aa(pixels, W, H, crow_x + 2, crow_y - 4, crow_x + 10, crow_y - 2, neon_yellow, 1.5f);
                draw_line_aa(pixels, W, H, crow_x + 2, crow_y - 3, crow_x + 10, crow_y - 2, neon_yellow, 1.0f);
            }

            // 4. Feathery wings flapping
            AB4HPixel wing_color = make_ab4h_pixel(0.04f, 0.04f, 0.06f, 1.0f);
            // Left wing
            draw_line_aa(pixels, W, H, crow_x, crow_y, crow_x - 18, crow_y - wing_sweep, wing_color, 2.2f);
            draw_line_aa(pixels, W, H, crow_x - 6, crow_y, crow_x - 16, crow_y + 4 - wing_sweep * 0.7f, wing_color, 1.5f); // secondary feather
            draw_line_aa(pixels, W, H, crow_x - 12, crow_y, crow_x - 22, crow_y + 8 - wing_sweep, wing_color, 1.0f); // outer feather
            
            // Right wing
            draw_line_aa(pixels, W, H, crow_x, crow_y, crow_x + 18, crow_y - wing_sweep, wing_color, 2.2f);
            draw_line_aa(pixels, W, H, crow_x + 6, crow_y, crow_x + 16, crow_y + 4 - wing_sweep * 0.7f, wing_color, 1.5f); // secondary feather
            draw_line_aa(pixels, W, H, crow_x + 12, crow_y, crow_x + 22, crow_y + 8 - wing_sweep, wing_color, 1.0f); // outer feather

            // 5. Tail feathers pointing backwards
            if (face_left) {
                draw_line_aa(pixels, W, H, crow_x + 2, crow_y + 3, crow_x + 12, crow_y + 6, wing_color, 2.0f);
                draw_line_aa(pixels, W, H, crow_x + 2, crow_y + 4, crow_x + 9, crow_y + 8, wing_color, 1.5f);
            } else {
                draw_line_aa(pixels, W, H, crow_x - 2, crow_y + 3, crow_x - 12, crow_y + 6, wing_color, 2.0f);
                draw_line_aa(pixels, W, H, crow_x - 2, crow_y + 4, crow_x - 9, crow_y + 8, wing_color, 1.5f);
            }

            // 6. Glowing Yellow Eyes
            draw_rect_ab4h(pixels, W, H, crow_x + (face_left ? -2 : 1), crow_y - 6, 2, 2, neon_yellow);
        } else if (game_screen == 3) {
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

            // Smurfette inside
            int sfe_x = cage_x + 35;
            int sfe_y = cage_y + 50;
            // Golden rescue glow behind Smurfette
            draw_radial_glow(pixels, W, H, sfe_x, sfe_y - 8, 20.0f, make_ab4h_pixel(1.0f, 0.9f, 0.0f, 0.35f));
            
            // 1. Blonde Hair: Flowing locks drawn with multiple curved lines
            draw_radial_glow(pixels, W, H, sfe_x, sfe_y - 16, 12.0f, neon_yellow); // Back hair volume
            draw_line_aa(pixels, W, H, sfe_x - 7, sfe_y - 20, sfe_x - 4, sfe_y - 12, neon_yellow, 2.0f); // Left lock
            draw_line_aa(pixels, W, H, sfe_x + 7, sfe_y - 20, sfe_x + 4, sfe_y - 12, neon_yellow, 2.0f); // Right lock
            draw_line_aa(pixels, W, H, sfe_x, sfe_y - 22, sfe_x - 6, sfe_y - 15, neon_yellow, 2.5f); // Bangs left
            draw_line_aa(pixels, W, H, sfe_x, sfe_y - 22, sfe_x + 6, sfe_y - 15, neon_yellow, 2.5f); // Bangs right

            // 2. Head & face (felt softness)
            draw_radial_glow(pixels, W, H, sfe_x, sfe_y - 12, 6.0f, smurf_blue); // Round head
            draw_rect_ab4h(pixels, W, H, sfe_x - 3, sfe_y - 14, 6, 6, smurf_blue);
            
            // Stitched eyes
            AB4HPixel sfe_stitch = make_ab4h_pixel(0.05f, 0.05f, 0.05f, 1.0f);
            draw_line_aa(pixels, W, H, sfe_x - 2, sfe_y - 12, sfe_x - 1, sfe_y - 11, sfe_stitch, 0.8f);
            draw_line_aa(pixels, W, H, sfe_x + 2, sfe_y - 12, sfe_x + 1, sfe_y - 11, sfe_stitch, 0.8f);

            // 3. White Hat: classic smurfette curved cap
            draw_line_aa(pixels, W, H, sfe_x - 6, sfe_y - 20, sfe_x + 6, sfe_y - 20, neon_white, 2.0f); // brim
            draw_radial_glow(pixels, W, H, sfe_x - 2, sfe_y - 23, 6.0f, neon_white); // crown
            draw_line_aa(pixels, W, H, sfe_x - 1, sfe_y - 22, sfe_x - 6, sfe_y - 24, neon_white, 3.0f); // floppy tip

            // 4. White Dress: triangular dress flare
            draw_line_aa(pixels, W, H, sfe_x, sfe_y - 4, sfe_x - 8, sfe_y + 4, neon_white, 2.5f); // Left seam
            draw_line_aa(pixels, W, H, sfe_x, sfe_y - 4, sfe_x + 8, sfe_y + 4, neon_white, 2.5f); // Right seam
            draw_rect_ab4h(pixels, W, H, sfe_x - 6, sfe_y - 4, 12, 8, neon_white); // Body filler

            // 5. Blue limbs
            draw_line_aa(pixels, W, H, sfe_x - 4, sfe_y - 6, sfe_x - 10, sfe_y, smurf_blue, 1.5f); // Left arm
            draw_line_aa(pixels, W, H, sfe_x + 4, sfe_y - 6, sfe_x + 10, sfe_y, smurf_blue, 1.5f); // Right arm
            draw_line_aa(pixels, W, H, sfe_x - 3, sfe_y + 4, sfe_x - 4, sfe_y + 7, smurf_blue, 2.0f); // Left leg
            draw_line_aa(pixels, W, H, sfe_x + 3, sfe_y + 4, sfe_x + 4, sfe_y + 7, smurf_blue, 2.0f); // Right leg

            // 6. Detailed Red Shoes
            draw_rect_ab4h(pixels, W, H, sfe_x - 6, sfe_y + 7, 3, 2, make_ab4h_pixel(1.0f, 0.05f, 0.1f, 1.0f));
            draw_rect_ab4h(pixels, W, H, sfe_x + 3, sfe_y + 7, 3, 2, make_ab4h_pixel(1.0f, 0.05f, 0.1f, 1.0f));

            // Draw Gargamel
            // Evil purple aura around Gargamel
            draw_radial_glow(pixels, W, H, gargamel_x, floor_y - 20, 25.0f, make_ab4h_pixel(0.3f, 0.0f, 0.6f, 0.25f));
            
            // 1. Dark Robe/Cloak body with patches
            draw_rect_ab4h(pixels, W, H, gargamel_x - 12, floor_y - 36, 24, 36, shadow_black); // Dark undercloak
            draw_line_aa(pixels, W, H, gargamel_x - 12, floor_y, gargamel_x, floor_y - 36, gargamel_grey, 2.0f); // Left fold
            draw_line_aa(pixels, W, H, gargamel_x + 12, floor_y, gargamel_x, floor_y - 36, gargamel_grey, 2.0f); // Right fold
            draw_rect_ab4h(pixels, W, H, (int)gargamel_x - 7, (int)floor_y - 14, 4, 4, make_ab4h_pixel(0.7f, 0.1f, 0.1f, 1.0f)); // Red patch on cloak

            // 2. Bald Head & fringe black hair
            AB4HPixel skin_color = make_ab4h_pixel(0.85f, 0.70f, 0.60f, 1.0f);
            draw_radial_glow(pixels, W, H, gargamel_x, floor_y - 43, 6.0f, skin_color); // Bald pate
            draw_rect_ab4h(pixels, W, H, gargamel_x - 4, floor_y - 45, 8, 6, skin_color);
            draw_line_aa(pixels, W, H, gargamel_x - 6, floor_y - 42, gargamel_x - 6, floor_y - 37, shadow_black, 2.0f); // Left hair fringe
            draw_line_aa(pixels, W, H, gargamel_x + 6, floor_y - 42, gargamel_x + 6, floor_y - 37, shadow_black, 2.0f); // Right hair fringe

            // 3. Prominent Evil Nose & Face details
            draw_radial_glow(pixels, W, H, gargamel_x - 4, floor_y - 41, 4.0f, make_ab4h_pixel(0.9f, 0.1f, 0.1f, 0.8f)); // Bulbous nose
            draw_line_aa(pixels, W, H, gargamel_x, floor_y - 43, gargamel_x - 6, floor_y - 41, skin_color, 1.5f); // Nose ridge
            
            // Evil glowing yellow eyes with tiny red pupils
            draw_rect_ab4h(pixels, W, H, gargamel_x - 3, floor_y - 45, 2, 2, neon_yellow);
            draw_rect_ab4h(pixels, W, H, gargamel_x + 1, floor_y - 45, 2, 2, neon_yellow);
            draw_rect_ab4h(pixels, W, H, gargamel_x - 2, floor_y - 44, 1, 1, make_ab4h_pixel(1.0f, 0.0f, 0.0f, 1.0f)); // left pupil
            draw_rect_ab4h(pixels, W, H, gargamel_x + 2, floor_y - 44, 1, 1, make_ab4h_pixel(1.0f, 0.0f, 0.0f, 1.0f)); // right pupil

            // 4. Evil hands clutching forward
            draw_line_aa(pixels, W, H, gargamel_x - 8, floor_y - 30, gargamel_x - 16, floor_y - 28, skin_color, 1.5f); // Left arm clutching
            draw_line_aa(pixels, W, H, gargamel_x + 8, floor_y - 30, gargamel_x + 16, floor_y - 28, skin_color, 1.5f); // Right arm clutching

            // 5. Worn out boots
            draw_rect_ab4h(pixels, W, H, gargamel_x - 10, floor_y - 2, 6, 2, make_ab4h_pixel(0.2f, 0.15f, 0.1f, 1.0f));
            draw_rect_ab4h(pixels, W, H, gargamel_x + 4, floor_y - 2, 6, 2, make_ab4h_pixel(0.2f, 0.15f, 0.1f, 1.0f));
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

        // Shadow under Smurf
        draw_rect_ab4h(pixels, W, H, draw_sx - (int)(12 * scale), floor_y - 2, (int)(24 * scale), 4, shadow_black);

        // Body Elements (Stuffed felt blue body)
        draw_radial_glow(pixels, W, H, draw_sx, draw_sy - (int)(18 * scale), (int)(14 * scale), make_ab4h_pixel(0.0f, 0.4f, 0.9f, 0.6f)); // felt softness
        draw_radial_glow(pixels, W, H, draw_sx, draw_sy - (int)(24 * scale), (int)(8 * scale), smurf_blue); // Round Head shape
        draw_rect_ab4h(pixels, W, H, draw_sx - (int)(5 * scale), draw_sy - (int)(25 * scale), (int)(10 * scale), (int)(8 * scale), smurf_blue); // head filler

        // Stitched Button Eyes (x cross stitches for eyes)
        AB4HPixel stitch_color = make_ab4h_pixel(0.1f, 0.1f, 0.1f, 1.0f);
        if (anim_state == 3) {
            // Wounded/Gameover: closed eyes (crossed horizontal dashes)
            draw_line_aa(pixels, W, H, draw_sx - (int)(5 * scale), draw_sy - (int)(19 * scale), draw_sx - (int)(1 * scale), draw_sy - (int)(19 * scale), stitch_color, 1.0f * scale);
            draw_line_aa(pixels, W, H, draw_sx + (int)(1 * scale), draw_sy - (int)(19 * scale), draw_sx + (int)(5 * scale), draw_sy - (int)(19 * scale), stitch_color, 1.0f * scale);
        } else {
            draw_line_aa(pixels, W, H, draw_sx - (int)(4 * scale), draw_sy - (int)(20 * scale), draw_sx - (int)(1 * scale), draw_sy - (int)(17 * scale), stitch_color, 1.0f * scale);
            draw_line_aa(pixels, W, H, draw_sx - (int)(1 * scale), draw_sy - (int)(20 * scale), draw_sx - (int)(4 * scale), draw_sy - (int)(17 * scale), stitch_color, 1.0f * scale);
            draw_line_aa(pixels, W, H, draw_sx + (int)(1 * scale), draw_sy - (int)(20 * scale), draw_sx + (int)(4 * scale), draw_sy - (int)(17 * scale), stitch_color, 1.0f * scale);
            draw_line_aa(pixels, W, H, draw_sx + (int)(4 * scale), draw_sy - (int)(20 * scale), draw_sx + (int)(1 * scale), draw_sy - (int)(17 * scale), stitch_color, 1.0f * scale);
        }

        // Stuffed seams (center vertical stitches down face and body)
        for (int sy_pos = draw_sy - (int)(24 * scale); sy_pos < draw_sy; sy_pos += 4) {
            draw_line_aa(pixels, W, H, draw_sx - 1, sy_pos, draw_sx + 1, sy_pos, stitch_color, 1.0f);
        }

        // Detailed curved C64 white hat (stuffed plushy hat)
        draw_radial_glow(pixels, W, H, draw_sx, draw_sy - (int)(30 * scale), (int)(9 * scale), neon_white); // Hat crown volume
        draw_line_aa(pixels, W, H, draw_sx - (int)(9 * scale), draw_sy - (int)(27 * scale), draw_sx + (int)(9 * scale), draw_sy - (int)(27 * scale), neon_white, 2.0f * scale); // brim line
        draw_line_aa(pixels, W, H, draw_sx, draw_sy - (int)(30 * scale), draw_sx - (int)(8 * scale), draw_sy - (int)(34 * scale), neon_white, 4.0f * scale); // curve forward tip
        draw_radial_glow(pixels, W, H, draw_sx - (int)(6 * scale), draw_sy - (int)(34 * scale), (int)(5 * scale), neon_white); // tip fluffiness
        
        // White trousers/boots (plushy felt)
        draw_radial_glow(pixels, W, H, draw_sx, draw_sy - (int)(8 * scale), (int)(8 * scale), neon_white); // pants roundness
        draw_rect_ab4h(pixels, W, H, draw_sx - (int)(6 * scale), draw_sy - (int)(12 * scale), (int)(12 * scale), (int)(10 * scale), neon_white);
        draw_rect_ab4h(pixels, W, H, draw_sx - (int)(7 * scale), draw_sy - (int)(3 * scale), (int)(5 * scale), (int)(4 * scale), neon_white); // left boot
        draw_rect_ab4h(pixels, W, H, draw_sx + (int)(2 * scale), draw_sy - (int)(3 * scale), (int)(5 * scale), (int)(4 * scale), neon_white); // right boot

        // kr0wZ Sickness: glowing toxic green digital glitch particles and neon green aura
        float krowz_pulse = 0.5f + 0.5f * sinf(frame_counter * 0.2f);
        float ngr = half_to_float(neon_green.r);
        float ngg = half_to_float(neon_green.g);
        float ngb = half_to_float(neon_green.b);
        draw_radial_glow(pixels, W, H, draw_sx, draw_sy - (int)(15 * scale), (int)(22 * scale), make_ab4h_pixel(ngr, ngg, ngb, 0.25f * krowz_pulse));
        
        // Random digital scanline glitch blocks emanating from the sickness
        if (frame_counter % 8 < 4) {
            int glitch_x = draw_sx + (rand() % (int)(40 * scale)) - (int)(20 * scale);
            int glitch_y = draw_sy - (int)(25 * scale) + (rand() % (int)(30 * scale)) - (int)(15 * scale);
            draw_rect_ab4h(pixels, W, H, glitch_x, glitch_y, (int)(8 * scale), (int)(2 * scale), make_ab4h_pixel(ngr, ngg * 1.2f, ngb, 0.9f));
        }

        // Animation state pose rendering
        if (anim_state == 2) {
            // JUMP pose: arms raised high
            draw_line_aa(pixels, W, H, draw_sx, draw_sy - (int)(12 * scale), draw_sx - (int)(12 * scale), draw_sy - (int)(24 * scale), smurf_blue, 2.0f * scale);
            draw_radial_glow(pixels, W, H, draw_sx - (int)(12 * scale), draw_sy - (int)(24 * scale), (int)(3 * scale), smurf_blue);
            draw_line_aa(pixels, W, H, draw_sx, draw_sy - (int)(12 * scale), draw_sx + (int)(12 * scale), draw_sy - (int)(24 * scale), smurf_blue, 2.0f * scale);
            draw_radial_glow(pixels, W, H, draw_sx + (int)(12 * scale), draw_sy - (int)(24 * scale), (int)(3 * scale), smurf_blue);
        } else if (anim_state == 4) {
            // PANICKED pose: shiver and swing arms frantically
            float panic_wave = sinf(frame_counter * 0.9f) * (15.0f * scale);
            draw_line_aa(pixels, W, H, draw_sx, draw_sy - (int)(10 * scale), draw_sx - (int)(12 * scale), draw_sy - (int)(10 * scale) + panic_wave, smurf_blue, 2.0f * scale);
            draw_radial_glow(pixels, W, H, draw_sx - (int)(12 * scale), draw_sy - (int)(10 * scale) + panic_wave, (int)(3 * scale), smurf_blue);
            draw_line_aa(pixels, W, H, draw_sx, draw_sy - (int)(10 * scale), draw_sx + (int)(12 * scale), draw_sy - (int)(10 * scale) - panic_wave, smurf_blue, 2.0f * scale);
            draw_radial_glow(pixels, W, H, draw_sx + (int)(12 * scale), draw_sy - (int)(10 * scale) - panic_wave, (int)(3 * scale), smurf_blue);
        } else if (anim_state == 1) {
            // WALK pose: arms swinging
            float arm_angle = sinf(frame_counter * 0.4f) * 0.8f;
            draw_line_aa(pixels, W, H, draw_sx, draw_sy - (int)(10 * scale), draw_sx + cosf(arm_angle)*(14.0f * scale), draw_sy - (int)(10 * scale) + sinf(arm_angle)*(14.0f * scale), smurf_blue, 2.0f * scale);
            draw_radial_glow(pixels, W, H, draw_sx + cosf(arm_angle)*(14.0f * scale), draw_sy - (int)(10 * scale) + sinf(arm_angle)*(14.0f * scale), (int)(3 * scale), smurf_blue);
        } else if (anim_state == 3) {
            // WOUNDED pose: arms hanging weakly
            float shiver = sinf(frame_counter * 1.5f) * 1.5f;
            draw_line_aa(pixels, W, H, draw_sx, draw_sy - (int)(10 * scale), draw_sx - (int)(4 * scale) + shiver, draw_sy, smurf_blue, 2.0f * scale);
            draw_line_aa(pixels, W, H, draw_sx, draw_sy - (int)(10 * scale), draw_sx + (int)(4 * scale) - shiver, draw_sy, smurf_blue, 2.0f * scale);
        } else {
            // IDLE pose: normal arms down
            draw_line_aa(pixels, W, H, draw_sx, draw_sy - (int)(10 * scale), draw_sx - (int)(6 * scale), draw_sy - (int)(2 * scale), smurf_blue, 2.0f * scale);
            draw_radial_glow(pixels, W, H, draw_sx - (int)(6 * scale), draw_sy - (int)(2 * scale), (int)(3 * scale), smurf_blue);
        }

        // Draw kr0wZ glitch damage burst particles
        for (int p_idx = 0; p_idx < MAX_PARTICLES; p_idx++) {
            if (particles[p_idx].active) {
                AB4HPixel p_color = make_ab4h_pixel(ngr, ngg * 1.2f, ngb, particles[p_idx].alpha);
                draw_rect_ab4h(pixels, W, H, (int)particles[p_idx].x - 2, (int)particles[p_idx].y - 2, 4, 4, p_color);
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
        
        snprintf(hud_right, sizeof(hud_right), "TRAUMA: %s  |  FEAR: %s", phys_str, ment_str);
        draw_string_ab4h(pixels, W, H, hud_right, W - 480, 15, neon_yellow);

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

        // Render Frame
        draw_frame(s);
        frame_counter++;
        tsfi_raw_usleep(16000);
    }

    printf("[SMURFS] Teardown Wayland-Vulkan context.\n");
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
