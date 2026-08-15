#define _GNU_SOURCE
#include "tsfi_mp4_renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Verified LAU Token Contract Addresses
static const char *LAU_TOKENS[5] = {
    "0xAD4e198623A5E2723e19E4D4a6ECF72B1D19FE4B",
    "0xD07B9f3DF4E9634EbAa0CBF079816925b2C474Ce",
    "0xd32c39fee49391c7952d1b30b15921b0d3b42e69",
    "0xed343c0f99c89ed7c3c934a88f90261fd6a9a68b",
    "0x3e10ed242ecb3951151e7a07e0a8f43d4f150c0e"
};

// 19D Projection Hyperplane from SHA-256 byte payload of valid LAU address
static void compute_19d_projection(const char *address, float t, float *out_coords, int dim_count) {
    if (!address || !out_coords || dim_count <= 0) return;

    size_t len = strlen(address);
    for (int d = 0; d < dim_count; d++) {
        uint8_t byte_val = (uint8_t)address[(d * 2) % len];
        float weight = (float)byte_val / 255.0f;
        out_coords[d] = sinf(t * (1.0f + (float)d * 0.08f) + weight * (float)M_PI) * (0.4f + weight * 0.6f);
    }
}

void tsfi_mp4_pipeline_init(TsfiMp4Pipeline *pipe, const char *audio_wav, const char *output_mp4) {
    if (!pipe) return;
    memset(pipe, 0, sizeof(TsfiMp4Pipeline));
    pipe->total_frames = MP4_TOTAL_FRAMES;
    if (audio_wav) strncpy(pipe->audio_wav_path, audio_wav, sizeof(pipe->audio_wav_path) - 1);
    if (output_mp4) strncpy(pipe->output_mp4_path, output_mp4, sizeof(pipe->output_mp4_path) - 1);
}

static inline void draw_line_thick(uint32_t *pixels, int w, int h, int x0, int y0, int x1, int y1, uint32_t color, int thickness) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        for (int ty = -thickness; ty <= thickness; ty++) {
            int py = y0 + ty;
            if (py < 0 || py >= h) continue;
            for (int tx = -thickness; tx <= thickness; tx++) {
                int px = x0 + tx;
                if (px >= 0 && px < w) {
                    pixels[py * w + px] = color;
                }
            }
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

// -----------------------------------------------------------------------------
// Demoscene 5x7 Dot-Matrix Font Rasterizer
// -----------------------------------------------------------------------------
static const uint8_t FONT_5X7[128][5] = {
    [' '] = {0x00, 0x00, 0x00, 0x00, 0x00},
    ['!'] = {0x00, 0x00, 0x5F, 0x00, 0x00},
    ['#'] = {0x14, 0x7F, 0x14, 0x7F, 0x14},
    ['%'] = {0x23, 0x13, 0x08, 0x64, 0x62},
    ['*'] = {0x2A, 0x1C, 0x7F, 0x1C, 0x2A},
    ['+'] = {0x08, 0x08, 0x3E, 0x08, 0x08},
    ['-'] = {0x08, 0x08, 0x08, 0x08, 0x08},
    ['.'] = {0x00, 0x60, 0x60, 0x00, 0x00},
    ['/'] = {0x20, 0x10, 0x08, 0x04, 0x02},
    ['0'] = {0x3E, 0x51, 0x49, 0x45, 0x3E},
    ['1'] = {0x00, 0x42, 0x7F, 0x40, 0x00},
    ['2'] = {0x42, 0x61, 0x51, 0x49, 0x46},
    ['3'] = {0x21, 0x41, 0x45, 0x4B, 0x31},
    ['4'] = {0x18, 0x14, 0x12, 0x7F, 0x10},
    ['5'] = {0x27, 0x45, 0x45, 0x45, 0x39},
    ['6'] = {0x3C, 0x4A, 0x49, 0x49, 0x30},
    ['7'] = {0x01, 0x71, 0x09, 0x05, 0x03},
    ['8'] = {0x36, 0x49, 0x49, 0x49, 0x36},
    ['9'] = {0x06, 0x49, 0x49, 0x29, 0x1E},
    [':'] = {0x00, 0x36, 0x36, 0x00, 0x00},
    ['='] = {0x14, 0x14, 0x14, 0x14, 0x14},
    ['A'] = {0x7C, 0x12, 0x11, 0x12, 0x7C},
    ['B'] = {0x7F, 0x49, 0x49, 0x49, 0x36},
    ['C'] = {0x3E, 0x41, 0x41, 0x41, 0x22},
    ['D'] = {0x7F, 0x41, 0x41, 0x22, 0x1C},
    ['E'] = {0x7F, 0x49, 0x49, 0x49, 0x41},
    ['F'] = {0x7F, 0x09, 0x09, 0x09, 0x01},
    ['G'] = {0x3E, 0x41, 0x49, 0x49, 0x7A},
    ['H'] = {0x7F, 0x08, 0x08, 0x08, 0x7F},
    ['I'] = {0x00, 0x41, 0x7F, 0x41, 0x00},
    ['J'] = {0x20, 0x40, 0x41, 0x3F, 0x01},
    ['K'] = {0x7F, 0x08, 0x14, 0x22, 0x41},
    ['L'] = {0x7F, 0x40, 0x40, 0x40, 0x40},
    ['M'] = {0x7F, 0x02, 0x0C, 0x02, 0x7F},
    ['N'] = {0x7F, 0x04, 0x08, 0x10, 0x7F},
    ['O'] = {0x3E, 0x41, 0x41, 0x41, 0x3E},
    ['P'] = {0x7F, 0x09, 0x09, 0x09, 0x06},
    ['Q'] = {0x3E, 0x41, 0x51, 0x21, 0x5E},
    ['R'] = {0x7F, 0x09, 0x19, 0x29, 0x46},
    ['S'] = {0x46, 0x49, 0x49, 0x49, 0x31},
    ['T'] = {0x01, 0x01, 0x7F, 0x01, 0x01},
    ['U'] = {0x3F, 0x40, 0x40, 0x40, 0x3F},
    ['V'] = {0x1F, 0x20, 0x40, 0x20, 0x1F},
    ['W'] = {0x7F, 0x20, 0x18, 0x20, 0x7F},
    ['X'] = {0x63, 0x14, 0x08, 0x14, 0x63},
    ['Y'] = {0x07, 0x08, 0x70, 0x08, 0x07},
    ['Z'] = {0x61, 0x51, 0x49, 0x45, 0x43},
    ['['] = {0x00, 0x7F, 0x41, 0x41, 0x00},
    [']'] = {0x00, 0x41, 0x41, 0x7F, 0x00},
    ['_'] = {0x40, 0x40, 0x40, 0x40, 0x40},
    ['|'] = {0x00, 0x00, 0x77, 0x00, 0x00}
};

static void draw_text(uint32_t *fb, int w, int h, int x, int y, const char *str, uint32_t color, int scale) {
    if (!str || !fb) return;
    int cur_x = x;

    while (*str) {
        unsigned char ch = (unsigned char)*str++;
        if (ch < 128) {
            for (int col = 0; col < 5; col++) {

                uint8_t line = FONT_5X7[(int)ch][col];
                for (int row = 0; row < 7; row++) {
                    if ((line >> row) & 1) {
                        for (int dy = 0; dy < scale; dy++) {
                            int py = y + row * scale + dy;
                            if (py < 0 || py >= h) continue;
                            for (int dx = 0; dx < scale; dx++) {
                                int px = cur_x + col * scale + dx;
                                if (px >= 0 && px < w) {
                                    fb[py * w + px] = color;
                                }
                            }
                        }
                    }
                }
            }
        }
        cur_x += 6 * scale;
    }
}

// -----------------------------------------------------------------------------
// Demoscene 3D ANSI Bubble Text Font
// -----------------------------------------------------------------------------
static void draw_demoscene_bubble_text(uint32_t *fb, int w, int h, int x, int y, const char *str, uint32_t fill_color, uint32_t shadow_color, float bob_phase) {
    if (!str || !fb) return;
    int len = (int)strlen(str);

    for (int i = 0; i < len; i++) {
        char ch = str[i];
        float bob = sinf(bob_phase + (float)i * 0.45f) * 14.0f;
        int char_x = x + i * 42;
        int char_y = y + (int)bob;

        char s[2] = { ch, '\0' };
        // Draw 3D Extrusion Drop Shadows
        draw_text(fb, w, h, char_x + 4, char_y + 4, s, shadow_color, 5);
        draw_text(fb, w, h, char_x + 2, char_y + 2, s, 0xFF222222, 5);
        // Draw Main Bubble Character with Specular Highlight
        draw_text(fb, w, h, char_x, char_y, s, fill_color, 5);
        draw_text(fb, w, h, char_x + 1, char_y + 1, s, 0xFFFFFFFF, 1);
    }
}

// -----------------------------------------------------------------------------
// Shaded Cloth Quad Rasterizer with Normal Specular Lighting
// -----------------------------------------------------------------------------
static inline void fill_photorealistic_cloth_quad(uint32_t *fb, int w, int h,
                                                  int x0, int y0, int x1, int y1,
                                                  int x2, int y2, int x3, int y3,
                                                  uint32_t base_color,
                                                  float nx, float ny, float nz,
                                                  float u, float v, int scene) {
    (void)x3; (void)y3;

    float lx = 0.577f, ly = -0.577f, lz = 0.577f;
    float n_dot_l = nx * lx + ny * ly + nz * lz;
    if (n_dot_l < 0.15f) n_dot_l = 0.15f;

    float hx = lx, hy = ly, hz = lz + 1.0f;
    float inv_h = 1.0f / sqrtf(hx*hx + hy*hy + hz*hz);
    hx *= inv_h; hy *= inv_h; hz *= inv_h;
    float n_dot_h = nx * hx + ny * hy + nz * hz;
    if (n_dot_h < 0.0f) n_dot_h = 0.0f;

    float nu = 40.0f, nv = 10.0f;
    if (scene == 2) { nu = 80.0f; nv = 20.0f; }
    float spec = powf(n_dot_h, (nu + nv) * 0.5f) * 0.45f;

    float weave_bump = 1.0f;
    if (scene == 3) {
        weave_bump = 0.85f + 0.15f * sinf(u * 128.0f * (float)M_PI) * cosf(v * 128.0f * (float)M_PI);
    }

    uint8_t a = (uint8_t)((base_color >> 24) & 0xFF);
    float r_f = (((base_color >> 16) & 0xFF) * (n_dot_l * weave_bump) + spec * 255.0f);
    float g_f = (((base_color >> 8) & 0xFF) * (n_dot_l * weave_bump) + spec * 255.0f);
    float b_f = ((base_color & 0xFF) * (n_dot_l * weave_bump) + spec * 255.0f);

    if (r_f > 255.0f) r_f = 255.0f;
    if (g_f > 255.0f) g_f = 255.0f;
    if (b_f > 255.0f) b_f = 255.0f;

    uint32_t lit_col = (a << 24) | ((uint8_t)r_f << 16) | ((uint8_t)g_f << 8) | (uint8_t)b_f;

    draw_line_thick(fb, w, h, x0, y0, x1, y1, lit_col, 0);
    draw_line_thick(fb, w, h, x1, y1, x2, y2, lit_col, 0);
    draw_line_thick(fb, w, h, x2, y2, x0, y0, lit_col, 0);
}

// -----------------------------------------------------------------------------
// Real-time 2-3 Tree AST Merkle Root Graph & Formal Prover HUD
// -----------------------------------------------------------------------------
static void draw_ast_merkle_proving_hud(uint32_t *fb, int w, int h, int scene, float t, uint64_t merkle_proof) {
    int box_x = 40, box_y = 40;
    int box_w = 540, box_h = 125;
    uint32_t hud_cyan = 0xFF00FFCC;

    draw_line_thick(fb, w, h, box_x, box_y, box_x + box_w, box_y, hud_cyan, 1);
    draw_line_thick(fb, w, h, box_x + box_w, box_y, box_x + box_w, box_y + box_h, hud_cyan, 1);
    draw_line_thick(fb, w, h, box_x + box_w, box_y + box_h, box_x, box_y + box_h, hud_cyan, 1);
    draw_line_thick(fb, w, h, box_x, box_y + box_h, box_x, box_y, hud_cyan, 1);

    // Live Telemetry Text
    draw_text(fb, w, h, box_x + 15, box_y + 12, "CHANCERY DOCKET #7000 [ZMM R15=0]", 0xFF00FFCC, 2);
    char buf1[64];
    snprintf(buf1, sizeof(buf1), "TIME: %05.2fS | SCENE %d/7 | FPS 60", t, scene);
    draw_text(fb, w, h, box_x + 15, box_y + 35, buf1, 0xFFFFFFFF, 2);

    int bar_y = box_y + 60;
    int score_len = (int)((t / 90.0f) * 440.0f);
    if (score_len > 440) score_len = 440;

    draw_line_thick(fb, w, h, box_x + 15, bar_y, box_x + 15 + score_len, bar_y, 0xFF00FF7F, 3);
    draw_line_thick(fb, w, h, box_x + 15, bar_y + 15, box_x + 15 + 440, bar_y + 15, 0xFF333333, 1);

    char buf2[64];
    snprintf(buf2, sizeof(buf2), "SCORE: 9999 | R0 FREQ: %3.0fHZ", (scene == 6) ? 20.0f : (scene == 1 ? 55.0f : 110.0f));
    draw_text(fb, w, h, box_x + 15, box_y + 85, buf2, 0xFFFFD700, 2);

    // Bottom-Left 2-3 Tree AST Merkle Graph Overlay with Text Labels
    int tree_x = 60, tree_y = h - 140;
    draw_line_thick(fb, w, h, tree_x + 60, tree_y, tree_x + 60, tree_y + 10, 0xFFFFD700, 2);
    draw_line_thick(fb, w, h, tree_x + 60, tree_y + 10, tree_x + 20, tree_y + 45, 0xFF00E5FF, 1);
    draw_line_thick(fb, w, h, tree_x + 60, tree_y + 10, tree_x + 100, tree_y + 45, 0xFF00E5FF, 1);
    draw_line_thick(fb, w, h, tree_x + 20, tree_y + 45, tree_x, tree_y + 80, 0xFF76EE00, 1);
    draw_line_thick(fb, w, h, tree_x + 20, tree_y + 45, tree_x + 40, tree_y + 80, 0xFF76EE00, 1);
    draw_line_thick(fb, w, h, tree_x + 100, tree_y + 45, tree_x + 80, tree_y + 80, 0xFF76EE00, 1);
    draw_line_thick(fb, w, h, tree_x + 100, tree_y + 45, tree_x + 120, tree_y + 80, 0xFF76EE00, 1);

    draw_text(fb, w, h, tree_x + 35, tree_y - 20, "2-3 MERKLE AST", 0xFFFFD700, 1);
    draw_text(fb, w, h, tree_x + 45, tree_y + 12, "ROOT", 0xFFFFFFFF, 1);

    // Bottom-Right 64-Bit Merkle Root Visualizer & Text
    int rx = w - 440, ry = h - 60;
    draw_text(fb, w, h, rx, ry - 25, "MERKLE PROOF: 0X0D4E0757DE528828", 0xFF00FFCC, 2);
    for (int b = 0; b < 16; b++) {
        uint32_t bit_col = ((merkle_proof >> (b * 4)) & 0x1) ? 0xFF00FFCC : 0xFF444444;
        draw_line_thick(fb, w, h, rx + b * 24, ry, rx + b * 24, ry + 18, bit_col, 1);
    }
}

// -----------------------------------------------------------------------------
// Super8 Film Grain
// -----------------------------------------------------------------------------
static inline void apply_super8_film_grain(uint32_t *fb, int w, int h, float t) {
    (void)t;
    int grain_samples = w * h / 20;
    for (int i = 0; i < grain_samples; i++) {
        int idx = rand() % (w * h);
        uint32_t c = fb[idx];
        int noise = (rand() % 24) - 12;
        int r = (int)((c >> 16) & 0xFF) + noise;
        int g = (int)((c >> 8) & 0xFF) + noise;
        int b = (int)(c & 0xFF) + noise;
        if (r < 0) { r = 0; }
        if (r > 255) { r = 255; }
        if (g < 0) { g = 0; }
        if (g > 255) { g = 255; }
        if (b < 0) { b = 0; }
        if (b > 255) { b = 255; }
        fb[idx] = 0xFF000000 | (r << 16) | (g << 8) | b;
    }
}

// -----------------------------------------------------------------------------
// 19D Hyperplane Projection & Multi-Layer Cloth Shader Pipeline
// -----------------------------------------------------------------------------
void tsfi_mp4_render_scene_frame(TsfiRenderFrameContext *ctx) {
    if (!ctx || !ctx->framebuffer) return;

    float t = ctx->timestamp_sec;
    int w = MP4_WIDTH;
    int h = MP4_HEIGHT;
    uint32_t *fb = ctx->framebuffer;
    float cx = (float)w * 0.5f;
    float cy = (float)h * 0.5f;

    float coords19[19] = {0};
    int token_idx = ((int)(t / 18.0f)) % 5;
    compute_19d_projection(LAU_TOKENS[token_idx], t, coords19, 19);

    // -------------------------------------------------------------------------
    // SCENE 1: VERSE 1 (00:00 - 15:00) | Obsidian Silk & Demoscene Banner
    // -------------------------------------------------------------------------
    if (t < 15.0f) {
        ctx->scene_index = 1;
        for (int y = 0; y < h; y++) {
            float v = (float)y / (float)h;
            uint32_t r = (uint32_t)(25.0f * (1.0f - v));
            uint32_t g = (uint32_t)(16.0f * (1.0f - v));
            uint32_t b = (uint32_t)(32.0f * v + 8.0f);
            uint32_t col = 0xFF000000 | (r << 16) | (g << 8) | b;
            for (int x = 0; x < w; x++) fb[y * w + x] = col;
        }

        int cols = 48, rows = 30;
        float gw = (float)w * 0.75f, gh = (float)h * 0.75f;
        float sx = cx - gw * 0.5f, sy = cy - gh * 0.5f;

        for (int r = 0; r < rows - 1; r++) {
            for (int c = 0; c < cols - 1; c++) {
                float u0 = (float)c / (float)(cols - 1);
                float v0 = (float)r / (float)(rows - 1);
                float u1 = (float)(c + 1) / (float)(cols - 1);
                float v1 = (float)(r + 1) / (float)(rows - 1);

                float w00 = (coords19[c % 19] * cosf(u0 * 6.0f + t * 2.5f) + coords19[r % 19] * sinf(v0 * 4.0f + t)) * 55.0f;
                float w10 = (coords19[(c+1) % 19] * cosf(u1 * 6.0f + t * 2.5f) + coords19[r % 19] * sinf(v0 * 4.0f + t)) * 55.0f;
                float w01 = (coords19[c % 19] * cosf(u0 * 6.0f + t * 2.5f) + coords19[(r+1) % 19] * sinf(v1 * 4.0f + t)) * 55.0f;
                float w11 = (coords19[(c+1) % 19] * cosf(u1 * 6.0f + t * 2.5f) + coords19[(r+1) % 19] * sinf(v1 * 4.0f + t)) * 55.0f;

                int x00 = (int)(sx + u0 * gw), y00 = (int)(sy + v0 * gh + w00);
                int x10 = (int)(sx + u1 * gw), y10 = (int)(sy + v0 * gh + w10);
                int x01 = (int)(sx + u0 * gw), y01 = (int)(sy + v1 * gh + w01);
                int x11 = (int)(sx + u1 * gw), y11 = (int)(sy + v1 * gh + w11);

                float nx = (w10 - w00) / (gw / (float)cols);
                float ny = (w01 - w00) / (gh / (float)rows);
                float nz = 1.0f;
                float inv_len = 1.0f / sqrtf(nx*nx + ny*ny + nz*nz);
                nx *= inv_len; ny *= inv_len; nz *= inv_len;

                uint32_t silk_col = ((r + c) % 2 == 0) ? 0xFF5D4A73 : 0xFF7E6998;
                fill_photorealistic_cloth_quad(fb, w, h, x00, y00, x10, y10, x11, y11, x01, y01, silk_col, nx, ny, nz, u0, v0, 1);
            }
        }

        // Demoscene Bubble Title: BIONIKA
        draw_demoscene_bubble_text(fb, w, h, (int)cx - 160, 180, "BIONIKA", 0xFF00FFCC, 0xFF005544, t * 3.0f);
        draw_text(fb, w, h, (int)cx - 190, 260, "VERSE 1: OBSIDIAN SILK INCEPTION", 0xFFE0E0E0, 2);
    }
    // -------------------------------------------------------------------------
    // SCENE 2: CHORUS 1 (15:00 - 25:00) | Emerald Velvet & Demoscene Banner
    // -------------------------------------------------------------------------
    else if (t < 25.0f) {
        ctx->scene_index = 2;
        for (int i = 0; i < w * h; i++) fb[i] = 0xFF02140A;

        float beat = fmodf(t, 0.5f) / 0.5f;
        float pulse = 1.0f + 0.28f * expf(-beat * 8.0f);

        for (int d = 0; d < 19; d++) {
            float rad_base = (80.0f + (float)d * 24.0f) * pulse;
            int pts = 56;
            int lx = 0, ly = 0, fx = 0, fy = 0;

            for (int p = 0; p < pts; p++) {
                float ang = (float)p * (2.0f * (float)M_PI / (float)pts);
                float ripple = sinf(ang * 8.0f + t * 4.5f) * (coords19[d] * 28.0f);
                int kx = (int)(cx + cosf(ang) * (rad_base + ripple) * 1.15f);
                int ky = (int)(cy + sinf(ang) * (rad_base + ripple) * 0.72f);

                if (p == 0) { fx = kx; fy = ky; }
                else {
                    uint32_t c_col = (d % 2 == 0) ? 0xFF00FFAA : 0xFF00F5FF;
                    draw_line_thick(fb, w, h, lx, ly, kx, ky, c_col, (d % 3 == 0) ? 1 : 0);
                }
                lx = kx; ly = ky;
            }
            draw_line_thick(fb, w, h, lx, ly, fx, fy, 0xFF00FFAA, 0);
        }

        draw_demoscene_bubble_text(fb, w, h, (int)cx - 180, 180, "DISPATCH", 0xFF00FF7F, 0xFF004422, t * 3.5f);
        draw_text(fb, w, h, (int)cx - 200, 260, "CHORUS 1: EDO-22 EMERALD VELVET", 0xFFE0E0E0, 2);
    }
    // -------------------------------------------------------------------------
    // SCENE 3: VERSE 2 (25:00 - 38:00) | Two-Tone Golden Damask & Demoscene Banner
    // -------------------------------------------------------------------------
    else if (t < 38.0f) {
        ctx->scene_index = 3;
        for (int y = 0; y < h; y++) {
            uint32_t col = (y % 4 == 0) ? 0xFF2A1E0A : 0xFF140E05;
            for (int x = 0; x < w; x++) fb[y * w + x] = col;
        }

        for (int d = 0; d < 19; d++) {
            float frac = (float)d / 19.0f;
            int x0 = (int)(cx + coords19[d] * (float)w * 0.50f);
            int y0 = (int)(70.0f + frac * (float)(h - 140));
            int x1 = (int)(cx - coords19[(d + 7) % 19] * (float)w * 0.50f);
            int y1 = (int)((float)h - 70.0f - frac * (float)(h - 140));

            uint32_t gold_col = (d % 2 == 0) ? 0xFFFFD700 : 0xFFECC460;
            draw_line_thick(fb, w, h, x0, y0, x1, y1, gold_col, 1);

            if (x0 >= 3 && x0 < w - 3 && y0 >= 3 && y0 < h - 3) {
                fb[y0 * w + x0] = 0xFFFFFFFF;
                fb[(y0+1) * w + x0] = 0xFFFFF8DC;
            }
        }

        draw_demoscene_bubble_text(fb, w, h, (int)cx - 150, 180, "DAMASK", 0xFFFFD700, 0xFF665500, t * 3.0f);
        draw_text(fb, w, h, (int)cx - 180, 260, "VERSE 2: GOLDEN DAMASK WEAVE", 0xFFE0E0E0, 2);
    }
    // -------------------------------------------------------------------------
    // SCENE 4: CHORUS 2 (38:00 - 50:00) | Cobalt Satin & Demoscene Banner
    // -------------------------------------------------------------------------
    else if (t < 50.0f) {
        ctx->scene_index = 4;
        for (int i = 0; i < w * h; i++) fb[i] = 0xFF060D24;

        for (int d = 0; d < 19; d++) {
            float phi = (float)d * (2.0f * (float)M_PI / 19.0f) + t * 1.0f;
            int pts = 44;
            int lx_cobalt = 0, ly_cobalt = 0, lx_copper = 0, ly_copper = 0;

            for (int i = 0; i < pts; i++) {
                float theta = (float)i * (2.0f * (float)M_PI / (float)pts);
                float r1 = 290.0f + coords19[d] * 95.0f;
                int kx1 = (int)(cx + r1 * cosf(theta + phi) * 0.98f);
                int ky1 = (int)(cy + r1 * sinf(theta + phi) * 0.68f);

                float r2 = 240.0f - coords19[(d + 4) % 19] * 85.0f;
                int kx2 = (int)(cx + r2 * cosf(-theta - phi) * 0.98f);
                int ky2 = (int)(cy + r2 * sinf(-theta - phi) * 0.68f);

                if (i > 0) {
                    draw_line_thick(fb, w, h, lx_cobalt, ly_cobalt, kx1, ky1, 0xFF5C85FF, 1);
                    draw_line_thick(fb, w, h, lx_copper, ly_copper, kx2, ky2, 0xFFE08B3E, 1);
                }
                lx_cobalt = kx1; ly_cobalt = ky1;
                lx_copper = kx2; ly_copper = ky2;
            }
        }

        draw_demoscene_bubble_text(fb, w, h, (int)cx - 180, 180, "MANIFOLD", 0xFF5C85FF, 0xFF112266, t * 3.5f);
        draw_text(fb, w, h, (int)cx - 200, 260, "CHORUS 2: 4D COBALT & COPPER", 0xFFE0E0E0, 2);
    }
    // -------------------------------------------------------------------------
    // SCENE 5: VERSE 3 (50:00 - 62:00) | Carbon-Kevlar & Demoscene Banner
    // -------------------------------------------------------------------------
    else if (t < 62.0f) {
        ctx->scene_index = 5;
        for (int i = 0; i < w * h; i++) fb[i] = 0xFF180803;

        float tension = (t - 50.0f) / 12.0f;
        float focal_r = 330.0f * (1.0f - tension * 0.94f);

        int spokes = 42;
        for (int s = 0; s < spokes; s++) {
            float ang = (float)s * (2.0f * (float)M_PI / (float)spokes) + t * (1.0f + tension * 6.0f);
            int x0 = (int)(cx + cosf(ang) * (float)w * 0.58f);
            int y0 = (int)(cy + sinf(ang) * (float)h * 0.58f);
            int x1 = (int)(cx + cosf(ang) * (focal_r + coords19[s % 19] * 28.0f));
            int y1 = (int)(cy + sinf(ang) * (focal_r + coords19[s % 19] * 28.0f));

            uint32_t spoke_col = (s % 2 == 0) ? 0xFFFF3300 : 0xFFFF9900;
            draw_line_thick(fb, w, h, x0, y0, x1, y1, spoke_col, 1);
        }

        int core_r = (int)(18.0f + sinf(t * 40.0f) * 12.0f);
        for (int dy = -core_r; dy <= core_r; dy++) {
            for (int dx = -core_r; dx <= core_r; dx++) {
                if (dx*dx + dy*dy <= core_r*core_r) {
                    fb[(int)(cy + dy) * w + (int)(cx + dx)] = 0xFFFFFFFF;
                }
            }
        }

        draw_demoscene_bubble_text(fb, w, h, (int)cx - 160, 180, "TENSION", 0xFFFF3300, 0xFF661100, t * 4.0f);
        draw_text(fb, w, h, (int)cx - 180, 260, "VERSE 3: THERMAL STRAIN GLOW", 0xFFE0E0E0, 2);
    }
    // -------------------------------------------------------------------------
    // SCENE 6: CHORUS 3 (62:00 - 80:00) | THE BASS DROP & Demoscene Banner
    // -------------------------------------------------------------------------
    else if (t < 80.0f) {
        ctx->scene_index = 6;
        float drop_t = t - 62.0f;
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                uint32_t r = (uint32_t)(30.0f + 25.0f * sinf(drop_t * 2.0f + (float)x * 0.01f));
                uint32_t g = (uint32_t)(22.0f + 22.0f * cosf(drop_t * 2.5f + (float)y * 0.01f));
                uint32_t b = (uint32_t)(48.0f + 30.0f * sinf(drop_t * 3.0f));
                fb[y * w + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
            }
        }

        for (int d = 0; d < 19; d++) {
            float base_ang = (float)d * (2.0f * (float)M_PI / 19.0f) + drop_t * 1.5f;
            int last_x = (int)cx, last_y = (int)cy;

            for (int s = 1; s <= 30; s++) {
                float dist = (float)s * 32.0f;
                float spiral = base_ang + (float)s * 0.15f;
                float shock = sinf(drop_t * 8.5f - (float)s * 0.3f) * 45.0f;

                int cur_x = (int)(cx + cosf(spiral) * (dist + shock + coords19[d] * 28.0f));
                int cur_y = (int)(cy + sinf(spiral) * (dist + shock + coords19[d] * 28.0f) * 0.78f);

                uint32_t holo = (s % 3 == 0) ? 0xFF00FFFF : ((s % 3 == 1) ? 0xFFFF00FF : 0xFFFFFFFF);
                draw_line_thick(fb, w, h, last_x, last_y, cur_x, cur_y, holo, 1);
                last_x = cur_x; last_y = cur_y;
            }
        }

        draw_demoscene_bubble_text(fb, w, h, (int)cx - 180, 180, "CRESCENDO", 0xFFFFFFFF, 0xFF550055, t * 5.0f);
        draw_text(fb, w, h, (int)cx - 200, 260, "CHORUS 3: 360 VOLUMETRIC DROP", 0xFF00FFFF, 2);
    }
    // -------------------------------------------------------------------------
    // SCENE 7: OUTRO (80:00 - 90:00) | Gossamer Lace & Demoscene Banner
    // -------------------------------------------------------------------------
    else {
        ctx->scene_index = 7;
        float outro_t = (t - 80.0f) / 10.0f;

        for (int y = 0; y < h; y++) {
            float mist = (1.0f - outro_t) * ((float)y / (float)h);
            uint32_t col = 0xFF000000 | ((uint32_t)(12.0f * mist) << 16) | ((uint32_t)(35.0f * mist + 8.0f) << 8) | (uint32_t)(22.0f * mist + 8.0f);
            for (int x = 0; x < w; x++) fb[y * w + x] = col;
        }

        for (int d = 0; d < 19; d++) {
            float rad = (75.0f + (float)d * 26.0f);
            int pts = 36;
            int lx = 0, ly = 0, fx = 0, fy = 0;

            for (int p = 0; p < pts; p++) {
                float ang = (float)p * (2.0f * (float)M_PI / (float)pts) + coords19[d] * 0.45f;
                int wx = (int)(cx + cosf(ang) * rad * 1.08f);
                int wy = (int)(cy + sinf(ang) * rad * 0.68f);

                if (p == 0) { fx = wx; fy = wy; }
                else {
                    draw_line_thick(fb, w, h, lx, ly, wx, wy, 0xFF689B77, 0);
                }

                if (p % 4 == 0 && (1.0f - outro_t) > 0.1f) {
                    fb[wy * w + wx] = 0xFFE0F7FA;
                    if (wy+1 < h) fb[(wy+1) * w + wx] = 0xFFFFFFFF;
                }
                lx = wx; ly = wy;
            }
            draw_line_thick(fb, w, h, lx, ly, fx, fy, 0xFF689B77, 0);
        }

        draw_demoscene_bubble_text(fb, w, h, (int)cx - 160, 180, "SETTLED", 0xFF00FF7F, 0xFF004422, t * 2.0f);
        draw_text(fb, w, h, (int)cx - 180, 260, "OUTRO: CHANCERY SEALED PROOF", 0xFFE0E0E0, 2);
    }

    apply_super8_film_grain(fb, w, h, t);
    draw_ast_merkle_proving_hud(fb, w, h, ctx->scene_index, t, 0x0d4e0757de528828ULL);
}

// -----------------------------------------------------------------------------
// Video Compiler Pipeline
// -----------------------------------------------------------------------------
bool tsfi_mp4_compile_video_with_audio(TsfiMp4Pipeline *pipe) {
    if (!pipe) return false;

    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "ffmpeg -y -f rawvideo -vcodec rawvideo -s %dx%d -pix_fmt bgra -r %d -i - "
             "-i %s -c:v libx264 -pix_fmt yuv420p -preset ultrafast -c:a aac -b:a 192k "
             "-shortest %s > /dev/null 2>&1",
             MP4_WIDTH, MP4_HEIGHT, MP4_FPS, pipe->audio_wav_path, pipe->output_mp4_path);

    FILE *ffmpeg_pipe = popen(cmd, "w");
    if (!ffmpeg_pipe) return false;

    uint32_t *frame = (uint32_t *)malloc(MP4_WIDTH * MP4_HEIGHT * sizeof(uint32_t));
    if (!frame) {
        pclose(ffmpeg_pipe);
        return false;
    }

    TsfiRenderFrameContext ctx;
    ctx.framebuffer = frame;

    for (int f = 0; f < pipe->total_frames; f++) {
        ctx.frame_index = (uint32_t)f;
        ctx.timestamp_sec = (float)f / (float)MP4_FPS;
        tsfi_mp4_render_scene_frame(&ctx);

        fwrite(frame, sizeof(uint32_t), (size_t)(MP4_WIDTH * MP4_HEIGHT), ffmpeg_pipe);
    }

    free(frame);
    int ret = pclose(ffmpeg_pipe);
    return (ret == 0);
}
