#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
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
        x += 16;
        str++;
    }
}

// Sound stub
static void play_synth_sound(const char *name) {
    printf("[AUDIO] Play synthesized sound: %s\n", name);
}

// Game State Values
static float yar_x = 100.0f;
static float yar_y = 270.0f;
static float qotile_x = 700.0f;
static float qotile_y = 270.0f;
static float cannon_x = 0.0f;
static float cannon_y = 0.0f;
static float swirl_x = 0.0f;
static float swirl_y = 0.0f;

static int lives = 3;
static int score = 0;
static int game_status = 0; // 0: playing, 1: victory, 2: round reset/dead, 3: game over
static int cannon_active = 0;
static int swirl_active = 0;
static float neutral_zone_x = 400.0f;

// Shield bitfields (5 columns, 32 rows each)
static uint64_t shield_data[5] = {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};

static int key_w_held = 0;
static int key_s_held = 0;
static int key_a_held = 0;
static int key_d_held = 0;
static int key_space_pressed = 0;
static int key_shift_held = 0;

static int exit_requested = 0;
static char status_message[512] = "^B^U[YARS' REVENGE]^C ^IReady for Battle!^C";

static void update_yars_yul(int tx, int ty, int fire, int action) {
    uint8_t calldata[4 + 32 * 4] = {0};
    
    // Selector for updateGame: 0xa9e7f536
    calldata[0] = 0xa9;
    calldata[1] = 0xe7;
    calldata[2] = 0xf5;
    calldata[3] = 0x36;

    // Pack inputs (tx, ty, fire, action)
    int64_t val_tx = (int64_t)tx;
    for (int i = 0; i < 8; i++) {
        calldata[4 + 24 + 7 - i] = (val_tx >> (i * 8)) & 0xFF;
    }
    int64_t val_ty = (int64_t)ty;
    for (int i = 0; i < 8; i++) {
        calldata[4 + 32 + 24 + 7 - i] = (val_ty >> (i * 8)) & 0xFF;
    }
    int64_t val_fire = (int64_t)fire;
    for (int i = 0; i < 8; i++) {
        calldata[4 + 64 + 24 + 7 - i] = (val_fire >> (i * 8)) & 0xFF;
    }
    int64_t val_action = (int64_t)action;
    for (int i = 0; i < 8; i++) {
        calldata[4 + 96 + 24 + 7 - i] = (val_action >> (i * 8)) & 0xFF;
    }

    uint8_t retval[608] = {0};
    size_t retval_len = 608;

    if (lau_yul_thunk_execute("yars_revenge", calldata, 4 + 32 * 4, retval, &retval_len)) {
        uint64_t u_yx = 0, u_yy = 0, u_qx = 0, u_qy = 0, u_cx = 0, u_cy = 0, u_sx = 0, u_sy = 0;
        uint64_t u_lv = 0, u_sc = 0, u_st = 0, u_ca = 0, u_sa = 0, u_nz = 0;
        uint64_t u_sh0 = 0, u_sh1 = 0, u_sh2 = 0, u_sh3 = 0, u_sh4 = 0;

        for (int i = 0; i < 8; i++) u_yx |= ((uint64_t)retval[31 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_yy |= ((uint64_t)retval[63 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_qx |= ((uint64_t)retval[95 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_qy |= ((uint64_t)retval[127 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_cx |= ((uint64_t)retval[159 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_cy |= ((uint64_t)retval[191 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_sx |= ((uint64_t)retval[223 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_sy |= ((uint64_t)retval[255 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_lv |= ((uint64_t)retval[287 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_sc |= ((uint64_t)retval[319 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_st |= ((uint64_t)retval[351 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_ca |= ((uint64_t)retval[383 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_sa |= ((uint64_t)retval[415 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_nz |= ((uint64_t)retval[447 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_sh0 |= ((uint64_t)retval[479 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_sh1 |= ((uint64_t)retval[511 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_sh2 |= ((uint64_t)retval[543 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_sh3 |= ((uint64_t)retval[575 - i]) << (i * 8);
        for (int i = 0; i < 8; i++) u_sh4 |= ((uint64_t)retval[607 - i]) << (i * 8);

        yar_x = (float)u_yx / 100.0f;
        yar_y = (float)u_yy / 100.0f;
        qotile_x = (float)u_qx / 100.0f;
        qotile_y = (float)u_qy / 100.0f;
        cannon_x = (float)u_cx / 100.0f;
        cannon_y = (float)u_cy / 100.0f;
        swirl_x = (float)u_sx / 100.0f;
        swirl_y = (float)u_sy / 100.0f;

        lives = (int)u_lv;
        score = (int)u_sc;
        
        int old_status = game_status;
        game_status = (int)u_st;
        
        if (game_status != old_status) {
            if (game_status == 1) play_synth_sound("victory");
            else if (game_status == 2) play_synth_sound("death");
            else if (game_status == 3) play_synth_sound("game_over");
        }

        cannon_active = (int)u_ca;
        swirl_active = (int)u_sa;
        neutral_zone_x = (float)u_nz / 100.0f;

        shield_data[0] = u_sh0;
        shield_data[1] = u_sh1;
        shield_data[2] = u_sh2;
        shield_data[3] = u_sh3;
        shield_data[4] = u_sh4;
    }
}

static void init_yars_yul() {
    uint8_t calldata[4] = {0xd6, 0xb0, 0x46, 0xa2}; // initializeGame selector
    uint8_t retval[32];
    size_t retval_len = 32;
    lau_yul_thunk_execute("yars_revenge", calldata, 4, retval, &retval_len);
    update_yars_yul(0, 0, 0, 0);
}

static void yars_key_hook(void *data, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    (void)data; (void)serial; (void)time;
    bool pressed = (state != 0);

    if (key == 1) { // ESC
        exit_requested = 1;
    } else if (key == 30 || key == 105) { // A / Left Arrow
        key_a_held = pressed;
    } else if (key == 32 || key == 106) { // D / Right Arrow
        key_d_held = pressed;
    } else if (key == 17 || key == 103) { // W / Up Arrow
        key_w_held = pressed;
    } else if (key == 31 || key == 108) { // S / Down Arrow
        key_s_held = pressed;
    } else if (key == 57) { // Space -> Fire Cannon / Action
        if (pressed) {
            key_space_pressed = 1;
            play_synth_sound("shoot");
        }
    } else if (key == 42 || key == 54) { // L/R Shift -> Eat/Bite Action
        key_shift_held = pressed;
    } else if (key == 19) { // R -> Reset
        init_yars_yul();
        strcpy(status_message, "^B^U[YARS' REVENGE]^C ^IReady for Round 1!^C");
    }
}

typedef struct {
    AB4HPixel *pixels;
    int width;
    int height;
    int frame_counter;
} DeepSeekEvolveContext;

// Implementation of the full-screen neutral zone colorful static noise
void deepseek_evolve_impl(void *ctx, float intensity) {
    DeepSeekEvolveContext *dec = (DeepSeekEvolveContext*)ctx;
    AB4HPixel *pixels = dec->pixels;
    int W = dec->width;
    int H = dec->height;
    int frame = dec->frame_counter;
    (void)intensity;

    // Neutral zone region: Center vertical band
    float scale_x = (float)W / 800.0f;
    int nz_left = (int)((neutral_zone_x - 50.0f) * scale_x);
    int nz_right = (int)((neutral_zone_x + 50.0f) * scale_x);

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            if (x >= nz_left && x <= nz_right) {
                // Generate colorful POKEY static LFSR noise inside the Neutral Zone
                float input_noise = (float)x * 12.9898f + (float)y * 78.233f + (float)(frame % 10) * 1.543f;
                float noise_r = sinf(input_noise * 1.1f) * 43758.5453f;
                float noise_g = sinf(input_noise * 1.3f) * 43758.5453f;
                float noise_b = sinf(input_noise * 1.7f) * 43758.5453f;
                
                noise_r = noise_r - floorf(noise_r);
                noise_g = noise_g - floorf(noise_g);
                noise_b = noise_b - floorf(noise_b);

                // Blend with existing backdrop
                AB4HPixel curr = pixels[y * W + x];
                float cr = half_to_float(curr.r), cg = half_to_float(curr.g), cb = half_to_float(curr.b), ca = half_to_float(curr.a);
                
                float nr = cr * 0.4f + noise_r * 0.6f;
                float ng = cg * 0.4f + noise_g * 0.6f;
                float nb = cb * 0.4f + noise_b * 0.6f;
                float na = ca * 0.4f + 0.6f;

                pixels[y * W + x] = make_ab4h_pixel(nr, ng, nb, na);
            }
        }
    }
}

int main() {
    printf("[YAR] Booting Wayland Vulkan Yars' Revenge...\n");
    signal(SIGPIPE, SIG_IGN);
    setenv("TSFI_AB4H", "1", 1);

    VulkanSystem *s = create_vulkan_system();
    if (!s) return 1;
    lau_unseal_object(s);
    s->disable_ui_overlay = true;

    if (!lau_yul_thunk_init("yars_revenge", "../solidity/bin/yars_revenge.yul", 0x1)) {
        printf("[YAR] Error: Failed to initialize Yul yars_revenge thunk!\n");
        destroy_vulkan_system(s);
        return 1;
    }

    init_yars_yul();
    tsfi_input_set_key_hook((void*)yars_key_hook);

    TSFiFontSystem *ai_fs = tsfi_font_init();
    if (ai_fs) {
        tsfi_font_ai_init(ai_fs);
    }

    int frame_counter = 0;
    AB4HPixel neon_pink = make_ab4h_pixel(1.0f, 0.05f, 0.6f, 1.0f);
    AB4HPixel neon_cyan = make_ab4h_pixel(0.0f, 0.9f, 1.0f, 1.0f);
    AB4HPixel neon_yellow = make_ab4h_pixel(0.9f, 0.9f, 0.0f, 1.0f);
    AB4HPixel neon_green = make_ab4h_pixel(0.0f, 1.0f, 0.3f, 1.0f);
    AB4HPixel block_color = make_ab4h_pixel(0.8f, 0.3f, 0.9f, 1.0f);
    AB4HPixel neutral_tint = make_ab4h_pixel(0.1f, 0.05f, 0.2f, 0.3f);

    while (!exit_requested) {
        int W = s->width;
        int H = s->height;

        AB4HPixel *pixels = (AB4HPixel *)s->paint_buffer->data;
        if (!pixels) {
            tsfi_raw_usleep(10000);
            continue;
        }

        // 1. Clear background to dark space
        memset(pixels, 0, W * H * sizeof(AB4HPixel));

        // 2. Gather Inputs
        int tx = 0, ty = 0, fire = 0, action = 0;
        if (key_a_held) tx = -1;
        else if (key_d_held) tx = 1;
        
        if (key_w_held) ty = -1;
        else if (key_s_held) ty = 1;

        if (key_space_pressed) {
            fire = 1;
            key_space_pressed = 0;
        }
        if (key_shift_held) {
            action = 1;
        }

        // Tick game state
        update_yars_yul(tx, ty, fire, action);

        // Grid/scaling parameters mapping 800x540 logical view to Wayland window resolution
        float scale_x = (float)W / 800.0f;
        float scale_y = (float)(H - 40) / 540.0f;

        // 3. Draw Neutral Zone Tint
        float nz_x = neutral_zone_x * scale_x;
        draw_rect_ab4h(pixels, W, H, (int)(nz_x - 50.0f * scale_x), 0, (int)(100.0f * scale_x), H - 40, neutral_tint);

        // 4. Draw Shield Blocks
        // 5 columns at X: 600, 610, 620, 630, 640
        // 32 rows at Y: 110 to 430
        for (int col = 0; col < 5; col++) {
            uint64_t col_mask = shield_data[col];
            int block_x = (int)((600.0f + col * 10.0f) * scale_x);
            for (int row = 0; row < 32; row++) {
                if (col_mask & (1U << row)) {
                    int block_y = (int)((110.0f + row * 10.0f) * scale_y);
                    draw_rect_ab4h(pixels, W, H, block_x, block_y, (int)(8.0f * scale_x), (int)(8.0f * scale_y), block_color);
                }
            }
        }

        // 5. Draw Yar (Insect/Fly sprite representation)
        float yx = yar_x * scale_x;
        float yy = yar_y * scale_y;
        if (game_status != 2) {
            // Draw Wings
            float wing_offset = 8.0f * sinf(frame_counter * 0.5f);
            draw_line_aa(pixels, W, H, yx - 12, yy - wing_offset, yx, yy, neon_cyan, 2.0f);
            draw_line_aa(pixels, W, H, yx - 12, yy + wing_offset, yx, yy, neon_cyan, 2.0f);
            
            // Draw Body
            draw_rect_ab4h(pixels, W, H, (int)yx - 6, (int)yy - 8, 12, 16, neon_green);
            
            // Draw Head & Antennae
            draw_line_aa(pixels, W, H, yx, yy - 8, yx + 6, yy - 12, neon_pink, 1.5f);
            draw_line_aa(pixels, W, H, yx, yy - 8, yx + 6, yy - 4, neon_pink, 1.5f);
        }

        // 6. Draw Qotile (Shield/Base Swirl Enemy)
        float qx = qotile_x * scale_x;
        float qy = qotile_y * scale_y;
        if (swirl_active == 1) {
            // Spinning warning swirl
            float spin_ang = frame_counter * 0.2f;
            draw_line_aa(pixels, W, H, qx - 15 * cosf(spin_ang), qy - 15 * sinf(spin_ang), 
                         qx + 15 * cosf(spin_ang), qy + 15 * sinf(spin_ang), neon_pink, 3.0f);
            draw_line_aa(pixels, W, H, qx - 15 * sinf(spin_ang), qy + 15 * cosf(spin_ang), 
                         qx + 15 * sinf(spin_ang), qy - 15 * cosf(spin_ang), neon_pink, 3.0f);
        } else {
            // Normal base
            draw_rect_ab4h(pixels, W, H, (int)qx - 10, (int)qy - 25, 20, 50, neon_yellow);
        }

        // 7. Draw Zorlon Cannon ( primed glowing ball )
        if (cannon_active == 2) {
            float cx = cannon_x * scale_x;
            float cy = cannon_y * scale_y;
            draw_rect_ab4h(pixels, W, H, (int)cx - 8, (int)cy - 8, 16, 16, neon_pink);
            draw_line_aa(pixels, W, H, cx - 20, cy, cx + 20, cy, neon_yellow, 2.0f);
        }

        // 8. Draw Swirl (Fired swirling projectile)
        if (swirl_active == 2) {
            float sx = swirl_x * scale_x;
            float sy = swirl_y * scale_y;
            float spin_ang = frame_counter * 0.4f;
            draw_line_aa(pixels, W, H, sx - 12 * cosf(spin_ang), sy - 12 * sinf(spin_ang), 
                         sx + 12 * cosf(spin_ang), sy + 12 * sinf(spin_ang), neon_pink, 2.5f);
        }

        // 9. Status bar layout
        if (game_status == 1) {
            strcpy(status_message, "^B^U[VICTORY]^C ^IGREAT SHOT! QOTILE DESTROYED! ^C[R] TO NEXT ROUND.");
        } else if (game_status == 2) {
            strcpy(status_message, "^B^U[RESPAWN]^C ^IYAR WAS HIT! ^C[R] TO REPLAY ROUND.");
        } else if (game_status == 3) {
            strcpy(status_message, "^B^U[GAME OVER]^C ^INO LIVES REMAINING! ^C[R] TO REPLAY.");
        } else {
            sprintf(status_message, "^B[YARS] Lives: %d | Score: %d | Cannon: %s | Swirl: %s^C", 
                    lives, score, 
                    cannon_active == 1 ? "READY" : (cannon_active == 2 ? "FIRED" : "INACTIVE"),
                    swirl_active == 2 ? "LAUNCHED" : (swirl_active == 1 ? "SPINNING" : "INACTIVE"));
        }

        // Draw Footer Interface bar
        draw_rect_ab4h(pixels, W, H, 0, H - 40, W, 40, make_ab4h_pixel(0.01f, 0.01f, 0.06f, 1.0f));
        draw_line_aa(pixels, W, H, 0.0f, (float)(H - 40), (float)W, (float)(H - 40), neon_cyan, 1.5f);
        draw_string_ab4h(pixels, W, H, status_message, 20, H - 30, neon_yellow);

        // 10. DeepSeek Static Noise Layer
        if (ai_fs) {
            DeepSeekEvolveContext dec = {
                .pixels = pixels,
                .width = W,
                .height = H,
                .frame_counter = frame_counter
            };
            tsfi_font_ai_bind_evolve(ai_fs, deepseek_evolve_impl, &dec);
            tsfi_font_ai_invoke_evolve(ai_fs, 0.6f);
        }

        draw_frame(s);
        frame_counter++;
        tsfi_raw_usleep(16000);
    }

    printf("[YAR] Teardown Vulkan context.\n");
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
