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

// Game State Definitions
static float lander_x = 150.0f;
static float lander_y = 100.0f;
static float lander_vx = 3.0f;
static float lander_vy = 0.0f;
static int fuel = 1000;
static int game_status = 0; // 0 = flying, 1 = landed, 2 = crashed
static int score = 0;
static char status_message[128] = "^B^U[LUNAR LANDER]^C ^I[W/S/A/D] to Fire thrusters. Save fuel!^C";
static int exit_requested = 0;
static bool key_a_held = false;
static bool key_d_held = false;
static bool key_w_held = false;
static bool ai_mode = true;

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
                buf[i] = 128 + (rand() % 40) - 20; // white noise thruster hiss
            }
        }
    } else if (strcmp(sd->type, "land") == 0) {
        len = 3200; buf = malloc(len);
        if (buf) {
            for (int i = 0; i < len; i++) {
                float t = (float)i / 8000.0f;
                float phase = 400.0f * t * 2.0f * 3.14159f;
                buf[i] = 128 + (int)((1.0f - t/0.4f) * 100.0f * sinf(phase));
            }
        }
    } else if (strcmp(sd->type, "crash") == 0) {
        len = 4800; buf = malloc(len);
        if (buf) {
            for (int i = 0; i < len; i++) {
                float t = (float)i / 8000.0f;
                buf[i] = 128 + (int)((1.0f - t/0.6f) * 110.0f * ((rand() % 100) - 50.0f) / 50.0f);
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

static void update_lander_yul(int tx, int ty) {
    uint8_t calldata[4 + 32 + 32] = {0};
    calldata[0] = 0xa9; calldata[1] = 0xe7; calldata[2] = 0xf5; calldata[3] = 0x36; // Selector
    
    // Sign extend tx safely (32 bytes)
    int64_t val_x = (int64_t)tx;
    uint8_t fill_x = (val_x < 0) ? 0xFF : 0x00;
    for (int i = 0; i < 24; i++) {
        calldata[4 + i] = fill_x;
    }
    for (int i = 0; i < 8; i++) {
        calldata[4 + 24 + 7 - i] = (val_x >> (i * 8)) & 0xFF;
    }

    // Sign extend ty safely (32 bytes)
    int64_t val_y = (int64_t)ty;
    uint8_t fill_y = (val_y < 0) ? 0xFF : 0x00;
    for (int i = 0; i < 24; i++) {
        calldata[4 + 32 + i] = fill_y;
    }
    for (int i = 0; i < 8; i++) {
        calldata[4 + 32 + 24 + 7 - i] = (val_y >> (i * 8)) & 0xFF;
    }

    uint8_t retval[224] = {0};
    size_t retval_len = 224;
    
    if (lau_yul_thunk_execute("lunar_lander", calldata, 4 + 32 + 32, retval, &retval_len)) {
        uint64_t u_px = 0, u_py = 0, u_vx = 0, u_vy = 0, u_fl = 0, u_st = 0, u_sc = 0;
        for (int i = 0; i < 8; i++) u_px |= ((uint64_t)retval[31 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_py |= ((uint64_t)retval[63 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_vx |= ((uint64_t)retval[95 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_vy |= ((uint64_t)retval[127 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_fl |= ((uint64_t)retval[159 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_st |= ((uint64_t)retval[191 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_sc |= ((uint64_t)retval[223 - i]) << (i * 8);
        
        int64_t px = (int64_t)u_px;
        int64_t py = (int64_t)u_py;
        int64_t vx = (int64_t)u_vx;
        int64_t vy = (int64_t)u_vy;
        
        lander_x = (float)px / 100.0f;
        lander_y = (float)py / 100.0f;
        lander_vx = (float)vx / 100.0f;
        lander_vy = (float)vy / 100.0f;
        
        fuel = (int)u_fl;
        score = (int)u_sc;
        
        int prev_status = game_status;
        game_status = (int)u_st;
        
        if (game_status != prev_status) {
            if (game_status == 1) play_synth_sound("land");
            else if (game_status == 2) play_synth_sound("crash");
        }
        
        fprintf(stderr, "[TELEMETRY] X: %.2f, Y: %.2f, VX: %.2f, VY: %.2f, TX: %d, TY: %d, Status: %d, Fuel: %d\n",
                lander_x, lander_y, lander_vx, lander_vy, tx, ty, game_status, fuel);
    }
}

static void init_lander_yul() {
    uint8_t calldata[4] = {0xd6, 0xb0, 0x46, 0xa2};
    uint8_t retval[32];
    size_t retval_len = 32;
    lau_yul_thunk_execute("lunar_lander", calldata, 4, retval, &retval_len);
    update_lander_yul(0, 0);
}

static void lander_key_hook(void *data, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    (void)data; (void)serial; (void)time;
    bool pressed = (state != 0);
    
    if (key == 1) { // ESC
        exit_requested = 1;
    } else if (key == 30 || key == 105) { // A or LEFT
        key_a_held = pressed;
    } else if (key == 32 || key == 106) { // D or RIGHT
        key_d_held = pressed;
    } else if (key == 17 || key == 57 || key == 103) { // W or Space or UP
        key_w_held = pressed;
        if (pressed && fuel > 0 && game_status == 0) play_synth_sound("thrust");
    } else if (key == 19) { // R -> Restart
        init_lander_yul();
        strcpy(status_message, "^B^U[LUNAR LANDER]^C ^IReady for Launch!^C");
    } else if (key == 20) { // T -> Tramiel Autopilot
        ai_mode = !ai_mode;
        if (ai_mode) {
            printf("[LANDER] \"COMPUTERS FOR THE MASSES, NOT THE CLASSES\" - Autopilot Active!\n");
            strcpy(status_message, "^B^U[TRAMIEL AUTOPILOT]^C ^IBUSINESS IS WAR! INITIATING SAFE DESCENT...^C");
        } else {
            strcpy(status_message, "^B[MANUAL]^C ^IControl returned to player.^C");
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

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            AB4HPixel p = pixels[y * W + x];
            float r = half_to_float(p.r);
            float g = half_to_float(p.g);
            float b = half_to_float(p.b);
            float a = half_to_float(p.a);
            
            float input_noise = (float)x * 12.9898f + (float)y * 78.233f + (float)(frame % 100) * 0.01f;
            float noise = sinf(input_noise) * 43758.5453f;
            noise = noise - floorf(noise); // fract
            
            if (a > 0.0f) {
                float noise_factor = 1.0f + (noise - 0.5f) * 0.04f;
                r *= noise_factor;
                g *= noise_factor;
                b *= noise_factor;
                pixels[y * W + x] = make_ab4h_pixel(r, g, b, a);
            }
        }
    }
}

int main() {
    printf("[LANDER] Launching Vulkan-Wayland Lunar Lander Module...\n");
    signal(SIGPIPE, SIG_IGN);
    setenv("TSFI_AB4H", "1", 1);

    VulkanSystem *s = create_vulkan_system();
    if (!s) return 1;
    lau_unseal_object(s);
    s->disable_ui_overlay = true;

    if (!lau_yul_thunk_init("lunar_lander", "../solidity/bin/lunar_lander.yul", 0x1)) {
        printf("[LANDER] Error: Failed to initialize Yul lunar_lander thunk!\n");
        destroy_vulkan_system(s);
        return 1;
    }

    init_lander_yul();
    tsfi_input_set_key_hook((void*)lander_key_hook);

    TSFiFontSystem *ai_fs = tsfi_font_init();
    if (ai_fs) {
        tsfi_font_ai_init(ai_fs);
    }

    // Palette
    AB4HPixel bg_space = make_ab4h_pixel(0.01f, 0.01f, 0.04f, 1.0f);
    AB4HPixel moon_grey = make_ab4h_pixel(0.5f, 0.5f, 0.55f, 1.0f);
    AB4HPixel pad_cyan = make_ab4h_pixel(0.0f, 0.9f, 1.0f, 1.0f);
    AB4HPixel neon_pink = make_ab4h_pixel(1.0f, 0.05f, 0.6f, 1.0f);
    AB4HPixel neon_yellow = make_ab4h_pixel(1.2f, 1.2f, 0.0f, 1.0f);
    AB4HPixel thrust_fire = make_ab4h_pixel(1.2f, 0.4f, 0.0f, 1.0f);
    AB4HPixel thruster_blue = make_ab4h_pixel(0.0f, 0.8f, 1.2f, 0.8f);

    int frame_counter = 0;

    while (s->running && !exit_requested) {
        wl_display_roundtrip(s->display);
        
        if (!s->vk->swapchain) {
            init_swapchain(s);
        }

        AB4HPixel *pixels = (AB4HPixel*)s->paint_buffer->data;
        int W = s->paint_buffer->width;
        int H = s->paint_buffer->height;

        // Clear Screen with starry background
        draw_rect_ab4h(pixels, W, H, 0, 0, W, H, bg_space);
        for (int i = 0; i < 20; i++) {
            int sx = (int)(fmodf((float)i * 123.45f, W));
            int sy = (int)(fmodf((float)i * 87.65f, H * 0.7f));
            draw_rect_ab4h(pixels, W, H, sx, sy, 2, 2, make_ab4h_pixel(1.0f, 1.0f, 1.0f, 0.6f));
        }

        // Autopilot calculations
        int tx = 0;
        int ty = 0;
        if (ai_mode && game_status == 0) {
            // Horizontal alignment
            float target_x = 400.0f;
            float dx = target_x - lander_x;
            
            if (dx > 15.0f) {
                if (lander_vx < 2.0f) tx = 1; // thrust right
            } else if (dx < -15.0f) {
                if (lander_vx > -2.0f) tx = -1; // thrust left
            } else {
                // Stabilize drift
                if (lander_vx > 0.2f) tx = -1;
                else if (lander_vx < -0.2f) tx = 1;
            }

            // Descent velocity control
            float target_vy = 2.0f;
            if (lander_y > 380.0f) target_vy = 0.8f; // slower descent close to ground
            
            if (lander_vy > target_vy) {
                ty = 1; // fire thruster
            }
        } else {
            if (key_a_held) tx = -1;
            else if (key_d_held) tx = 1;
            if (key_w_held) ty = 1;
        }

        // Tick Yul CPU
        update_lander_yul(tx, ty);

        // Scale factors to map logical 800x540 viewport to actual window resolution
        float scale_x = (float)W / 800.0f;
        float scale_y = (float)(H - 40) / 540.0f;
        float ground_y = 500.0f * scale_y;

        // Draw Jagged Lunar Mountains
        // Jagged mountain points
        float mount_points[7][2] = {
            {0.0f, 400.0f}, {150.0f, 320.0f}, {350.0f, 500.0f},
            {450.0f, 500.0f}, {600.0f, 380.0f}, {720.0f, 460.0f}, {800.0f, 390.0f}
        };
        for (int i = 0; i < 6; i++) {
            draw_line_aa(pixels, W, H, 
                         mount_points[i][0] * scale_x, mount_points[i][1] * scale_y, 
                         mount_points[i+1][0] * scale_x, mount_points[i+1][1] * scale_y, 
                         moon_grey, 2.0f);
        }

        // Draw Landing Pad (X = 350 to 450, Y = 500)
        draw_line_aa(pixels, W, H, 350.0f * scale_x, ground_y, 450.0f * scale_x, ground_y, pad_cyan, 3.5f);
        draw_radial_glow(pixels, W, H, 400.0f * scale_x, ground_y, 45.0f * fmaxf(scale_x, scale_y), make_ab4h_pixel(0.0f, 0.9f, 1.0f, 0.25f));

        // Draw Lander Module
        float lx = lander_x * scale_x;
        float ly = lander_y * scale_y;

        if (game_status != 2) {
            // Draw Main cockpit capsule
            draw_radial_glow(pixels, W, H, lx, ly - 15, 16.0f, make_ab4h_pixel(0.8f, 0.8f, 0.85f, 0.8f));
            draw_rect_ab4h(pixels, W, H, (int)lx - 12, (int)ly - 20, 24, 12, make_ab4h_pixel(0.7f, 0.7f, 0.75f, 1.0f));
            
            // Cockpit window
            draw_rect_ab4h(pixels, W, H, (int)lx - 6, (int)ly - 18, 12, 5, thruster_blue);

            // Landing legs
            draw_line_aa(pixels, W, H, lx - 10, ly - 8, lx - 18, ly, moon_grey, 1.5f);
            draw_line_aa(pixels, W, H, lx + 10, ly - 8, lx + 18, ly, moon_grey, 1.5f);
            draw_line_aa(pixels, W, H, lx - 18, ly, lx - 22, ly + 2, make_ab4h_pixel(1.0f, 1.0f, 1.0f, 1.0f), 2.0f); // footpads
            draw_line_aa(pixels, W, H, lx + 18, ly, lx + 22, ly + 2, make_ab4h_pixel(1.0f, 1.0f, 1.0f, 1.0f), 2.0f);

            // Engine nozzle
            draw_rect_ab4h(pixels, W, H, (int)lx - 4, (int)ly - 8, 8, 4, make_ab4h_pixel(0.2f, 0.2f, 0.2f, 1.0f));

            // Thruster fire plumes
            if (ty && fuel > 0 && game_status == 0) {
                // Main plume
                float plume_h = (15.0f + (rand() % 15)) * scale_y;
                draw_line_aa(pixels, W, H, lx, ly - 4, lx - 6 * scale_x, ly - 4 + plume_h, thrust_fire, 2.0f);
                draw_line_aa(pixels, W, H, lx, ly - 4, lx + 6 * scale_x, ly - 4 + plume_h, thrust_fire, 2.0f);
                draw_radial_glow(pixels, W, H, lx, ly + 5, 20.0f, make_ab4h_pixel(1.2f, 0.3f, 0.0f, 0.4f));
            }
            if (tx < 0 && fuel > 0 && game_status == 0) {
                // Right side thruster shooting left
                draw_line_aa(pixels, W, H, lx + 12, ly - 14, lx + 22 * scale_x, ly - 14, thrust_fire, 1.0f);
            }
            if (tx > 0 && fuel > 0 && game_status == 0) {
                // Left side thruster shooting right
                draw_line_aa(pixels, W, H, lx - 12, ly - 14, lx - 22 * scale_x, ly - 14, thrust_fire, 1.0f);
            }
        } else {
            // Crashed state: Draw explosion particles
            draw_radial_glow(pixels, W, H, lx, ly, 45.0f, make_ab4h_pixel(1.2f, 0.2f, 0.0f, 0.7f));
            draw_string_ab4h(pixels, W, H, "* CRASH *", (int)lx - 35, (int)ly - 10, neon_pink);
        }

        // Set status message text based on screen state
        if (game_status == 1) {
            strcpy(status_message, "^B^U[VICTORY]^C ^IEXCELLENT LANDING! SCORE UPDATED! ^C[R] TO REPLAY.");
        } else if (game_status == 2) {
            strcpy(status_message, "^B^U[CRASH]^C ^ILANDER WAS DESTROYED! ^C[R] TO RESTART.");
        } else {
            sprintf(status_message, "^B[LANDER] Fuel: %d | Speed: (VX: %.1f, VY: %.1f) | Score: %d^C", fuel, lander_vx, lander_vy, score);
        }

        // Footer status bar
        draw_rect_ab4h(pixels, W, H, 0, H - 40, W, 40, make_ab4h_pixel(0.02f, 0.02f, 0.08f, 1.0f));
        draw_line_aa(pixels, W, H, 0.0f, (float)(H - 40), (float)W, (float)(H - 40), neon_pink, 1.5f);
        draw_string_ab4h(pixels, W, H, status_message, 20, H - 30, neon_yellow);

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

        // Render Frame
        draw_frame(s);
        frame_counter++;
        tsfi_raw_usleep(16000);
    }

    printf("[LANDER] Teardown Vulkan context.\n");
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
