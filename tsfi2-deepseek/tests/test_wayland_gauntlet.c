#define _GNU_SOURCE
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

static int g_frame_counter = 0;

static inline float smin(float a, float b, float k) {
    float h = fmaxf(k - fabsf(a - b), 0.0f) / k;
    return fminf(a, b) - h * h * k * 0.25f;
}

static float sdf_teddy(float x, float y, float z) {
    float d_body = sqrtf(x*x*1.2f + y*y*0.8f + z*z*1.2f) - 0.35f;
    float d_head = sqrtf(x*x + (y - 0.35f)*(y - 0.35f) + z*z) - 0.25f;
    float ex = fabsf(x) - 0.2f, ey = y - 0.55f, ez = z;
    float d_ear = sqrtf(ex*ex + ey*ey + ez*ez) - 0.08f;
    float d_snout = sqrtf(x*x + (y - 0.3f)*(y - 0.3f) + (z - 0.2f)*(z - 0.2f)) - 0.1f;
    
    float ax = fabsf(x) - 0.35f, ay = y - 0.1f, az = z;
    float d_arm = sqrtf(ax*ax*1.5f + ay*ay*1.5f + az*az) - 0.1f;
    
    float lx = fabsf(x) - 0.22f, ly = y + 0.35f, lz = z - 0.1f;
    float d_leg = sqrtf(lx*lx + ly*ly + lz*lz) - 0.12f;
    
    float res = smin(d_body, d_head, 0.12f);
    res = smin(res, d_ear, 0.04f);
    res = smin(res, d_snout, 0.05f);
    res = smin(res, d_arm, 0.05f);
    res = smin(res, d_leg, 0.05f);
    return res;
}

static float sdf_crow(float x, float y, float z) {
    float d_body = sqrtf(x*x*1.5f + y*y + z*z) - 0.3f;
    float d_head = sqrtf((x - 0.3f)*(x - 0.3f) + (y - 0.2f)*(y - 0.2f) + z*z) - 0.15f;
    float bx = x - 0.45f, by = y - 0.2f, bz = z;
    float d_beak = sqrtf(bx*bx*5.0f + by*by*5.0f + bz*bz*5.0f) - 0.1f;
    float wx = fabsf(x), wy = y - 0.1f, wz = fabsf(z) - 0.25f;
    float d_wing = sqrtf(wx*wx*2.0f + wy*wy*0.5f + wz*wz*3.0f) - 0.12f;
    float res = smin(d_body, d_head, 0.08f);
    res = smin(res, d_wing, 0.05f);
    res = smin(res, d_beak, 0.03f);
    return res;
}

static float eval_sdf(const char *query, float x, float y, float z) {
    if (strcasestr(query, "crow") || strcasestr(query, "bird")) {
        return sdf_crow(x, y, z);
    }
    return sdf_teddy(x, y, z);
}

static uint32_t get_sdf_color(const char *query, float x, float y, float z, float intensity) {
    uint8_t r = 0, g = 0, b = 0;
    if (strcasestr(query, "crow") || strcasestr(query, "bird")) {
        float bx = x - 0.45f, by = y - 0.2f, bz = z;
        float eyex = x - 0.32f, eyey = y - 0.23f, eyez = fabsf(z) - 0.12f;
        if (sqrtf(eyex*eyex + eyey*eyey + eyez*eyez) < 0.03f) {
            r = 248; g = 248; b = 242; // White eye
        } else if (sqrtf(eyex*eyex + eyey*eyey + (eyez-0.01f)*(eyez-0.01f)) < 0.015f) {
            r = 0; g = 0; b = 0; // Black pupil
        } else if (sqrtf(bx*bx*5.0f + by*by*5.0f + bz*bz*5.0f) - 0.1f < 0.05f) {
            r = 255; g = 184; b = 108; // Orange beak
        } else {
            r = 40; g = 42; b = 54; // Dark gray body
        }
    } else {
        // Teddy Bear
        float eyex = fabsf(x) - 0.08f;
        float eyey = y - 0.4f;
        float eyez = z - 0.22f;
        float nosex = x, nosey = y - 0.32f, nosez = z - 0.29f;
        if (sqrtf(eyex*eyex + eyey*eyey + eyez*eyez) < 0.04f) {
            r = 10; g = 10; b = 10;
        } else if (sqrtf(nosex*nosex + nosey*nosey + nosez*nosez) < 0.03f) {
            r = 0; g = 0; b = 0;
        } else {
            r = 180; g = 120; b = 80; // Brown body
        }
    }
    uint32_t cr = (uint32_t)(r * intensity);
    uint32_t cg = (uint32_t)(g * intensity);
    uint32_t cb = (uint32_t)(b * intensity);
    if (cr > 255) cr = 255;
    if (cg > 255) cg = 255;
    if (cb > 255) cb = 255;
    return 0xFF000000 | (cr << 16) | (cg << 8) | cb;
}

static void draw_3d_stuffed_animal(uint32_t *buffer, int w_width, int w_height, int cx, int cy, int size, const char *query, int frame) {
    float cosY = cosf(frame * 0.08f), sinY = sinf(frame * 0.08f);
    float cosX = cosf(frame * 0.05f), sinX = sinf(frame * 0.05f);
    int r_bound = size;
    int halo_bound = (int)(size * 1.3f);
    for (int dy = -halo_bound; dy <= halo_bound; dy++) {
        for (int dx = -halo_bound; dx <= halo_bound; dx++) {
            int tx = cx + dx;
            int ty = cy + dy;
            if (tx < 0 || tx >= w_width || ty < 0 || ty >= w_height) continue;
            
            float dist_from_center = sqrtf(dx*dx + dy*dy) / (float)size;
            if (dist_from_center < 1.3f) {
                uint32_t orig = buffer[ty * w_width + tx];
                uint8_t r = (orig >> 16) & 0xFF;
                uint8_t g = (orig >> 8) & 0xFF;
                uint8_t b = orig & 0xFF;
                float dim = 0.15f + 0.85f * (dist_from_center / 1.3f);
                r = (uint8_t)(r * dim);
                g = (uint8_t)(g * dim);
                b = (uint8_t)(b * dim);
                buffer[ty * w_width + tx] = 0xFF000000 | (r << 16) | (g << 8) | b;
            }
            
            if (abs(dx) <= r_bound && abs(dy) <= r_bound) {
                float rx = (float)dx / (float)size;
                float ry = -(float)dy / (float)size;
                float ro_x = rx, ro_y = ry, ro_z = -1.5f;
                float rd_x = 0.0f, rd_y = 0.0f, rd_z = 1.0f;
                float t = 0.0f;
                int hit = 0;
                float hx = 0, hy = 0, hz = 0;
                for (int step = 0; step < 32; step++) {
                    float px = ro_x + rd_x * t;
                    float py = ro_y + rd_y * t;
                    float pz = ro_z + rd_z * t;
                    float rot_x = px * cosY - pz * sinY;
                    float rot_z = px * sinY + pz * cosY;
                    float rot_y = py * cosX - rot_z * sinX;
                    rot_z = py * sinX + rot_z * cosX;
                    float d = eval_sdf(query, rot_x, rot_y, rot_z);
                    if (d < 0.01f) {
                        hit = 1;
                        hx = rot_x; hy = rot_y; hz = rot_z;
                        break;
                    }
                    t += d;
                    if (t > 3.0f) break;
                }
                if (hit) {
                    float eps = 0.01f;
                    float nx = eval_sdf(query, hx + eps, hy, hz) - eval_sdf(query, hx - eps, hy, hz);
                    float ny = eval_sdf(query, hx, hy + eps, hz) - eval_sdf(query, hx, hy - eps, hz);
                    float nz = eval_sdf(query, hx, hy, hz + eps) - eval_sdf(query, hx, hy, hz - eps);
                    float n_len = sqrtf(nx*nx + ny*ny + nz*nz);
                    if (n_len > 0.0f) {
                        nx /= n_len; ny /= n_len; nz /= n_len;
                    }
                    float lx = 0.577f, ly = 0.577f, lz = -0.577f;
                    float dot = nx * lx + ny * ly + nz * lz;
                    float intensity = dot * 0.6f + 0.4f;
                    if (intensity < 0.0f) intensity = 0.0f;
                    if (intensity > 1.0f) intensity = 1.0f;
                    buffer[ty * w_width + tx] = get_sdf_color(query, hx, hy, hz, intensity);
                }
            }
        }
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
    thunk_poke(54696, 0xd17a6);  // pmgAddress target
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
        printf("[GAUNTLET] ESC pressed. Exiting game...\n");
        exit_requested = 1;
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
// (Removed unused sprite_warrior/sprite_ghost layouts to comply with -Werror compilation rules)

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
    const char *yul_path = "../solidity/bin/folklore.yul";
    FILE *f_check = fopen(yul_path, "r");
    if (f_check) {
        fclose(f_check);
    } else {
        yul_path = "solidity/bin/folklore.yul";
    }
    char init_cmd[256];
    snprintf(init_cmd, sizeof(init_cmd), "YULINIT \"cpu6502\", \"%s\", 1", yul_path);
    tsfi_zmm_vm_exec(&vm, init_cmd);

    // Compile and initialize PMG System inside ZMM VM
    printf("[ZMM] Compiler loading pmgSystem.yul...\n");
    const char *pmg_yul_path = "../solidity/bin/pmgSystem.yul";
    FILE *pmg_check = fopen(pmg_yul_path, "r");
    if (pmg_check) {
        fclose(pmg_check);
    } else {
        pmg_yul_path = "solidity/bin/pmgSystem.yul";
    }
    char pmg_init_cmd[256];
    snprintf(pmg_init_cmd, sizeof(pmg_init_cmd), "YULINIT \"PmgSystem\", \"%s\", 858022", pmg_yul_path);
    tsfi_zmm_vm_exec(&vm, pmg_init_cmd);

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
    AB4HPixel hud_pink = make_ab4h_pixel(1.0f, 0.05f, 0.6f, 1.0f);
    AB4HPixel gold_yellow = make_ab4h_pixel(1.2f, 1.1f, 0.0f, 1.0f);
    g_frame_counter = 0;

    while (s->running && !exit_requested) {
        wl_display_roundtrip(s->display);
        
        if (!s->vk->swapchain) {
            init_swapchain(s);
        }

        AB4HPixel *pixels = (AB4HPixel*)s->paint_buffer->data;
        int W = s->paint_buffer->width;
        int H = s->paint_buffer->height;

        // VAE presentation state & dynamic LTI step
        #include "tsfi_vae_firmware.h"
        static LauVaeFirmware_State g_gauntlet_vae;
        static float *g_gauntlet_density = NULL;
        static int g_gauntlet_density_w = 0;
        static int g_gauntlet_density_h = 0;
        static NandTrapState nt_g;
        static bool nt_g_init = false;

        if (!g_gauntlet_density || g_gauntlet_density_w != W || g_gauntlet_density_h != H) {
            if (g_gauntlet_density) free(g_gauntlet_density);
            g_gauntlet_density = (float*)malloc(W * H * sizeof(float));
            g_gauntlet_density_w = W;
            g_gauntlet_density_h = H;
        }

        if (!nt_g_init) {
            memset(&nt_g, 0, sizeof(NandTrapState));
            nt_g.magic = NAND_TRAP_MAGIC;
            nt_g.version = NAND_TRAP_VERSION;
            for (int i = 0; i < GRANS; i++) {
                nt_g.fibers[i].x = sinf((float)i * 0.05f) * 0.5f;
                nt_g.fibers[i].y = cosf((float)i * 0.05f) * 0.5f;
                nt_g.fibers[i].weight = 1.0f;
            }
            nt_g_init = true;
        }

        // Apply LTI Causal Discovery step to morph background
        NandTrapState next_nt_g;
        tsfi_nand_trap_causal_discovery(&nt_g, &next_nt_g, 0.005f);
        nt_g = next_nt_g;

        tsfi_vae_firmware_boot(&g_gauntlet_vae, &nt_g);
        tsfi_vae_decode_to_field(&g_gauntlet_vae, g_gauntlet_density, W, H);

        // Render VAE Wilderness Background (Modulated by density cache) in AB4H pixels format
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                int idx = y * W + x;
                float density = g_gauntlet_density[idx];
                if (density < 8.0f) {
                    float intensity = 1.0f - (density / 8.0f);
                    if (intensity < 0.0f) intensity = 0.0f;
                    // Moss forest greens & deep wild shadows
                    float r = 0.05f + intensity * 0.10f;
                    float g = 0.15f + intensity * 0.25f;
                    float b = 0.08f + intensity * 0.12f;
                    pixels[idx] = make_ab4h_pixel(r, g, b, 1.0f);
                } else {
                    pixels[idx] = bg_cyber_black;
                }
            }
        }

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
                
                // Align Y-axis first to prevent zig-zagging and shoot from a distance
                if (labs(dy) >= 20) {
                    if (dy > 0) moveDir = 2; // Down
                    else moveDir = 1;        // Up
                } else {
                    if (dx > 0) moveDir = 4; // Right
                    else moveDir = 3;        // Left
                }
                
                // Shoot only if aligned with target (within collision tolerance of 20 pixels) and facing it
                bool aligned_x = (labs(dx) < 20);
                bool aligned_y = (labs(dy) < 20);
                bool facing_target = false;
                if (aligned_x) {
                    if (dy > 0 && moveDir == 2) facing_target = true;
                    if (dy < 0 && moveDir == 1) facing_target = true;
                }
                if (aligned_y) {
                    if (dx > 0 && moveDir == 4) facing_target = true;
                    if (dx < 0 && moveDir == 3) facing_target = true;
                }
                if (facing_target) {
                    fire = (g_frame_counter % 6 == 0); // Limit rate of fire
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
        float proj_x = (float)thunk_peek(55070) * scale_x;
        float proj_y = (float)thunk_peek(55071) * scale_y;
        uint64_t proj_active = thunk_peek(55074);

        // Update projectile trail history for maximum visibility
        #define PROJ_TRAIL_MAX 10
        static float trail_x[PROJ_TRAIL_MAX] = {0};
        static float trail_y[PROJ_TRAIL_MAX] = {0};
        static bool trail_active[PROJ_TRAIL_MAX] = {false};

        if (proj_active && proj_x > 0.0f && proj_y > 0.0f) {
            for (int i = PROJ_TRAIL_MAX - 1; i > 0; i--) {
                trail_x[i] = trail_x[i - 1];
                trail_y[i] = trail_y[i - 1];
                trail_active[i] = trail_active[i - 1];
            }
            trail_x[0] = proj_x;
            trail_y[0] = proj_y;
            trail_active[0] = true;
        } else {
            for (int i = PROJ_TRAIL_MAX - 1; i > 0; i--) {
                trail_x[i] = trail_x[i - 1];
                trail_y[i] = trail_y[i - 1];
                trail_active[i] = trail_active[i - 1];
            }
            trail_active[0] = false;
        }

        uint64_t health = thunk_peek(55053);
        uint64_t score = thunk_peek(55032);
        uint64_t keys = thunk_peek(55054);
        (void)keys;

        // Draw Spawner (pulsing brick generator)
        if (sx > 0.0f && sy > 0.0f) {
            float spawner_pulse = 2.4f + 0.2f * sinf(g_frame_counter * 0.1f);
            draw_sprite_16x16(pixels, W, H, sx, sy, sprite_spawner, spawner_pulse * scale_x * 2.5f, spawner_pulse * scale_y * 2.5f);
            draw_radial_glow(pixels, W, H, sx, sy, 35.0f, make_ab4h_pixel(1.0f, 0.1f, 0.0f, 0.3f));
        }

        // Draw Ghost as 3D Crow Puppet
        if (gx > 0.0f && gy > 0.0f) {
            draw_3d_stuffed_animal((uint32_t*)pixels, W, H, (int)gx, (int)gy, 32, "crow", g_frame_counter);
        }

        // Draw Genealogy Lineage Line (William C. Brauch Family Tree connection)
        if (gx > 0.0f && gy > 0.0f && sx > 0.0f && sy > 0.0f) {
            for (float t = 0.0f; t <= 1.0f; t += 0.06f) {
                float lx = sx + (gx - sx) * t;
                float ly = sy + (gy - sy) * t;
                draw_rect_ab4h(pixels, W, H, (int)lx, (int)ly, 3, 3, make_ab4h_pixel(1.2f, 0.9f, 0.0f, 0.5f));
            }
        }

        // Draw Projectile Trail
        for (int i = 0; i < PROJ_TRAIL_MAX; i++) {
            if (trail_active[i] && trail_x[i] > 0.0f && trail_y[i] > 0.0f) {
                float ratio = (float)i / (float)PROJ_TRAIL_MAX;
                float alpha = 1.0f - ratio;
                float radius = 18.0f * (1.0f - ratio);
                if (radius < 4.0f) radius = 4.0f;
                draw_radial_glow(pixels, W, H, trail_x[i], trail_y[i], radius, make_ab4h_pixel(1.5f, 1.2f, 0.0f, alpha * 0.4f));
                draw_rect_ab4h(pixels, W, H, (int)trail_x[i] - 3, (int)trail_y[i] - 3, 6, 6, make_ab4h_pixel(1.5f, 1.2f, 0.0f, alpha));
            }
        }

        // Draw Projectile (Atari PMG Missile 0)
        if (proj_active && proj_x > 0.0f && proj_y > 0.0f) {
            draw_rect_ab4h(pixels, W, H, (int)proj_x - 5, (int)proj_y - 5, 10, 10, make_ab4h_pixel(2.0f, 1.8f, 0.2f, 1.0f));
            draw_radial_glow(pixels, W, H, proj_x, proj_y, 25.0f, make_ab4h_pixel(2.0f, 1.5f, 0.0f, 0.6f));
            static int last_active_frame = -1;
            if (g_frame_counter != last_active_frame) {
                printf("[CLIENT] Render Projectile (Atari PMG Missile 0) at x=%.1f, y=%.1f\n", proj_x, proj_y);
                last_active_frame = g_frame_counter;
            }
        }

        // Draw Player as 3D Teddy Bear Puppet
        if (px > 0.0f && py > 0.0f) {
            draw_3d_stuffed_animal((uint32_t*)pixels, W, H, (int)px, (int)py, 36, "teddy", g_frame_counter);
            draw_radial_glow(pixels, W, H, px, py, 22.0f, make_ab4h_pixel(0.0f, 0.8f, 1.2f, 0.3f));
        }

        // Format and render HUD details
        static int victory_exit_timer = -1;
        uint64_t generation = thunk_peek(55062);
        if (gx <= 0.0f && sx <= 0.0f) {
            sprintf(status_message, "^B^U[VICTORY]^C ^IAll threats neutralized! Alchemical gold harvested! SCORE: %lu^C", score);
            if (victory_exit_timer == -1) {
                printf("[VICTORY] AI pilot successfully destroyed all threats and won the game! Final Score: %lu\n", score);
                victory_exit_timer = 300; // Wait 300 frames (5s) to show screen before exiting
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
                .frame_counter = g_frame_counter
            };
            tsfi_font_ai_bind_evolve(ai_fs, deepseek_evolve_impl, &dec);
            float current_intensity = 0.5f + 0.5f * sinf(g_frame_counter * 0.08f);
            tsfi_font_ai_invoke_evolve(ai_fs, current_intensity);
        }

        draw_frame(s);
        g_frame_counter++;
        tsfi_raw_usleep(16000); // Checked timer speed limit ~60fps
    }

    printf("[GAUNTLET] Tearing down resources...\n");
    _exit(0);
}
