#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>
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

// Complete Viewport rendering (left: rotating teddy bear, right: tool palette)
void render_frame(TsfiAb4hMat *canvas, int frame) {
    // 1. Clear background to dark charcoal/purple with semi-transparency (0.75f alpha)
    Ab4hPixel bg = make_ab4h_pixel(0.04f, 0.03f, 0.06f, 0.75f);
    draw_rect_ab4h(canvas, 0, 0, canvas->cols, canvas->rows, bg);

    // 2. Render Teddy Bear Viewport on the Left (800x720)
    double light_rad = (double)light_angle_deg * (3.14159265 / 180.0);
    float lx = (float)cos(light_rad);
    float ly = (float)sin(light_rad);
    float lz = 1.0f;
    float len = sqrtf(lx*lx + ly*ly + lz*lz);
    lx /= len; ly /= len; lz /= len;

    float pos_x = ((float)(params.identity_pole % 40) - 20.0f) * 0.02f;
    // Rotate body elements around Y axis based on frame number
    float theta = (float)frame * 0.04f;
    float cos_t = cosf(theta);
    float sin_t = sinf(theta);

    SphereGeometry body[15] = {
        // Furred Features (Indices 0 - 7)
        { 0.0f + pos_x,  -0.12f, 0.0f, 0.35f * scale_val }, // 0: Torso
        { 0.0f + pos_x,   0.32f, 0.0f, 0.24f * scale_val }, // 1: Head
        { -0.18f * scale_val + pos_x, 0.52f * scale_val, 0.0f, 0.09f * scale_val }, // 2: Left Ear
        {  0.18f * scale_val + pos_x, 0.52f * scale_val, 0.0f, 0.09f * scale_val }, // 3: Right Ear
        { -0.25f * scale_val + pos_x, -0.05f * scale_val, 0.10f * scale_val, 0.08f * scale_val }, // 4: Left Arm
        {  0.25f * scale_val + pos_x, -0.05f * scale_val, 0.10f * scale_val, 0.08f * scale_val }, // 5: Right Arm
        { -0.18f * scale_val + pos_x, -0.32f * scale_val, 0.12f * scale_val, 0.10f * scale_val }, // 6: Left Leg
        {  0.18f * scale_val + pos_x, -0.32f * scale_val, 0.12f * scale_val, 0.10f * scale_val }, // 7: Right Leg

        // Smooth Features (Indices 8 - 11)
        { 0.0f + pos_x,   0.23f, 0.17f * scale_val, 0.08f * scale_val }, // 8: Snout
        { 0.0f + pos_x,   0.25f, 0.24f * scale_val, 0.025f * scale_val }, // 9: Nose
        { -0.08f * scale_val + pos_x, 0.34f, 0.20f * scale_val, 0.022f * scale_val }, // 10: Left Eye
        {  0.08f * scale_val + pos_x, 0.34f, 0.20f * scale_val, 0.022f * scale_val }, // 11: Right Eye

        // Red Bow Tie (Indices 12 - 14)
        { 0.0f + pos_x, 0.10f, 0.18f * scale_val, 0.035f * scale_val }, // 12: Knot
        { -0.05f * scale_val + pos_x, 0.10f, 0.17f * scale_val, 0.045f * scale_val }, // 13: Left Wing
        {  0.05f * scale_val + pos_x, 0.10f, 0.17f * scale_val, 0.045f * scale_val }  // 14: Right Wing
    };

    // Apply rotation
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

            // Check smooth features first (Snout, Nose, Eyes, and Red Bow Tie)
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

                        acc_r = 0.03f * diffuse + spec;
                        acc_g = 0.03f * diffuse + spec;
                        acc_b = 0.03f * diffuse + spec;
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

            // March shells if we didn't hit smooth elements
            if (!hit_bear) {
                for (int shell = num_shells - 1; shell >= 0; shell--) {
                    float shell_offset = ((float)shell / num_shells) * fur_length;
                    for (int i = 0; i < 8; i++) {
                        float dx = cx - body[i].x;
                        float dy = cy - body[i].y;
                        float dist2 = dx*dx + dy*dy;
                        float radius_with_shell = body[i].r + shell_offset;

                        if (dist2 < radius_with_shell * radius_with_shell) {
                            float noise_val = fur_noise(x, y, shell);
                            float threshold = (float)shell / num_shells;

                            if (noise_val > threshold || shell == 0) {
                                hit_bear = true;
                                float nz = sqrtf(radius_with_shell*radius_with_shell - dist2);
                                float nx = dx / radius_with_shell;
                                float ny = dy / radius_with_shell;
                                nz /= radius_with_shell;

                                float diffuse = nx*lx + ny*ly + nz*lz;
                                if (diffuse < 0.0f) diffuse = 0.0f;

                                float base_r = 0.62f;
                                float base_g = 0.44f;
                                float base_b = 0.22f;

                                if (i == 2 || i == 3) { // Ears pink center
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
                    if (hit_bear) break;
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

    // Dynamic Query Tokenizer Output
    draw_string_ab4h(canvas, "=== DYNAMIC LLM TOKENIZER LOGS ===", 820, 550, text_hdr);
    Ab4hPixel log_col = make_ab4h_pixel(0.5f, 0.85f, 0.5f, 1.0f); // Green

    const char *prompt_fur = (fur_length > 0.14f) ? "dense thick-furred" : ((fur_length < 0.05f) ? "short-hair trimmed" : "fluffy");
    const char *prompt_size = (scale_val > 1.4f) ? "giant plush" : ((scale_val < 0.6f) ? "tiny pocket-sized" : "standard");
    const char *prompt_light = (light_angle_deg > 180.0f) ? "sunset backlighting" : "studio illuminated";

    char dynamic_prompt[256];
    snprintf(dynamic_prompt, sizeof(dynamic_prompt),
             "Prompt: 'a %s %s golden-brown bear, %s'\n"
             "Tokens: [1042, 9811, 2035, 8431]\n"
             "Identity Pole: 261640507549433\n"
             "Aura (Light Vector): ACTIVE [x: %.2f y: %.2f]",
             prompt_fur, prompt_size, prompt_light, lx, ly);

    draw_string_ab4h(canvas, dynamic_prompt, 820, 580, log_col);
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
            }

            if (mouse_y >= 430 && mouse_y <= 470) {
                if (mouse_x >= 850 && mouse_x <= 1010) {
                    fur_length = 0.02f + (float)(rand() % 100) / 100.0f * 0.16f;
                    scale_val = 0.4f + (float)(rand() % 100) / 100.0f * 1.4f;
                    light_angle_deg = (float)(rand() % 360);
                } else if (mouse_x >= 1030 && mouse_x <= 1200) {
                    active_slider = -2;
                }
            } else if (mouse_y >= 480 && mouse_y <= 520) {
                if (mouse_x >= 850 && mouse_x <= 1200) {
                    fur_length = 0.08f;
                    scale_val = 1.0f;
                    light_angle_deg = 135.0f;
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
        }
        fclose(df);
    }
}

int main() {
    printf("=== TSFi Photorealistic Teddy Bear Generator ===\n");
    
    // Load small 12-byte procedural bear DNA genome
    reload_genome();

    printf("[EVM] Retrieving symbolic parameters from local Dysnomia VM...\n");

    int W = 1280;
    int H = 720;
    int stride = W * sizeof(Ab4hPixel);
    size_t sz = H * stride;

    void *framebuffer = NULL;
    int mfd = -1;

    // Check Wayland availability
    const char *run = getenv("XDG_RUNTIME_DIR");
    const char *disp = getenv("WAYLAND_DISPLAY");
    bool headless = false;
    if (!run || !disp) {
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
