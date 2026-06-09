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
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"
#include "tsfi_staging.h"
#include "tsfi_font_vectors.h"
#include "tsfi_font_ai.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Forward declaration of input hook setter
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
    int min_y = fmaxf(0.0f, fminf(x0, x1) - thickness - 1.0f);
    int max_y = fminf(h - 1, fmaxf(x0, x1) + thickness + 1.0f);
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

// ZMM VM State & helper execution commands
static TsfiZmmVmState vm;
static char status_message[256] = "^B^U[GAUNTLET ARCHON]^C ^IWarrior needs alchemical gold. Move: WASD, Fire: Space^C";
static int exit_requested = 0;
static bool key_a_held = false;
static bool key_d_held = false;
static bool key_w_held = false;
static bool key_s_held = false;
static bool key_space_held = false;
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
    if (strcmp(sd->type, "shoot") == 0) {
        len = 1200; buf = malloc(len);
        if (buf) {
            for (int i = 0; i < len; i++) {
                float t = (float)i / 8000.0f;
                buf[i] = 128 + (int)((1.0f - t/0.15f) * 80.0f * sinf(300.0f * t * 2.0f * 3.14159f + 5.0f * sinf(120.0f * t)));
            }
        }
    } else if (strcmp(sd->type, "hit") == 0) {
        len = 2400; buf = malloc(len);
        if (buf) {
            for (int i = 0; i < len; i++) {
                float t = (float)i / 8000.0f;
                buf[i] = 128 + (int)((1.0f - t/0.3f) * 90.0f * sinf(100.0f * t * 2.0f * 3.14159f));
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

static void thunk_poke(uint64_t addr, uint64_t val) {
    char cmd[512];
    sprintf(cmd, "YULEXEC \"cpu6502\", \"8029e7c0%064lx%064lx\"", addr, val);
    tsfi_zmm_vm_exec(&vm, cmd);
}

static uint64_t thunk_peek(uint64_t addr) {
    char cmd[512];
    sprintf(cmd, "YULEXEC \"cpu6502\", \"7861d269%064lx\"", addr);
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    
    char *endptr = NULL;
    uint64_t ret = 0;
    if (strlen(vm.output_buffer) >= 16) {
        ret = strtoull(vm.output_buffer + strlen(vm.output_buffer) - 16, &endptr, 16);
    } else {
        ret = strtoull(vm.output_buffer, &endptr, 16);
    }
    return ret;
}

static void init_gauntlet_yul() {
    thunk_poke(54695, 0xd17a5);  // diyatAddress mock
    thunk_poke(55050, 1);    // gauntletActive
    thunk_poke(55051, 120);  // playerX
    thunk_poke(55052, 120);  // playerY
    thunk_poke(55053, 2000); // health
    thunk_poke(55054, 0);    // keys
    thunk_poke(55055, 320);  // ghostX
    thunk_poke(55056, 240);  // ghostY
    thunk_poke(55057, 440);  // spawnerX
    thunk_poke(55058, 360);  // spawnerY
    thunk_poke(55061, 0);    // spawnTimer
    thunk_poke(55062, 1);    // generation (linage counter)
}

static void gauntlet_key_hook(void *data, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    (void)data; (void)serial; (void)time;
    bool pressed = (state != 0);
    
    if (key == 1) { // ESC
        printf("[GAUNTLET] ESC pressed (ignoring to allow AI playout)\n");
    } else if (key == 30) { // A
        key_a_held = pressed;
    } else if (key == 32) { // D
        key_d_held = pressed;
    } else if (key == 17) { // W
        key_w_held = pressed;
    } else if (key == 31) { // S
        key_s_held = pressed;
    } else if (key == 57) { // Space
        key_space_held = pressed;
    } else if (key == 19) { // R -> restart
        init_gauntlet_yul();
    } else if (key == 20) { // T -> Autopilot toggle
        ai_mode = !ai_mode;
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
                float noise_factor = 1.0f + (noise - 0.5f) * 0.03f;
                r *= noise_factor;
                g *= noise_factor;
                b *= noise_factor;
                pixels[y * W + x] = make_ab4h_pixel(r, g, b, a);
            }
        }
    }
}
static const char *sprite_warrior[16] = {
    "..HHHHHH........",
    ".HHHHHHHH.......",
    "HHHDHHDHHH..H...",
    "HWWWWWWWWH..HH..",
    ".WDWWWWDW...HH..",
    "..WWWWWW..HHHHHH",
    "..BBBBBB...HH...",
    ".BBBBBBBB..HH...",
    ".BBBBBBBB.......",
    "..BBBBBB........",
    "..BBBBBB........",
    "..HHHHHH........",
    "..H....H........",
    ".DD....DD.......",
    ".DD....DD.......",
    "................"
};

static const char *sprite_ghost[16] = {
    ".....GGGGGG.....",
    "...GGGGGGGGGG...",
    "..GGGGGGGGGGGG..",
    ".GGGGGGGGGGGGGG.",
    "GGwDGGGGGGwDGGGG",
    "GDDGGGGGGDDGGGGG",
    "GGGGGGGGGGGGGGGG",
    "GGGGGGGGGGGGGGGG",
    "GGGGGGGGGGGGGGGG",
    "GGGGGGGGGGGGGGGG",
    "GGGGGGGGGGGGGGGG",
    "GGGGGGGGGGGGGGGG",
    ".GGGGGGGGGGGGGG.",
    ".G.G.GGGG.G.G...",
    "G...G..G...G....",
    "................"
};

static const char *sprite_spawner[16] = {
    "OOOOOOOOOOOOOOOO",
    "ODDDDDDDDDDDDDDO",
    "ODwwDwwDDwwDwwDO",
    "ODwDwwDwwDwwDwDO",
    "ODDDDDDDDDDDDDDO",
    "ODwwwwwwwwwwwwDO",
    "ODwDDwDwDwDDwDDO",
    "ODwDwwDwDwwDwDDO",
    "ODwDwwDwDwwDwDDO",
    "ODwDDwDwDwDDwDDO",
    "ODwwwwwwwwwwwwDO",
    "ODDDDDDDDDDDDDDO",
    "ODwDwwDwwDwwDwDO",
    "ODwwDwwDDwwDwwDO",
    "ODDDDDDDDDDDDDDO",
    "OOOOOOOOOOOOOOOO"
};

void draw_sprite_16x16(AB4HPixel *pixels, int w, int h, float cx, float cy, const char *sprite[16], float scale_x, float scale_y) {
    AB4HPixel skin = make_ab4h_pixel(0.9f, 0.7f, 0.5f, 1.0f);
    AB4HPixel warrior_blue = make_ab4h_pixel(0.0f, 0.6f, 1.0f, 1.0f);
    AB4HPixel ghost_green = make_ab4h_pixel(0.2f, 0.9f, 0.3f, 1.0f);
    AB4HPixel spawner_red = make_ab4h_pixel(1.0f, 0.2f, 0.2f, 1.0f);
    AB4HPixel gold_yellow = make_ab4h_pixel(1.0f, 0.8f, 0.0f, 1.0f);
    AB4HPixel dark_gray = make_ab4h_pixel(0.15f, 0.15f, 0.15f, 1.0f);
    AB4HPixel steel = make_ab4h_pixel(0.7f, 0.7f, 0.7f, 1.0f);
    AB4HPixel orange = make_ab4h_pixel(0.8f, 0.4f, 0.1f, 1.0f);
    AB4HPixel white = make_ab4h_pixel(1.0f, 1.0f, 1.0f, 1.0f);

    int sprite_w = 16;
    int sprite_h = 16;
    int half_w = (int)((sprite_w * scale_x) / 2);
    int half_h = (int)((sprite_h * scale_y) / 2);

    int start_x = (int)cx - half_w;
    int start_y = (int)cy - half_h;

    for (int sy = 0; sy < sprite_h; sy++) {
        for (int sx = 0; sx < sprite_w; sx++) {
            char c = sprite[sy][sx];
            if (c == '.') continue;

            AB4HPixel color = dark_gray;
            switch(c) {
                case 'W': color = skin; break;
                case 'B': color = warrior_blue; break;
                case 'G': color = ghost_green; break;
                case 'R': color = spawner_red; break;
                case 'Y': color = gold_yellow; break;
                case 'D': color = dark_gray; break;
                case 'H': color = steel; break;
                case 'O': color = orange; break;
                case 'w': color = white; break;
            }

            int cell_start_x = start_x + (int)(sx * scale_x);
            int cell_start_y = start_y + (int)(sy * scale_y);
            int cell_w = (int)scale_x;
            if (cell_w < 1) cell_w = 1;
            int cell_h = (int)scale_y;
            if (cell_h < 1) cell_h = 1;

            draw_rect_ab4h(pixels, w, h, cell_start_x, cell_start_y, cell_w, cell_h, color);
        }
    }
}

int main() {
    printf("[GAUNTLET] Launching ZMM VM Vulkan-Wayland Gauntlet Module...\n");
    signal(SIGPIPE, SIG_IGN);
    setenv("TSFI_AB4H", "1", 1);

    tsfi_wire_firmware_init();
    tsfi_zmm_vm_init(&vm);

    // Compile and initialize Yul CPU target inside ZMM VM
    printf("[ZMM] Compiler loading folklore.yul...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"cpu6502\", \"../solidity/bin/folklore.yul\", 1");

    VulkanSystem *s = create_vulkan_system();
    if (!s) {
        tsfi_zmm_vm_destroy(&vm);
        return 1;
    }
    lau_unseal_object(s);
    s->disable_ui_overlay = true;

    init_gauntlet_yul();
    tsfi_input_set_key_hook((void*)gauntlet_key_hook);

    TSFiFontSystem *ai_fs = tsfi_font_init();
    if (ai_fs) {
        tsfi_font_ai_init(ai_fs);
    }

    // Curated Neon Palette
    AB4HPixel bg_cyber_black = make_ab4h_pixel(0.01f, 0.0f, 0.02f, 1.0f);
    AB4HPixel warrior_blue = make_ab4h_pixel(0.0f, 0.8f, 1.2f, 1.0f);
    AB4HPixel ghost_green = make_ab4h_pixel(0.05f, 0.9f, 0.3f, 1.0f);
    AB4HPixel spawner_red = make_ab4h_pixel(1.2f, 0.1f, 0.2f, 1.0f);
    (void)warrior_blue; (void)ghost_green; (void)spawner_red;
    AB4HPixel hud_pink = make_ab4h_pixel(1.0f, 0.05f, 0.6f, 1.0f);
    AB4HPixel gold_yellow = make_ab4h_pixel(1.2f, 1.1f, 0.0f, 1.0f);

    int frame_counter = 0;

    while (s->running && !exit_requested) {
        wl_display_roundtrip(s->display);
        
        if (!s->vk->swapchain) {
            init_swapchain(s);
        }

        AB4HPixel *pixels = (AB4HPixel*)s->paint_buffer->data;
        int W = s->paint_buffer->width;
        int H = s->paint_buffer->height;

        // Clear view
        draw_rect_ab4h(pixels, W, H, 0, 0, W, H, bg_cyber_black);

        // Scale factors
        float scale_x = (float)W / 720.0f;
        float scale_y = (float)(H - 50) / 480.0f;

        // Grid lines
        for (int x = 0; x < 720; x += 40) {
            draw_line_aa(pixels, W, H, x * scale_x, 0, x * scale_x, 480 * scale_y, make_ab4h_pixel(0.1f, 0.05f, 0.15f, 0.3f), 1.0f);
        }
        for (int y = 0; y < 480; y += 40) {
            draw_line_aa(pixels, W, H, 0, y * scale_y, 720 * scale_x, y * scale_y, make_ab4h_pixel(0.1f, 0.05f, 0.15f, 0.3f), 1.0f);
        }

        // Gather Inputs
        int moveDir = 0;
        bool fire = false;
        
        if (ai_mode) {
            uint64_t raw_px = thunk_peek(55051);
            uint64_t raw_py = thunk_peek(55052);
            uint64_t raw_gx = thunk_peek(55055);
            uint64_t raw_gy = thunk_peek(55056);
            uint64_t raw_sx = thunk_peek(55057);
            uint64_t raw_sy = thunk_peek(55058);

            uint64_t tx = 0, ty = 0;
            bool target_found = false;
            
            // Prioritize destroying the spawner first, then target the ghost
            if (raw_sx > 0 && raw_sy > 0) {
                tx = raw_sx;
                ty = raw_sy;
                target_found = true;
            } else if (raw_gx > 0 && raw_gy > 0) {
                tx = raw_gx;
                ty = raw_gy;
                target_found = true;
            }
            
            if (target_found) {
                int64_t dx = (int64_t)tx - (int64_t)raw_px;
                int64_t dy = (int64_t)ty - (int64_t)raw_py;
                
                // Move along the axis with the largest distance
                if (labs(dx) > labs(dy)) {
                    if (dx > 0) moveDir = 4; // Right
                    else moveDir = 3;        // Left
                } else {
                    if (dy > 0) moveDir = 2; // Down
                    else moveDir = 1;        // Up
                }
                
                // Shoot if lined up or close
                if (labs(dx) < 80 || labs(dy) < 80) {
                    fire = (frame_counter % 6 == 0); // Limit rate of fire
                }
            } else {
                // No threat remains -> Victory screen!
                moveDir = 0;
                fire = false;
            }
        } else {
            if (key_w_held) moveDir = 1;
            else if (key_s_held) moveDir = 2;
            else if (key_a_held) moveDir = 3;
            else if (key_d_held) moveDir = 4;
            if (key_space_held) fire = true;
        }

        thunk_poke(55059, moveDir);
        if (fire) {
            thunk_poke(55060, 1);
            play_synth_sound("shoot");
        }

        // Trigger physics tick in Yul
        thunk_poke(55024, 1);

        // Peek Yul memory for updated positions
        float px = (float)thunk_peek(55051) * scale_x;
        float py = (float)thunk_peek(55052) * scale_y;
        float gx = (float)thunk_peek(55055) * scale_x;
        float gy = (float)thunk_peek(55056) * scale_y;
        float sx = (float)thunk_peek(55057) * scale_x;
        float sy = (float)thunk_peek(55058) * scale_y;
        uint64_t health = thunk_peek(55053);
        uint64_t score = thunk_peek(55032);
        uint64_t keys = thunk_peek(55054);
        (void)keys;

        // Draw Spawner (pulsing brick generator)
        if (sx > 0.0f && sy > 0.0f) {
            float spawner_pulse = 2.4f + 0.2f * sinf(frame_counter * 0.1f);
            draw_sprite_16x16(pixels, W, H, sx, sy, sprite_spawner, spawner_pulse * scale_x, spawner_pulse * scale_y);
            draw_radial_glow(pixels, W, H, sx, sy, 35.0f, make_ab4h_pixel(1.0f, 0.1f, 0.0f, 0.3f));
        }

        // Draw Ghost (classic arcade ghost sprite)
        if (gx > 0.0f && gy > 0.0f) {
            draw_sprite_16x16(pixels, W, H, gx, gy, sprite_ghost, 2.0f * scale_x, 2.0f * scale_y);
        }

        // Draw Genealogy Lineage Line (William C. Brauch Family Tree connection)
        if (gx > 0.0f && gy > 0.0f && sx > 0.0f && sy > 0.0f) {
            for (float t = 0.0f; t <= 1.0f; t += 0.06f) {
                float lx = sx + (gx - sx) * t;
                float ly = sy + (gy - sy) * t;
                draw_rect_ab4h(pixels, W, H, (int)lx, (int)ly, 3, 3, make_ab4h_pixel(1.2f, 0.9f, 0.0f, 0.5f));
            }
        }

        // Draw Player (warrior holding axe)
        draw_sprite_16x16(pixels, W, H, px, py, sprite_warrior, 2.2f * scale_x, 2.2f * scale_y);
        draw_radial_glow(pixels, W, H, px, py, 20.0f, make_ab4h_pixel(0.0f, 0.8f, 1.2f, 0.3f));

        // Format and render HUD details
        static int victory_exit_timer = -1;
        uint64_t generation = thunk_peek(55062);
        if (gx <= 0.0f && sx <= 0.0f) {
            sprintf(status_message, "^B^U[VICTORY]^C ^IAll threats neutralized! Alchemical gold harvested! SCORE: %lu^C", score);
            if (victory_exit_timer == -1) {
                printf("[VICTORY] AI pilot successfully destroyed all threats and won the game! Final Score: %lu\n", score);
                victory_exit_timer = 60; // Wait 60 frames (1s) to show screen before exiting
            }
        } else {
            sprintf(status_message, "^B[WARRIOR] Health: %lu | Score: %lu | Ghost Gen: %lu^C", health, score, generation);
        }
        
        if (victory_exit_timer > 0) {
            victory_exit_timer--;
            if (victory_exit_timer == 0) {
                exit_requested = 1;
            }
        }
        draw_rect_ab4h(pixels, W, H, 0, H - 50, W, 50, make_ab4h_pixel(0.02f, 0.01f, 0.05f, 1.0f));
        draw_line_aa(pixels, W, H, 0.0f, (float)(H - 50), (float)W, (float)(H - 50), hud_pink, 2.0f);
        draw_string_ab4h(pixels, W, H, status_message, 20, H - 32, gold_yellow);

        // DeepSeek graphics processing
        if (ai_fs) {
            DeepSeekEvolveContext dec = {
                .pixels = pixels,
                .width = W,
                .height = H,
                .frame_counter = frame_counter
            };
            tsfi_font_ai_bind_evolve(ai_fs, deepseek_evolve_impl, &dec);
            float current_intensity = 0.5f + 0.5f * sinf(frame_counter * 0.08f);
            tsfi_font_ai_invoke_evolve(ai_fs, current_intensity);
        }

        draw_frame(s);
        frame_counter++;
        tsfi_raw_usleep(16000); // Checked timer speed limit ~60fps
    }

    printf("[GAUNTLET] Tearing down resources...\n");
    if (ai_fs) {
        tsfi_font_ai_destroy(ai_fs);
        tsfi_font_destroy(ai_fs);
    }
    destroy_vulkan_system(s);
    tsfi_zmm_vm_destroy(&vm);
    return 0;
}
