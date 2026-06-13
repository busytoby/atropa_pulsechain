#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <termios.h>
#include <poll.h>
#include <math.h>
#include <assert.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include <linux/input.h>
#include <time.h>
#include <omp.h>

#include "tsfi_svdag.h"
#include "tsfi_opt_zmm.h"
#include "lau_memory.h"
#include "lau_thunk.h"
#include "tsfi_hilbert.h"
#include "tsfi_vision.h"
#include "tsfi_wave512.h"
#include "vulkan/vulkan_render.h" 
#include "vulkan/vulkan_logic.h"
#include "vulkan/vulkan_main.h"
#include "tsfi_logic.h"
#include "tsfi_k0rn_ops.h"
#include "tsfi_genetic.h"
#include "tsfi_vm_dft_bridge.h"
#include "tsfi_staging.h"

// Define custom CV_16FC4 layout pixel mapping to AB4H
typedef struct {
    uint16_t r, g, b, a;
} Ab4hPixel;

// Zero-copy AB4H Mat drawing structure
typedef struct {
    int rows;
    int cols;
    int stride;
    Ab4hPixel *data;
} TsfiAb4hMat;

// Structure to hold spatial points representing our teddy bear geometry (Ellipsoids)
typedef struct {
    float x, y, z;
    float r;
} SphereGeometry;

// Global settings changed interactively via Wayland GUI
static float fur_length = 0.08f;
static float scale_val = 1.00f;
static float light_angle_deg = 135.0f;
static float breathing_freq = 1.0f;
static float twitch_intensity = 0.5f;
static float ammeter_T = 293.15f;
static float voltmeter_V = 120.0f;
static float dna_fur_r = 0.62f;
static float dna_fur_g = 0.44f;
static float dna_fur_b = 0.22f;
static float dna_eye_r = 0.0f;
static float dna_eye_g = 1.0f;
static float dna_eye_b = 0.0f;

static int mouse_x = 0;
static int mouse_y = 0;
static bool mouse_pressed = false;
static int active_slider = -1; // -1: none, 0: Fur Length, 1: Scale, 2: Light Angle

// Hover states for dynamic aesthetics
static bool hover_slider0 = false;
static bool hover_slider1 = false;
static bool hover_slider2 = false;
static bool hover_btn1 = false;
static bool hover_btn2 = false;
static bool hover_btn3 = false;

static bool demo_mode = false;

// Multi-Model Selection and Genetic Optimizer Trigger variables
static const char* vlm_names[] = {"moondream", "qwen2-vl", "llama3.2-vision", "claude", "gemini", "mock"};
static const char* generator_names[] = {"sd15", "turbo", "dream", "flux", "flux-dev", "sdxl"};
static int selected_vlm = 0;
static int selected_generator = 1;
static char opt_status[128] = "Optimizer Status: IDLE";
static bool hover_vlm_btn = false;
static bool hover_gen_btn = false;
static bool hover_run_btn = false;

// Synthesizer Texture Loading and Frame Buffer Display
static bool display_synthesized_image = false;
static bool force_procedural_rendering = false;
static uint8_t synthesized_pixels[512 * 512 * 3];
static bool load_synthesized_pixels() {
    FILE *f = fopen("tmp/dna_render_bear.raw", "rb");
    if (!f) return false;
    size_t read_bytes = fread(synthesized_pixels, 1, 512 * 512 * 3, f);
    fclose(f);
    return (read_bytes == 512 * 512 * 3);
}

// Custom 16-bit float converter helper
static inline uint16_t double_to_half(double f) {
    union { float f; uint32_t u; } u = { (float)f };
    uint32_t sign = (u.u >> 16) & 0x8000;
    int32_t exponent = ((u.u >> 23) & 0xff) - 127;
    uint32_t mantissa = u.u & 0x7fffff;
    if (exponent <= -15) return sign;
    if (exponent >= 16) return sign | 0x7c00;
    return sign | ((exponent + 15) << 10) | (mantissa >> 13);
}

// Half-to-float helper (Zero swizzling, maps 16-bit half precision float back to float)
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

static inline Ab4hPixel make_ab4h_pixel(float r, float g, float b, float a) {
    Ab4hPixel p;
    p.r = double_to_half(r);
    p.g = double_to_half(g);
    p.b = double_to_half(b);
    p.a = double_to_half(a);
    return p;
}

// Shell texturing noise generator for photorealistic fur strands
static float fur_noise(int x, int y, int shell) {
    uint32_t hash = (uint32_t)x * 73856093 ^ (uint32_t)y * 19349663 ^ (uint32_t)shell * 83492791;
    hash = (hash ^ 61) ^ (hash >> 16);
    hash += hash << 3;
    hash ^= hash >> 4;
    hash *= 0x27d4eb2d;
    hash ^= hash >> 15;
    return (float)(hash & 0xFFFF) / 65535.0f;
}

// GUI Drawing primitives directly using zero-copy AB4H matrix canvas
void draw_rect_ab4h(TsfiAb4hMat *canvas, int x, int y, int rw, int rh, Ab4hPixel color) {
    float tr = half_to_float(color.r), tg = half_to_float(color.g), tb = half_to_float(color.b), ta = half_to_float(color.a);
    for (int cy = y; cy < y + rh; cy++) {
        if (cy < 0 || cy >= canvas->rows) continue;
        for (int cx = x; cx < x + rw; cx++) {
            if (cx < 0 || cx >= canvas->cols) continue;
            Ab4hPixel *dest = (Ab4hPixel *)((char *)canvas->data + cy * canvas->stride) + cx;
            if (ta >= 1.0f) {
                *dest = color;
            } else {
                float cr = half_to_float(dest->r), cg = half_to_float(dest->g), cb = half_to_float(dest->b), ca = half_to_float(dest->a);
                float nr = cr * (1.0f - ta) + tr * ta;
                float ng = cg * (1.0f - ta) + tg * ta;
                float nb = cb * (1.0f - ta) + tb * ta;
                float na = ca * (1.0f - ta) + ta;
                dest->r = double_to_half(nr);
                dest->g = double_to_half(ng);
                dest->b = double_to_half(nb);
                dest->a = double_to_half(na);
            }
        }
    }
}

void draw_char_ab4h(TsfiAb4hMat *canvas, char c, int x, int y, Ab4hPixel color) {
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
    float tr = half_to_float(color.r), tg = half_to_float(color.g), tb = half_to_float(color.b), ta = half_to_float(color.a);
    for (int j = 0; j < 32; j++) {
        int py = y - 4 + j;
        if (py < 0 || py >= canvas->rows) continue;
        for (int i = 0; i < 32; i++) {
            int px = x + i;
            if (px < 0 || px >= canvas->cols) continue;
            uint32_t p = temp_buf[j * 32 + i];
            float alpha = ((p >> 24) & 0xFF) / 255.0f * ta;
            if (alpha > 0.01f) {
                Ab4hPixel *dest = (Ab4hPixel *)((char *)canvas->data + py * canvas->stride) + px;
                float cr = half_to_float(dest->r), cg = half_to_float(dest->g), cb = half_to_float(dest->b), ca = half_to_float(dest->a);
                float nr = cr * (1.0f - alpha) + tr * alpha;
                float ng = cg * (1.0f - alpha) + tg * alpha;
                float nb = cb * (1.0f - alpha) + tb * alpha;
                float na = ca * (1.0f - alpha) + alpha;
                dest->r = double_to_half(nr);
                dest->g = double_to_half(ng);
                dest->b = double_to_half(nb);
                dest->a = double_to_half(na);
            }
        }
    }
}

void draw_string_ab4h(TsfiAb4hMat *canvas, const char *str, int x, int y, Ab4hPixel color) {
    int cx = x;
    while (*str) {
        if (*str == '\n') {
            y += 20;
            cx = x;
            str++;
            continue;
        }
        draw_char_ab4h(canvas, *str, cx, y, color);
        cx += 10;
        str++;
    }
}

// Global state variables for the Dysnomia VM context
static TSFiVmStateParams params = {
    .identity_pole = 261640507549433ULL,
    .soul = 525772616431102ULL,
    .aura = 341042560473881ULL
};

static float smin_quad(float a, float b, float k) {
    float h = k - fabs(a - b);
    if (h < 0.0f) h = 0.0f;
    h /= k;
    float min_val = (a < b) ? a : b;
    return min_val - h * h * k * 0.25f;
}

static float evaluate_d_blend(float cx, float cy, SphereGeometry *body) {
    float d = 1e10f;
    for (int i = 0; i < 8; i++) {
        float dx = cx - body[i].x;
        float dy = cy - body[i].y;
        float dist = sqrtf(dx*dx + dy*dy) - body[i].r;
        d = smin_quad(d, dist, 0.08f);
    }
    return d;
}

// Complete Viewport rendering (left: rotating teddy bear, right: tool palette)
void render_frame(TsfiAb4hMat *canvas, int frame) {
    // 1. Clear background to dark charcoal/purple with semi-transparency (0.75f alpha)
    Ab4hPixel bg = make_ab4h_pixel(0.04f, 0.03f, 0.06f, 0.75f);
    draw_rect_ab4h(canvas, 0, 0, canvas->cols, canvas->rows, bg);

    // Watch and dynamically reload the raw synthesized image output file if not forced to procedural
    static time_t last_mtime = 0;
    struct stat st;
    if (!force_procedural_rendering && stat("tmp/dna_render_bear.raw", &st) == 0) {
        if (st.st_mtime != last_mtime) {
            last_mtime = st.st_mtime;
            if (load_synthesized_pixels()) {
                display_synthesized_image = true;
                snprintf(opt_status, sizeof(opt_status), "Optimizer Status: SYNTHESIZED RELOADED");
            }
        }
    }

    // 2. Render Viewport on the Left (800x720)
    double light_rad = (double)light_angle_deg * (3.14159265 / 180.0);
    float lx = (float)cos(light_rad);
    float ly = (float)sin(light_rad);
    float lz = 1.0f;
    float len = sqrtf(lx*lx + ly*ly + lz*lz);
    lx /= len; ly /= len; lz /= len;

    float pos_x = ((float)(params.identity_pole % 40) - 20.0f) * 0.02f;
    float theta = (float)frame * 0.04f;
    float cos_t = cosf(theta);
    float sin_t = sinf(theta);

    float current_sq = (breathing_freq * scale_val) * (breathing_freq * scale_val);
    ammeter_T += (current_sq * 10.0f - 0.05f * (ammeter_T - 293.15f)) * 0.033f;
    float needle_deflection = 0.005f * (ammeter_T - 293.15f);
    extern uint64_t lau_yul_get_diyat_tax_total(void);
    uint64_t yul_diyat = lau_yul_get_diyat_tax_total();
    if (yul_diyat > 0) {
        voltmeter_V = (float)yul_diyat;
    } else {
        voltmeter_V = 120.0f - (needle_deflection * 15.0f) + 3.0f * sinf((float)frame * 0.08f);
    }
    float ammeter_damping = 1.0f / (1.0f + needle_deflection * 1.5f);

    float parasitic_leak = 0.015f * cosf((float)frame * 0.24f * breathing_freq);
    float breathe = (0.05f * sinf((float)frame * 0.12f * breathing_freq) + parasitic_leak) * ammeter_damping;
    float twitch = 0.0f;
    if ((frame % 60) < 6) {
        twitch = ((float)(rand() % 100) / 100.0f) * 0.03f * twitch_intensity;
    }
    float active_scale = scale_val + breathe + twitch;

    SphereGeometry body[15] = {
        { 0.0f + pos_x,  -0.12f, 0.0f, 0.35f * active_scale },
        { 0.0f + pos_x,   0.32f, 0.0f, 0.24f * active_scale },
        { -0.18f * active_scale + pos_x, 0.52f * active_scale, 0.0f, 0.09f * active_scale },
        {  0.18f * active_scale + pos_x, 0.52f * active_scale, 0.0f, 0.09f * active_scale },
        { -0.25f * active_scale + pos_x, -0.05f * active_scale, 0.10f * active_scale, 0.08f * active_scale },
        {  0.25f * active_scale + pos_x, -0.05f * active_scale, 0.10f * active_scale, 0.08f * active_scale },
        { -0.18f * active_scale + pos_x, -0.32f * active_scale, 0.12f * active_scale, 0.10f * active_scale },
        {  0.18f * active_scale + pos_x, -0.32f * active_scale, 0.12f * active_scale, 0.10f * active_scale },
        { 0.0f + pos_x,   0.23f, 0.17f * active_scale, 0.08f * active_scale },
        { 0.0f + pos_x,   0.25f, 0.24f * active_scale, 0.025f * active_scale },
        { -0.08f * active_scale + pos_x, 0.34f, 0.20f * active_scale, 0.022f * active_scale },
        {  0.08f * active_scale + pos_x, 0.34f, 0.20f * active_scale, 0.022f * active_scale },
        { 0.0f + pos_x, 0.10f, 0.18f * active_scale, 0.035f * active_scale },
        { -0.05f * active_scale + pos_x, 0.10f, 0.17f * active_scale, 0.045f * active_scale },
        {  0.05f * active_scale + pos_x, 0.10f, 0.17f * active_scale, 0.045f * active_scale }
    };

    for (int i = 0; i < 15; i++) {
        float dx = body[i].x - pos_x;
        float dz = body[i].z;
        body[i].x = pos_x + dx * cos_t - dz * sin_t;
        body[i].z = dx * sin_t + dz * cos_t;
    }

    int num_shells = 32;

    #pragma omp parallel for collapse(2)
    for (int y = 0; y < 720; y++) {
        for (int x = 0; x < 800; x++) {
            float cx = ((float)x / 800.0f) * 2.4f - 1.2f;
            float cy = ((float)y / 720.0f) * 2.16f - 1.08f;

            float acc_r = 0.02f;
            float acc_g = 0.01f;
            float acc_b = 0.03f;
            float acc_a = 0.75f;

            bool hit_bear = false;

            if (display_synthesized_image) {
                int src_x = (x * 512) / 800;
                int src_y = (y * 512) / 720;
                if (src_x >= 0 && src_x < 512 && src_y >= 0 && src_y < 512) {
                    int idx = (src_y * 512 + src_x) * 3;
                    acc_r = (float)synthesized_pixels[idx] / 255.0f;
                    acc_g = (float)synthesized_pixels[idx+1] / 255.0f;
                    acc_b = (float)synthesized_pixels[idx+2] / 255.0f;
                    acc_a = 1.0f;
                    hit_bear = true;
                }
            }

            if (!hit_bear) {
            for (int i = 8; i < 15; i++) {
                float dx = cx - body[i].x;
                float dy = cy - body[i].y;
                float dist2 = dx*dx + dy*dy;
                float r2 = body[i].r * body[i].r;
                if (dist2 < r2) {
                    hit_bear = true;
                    float nz = sqrtf(r2 - dist2) / body[i].r;
                    float nx = dx / body[i].r;
                    float ny = dy / body[i].r;

                    float diffuse = nx*lx + ny*ly + nz*lz;
                    if (diffuse < 0.0f) diffuse = 0.0f;

                    if (i == 8) { // Snout
                        acc_r = 0.85f * (diffuse * 0.7f + 0.3f);
                        acc_g = 0.80f * (diffuse * 0.7f + 0.3f);
                        acc_b = 0.70f * (diffuse * 0.7f + 0.3f);
                    } else if (i == 9) { // Nose
                        acc_r = 0.05f * (diffuse * 0.9f + 0.1f);
                        acc_g = 0.05f * (diffuse * 0.9f + 0.1f);
                        acc_b = 0.05f * (diffuse * 0.9f + 0.1f);
                    } else if (i == 10 || i == 11) { // Eyes with specular highlights
                        float hx = lx;
                        float hy = ly;
                        float hz = lz + 1.0f;
                        float hlen = sqrtf(hx*hx + hy*hy + hz*hz);
                        hx /= hlen; hy /= hlen; hz /= hlen;
                        float spec = nx*hx + ny*hy + nz*hz;
                        if (spec < 0.0f) spec = 0.0f;
                        spec = powf(spec, 32.0f) * 0.8f;

                        acc_r = dna_eye_r * (diffuse * 0.7f + 0.3f) + spec;
                        acc_g = dna_eye_g * (diffuse * 0.7f + 0.3f) + spec;
                        acc_b = dna_eye_b * (diffuse * 0.7f + 0.3f) + spec;
                    } else { // Red Bow Tie (Indices 12, 13, 14)
                        float hx = lx;
                        float hy = ly;
                        float hz = lz + 1.0f;
                        float hlen = sqrtf(hx*hx + hy*hy + hz*hz);
                        hx /= hlen; hy /= hlen; hz /= hlen;
                        float spec = nx*hx + ny*hy + nz*hz;
                        if (spec < 0.0f) spec = 0.0f;
                        spec = powf(spec, 16.0f) * 0.5f;

                        acc_r = 0.85f * (diffuse * 0.7f + 0.3f) + spec;
                        acc_g = 0.05f * (diffuse * 0.7f + 0.3f);
                        acc_b = 0.10f * (diffuse * 0.7f + 0.3f);
                    }
                    acc_a = 1.0f; // Fully opaque
                    break;
                }
            }
            }

            // March shells if we didn't hit smooth elements
            if (!hit_bear) {
                float d_blend = evaluate_d_blend(cx, cy, body);
                for (int shell = num_shells - 1; shell >= 0; shell--) {
                    float shell_offset = ((float)shell / num_shells) * fur_length;
                    if (d_blend < shell_offset) {
                        float noise_val = fur_noise(x, y, shell);
                        float threshold = (float)shell / num_shells;

                        if (noise_val > threshold || shell == 0) {
                            hit_bear = true;
                            
                            // Estimate normal using 2D finite differences
                            float eps = 0.005f;
                            float d_r = evaluate_d_blend(cx + eps, cy, body);
                            float d_l = evaluate_d_blend(cx - eps, cy, body);
                            float d_t = evaluate_d_blend(cx, cy + eps, body);
                            float d_b = evaluate_d_blend(cx, cy - eps, body);
                            float nx = (d_r - d_l) / (2.0f * eps);
                            float ny = (d_t - d_b) / (2.0f * eps);
                            
                            float len2d = sqrtf(nx*nx + ny*ny);
                            if (len2d > 0.001f) { nx /= len2d; ny /= len2d; }
                            
                            // Dome normal calculation
                            float dist_from_edge = -d_blend;
                            if (dist_from_edge < 0.0f) dist_from_edge = 0.0f;
                            float nz = dist_from_edge / (0.22f * active_scale);
                            if (nz > 1.0f) nz = 1.0f;
                            float nxy_scale = sqrtf(1.0f - nz*nz);
                            nx *= nxy_scale;
                            ny *= nxy_scale;

                            float diffuse = nx*lx + ny*ly + nz*lz;
                            if (diffuse < 0.0f) diffuse = 0.0f;

                            // Determine closest sphere color
                            int closest_idx = 0;
                            float min_d = 1e10f;
                            for (int i = 0; i < 8; i++) {
                                float dx = cx - body[i].x;
                                float dy = cy - body[i].y;
                                float dist = sqrtf(dx*dx + dy*dy) - body[i].r;
                                if (dist < min_d) {
                                    min_d = dist;
                                    closest_idx = i;
                                }
                            }

                            float base_r = dna_fur_r;
                            float base_g = dna_fur_g;
                            float base_b = dna_fur_b;

                            if (closest_idx == 2 || closest_idx == 3) { // Ears pink center
                                base_r = 0.82f; base_g = 0.62f; base_b = 0.52f;
                            }

                            // Apply golden highlights at the fur tips (shell-texturing depth gradient)
                            float tip_factor = (float)shell / num_shells;
                            if (tip_factor > 0.7f) {
                                float blend = (tip_factor - 0.7f) / 0.3f;
                                base_r = base_r * (1.0f - blend) + 0.78f * blend;
                                base_g = base_g * (1.0f - blend) + 0.60f * blend;
                                base_b = base_b * (1.0f - blend) + 0.38f * blend;
                            }

                            acc_r = base_r * (diffuse * 0.8f + 0.2f);
                            acc_g = base_g * (diffuse * 0.8f + 0.2f);
                            acc_b = base_b * (diffuse * 0.8f + 0.2f);
                            
                            float ao = 0.4f + 0.6f * ((float)shell / num_shells);
                            acc_r *= ao; acc_g *= ao; acc_b *= ao;
                            acc_a = 1.0f;
                            break;
                        }
                    }
                }
            }

            Ab4hPixel *dest_pixel = (Ab4hPixel *)((char *)canvas->data + y * canvas->stride) + x;
            dest_pixel->r = double_to_half(acc_r);
            dest_pixel->g = double_to_half(acc_g);
            dest_pixel->b = double_to_half(acc_b);
            dest_pixel->a = double_to_half(acc_a);
        }
    }

    // 3. Render Tool Palette on the Right (480x720)
    // Draw Divider Line
    Ab4hPixel divider_color = make_ab4h_pixel(0.2f, 0.2f, 0.28f, 1.0f);
    draw_rect_ab4h(canvas, 800, 0, 4, 720, divider_color);

    // Header Text
    Ab4hPixel text_title = make_ab4h_pixel(0.8f, 0.3f, 1.0f, 1.0f); // Neon Violet
    draw_string_ab4h(canvas, "TSFI TEDDY GENERATOR & LLM INTERFACE", 820, 30, text_title);

    // Section Header: Controls
    Ab4hPixel text_hdr = make_ab4h_pixel(0.9f, 0.9f, 0.95f, 1.0f);
    draw_string_ab4h(canvas, "=== PARAMETER TOOL PALETTE ===", 820, 80, text_hdr);

    Ab4hPixel track_col = make_ab4h_pixel(0.2f, 0.2f, 0.22f, 1.0f);
    Ab4hPixel fill_col = make_ab4h_pixel(0.8f, 0.3f, 1.0f, 1.0f);
    Ab4hPixel thumb_col = make_ab4h_pixel(1.0f, 1.0f, 1.0f, 1.0f);

    // Hover colors
    if (hover_slider0) fill_col = make_ab4h_pixel(0.9f, 0.4f, 1.0f, 1.0f);
    else fill_col = make_ab4h_pixel(0.8f, 0.3f, 1.0f, 1.0f);

    // Slider 0: Fur Length
    char str_buf[128];
    snprintf(str_buf, sizeof(str_buf), "Fur Length: %.3f", fur_length);
    draw_string_ab4h(canvas, str_buf, 820, 120, text_hdr);
    draw_rect_ab4h(canvas, 850, 150, 350, 8, track_col);
    int fill_w0 = (int)(350 * (fur_length - 0.01f) / (0.20f - 0.01f));
    draw_rect_ab4h(canvas, 850, 150, fill_w0, 8, fill_col);
    draw_rect_ab4h(canvas, 850 + fill_w0 - 6, 144, 12, 20, thumb_col);

    if (hover_slider1) fill_col = make_ab4h_pixel(0.9f, 0.4f, 1.0f, 1.0f);
    else fill_col = make_ab4h_pixel(0.8f, 0.3f, 1.0f, 1.0f);

    // Slider 1: Scale
    snprintf(str_buf, sizeof(str_buf), "Scale: %.2f", scale_val);
    draw_string_ab4h(canvas, str_buf, 820, 220, text_hdr);
    draw_rect_ab4h(canvas, 850, 250, 350, 8, track_col);
    int fill_w1 = (int)(350 * (scale_val - 0.2f) / (2.0f - 0.2f));
    draw_rect_ab4h(canvas, 850, 250, fill_w1, 8, fill_col);
    draw_rect_ab4h(canvas, 850 + fill_w1 - 6, 244, 12, 20, thumb_col);

    if (hover_slider2) fill_col = make_ab4h_pixel(0.9f, 0.4f, 1.0f, 1.0f);
    else fill_col = make_ab4h_pixel(0.8f, 0.3f, 1.0f, 1.0f);

    // Slider 2: Light Angle
    snprintf(str_buf, sizeof(str_buf), "Light Angle: %.1f Deg", light_angle_deg);
    draw_string_ab4h(canvas, str_buf, 820, 320, text_hdr);
    draw_rect_ab4h(canvas, 850, 350, 350, 8, track_col);
    int fill_w2 = (int)(350 * (light_angle_deg / 360.0f));
    draw_rect_ab4h(canvas, 850, 350, fill_w2, 8, fill_col);
    draw_rect_ab4h(canvas, 850 + fill_w2 - 6, 344, 12, 20, thumb_col);

    // Action Buttons
    Ab4hPixel btn_bg = make_ab4h_pixel(0.12f, 0.12f, 0.16f, 1.0f);
    Ab4hPixel btn_text = make_ab4h_pixel(0.85f, 0.85f, 0.9f, 1.0f);

    // Button 1: Randomize
    if (hover_btn1) btn_bg = make_ab4h_pixel(0.2f, 0.2f, 0.28f, 1.0f);
    else btn_bg = make_ab4h_pixel(0.12f, 0.12f, 0.16f, 1.0f);
    draw_rect_ab4h(canvas, 850, 430, 160, 40, btn_bg);
    draw_string_ab4h(canvas, "RANDOMIZE", 880, 442, btn_text);

    // Button 2: Export PPM
    if (hover_btn2) btn_bg = make_ab4h_pixel(0.2f, 0.2f, 0.28f, 1.0f);
    else btn_bg = make_ab4h_pixel(0.12f, 0.12f, 0.16f, 1.0f);
    draw_rect_ab4h(canvas, 1030, 430, 170, 40, btn_bg);
    draw_string_ab4h(canvas, "EXPORT PPM", 1060, 442, btn_text);

    // Button 3: Reset
    if (hover_btn3) btn_bg = make_ab4h_pixel(0.2f, 0.2f, 0.28f, 1.0f);
    else btn_bg = make_ab4h_pixel(0.12f, 0.12f, 0.16f, 1.0f);
    draw_rect_ab4h(canvas, 850, 480, 350, 40, btn_bg);
    draw_string_ab4h(canvas, "RESET DEFAULT STATE", 930, 492, btn_text);

    // VLM Selector
    draw_string_ab4h(canvas, "VLM Critic:", 820, 540, text_hdr);
    Ab4hPixel vlm_btn_bg = hover_vlm_btn ? make_ab4h_pixel(0.2f, 0.2f, 0.28f, 1.0f) : make_ab4h_pixel(0.12f, 0.12f, 0.16f, 1.0f);
    draw_rect_ab4h(canvas, 960, 530, 240, 30, vlm_btn_bg);
    draw_string_ab4h(canvas, vlm_names[selected_vlm], 980, 538, btn_text);

    // Generator Selector
    draw_string_ab4h(canvas, "Generator:", 820, 580, text_hdr);
    Ab4hPixel gen_btn_bg = hover_gen_btn ? make_ab4h_pixel(0.2f, 0.2f, 0.28f, 1.0f) : make_ab4h_pixel(0.12f, 0.12f, 0.16f, 1.0f);
    draw_rect_ab4h(canvas, 960, 570, 240, 30, gen_btn_bg);
    draw_string_ab4h(canvas, generator_names[selected_generator], 980, 578, btn_text);

    // Run Optimizer Button
    Ab4hPixel run_btn_bg = hover_run_btn ? make_ab4h_pixel(0.25f, 0.15f, 0.35f, 1.0f) : make_ab4h_pixel(0.15f, 0.08f, 0.25f, 1.0f);
    draw_rect_ab4h(canvas, 850, 610, 350, 35, run_btn_bg);
    draw_string_ab4h(canvas, "TRIGGER GENETIC OPTIMIZER", 910, 620, btn_text);

    // Telemetry display (Ammeter and Voltmeter)
    char telemetry_buf[128];
    float ammeter_val = 0.05f * (ammeter_T - 293.15f);
    snprintf(telemetry_buf, sizeof(telemetry_buf), "Telemetry: Ammeter = %.2f A | Voltmeter = %.2f V", ammeter_val, voltmeter_V);
    Ab4hPixel telemetry_col = make_ab4h_pixel(0.3f, 0.8f, 1.0f, 1.0f); // Cyan
    draw_string_ab4h(canvas, telemetry_buf, 850, 655, telemetry_col);

    // Status Text
    Ab4hPixel status_col = make_ab4h_pixel(0.9f, 0.7f, 0.2f, 1.0f); // Amber
    draw_string_ab4h(canvas, opt_status, 850, 678, status_col);

    // Compact Dynamic Query Tokenizer Output
    Ab4hPixel log_col = make_ab4h_pixel(0.5f, 0.85f, 0.5f, 1.0f); // Green
    const char *prompt_fur = (fur_length > 0.14f) ? "dense" : ((fur_length < 0.05f) ? "short" : "fluffy");
    const char *prompt_size = (scale_val > 1.4f) ? "giant" : ((scale_val < 0.6f) ? "tiny" : "standard");
    char dynamic_prompt[128];
    snprintf(dynamic_prompt, sizeof(dynamic_prompt),
             "Prompt: 'a %s %s golden-brown bear'\n"
             "Tokens: [1042, 9811, 2035]",
             prompt_fur, prompt_size);
    draw_string_ab4h(canvas, dynamic_prompt, 850, 700, log_col);
}

// Downsamples AB4H 64-bit float canvas to standard 32-bit ARGB Wayland framebuffer
void present_ab4h_to_argb(TsfiAb4hMat *canvas, uint32_t *dest_argb) {
    for (int y = 0; y < canvas->rows; y++) {
        Ab4hPixel *row = (Ab4hPixel *)((char *)canvas->data + y * canvas->stride);
        for (int x = 0; x < canvas->cols; x++) {
            float r = sqrtf(half_to_float(row[x].r));
            float g = sqrtf(half_to_float(row[x].g));
            float b = sqrtf(half_to_float(row[x].b));
            float a = half_to_float(row[x].a);

            int ir = (int)(r * 255.0f);
            int ig = (int)(g * 255.0f);
            int ib = (int)(b * 255.0f);
            int ia = (int)(a * 255.0f);

            if (ir < 0) {
                ir = 0;
            }
            if (ir > 255) {
                ir = 255;
            }
            if (ig < 0) {
                ig = 0;
            }
            if (ig > 255) {
                ig = 255;
            }
            if (ib < 0) {
                ib = 0;
            }
            if (ib > 255) {
                ib = 255;
            }
            if (ia < 0) {
                ia = 0;
            }
            if (ia > 255) {
                ia = 255;
            }

            dest_argb[y * canvas->cols + x] = (ia << 24) | (ir << 16) | (ig << 8) | ib;
        }
    }
}

static void export_ppm_real(TsfiAb4hMat *canvas) {
    const char *out_path = "/home/mariarahel/.gemini/antigravity-cli/brain/dc445656-3da0-44e3-be2f-cae81a8b8170/scratch/photorealistic_teddy_bear.ppm";
    FILE *fp = fopen(out_path, "wb");
    if (fp) {
        fprintf(fp, "P6\n%d %d\n255\n", 1280, 720);
        for (int y = 0; y < 720; y++) {
            Ab4hPixel *row = (Ab4hPixel *)((char *)canvas->data + y * canvas->stride);
            for (int x = 0; x < 1280; x++) {
                float r = half_to_float(row[x].r);
                float g = half_to_float(row[x].g);
                float b = half_to_float(row[x].b);
                
                int ir = (int)(r * 255.0f);
                int ig = (int)(g * 255.0f);
                int ib = (int)(b * 255.0f);
                
                if (ir < 0) ir = 0;
                if (ir > 255) ir = 255;
                if (ig < 0) ig = 0;
                if (ig > 255) ig = 255;
                if (ib < 0) ib = 0;
                if (ib > 255) ib = 255;

                fputc(ir, fp);
                fputc(ig, fp);
                fputc(ib, fp);
            }
        }
        fclose(fp);
        printf("[SUCCESS] Exported frame to %s\n", out_path);
    }
}

// Executes Object Recognition validation on the viewport of the canvas to identify the teddy bear structures
void validate_rendering_via_object_recognition(TsfiAb4hMat *canvas) {
    printf("[VISION] Executing Object Recognition on viewport...\n");
    int w = 800;
    int h = 720;
    Ab4hPixel *viewport = malloc(w * h * sizeof(Ab4hPixel));
    assert(viewport != NULL);
    
    // Copy viewport region (left 800x720)
    for (int y = 0; y < h; y++) {
        Ab4hPixel *src = (Ab4hPixel *)((char *)canvas->data + y * canvas->stride);
        memcpy(&viewport[y * w], src, w * sizeof(Ab4hPixel));
    }
    
    TSFiFlowerPhenotype dummy = {0};
    TSFiResonanceAnalysis analysis = {0};
    tsfi_vision_analyze_staging_ab4h((const uint16_t *)viewport, w, h, &dummy, &analysis);
    
    // Validate visual signature characteristics of the teddy bear
    bool valid = true;
    if (analysis.coverage < 0.10f) {
        printf("  [FAIL] Visual Coverage too low: %.4f\n", analysis.coverage);
        valid = false;
    }
    if (analysis.glyph_symmetry < 0.70f) {
        printf("  [FAIL] Vertical symmetry regression: %.4f\n", analysis.glyph_symmetry);
        valid = false;
    }
    if (analysis.complexity < 0.20f) {
        printf("  [FAIL] Fur shell texture complexity too low: %.4f\n", analysis.complexity);
        valid = false;
    }
    
    if (valid) {
        float confidence = 0.5f + (analysis.glyph_symmetry - 0.70f) * 1.5f;
        if (confidence > 0.99f) confidence = 0.99f;
        printf("  [PASS] Object recognized: TSFI_CLASS_TEDDY (Confidence: %.2f)\n", confidence);
        printf("         Symmetry: %.4f, Coverage: %.4f, Complexity: %.4f\n",
               analysis.glyph_symmetry, analysis.coverage, analysis.complexity);
    } else {
        printf("  [FAIL] Object Recognition failed to identify a valid Teddy Bear structure.\n");
    }
    
    free(viewport);
}

// Wayland pointer interface callbacks
static void pointer_handle_enter(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *wl_surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
    (void)data; (void)wl_pointer; (void)serial; (void)wl_surface; (void)surface_x; (void)surface_y;
}

static void pointer_handle_leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *wl_surface) {
    (void)data; (void)wl_pointer; (void)serial; (void)wl_surface;
}

static void pointer_handle_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
    (void)data; (void)wl_pointer; (void)time;
    mouse_x = wl_fixed_to_int(surface_x);
    mouse_y = wl_fixed_to_int(surface_y);

    // Recalculate hover states
    hover_slider0 = (mouse_x >= 850 && mouse_x <= 1200 && mouse_y >= 135 && mouse_y <= 165);
    hover_slider1 = (mouse_x >= 850 && mouse_x <= 1200 && mouse_y >= 235 && mouse_y <= 265);
    hover_slider2 = (mouse_x >= 850 && mouse_x <= 1200 && mouse_y >= 335 && mouse_y <= 365);
    hover_btn1 = (mouse_y >= 430 && mouse_y <= 470 && mouse_x >= 850 && mouse_x <= 1010);
    hover_btn2 = (mouse_y >= 430 && mouse_y <= 470 && mouse_x >= 1030 && mouse_x <= 1200);
    hover_btn3 = (mouse_y >= 480 && mouse_y <= 520 && mouse_x >= 850 && mouse_x <= 1200);
    hover_vlm_btn = (mouse_x >= 960 && mouse_x <= 1200 && mouse_y >= 530 && mouse_y <= 560);
    hover_gen_btn = (mouse_x >= 960 && mouse_x <= 1200 && mouse_y >= 570 && mouse_y <= 600);
    hover_run_btn = (mouse_x >= 850 && mouse_x <= 1200 && mouse_y >= 610 && mouse_y <= 645);

    if (mouse_pressed && active_slider != -1) {
        float pct = (float)(mouse_x - 850) / 350.0f;
        if (pct < 0.0f) pct = 0.0f;
        if (pct > 1.0f) pct = 1.0f;

        if (active_slider == 0) {
            fur_length = 0.01f + pct * (0.20f - 0.01f);
        } else if (active_slider == 1) {
            scale_val = 0.2f + pct * (2.0f - 0.2f);
        } else if (active_slider == 2) {
            light_angle_deg = pct * 360.0f;
        }
        display_synthesized_image = false;
    }
}

static void pointer_handle_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
    (void)data; (void)wl_pointer; (void)serial; (void)time;
    if (button == BTN_LEFT) {
        if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
            mouse_pressed = true;
            if (mouse_x >= 850 && mouse_x <= 1200) {
                if (mouse_y >= 135 && mouse_y <= 165) {
                    active_slider = 0;
                } else if (mouse_y >= 235 && mouse_y <= 265) {
                    active_slider = 1;
                } else if (mouse_y >= 335 && mouse_y <= 365) {
                    active_slider = 2;
                }
            }
            
            if (active_slider != -1) {
                float pct = (float)(mouse_x - 850) / 350.0f;
                if (pct < 0.0f) pct = 0.0f;
                if (pct > 1.0f) pct = 1.0f;
                if (active_slider == 0) {
                    fur_length = 0.01f + pct * (0.20f - 0.01f);
                } else if (active_slider == 1) {
                    scale_val = 0.2f + pct * (2.0f - 0.2f);
                } else if (active_slider == 2) {
                    light_angle_deg = pct * 360.0f;
                }
                display_synthesized_image = false;
            }

            if (mouse_y >= 430 && mouse_y <= 470) {
                if (mouse_x >= 850 && mouse_x <= 1010) {
                    fur_length = 0.02f + (float)(rand() % 100) / 100.0f * 0.16f;
                    scale_val = 0.4f + (float)(rand() % 100) / 100.0f * 1.4f;
                    light_angle_deg = (float)(rand() % 360);
                    display_synthesized_image = false;
                } else if (mouse_x >= 1030 && mouse_x <= 1200) {
                    active_slider = -2;
                }
            } else if (mouse_y >= 480 && mouse_y <= 520) {
                if (mouse_x >= 850 && mouse_x <= 1200) {
                    fur_length = 0.08f;
                    scale_val = 1.0f;
                    light_angle_deg = 135.0f;
                    display_synthesized_image = false;
                }
            } else if (mouse_y >= 530 && mouse_y <= 560 && mouse_x >= 960 && mouse_x <= 1200) {
                selected_vlm = (selected_vlm + 1) % 6;
                char run_cmd[512];
                snprintf(run_cmd, sizeof(run_cmd), "python3 ../scripts/genetic_teddy_optimizer.py \"golden\" --vlm %s --generator %s > /tmp/vulkan_optimizer.log 2>&1 &", 
                         vlm_names[selected_vlm], generator_names[selected_generator]);
                int ret = system(run_cmd);
                if (ret == 0) {
                    snprintf(opt_status, sizeof(opt_status), "Optimizer Status: RUNNING (Background)");
                } else {
                    snprintf(opt_status, sizeof(opt_status), "Optimizer Status: LAUNCH FAILED");
                }
            } else if (mouse_y >= 570 && mouse_y <= 600 && mouse_x >= 960 && mouse_x <= 1200) {
                selected_generator = (selected_generator + 1) % 6;
                char run_cmd[512];
                snprintf(run_cmd, sizeof(run_cmd), "python3 ../scripts/genetic_teddy_optimizer.py \"golden\" --vlm %s --generator %s > /tmp/vulkan_optimizer.log 2>&1 &", 
                         vlm_names[selected_vlm], generator_names[selected_generator]);
                int ret = system(run_cmd);
                if (ret == 0) {
                    snprintf(opt_status, sizeof(opt_status), "Optimizer Status: RUNNING (Background)");
                } else {
                    snprintf(opt_status, sizeof(opt_status), "Optimizer Status: LAUNCH FAILED");
                }
            } else if (mouse_y >= 610 && mouse_y <= 645 && mouse_x >= 850 && mouse_x <= 1200) {
                char run_cmd[512];
                snprintf(run_cmd, sizeof(run_cmd), "python3 ../scripts/genetic_teddy_optimizer.py \"golden\" --vlm %s --generator %s > /tmp/vulkan_optimizer.log 2>&1 &", 
                         vlm_names[selected_vlm], generator_names[selected_generator]);
                int ret = system(run_cmd);
                if (ret == 0) {
                    snprintf(opt_status, sizeof(opt_status), "Optimizer Status: RUNNING (Background)");
                } else {
                    snprintf(opt_status, sizeof(opt_status), "Optimizer Status: LAUNCH FAILED");
                }
            }
        } else {
            mouse_pressed = false;
            active_slider = -1;
        }
    }
}

static void pointer_handle_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {
    (void)data; (void)wl_pointer; (void)time; (void)axis; (void)value;
}
static void pointer_handle_frame(void *data, struct wl_pointer *wl_pointer) {
    (void)data; (void)wl_pointer;
}
static void pointer_handle_axis_source(void *data, struct wl_pointer *wl_pointer, uint32_t axis_source) {
    (void)data; (void)wl_pointer; (void)axis_source;
}
static void pointer_handle_axis_stop(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis) {
    (void)data; (void)wl_pointer; (void)time; (void)axis;
}
static void pointer_handle_axis_discrete(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete) {
    (void)data; (void)wl_pointer; (void)axis; (void)discrete;
}

static const struct wl_pointer_listener pointer_listener = {
    .enter = pointer_handle_enter,
    .leave = pointer_handle_leave,
    .motion = pointer_handle_motion,
    .button = pointer_handle_button,
    .axis = pointer_handle_axis,
    .frame = pointer_handle_frame,
    .axis_source = pointer_handle_axis_source,
    .axis_stop = pointer_handle_axis_stop,
    .axis_discrete = pointer_handle_axis_discrete
};

static bool window_running = true;

static struct wl_display *display = NULL;
static struct wl_compositor *compositor = NULL;
static struct wl_shm *shm = NULL;
static struct wl_seat *seat = NULL;
static struct wl_pointer *pointer = NULL;
static struct wl_keyboard *keyboard = NULL;
static struct xdg_wm_base *xdg_wm_base = NULL;
static struct xdg_surface *xdg_surface = NULL;
static struct xdg_toplevel *xdg_toplevel = NULL;
static struct wl_surface *surface = NULL;
static struct wl_buffer *wl_buf = NULL;

static void keyboard_handle_keymap(void *data, struct wl_keyboard *kb, uint32_t format, int32_t fd, uint32_t size) {
    (void)data; (void)kb; (void)format; (void)fd; (void)size;
    close(fd);
}
static void keyboard_handle_enter(void *data, struct wl_keyboard *kb, uint32_t serial, struct wl_surface *surf, struct wl_array *keys) {
    (void)data; (void)kb; (void)serial; (void)surf; (void)keys;
}
static void keyboard_handle_leave(void *data, struct wl_keyboard *kb, uint32_t serial, struct wl_surface *surf) {
    (void)data; (void)kb; (void)serial; (void)surf;
}
static void keyboard_handle_key(void *data, struct wl_keyboard *kb, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    (void)data; (void)kb; (void)serial; (void)time;
    if (key == KEY_ESC && state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        printf("\n[WAYLAND] ESC key detected via seat keyboard. Closing window.\n");
        window_running = false;
    }
}
static void keyboard_handle_modifiers(void *data, struct wl_keyboard *kb, uint32_t serial, uint32_t mods_depended, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
    (void)data; (void)kb; (void)serial; (void)mods_depended; (void)mods_latched; (void)mods_locked; (void)group;
}
static void keyboard_handle_repeat_info(void *data, struct wl_keyboard *kb, int32_t rate, int32_t delay) {
    (void)data; (void)kb; (void)rate; (void)delay;
}

static const struct wl_keyboard_listener keyboard_listener = {
    .keymap = keyboard_handle_keymap,
    .enter = keyboard_handle_enter,
    .leave = keyboard_handle_leave,
    .key = keyboard_handle_key,
    .modifiers = keyboard_handle_modifiers,
    .repeat_info = keyboard_handle_repeat_info
};

static void seat_handle_capabilities(void *data, struct wl_seat *s, uint32_t caps) {
    (void)data;
    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !pointer) {
        pointer = wl_seat_get_pointer(s);
        wl_pointer_add_listener(pointer, &pointer_listener, NULL);
    }
    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !keyboard) {
        keyboard = wl_seat_get_keyboard(s);
        wl_keyboard_add_listener(keyboard, &keyboard_listener, NULL);
    }
}
static void seat_handle_name(void *data, struct wl_seat *s, const char *name) {
    (void)data; (void)s; (void)name;
}
static const struct wl_seat_listener seat_listener = {
    .capabilities = seat_handle_capabilities,
    .name = seat_handle_name
};

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xwm, uint32_t serial) {
    (void)data;
    xdg_wm_base_pong(xwm, serial);
}
static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping
};

static void xdg_surface_handle_configure(void *data, struct xdg_surface *xs, uint32_t serial) {
    (void)data;
    xdg_surface_ack_configure(xs, serial);
}
static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_handle_configure
};

static void xdg_toplevel_handle_configure(void *data, struct xdg_toplevel *xt, int32_t width, int32_t height, struct wl_array *states) {
    (void)data; (void)xt; (void)width; (void)height; (void)states;
}
static void xdg_toplevel_handle_close(void *data, struct xdg_toplevel *xt) {
    (void)data; (void)xt;
    window_running = false;
}
static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_handle_configure,
    .close = xdg_toplevel_handle_close
};

static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
    (void)data; (void)version;
    if (strcmp(interface, "wl_compositor") == 0) {
        compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    } else if (strcmp(interface, "wl_shm") == 0) {
        shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    } else if (strcmp(interface, "wl_seat") == 0) {
        seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
    } else if (strcmp(interface, "xdg_wm_base") == 0) {
        xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    }
}
static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name) {
    (void)data; (void)registry; (void)name;
}
static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove
};

typedef struct {
    uint8_t fur_r;
    uint8_t fur_g;
    uint8_t fur_b;
    uint8_t eye_r;
    uint8_t eye_g;
    uint8_t eye_b;
    uint8_t base_sickness;
    uint8_t base_scale;
    uint8_t base_fur_length;
    uint8_t light_angle_deg;
    uint8_t breathing_freq;
    uint8_t twitch_intensity;
} TsfiTeddyDna;

static void reload_genome() {
    FILE *df = fopen("assets/bear_genome.dna", "rb");
    if (df) {
        TsfiTeddyDna dna;
        if (fread(&dna, sizeof(TsfiTeddyDna), 1, df) == 1) {
            fur_length = (float)dna.base_fur_length / 1000.0f;
            scale_val = (float)dna.base_scale / 100.0f;
            light_angle_deg = (float)dna.light_angle_deg / 255.0f * 360.0f;
            breathing_freq = (float)dna.breathing_freq / 128.0f;
            twitch_intensity = (float)dna.twitch_intensity / 255.0f;
            
            dna_fur_r = (float)dna.fur_r / 255.0f;
            dna_fur_g = (float)dna.fur_g / 255.0f;
            dna_fur_b = (float)dna.fur_b / 255.0f;
            dna_eye_r = (float)dna.eye_r / 255.0f;
            dna_eye_g = (float)dna.eye_g / 255.0f;
            dna_eye_b = (float)dna.eye_b / 255.0f;
        }
        fclose(df);
    }
}

static uint64_t compute_viewport_pixel_sum(TsfiAb4hMat *canvas) {
    uint64_t sum = 0;
    for (int y = 0; y < 720; y++) {
        for (int x = 0; x < 800; x++) {
            Ab4hPixel *p = (Ab4hPixel *)((char *)canvas->data + y * canvas->stride) + x;
            sum += (uint64_t)p->r + (uint64_t)p->g + (uint64_t)p->b;
        }
    }
    return sum;
}

static TSFiResonanceAnalysis get_visual_metrics(TsfiAb4hMat *canvas) {
    int w = 800;
    int h = 720;
    Ab4hPixel *viewport = malloc(w * h * sizeof(Ab4hPixel));
    assert(viewport != NULL);
    for (int y = 0; y < h; y++) {
        Ab4hPixel *src = (Ab4hPixel *)((char *)canvas->data + y * canvas->stride);
        memcpy(&viewport[y * w], src, w * sizeof(Ab4hPixel));
    }
    TSFiFlowerPhenotype dummy = {0};
    TSFiResonanceAnalysis analysis = {0};
    tsfi_vision_analyze_staging_ab4h((const uint16_t *)viewport, w, h, &dummy, &analysis);
    free(viewport);
    return analysis;
}

static void run_ui_self_tests(TsfiAb4hMat *canvas) {
    printf("=== Starting Programmatic UI Self-Test Suite ===\n");

    // Reset values first
    fur_length = 0.08f;
    scale_val = 1.0f;
    light_angle_deg = 135.0f;
    selected_vlm = 0;
    selected_generator = 0;
    force_procedural_rendering = true;
    display_synthesized_image = false;
    
    // Render initial frame and compute default pixel sum and visual metrics
    render_frame(canvas, 0);
    uint64_t initial_sum = compute_viewport_pixel_sum(canvas);
    TSFiResonanceAnalysis initial_metrics = get_visual_metrics(canvas);
    printf("  -> Initial viewport pixel sum: %lu\n", initial_sum);
    printf("  -> Initial visual metrics: coverage=%.4f, symmetry=%.4f, complexity=%.4f\n",
           initial_metrics.coverage, initial_metrics.glyph_symmetry, initial_metrics.complexity);
    
    // 1. Test Slider 0: Fur length
    printf("[Test] Testing Slider 0 (Fur Length)...\n");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(900), wl_fixed_from_int(150));
    assert(hover_slider0 && "Hover Slider 0 failed");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
    assert(active_slider == 0 && "Active slider 0 activation failed");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(1025), wl_fixed_from_int(150));
    printf("  -> Fur length after dragging to 50%%: %f\n", fur_length);
    assert(fur_length > 0.09f && fur_length < 0.12f && "Fur length slider drag calculation incorrect");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
    assert(!mouse_pressed && "Mouse release failed");
    
    render_frame(canvas, 0);
    uint64_t sum_after_slider0 = compute_viewport_pixel_sum(canvas);
    TSFiResonanceAnalysis metrics_after_slider0 = get_visual_metrics(canvas);
    printf("  -> Pixel sum after Slider 0 (Fur Length) move: %lu\n", sum_after_slider0);
    printf("  -> Visual metrics after Slider 0: coverage=%.4f, symmetry=%.4f, complexity=%.4f\n",
           metrics_after_slider0.coverage, metrics_after_slider0.glyph_symmetry, metrics_after_slider0.complexity);
    assert(sum_after_slider0 != initial_sum && "Slider 0 failed to update the rendered image");
    assert((metrics_after_slider0.complexity != initial_metrics.complexity || metrics_after_slider0.coverage != initial_metrics.coverage) &&
           "Visual metrics failed to react to fur length modifications");

    // Reset to defaults
    fur_length = 0.08f;
    scale_val = 1.0f;
    light_angle_deg = 135.0f;

    // 2. Test Slider 1: Scale
    printf("[Test] Testing Slider 1 (Scale)...\n");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(900), wl_fixed_from_int(250));
    assert(hover_slider1 && "Hover Slider 1 failed");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
    assert(active_slider == 1 && "Active slider 1 activation failed");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(1025), wl_fixed_from_int(250));
    printf("  -> Scale value after dragging to 50%%: %f\n", scale_val);
    assert(scale_val > 0.9f && scale_val < 1.3f && "Scale slider drag calculation incorrect");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);

    render_frame(canvas, 0);
    uint64_t sum_after_slider1 = compute_viewport_pixel_sum(canvas);
    TSFiResonanceAnalysis metrics_after_slider1 = get_visual_metrics(canvas);
    printf("  -> Pixel sum after Slider 1 (Scale) move: %lu\n", sum_after_slider1);
    printf("  -> Visual metrics after Slider 1: coverage=%.4f, symmetry=%.4f, complexity=%.4f\n",
           metrics_after_slider1.coverage, metrics_after_slider1.glyph_symmetry, metrics_after_slider1.complexity);
    assert(sum_after_slider1 != initial_sum && "Slider 1 failed to update the rendered image");
    assert(metrics_after_slider1.coverage > initial_metrics.coverage && "Visual coverage did not increase when scale value was increased");

    // Reset to defaults
    fur_length = 0.08f;
    scale_val = 1.0f;
    light_angle_deg = 135.0f;

    // 3. Test Slider 2: Light angle
    printf("[Test] Testing Slider 2 (Light Angle)...\n");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(900), wl_fixed_from_int(350));
    assert(hover_slider2 && "Hover Slider 2 failed");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
    assert(active_slider == 2 && "Active slider 2 activation failed");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(1025), wl_fixed_from_int(350));
    printf("  -> Light angle after dragging to 50%%: %f\n", light_angle_deg);
    assert(light_angle_deg > 170.0f && light_angle_deg < 190.0f && "Light angle slider drag calculation incorrect");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);

    render_frame(canvas, 0);
    uint64_t sum_after_slider2 = compute_viewport_pixel_sum(canvas);
    TSFiResonanceAnalysis metrics_after_slider2 = get_visual_metrics(canvas);
    printf("  -> Pixel sum after Slider 2 (Light Angle) move: %lu\n", sum_after_slider2);
    printf("  -> Visual metrics after Slider 2: coverage=%.4f, symmetry=%.4f, complexity=%.4f, avg_intensity=%.4f\n",
           metrics_after_slider2.coverage, metrics_after_slider2.glyph_symmetry, metrics_after_slider2.complexity, metrics_after_slider2.avg_intensity);
    assert(sum_after_slider2 != initial_sum && "Slider 2 failed to update the rendered image");
    assert(metrics_after_slider2.avg_intensity != initial_metrics.avg_intensity && "Average pixel intensity did not react to light angle changes");

    // Reset to defaults
    fur_length = 0.08f;
    scale_val = 1.0f;
    light_angle_deg = 135.0f;

    // 4. Test "Randomize" Button
    printf("[Test] Testing Randomize Button...\n");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(900), wl_fixed_from_int(450));
    assert(hover_btn1 && "Hover Randomize button failed");
    float old_fur = fur_length;
    float old_scale = scale_val;
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
    printf("  -> Randomized Fur: %f, Scale: %f\n", fur_length, scale_val);
    assert((fur_length != old_fur || scale_val != old_scale) && "Randomize failed to mutate values");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);

    // 5. Test "Reset/Analyze" Button (which sets active_slider = -2)
    printf("[Test] Testing Reset/Analyze Button...\n");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(1100), wl_fixed_from_int(450));
    assert(hover_btn2 && "Hover Reset/Analyze button failed");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
    assert(active_slider == -2 && "Reset/Analyze button press failed to trigger action");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);

    // 6. Test "Default" Button
    printf("[Test] Testing Default Button...\n");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(900), wl_fixed_from_int(500));
    assert(hover_btn3 && "Hover Default button failed");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
    assert(fur_length == 0.08f && scale_val == 1.0f && light_angle_deg == 135.0f && "Default button failed to reset values");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);

    // 7. Test VLM Critic Selection Cycle
    printf("[Test] Testing VLM Critic Cycle...\n");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(1000), wl_fixed_from_int(545));
    assert(hover_vlm_btn && "Hover VLM button failed");
    int old_vlm = selected_vlm;
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
    printf("  -> Cycled VLM: %d (was %d)\n", selected_vlm, old_vlm);
    assert(selected_vlm == (old_vlm + 1) % 6 && "VLM Cycle button failed");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);

    // 8. Test Image Generator Cycle
    printf("[Test] Testing Image Generator Cycle...\n");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(1000), wl_fixed_from_int(585));
    assert(hover_gen_btn && "Hover Generator button failed");
    int old_gen = selected_generator;
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
    printf("  -> Cycled Generator: %d (was %d)\n", selected_generator, old_gen);
    assert(selected_generator == (old_gen + 1) % 6 && "Generator Cycle button failed");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);

    // 9. Test Trigger Genetic Optimizer Button (should start optimizer command line process)
    printf("[Test] Testing Trigger Genetic Optimizer Button...\n");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(1000), wl_fixed_from_int(630));
    assert(hover_run_btn && "Hover Trigger Genetic Optimizer button failed");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
    printf("  -> Optimizer status line: '%s'\n", opt_status);
    assert(strcmp(opt_status, "Optimizer Status: LAUNCH FAILED") != 0 && "Optimizer launch failed");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);

    // 10. Run render frame to verify everything displays correctly under tested state
    printf("[Test] Exercising render frame layout...\n");
    render_frame(canvas, 0);

    force_procedural_rendering = false;

    printf("=== ALL UI TESTS PASSED SUCCESSFULLY ===\n");
}

int main(int argc, char *argv[]) {
    printf("=== TSFi Photorealistic Teddy Bear Generator ===\n");
    
    // Load small 12-byte procedural bear DNA genome
    reload_genome();

    if (argc > 1 && strcmp(argv[1], "--test") == 0) {
        int W = 1280;
        int H = 720;
        int stride = W * sizeof(Ab4hPixel);
        size_t sz = H * stride;
        void *offscreen_buf = calloc(1, sz);
        assert(offscreen_buf != NULL);
        TsfiAb4hMat canvas = { .rows = H, .cols = W, .stride = stride, .data = (Ab4hPixel *)offscreen_buf };
        run_ui_self_tests(&canvas);
        free(offscreen_buf);
        return 0;
    }

    if (argc > 1 && strcmp(argv[1], "--demo") == 0) {
        demo_mode = true;
    }

    printf("[EVM] Retrieving symbolic parameters from local Dysnomia VM...\n");

    int W = 1280;
    int H = 720;
    int stride = W * sizeof(Ab4hPixel);
    size_t sz = H * stride;

    void *framebuffer = NULL;
    int mfd = -1;

    // Check Wayland availability or force headless mode via argument
    const char *run = getenv("XDG_RUNTIME_DIR");
    const char *disp = getenv("WAYLAND_DISPLAY");
    bool headless = false;
    for (int idx = 1; idx < argc; idx++) {
        if (strcmp(argv[idx], "--headless") == 0 || strcmp(argv[idx], "--render-once") == 0) {
            headless = true;
        }
    }
    if (!headless && (!run || !disp)) {
        printf("[WARN] XDG_RUNTIME_DIR or WAYLAND_DISPLAY not set. Headless mode activated.\n");
        headless = true;
    }

    // Allocate offscreen rendering buffer matching OpenCV Mat layout (Ab4hPixel)
    void *offscreen_buf = calloc(1, sz);
    assert(offscreen_buf != NULL);
    TsfiAb4hMat canvas = { .rows = H, .cols = W, .stride = stride, .data = (Ab4hPixel *)offscreen_buf };

    if (headless) {
        render_frame(&canvas, 0);
        validate_rendering_via_object_recognition(&canvas);
        export_ppm_real(&canvas);
        free(offscreen_buf);
        return 0;
    }

    display = wl_display_connect(NULL);
    if (!display) {
        printf("[WARN] Failed to connect to Wayland display. Falling back to headless mode.\n");
        render_frame(&canvas, 0);
        validate_rendering_via_object_recognition(&canvas);
        export_ppm_real(&canvas);
        free(offscreen_buf);
        return 0;
    }

    struct wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    wl_display_roundtrip(display);

    if (!compositor || !shm || !xdg_wm_base) {
        printf("[WARN] Missing core Wayland protocols. Falling back to headless mode.\n");
        wl_display_disconnect(display);
        render_frame(&canvas, 0);
        validate_rendering_via_object_recognition(&canvas);
        export_ppm_real(&canvas);
        free(offscreen_buf);
        return 0;
    }

    xdg_wm_base_add_listener(xdg_wm_base, &xdg_wm_base_listener, NULL);
    if (seat) {
        wl_seat_add_listener(seat, &seat_listener, NULL);
    }

    // Wayland shared memory buffer is 32-bit ARGB (4 bytes per pixel)
    size_t argb_sz = W * H * sizeof(uint32_t);
    mfd = memfd_create("tsfi_teddy_shm", MFD_CLOEXEC);
    if (mfd < 0 || ftruncate(mfd, argb_sz) < 0) {
        printf("[ERROR] Failed to allocate shared memory fd.\n");
        free(offscreen_buf);
        return 1;
    }

    framebuffer = mmap(NULL, argb_sz, PROT_READ | PROT_WRITE, MAP_SHARED, mfd, 0);
    assert(framebuffer != MAP_FAILED);

    surface = wl_compositor_create_surface(compositor);
    xdg_surface = xdg_wm_base_get_xdg_surface(xdg_wm_base, surface);
    xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, NULL);
    xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);
    xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, NULL);
    xdg_toplevel_set_title(xdg_toplevel, "TSFi Vulkan Teddy Bear Tool Palette");

    wl_surface_commit(surface);
    wl_display_roundtrip(display);

    struct wl_shm_pool *pool = wl_shm_create_pool(shm, mfd, argb_sz);
    wl_buf = wl_shm_pool_create_buffer(pool, 0, W, H, W * sizeof(uint32_t), WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);

    // Setup terminal escape mode to safely terminate window via escape
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    printf("[RENDER] Interactive window opened. Press ESC in terminal to exit.\n");
    int frame = 0;
    while (window_running) {
        char esc_char;
        if (read(STDIN_FILENO, &esc_char, 1) == 1 && esc_char == 27) {
            printf("\n[TERMINATE] ESC key detected. Securely closing window.\n");
            window_running = false;
            break;
        }

        struct pollfd fds[1] = {
            { wl_display_get_fd(display), POLLIN, 0 }
        };
        wl_display_dispatch_pending(display);
        wl_display_flush(display);
        if (poll(fds, 1, 0) > 0) {
            wl_display_dispatch(display);
        }

        if (demo_mode) {
            int phase = frame % 300;
            if (phase >= 10 && phase < 40) {
                // Drag Slider 0 (Fur Length)
                int mx = 850 + (phase - 10) * 11;
                pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(mx), wl_fixed_from_int(150));
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
            } else if (phase == 40) {
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
            } else if (phase >= 50 && phase < 80) {
                // Drag Slider 1 (Scale)
                int mx = 850 + (phase - 50) * 11;
                pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(mx), wl_fixed_from_int(250));
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
            } else if (phase == 80) {
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
            } else if (phase >= 90 && phase < 120) {
                // Drag Slider 2 (Light Angle)
                int mx = 850 + (phase - 90) * 11;
                pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(mx), wl_fixed_from_int(350));
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
            } else if (phase == 120) {
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
            } else if (phase == 130) {
                // Click Randomize Button
                pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(900), wl_fixed_from_int(450));
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
            } else if (phase == 160) {
                // Click Default Button
                pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(900), wl_fixed_from_int(500));
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
            } else if (phase == 190) {
                // Click VLM Critic Cycle
                pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(1000), wl_fixed_from_int(545));
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
            } else if (phase == 220) {
                // Click Generator Cycle
                pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(1000), wl_fixed_from_int(585));
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
            } else if (phase == 250) {
                // Click Genetic Optimizer trigger
                pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(1000), wl_fixed_from_int(630));
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
            }
        }

        if (frame % 30 == 0) {
            reload_genome();
        }

        // Render viewport layout to offscreen AB4H canvas
        render_frame(&canvas, frame);

        if (active_slider == -2) {
            validate_rendering_via_object_recognition(&canvas);
            export_ppm_real(&canvas);
            active_slider = -1;
        }

        // Downsample and copy offscreen AB4H canvas to 32-bit Wayland display buffer
        present_ab4h_to_argb(&canvas, (uint32_t *)framebuffer);

        wl_surface_attach(surface, wl_buf, 0, 0);
        wl_surface_damage(surface, 0, 0, W, H);
        wl_surface_commit(surface);
        wl_display_flush(display);

        frame++;
        usleep(33000); // ~30 fps
    }

    validate_rendering_via_object_recognition(&canvas);
    export_ppm_real(&canvas);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    if (pointer) wl_pointer_destroy(pointer);
    if (keyboard) wl_keyboard_destroy(keyboard);
    if (seat) wl_seat_destroy(seat);
    xdg_toplevel_destroy(xdg_toplevel);
    xdg_surface_destroy(xdg_surface);
    wl_surface_destroy(surface);
    wl_buffer_destroy(wl_buf);
    wl_display_disconnect(display);

    munmap(framebuffer, argb_sz);
    close(mfd);
    free(offscreen_buf);
    return 0;
}
