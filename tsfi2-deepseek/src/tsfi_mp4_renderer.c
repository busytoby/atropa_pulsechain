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
// 3D Perspective Projection Function
// -----------------------------------------------------------------------------
static inline void project_3d_point(float x, float y, float z,
                                    float rot_x, float rot_y, float cam_z,
                                    int w, int h,
                                    int *out_sx, int *out_sy, float *out_depth) {
    float rx1 = x * cosf(rot_y) - z * sinf(rot_y);
    float rz1 = x * sinf(rot_y) + z * cosf(rot_y);

    float ry2 = y * cosf(rot_x) - rz1 * sinf(rot_x);
    float rz2 = y * sinf(rot_x) + rz1 * cosf(rot_x) + cam_z;

    if (rz2 < 10.0f) rz2 = 10.0f;
    *out_depth = rz2;

    float fov = 1100.0f;
    float aspect = (float)w / (float)h;

    *out_sx = (int)((float)w * 0.5f + (rx1 * fov) / (rz2 * aspect));
    *out_sy = (int)((float)h * 0.5f + (ry2 * fov) / rz2);
}

// -----------------------------------------------------------------------------
// 3D Vaesen Character Wireframe Renderer
// -----------------------------------------------------------------------------
static void draw_3d_vaesen_character(uint32_t *fb, int w, int h, int scene, float t, float cam_yaw, float cam_pitch) {
    int sx[16] = {0}, sy[16] = {0};
    float depth[16] = {0};

    if (scene == 1) {
        float b_y = sinf(t * 2.0f) * 15.0f;
        float pts[7][3] = {
            { 0.0f, -120.0f + b_y, 0.0f },
            { 0.0f, -40.0f + b_y, 0.0f },
            { 0.0f, 60.0f + b_y, 0.0f },
            { -50.0f, -30.0f + b_y, 20.0f },
            { 60.0f, -10.0f + b_y, 40.0f },
            { 60.0f, 30.0f + b_y, 40.0f },
            { 0.0f, 150.0f + b_y, 0.0f }
        };

        for (int i = 0; i < 7; i++) {
            project_3d_point(pts[i][0] + 350.0f, pts[i][1], pts[i][2] + 450.0f, cam_pitch, cam_yaw, 800.0f, w, h, &sx[i], &sy[i], &depth[i]);
        }


        uint32_t col = 0xFFC5A059;
        draw_line_thick(fb, w, h, sx[0], sy[0], sx[1], sy[1], col, 2);
        draw_line_thick(fb, w, h, sx[1], sy[1], sx[2], sy[2], col, 2);
        draw_line_thick(fb, w, h, sx[1], sy[1], sx[3], sy[3], col, 1);
        draw_line_thick(fb, w, h, sx[1], sy[1], sx[4], sy[4], col, 1);
        draw_line_thick(fb, w, h, sx[4], sy[4], sx[5], sy[5], 0xFFFFD700, 2);
        draw_line_thick(fb, w, h, sx[2], sy[2], sx[6], sy[6], col, 2);
    } else if (scene == 2) {
        float f_arm = sinf(t * 12.0f) * 25.0f;
        float pts[6][3] = {
            { 0.0f, -100.0f, 0.0f },
            { 0.0f, -30.0f, 0.0f },
            { -45.0f, -20.0f, 30.0f },
            { 40.0f + f_arm, -10.0f, 40.0f },
            { -30.0f, 100.0f, 0.0f },
            { 30.0f, 100.0f, 0.0f }
        };

        for (int i = 0; i < 6; i++) {
            project_3d_point(pts[i][0] - 350.0f, pts[i][1], pts[i][2] + 450.0f, cam_pitch, cam_yaw, 800.0f, w, h, &sx[i], &sy[i], &depth[i]);
        }

        uint32_t col = 0xFF00FF88;
        draw_line_thick(fb, w, h, sx[0], sy[0], sx[1], sy[1], col, 2);
        draw_line_thick(fb, w, h, sx[1], sy[1], sx[2], sy[2], col, 2);
        draw_line_thick(fb, w, h, sx[2], sy[2], sx[3], sy[3], 0xFF00FFFF, 2);
        draw_line_thick(fb, w, h, sx[1], sy[1], sx[4], sy[4], col, 1);
        draw_line_thick(fb, w, h, sx[1], sy[1], sx[5], sy[5], col, 1);
    } else if (scene == 6) {
        float bob = sinf(t * 8.0f) * 20.0f;
        float pts[8][3] = {
            { 0.0f, -80.0f + bob, 0.0f },
            { -35.0f, -120.0f + bob, 10.0f },
            { 35.0f, -120.0f + bob, 10.0f },
            { 0.0f, 0.0f + bob, 0.0f },
            { -60.0f, -20.0f + bob, 30.0f },
            { 60.0f, -20.0f + bob, 30.0f },
            { -30.0f, 80.0f + bob, 10.0f },
            { 30.0f, 80.0f + bob, 10.0f }
        };

        for (int i = 0; i < 8; i++) {
            project_3d_point(pts[i][0], pts[i][1], pts[i][2] + 400.0f, cam_pitch, cam_yaw, 700.0f, w, h, &sx[i], &sy[i], &depth[i]);
        }

        uint32_t bear_col = 0xFFFFD700;
        draw_line_thick(fb, w, h, sx[0], sy[0], sx[1], sy[1], bear_col, 2);
        draw_line_thick(fb, w, h, sx[0], sy[0], sx[2], sy[2], bear_col, 2);
        draw_line_thick(fb, w, h, sx[0], sy[0], sx[3], sy[3], bear_col, 3);
        draw_line_thick(fb, w, h, sx[3], sy[3], sx[4], sy[4], bear_col, 2);
        draw_line_thick(fb, w, h, sx[3], sy[3], sx[5], sy[5], bear_col, 2);
        draw_line_thick(fb, w, h, sx[3], sy[3], sx[6], sy[6], bear_col, 2);
        draw_line_thick(fb, w, h, sx[3], sy[3], sx[7], sy[7], bear_col, 2);
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

static void draw_text_gradient(uint32_t *fb, int w, int h, int x, int y, const char *str, uint32_t col_top, uint32_t col_bot, int scale) {
    if (!str || !fb) return;
    int cur_x = x;

    while (*str) {
        unsigned char ch = (unsigned char)*str++;
        if (ch < 128) {
            for (int col = 0; col < 5; col++) {
                uint8_t line = FONT_5X7[(int)ch][col];
                for (int row = 0; row < 7; row++) {
                    if ((line >> row) & 1) {
                        float v = (float)row / 6.0f;
                        uint8_t r = (uint8_t)(((col_top >> 16) & 0xFF) * (1.0f - v) + ((col_bot >> 16) & 0xFF) * v);
                        uint8_t g = (uint8_t)(((col_top >> 8) & 0xFF) * (1.0f - v) + ((col_bot >> 8) & 0xFF) * v);
                        uint8_t b = (uint8_t)((col_top & 0xFF) * (1.0f - v) + (col_bot & 0xFF) * v);
                        uint32_t pix_col = 0xFF000000 | (r << 16) | (g << 8) | b;

                        for (int dy = 0; dy < scale; dy++) {
                            int py = y + row * scale + dy;
                            if (py < 0 || py >= h) continue;
                            for (int dx = 0; dx < scale; dx++) {
                                int px = cur_x + col * scale + dx;
                                if (px >= 0 && px < w) {
                                    fb[py * w + px] = pix_col;
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

static void draw_text(uint32_t *fb, int w, int h, int x, int y, const char *str, uint32_t color, int scale) {
    draw_text_gradient(fb, w, h, x, y, str, color, color, scale);
}

// -----------------------------------------------------------------------------
// Enhanced Multi-Layer Demoscene 3D ANSI Bubble Text with Rainbow Gradients
// -----------------------------------------------------------------------------
static void draw_demoscene_bubble_text(uint32_t *fb, int w, int h, int x, int y, const char *str,
                                       uint32_t col_top, uint32_t col_bot, uint32_t shadow_col, float bob_phase) {
    if (!str || !fb) return;
    int len = (int)strlen(str);

    for (int i = 0; i < len; i++) {
        char ch = str[i];
        float bob = sinf(bob_phase + (float)i * 0.45f) * 16.0f;
        int char_x = x + i * 44;
        int char_y = y + (int)bob;

        char s[2] = { ch, '\0' };

        // 1. Deep 3D Isometric Extrusion Cast Shadows
        for (int ext = 6; ext >= 2; ext -= 2) {
            draw_text(fb, w, h, char_x + ext, char_y + ext, s, shadow_col, 5);
        }

        // 2. Crisp Black Inking Border
        draw_text(fb, w, h, char_x + 1, char_y + 1, s, 0xFF101010, 5);
        draw_text(fb, w, h, char_x - 1, char_y - 1, s, 0xFF101010, 5);

        // 3. Multi-Stop Vertical Gradient Body (Upper Highlight -> Lower Rich Base)
        draw_text_gradient(fb, w, h, char_x, char_y, s, col_top, col_bot, 5);

        // 4. Photorealistic Top-Left Specular Glint
        draw_text(fb, w, h, char_x + 1, char_y + 1, s, 0xFFFFFFFF, 1);
        draw_text(fb, w, h, char_x + 2, char_y + 2, s, 0xFFFFFFFF, 1);
    }
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
    snprintf(buf2, sizeof(buf2), "VAESEN EMOTION: [VIGILANCE 0.95] | R0: %3.0fHZ", (scene == 6) ? 20.0f : (scene == 1 ? 55.0f : 110.0f));
    draw_text(fb, w, h, box_x + 15, box_y + 85, buf2, 0xFFFFD700, 2);

    // Bottom-Left 2-3 Tree AST Merkle Graph Overlay
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

    // Bottom-Right 64-Bit Merkle Root Visualizer
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
// Full 3D Volumetric Scene Frame Renderer with Enhanced ANSI Bubble Text
// -----------------------------------------------------------------------------
void tsfi_mp4_render_scene_frame(TsfiRenderFrameContext *ctx) {
    if (!ctx || !ctx->framebuffer) return;

    float t = ctx->timestamp_sec;
    int w = MP4_WIDTH;
    int h = MP4_HEIGHT;
    uint32_t *fb = ctx->framebuffer;
    float cx = (float)w * 0.5f;
    float cy = (float)h * 0.5f;
    (void)cy;

    float coords19[19] = {0};
    int token_idx = ((int)(t / 18.0f)) % 5;
    compute_19d_projection(LAU_TOKENS[token_idx], t, coords19, 19);

    float cam_yaw = t * 0.4f;
    float cam_pitch = sinf(t * 0.2f) * 0.15f;

    // -------------------------------------------------------------------------
    // SCENE 1: VERSE 1 (00:00 - 15:00) | 3D Obsidian Silk & Demoscene Banner
    // -------------------------------------------------------------------------
    if (t < 15.0f) {
        ctx->scene_index = 1;
        for (int y = 0; y < h; y++) {
            float v = (float)y / (float)h;
            uint32_t col = 0xFF000000 | ((uint32_t)(25.0f * (1.0f - v)) << 16) | ((uint32_t)(16.0f * (1.0f - v)) << 8) | (uint32_t)(32.0f * v + 8.0f);
            for (int x = 0; x < w; x++) fb[y * w + x] = col;
        }

        int rings = 24, segs = 36;
        for (int ri = 0; ri < rings; ri++) {
            float u = (float)ri / (float)rings;
            float phi = u * 2.0f * (float)M_PI;
            int px0 = 0, py0 = 0, first_x = 0, first_y = 0;

            for (int s = 0; s < segs; s++) {
                float v = (float)s / (float)segs;
                float theta = v * 2.0f * (float)M_PI;

                float r_maj = 280.0f + coords19[ri % 19] * 40.0f;
                float r_min = 90.0f + sinf(t * 3.0f + theta * 3.0f) * 20.0f;

                float x3d = (r_maj + r_min * cosf(theta)) * cosf(phi);
                float y3d = r_min * sinf(theta);
                float z3d = (r_maj + r_min * cosf(theta)) * sinf(phi);

                int sx = 0, sy = 0; float depth = 0;
                project_3d_point(x3d - 150.0f, y3d, z3d, cam_pitch, cam_yaw, 800.0f, w, h, &sx, &sy, &depth);

                if (s == 0) { first_x = sx; first_y = sy; }
                else {
                    uint32_t silk_col = (ri % 2 == 0) ? 0xFF5D4A73 : 0xFF7E6998;
                    draw_line_thick(fb, w, h, px0, py0, sx, sy, silk_col, 0);
                }
                px0 = sx; py0 = sy;
            }
            draw_line_thick(fb, w, h, px0, py0, first_x, first_y, 0xFF5D4A73, 0);
        }

        draw_3d_vaesen_character(fb, w, h, 1, t, cam_yaw, cam_pitch);

        // Enhanced Demoscene Bubble Title: BIONIKA (Cyan Glow -> Deep Emerald)
        draw_demoscene_bubble_text(fb, w, h, (int)cx - 170, 180, "BIONIKA", 0xFF00FFFF, 0xFF00A876, 0xFF003822, t * 3.0f);
        draw_text(fb, w, h, (int)cx - 240, 260, "VERSE 1: 3D OBSIDIAN SILK & AUNCIENT TOMTE", 0xFFE0E0E0, 2);
    }
    // -------------------------------------------------------------------------
    // SCENE 2: CHORUS 1 (15:00 - 25:00) | 3D Gyroscope & Demoscene Banner
    // -------------------------------------------------------------------------
    else if (t < 25.0f) {
        ctx->scene_index = 2;
        for (int i = 0; i < w * h; i++) fb[i] = 0xFF02140A;

        float beat = fmodf(t, 0.5f) / 0.5f;
        float pulse = 1.0f + 0.28f * expf(-beat * 8.0f);

        for (int d = 0; d < 19; d++) {
            float rad = (80.0f + (float)d * 22.0f) * pulse;
            float ring_pitch = (float)d * 0.25f + t * 0.8f;
            float ring_yaw = (float)d * 0.35f + t * 0.6f;
            int pts = 36;
            int px0 = 0, py0 = 0, first_x = 0, first_y = 0;

            for (int p = 0; p < pts; p++) {
                float ang = (float)p * (2.0f * (float)M_PI / (float)pts);
                float x3d = cosf(ang) * rad;
                float y3d = sinf(ang) * rad;
                float z3d = sinf(ang * 4.0f + t * 3.0f) * (coords19[d] * 20.0f);

                int sx = 0, sy = 0; float depth = 0;
                project_3d_point(x3d + 150.0f, y3d, z3d, ring_pitch, ring_yaw, 800.0f, w, h, &sx, &sy, &depth);

                if (p == 0) { first_x = sx; first_y = sy; }
                else {
                    uint32_t c_col = (d % 2 == 0) ? 0xFF00FFAA : 0xFF00F5FF;
                    draw_line_thick(fb, w, h, px0, py0, sx, sy, c_col, 0);
                }
                px0 = sx; py0 = sy;
            }
            draw_line_thick(fb, w, h, px0, py0, first_x, first_y, 0xFF00FFAA, 0);
        }

        draw_3d_vaesen_character(fb, w, h, 2, t, cam_yaw, cam_pitch);

        // Enhanced Demoscene Bubble Title: DISPATCH (Lime -> Dark Forest)
        draw_demoscene_bubble_text(fb, w, h, (int)cx - 190, 180, "DISPATCH", 0xFF76FF03, 0xFF00C853, 0xFF003815, t * 3.5f);
        draw_text(fb, w, h, (int)cx - 240, 260, "CHORUS 1: 3D GYROSCOPE & NACKEN EMERGENCE", 0xFFE0E0E0, 2);
    }
    // -------------------------------------------------------------------------
    // SCENE 3: VERSE 2 (25:00 - 38:00) | 3D DNA Lattice & Demoscene Banner
    // -------------------------------------------------------------------------
    else if (t < 38.0f) {
        ctx->scene_index = 3;
        for (int y = 0; y < h; y++) {
            uint32_t col = (y % 4 == 0) ? 0xFF2A1E0A : 0xFF140E05;
            for (int x = 0; x < w; x++) fb[y * w + x] = col;
        }

        int base_pairs = 44;
        for (int b = 0; b < base_pairs; b++) {
            float frac = (float)b / (float)base_pairs;
            float z3d = -350.0f + frac * 700.0f;
            float ang1 = frac * 4.0f * (float)M_PI + t * 1.5f;
            float ang2 = ang1 + (float)M_PI;

            float rad = 140.0f;
            float x1_3d = cosf(ang1) * rad, y1_3d = sinf(ang1) * rad;
            float x2_3d = cosf(ang2) * rad, y2_3d = sinf(ang2) * rad;

            int sx1 = 0, sy1 = 0, sx2 = 0, sy2 = 0;
            float d1 = 0, d2 = 0;

            project_3d_point(x1_3d, y1_3d, z3d, cam_pitch, cam_yaw, 800.0f, w, h, &sx1, &sy1, &d1);
            project_3d_point(x2_3d, y2_3d, z3d, cam_pitch, cam_yaw, 800.0f, w, h, &sx2, &sy2, &d2);

            uint32_t gold_col = (b % 2 == 0) ? 0xFFFFD700 : 0xFFECC460;
            draw_line_thick(fb, w, h, sx1, sy1, sx2, sy2, gold_col, 1);

            if (sx1 >= 2 && sx1 < w - 2 && sy1 >= 2 && sy1 < h - 2) fb[sy1 * w + sx1] = 0xFFFFFFFF;
            if (sx2 >= 2 && sx2 < w - 2 && sy2 >= 2 && sy2 < h - 2) fb[sy2 * w + sx2] = 0xFFFFFFFF;
        }

        // Enhanced Demoscene Bubble Title: DAMASK (Shimmering Gold -> Bronze)
        draw_demoscene_bubble_text(fb, w, h, (int)cx - 160, 180, "DAMASK", 0xFFFFEA00, 0xFFFFAB00, 0xFF4E342E, t * 3.0f);
        draw_text(fb, w, h, (int)cx - 220, 260, "VERSE 2: 3D DOUBLE-HELIX DNA LATTICE", 0xFFE0E0E0, 2);
    }
    // -------------------------------------------------------------------------
    // SCENE 4: CHORUS 2 (38:00 - 50:00) | 3D Trefoil Manifold & Demoscene Banner
    // -------------------------------------------------------------------------
    else if (t < 50.0f) {
        ctx->scene_index = 4;
        for (int i = 0; i < w * h; i++) fb[i] = 0xFF060D24;

        int pts = 120;
        int px1_prev = 0, py1_prev = 0, px2_prev = 0, py2_prev = 0;
        int first_x1 = 0, first_y1 = 0, first_x2 = 0, first_y2 = 0;

        for (int i = 0; i < pts; i++) {
            float theta = (float)i * (2.0f * (float)M_PI / (float)pts);

            float r = 180.0f + 60.0f * cosf(3.0f * theta);
            float x1_3d = r * cosf(2.0f * theta);
            float y1_3d = r * sinf(2.0f * theta);
            float z1_3d = -80.0f * sinf(3.0f * theta);

            float x2_3d = -x1_3d, y2_3d = -y1_3d, z2_3d = -z1_3d;

            int sx1 = 0, sy1 = 0, sx2 = 0, sy2 = 0;
            float d1 = 0, d2 = 0;

            project_3d_point(x1_3d, y1_3d, z1_3d, cam_pitch + t * 0.5f, cam_yaw + t * 0.8f, 800.0f, w, h, &sx1, &sy1, &d1);
            project_3d_point(x2_3d, y2_3d, z2_3d, cam_pitch + t * 0.5f, cam_yaw + t * 0.8f, 800.0f, w, h, &sx2, &sy2, &d2);

            if (i == 0) {
                first_x1 = sx1; first_y1 = sy1;
                first_x2 = sx2; first_y2 = sy2;
            } else {
                draw_line_thick(fb, w, h, px1_prev, py1_prev, sx1, sy1, 0xFF5C85FF, 1);
                draw_line_thick(fb, w, h, px2_prev, py2_prev, sx2, sy2, 0xFFE08B3E, 1);
            }
            px1_prev = sx1; py1_prev = sy1;
            px2_prev = sx2; py2_prev = sy2;
        }
        draw_line_thick(fb, w, h, px1_prev, py1_prev, first_x1, first_y1, 0xFF5C85FF, 1);
        draw_line_thick(fb, w, h, px2_prev, py2_prev, first_x2, first_y2, 0xFFE08B3E, 1);

        // Enhanced Demoscene Bubble Title: MANIFOLD (Electric Cobalt -> Deep Ultramarine)
        draw_demoscene_bubble_text(fb, w, h, (int)cx - 190, 180, "MANIFOLD", 0xFF448AFF, 0xFF2979FF, 0xFF0D47A1, t * 3.5f);
        draw_text(fb, w, h, (int)cx - 220, 260, "CHORUS 2: 3D DUAL-TREFOIL KNOT MANIFOLD", 0xFFE0E0E0, 2);
    }
    // -------------------------------------------------------------------------
    // SCENE 5: VERSE 3 (50:00 - 62:00) | 3D Singularity Collapse & Demoscene Banner
    // -------------------------------------------------------------------------
    else if (t < 62.0f) {
        ctx->scene_index = 5;
        for (int i = 0; i < w * h; i++) fb[i] = 0xFF180803;

        float tension = (t - 50.0f) / 12.0f;
        float r_scale = 320.0f * (1.0f - tension * 0.88f);

        float phi_gold = (1.0f + sqrtf(5.0f)) * 0.5f;
        float ico_verts[12][3] = {
            { -1.0f,  phi_gold, 0.0f }, {  1.0f,  phi_gold, 0.0f }, { -1.0f, -phi_gold, 0.0f }, {  1.0f, -phi_gold, 0.0f },
            { 0.0f, -1.0f,  phi_gold }, { 0.0f,  1.0f,  phi_gold }, { 0.0f, -1.0f, -phi_gold }, { 0.0f,  1.0f, -phi_gold },
            {  phi_gold, 0.0f, -1.0f }, {  phi_gold, 0.0f,  1.0f }, { -phi_gold, 0.0f, -1.0f }, { -phi_gold, 0.0f,  1.0f }
        };

        int ico_sx[12] = {0}, ico_sy[12] = {0};
        for (int v = 0; v < 12; v++) {
            float x3d = ico_verts[v][0] * (r_scale / phi_gold);
            float y3d = ico_verts[v][1] * (r_scale / phi_gold);
            float z3d = ico_verts[v][2] * (r_scale / phi_gold);
            float depth = 0;
            project_3d_point(x3d, y3d, z3d, t * 1.5f, t * 2.0f, 800.0f, w, h, &ico_sx[v], &ico_sy[v], &depth);
        }

        for (int i = 0; i < 12; i++) {
            for (int j = i + 1; j < 12; j++) {
                float dx = ico_verts[i][0] - ico_verts[j][0];
                float dy = ico_verts[i][1] - ico_verts[j][1];
                float dz = ico_verts[i][2] - ico_verts[j][2];
                if (fabsf(dx*dx + dy*dy + dz*dz - 4.0f) < 0.1f) {
                    uint32_t spoke_col = ((i + j) % 2 == 0) ? 0xFFFF3300 : 0xFFFF9900;
                    draw_line_thick(fb, w, h, ico_sx[i], ico_sy[i], ico_sx[j], ico_sy[j], spoke_col, 1);
                }
            }
        }

        // Enhanced Demoscene Bubble Title: TENSION (Fiery Crimson -> Burnt Amber)
        draw_demoscene_bubble_text(fb, w, h, (int)cx - 170, 180, "TENSION", 0xFFFF5252, 0xFFFF1744, 0xFF880E4F, t * 4.0f);
        draw_text(fb, w, h, (int)cx - 240, 260, "VERSE 3: 3D GEODESIC SINGULARITY COLLAPSE", 0xFFE0E0E0, 2);
    }
    // -------------------------------------------------------------------------
    // SCENE 6: CHORUS 3 (62:00 - 80:00) | 3D VOLUMETRIC BLAST & TEDDY BEAR
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

        int ribbons = 32;
        for (int rb = 0; rb < ribbons; rb++) {
            float ang = (float)rb * (2.0f * (float)M_PI / (float)ribbons);
            int prev_x = 0, prev_y = 0;

            for (int s = 1; s <= 20; s++) {
                float dist = (float)s * 35.0f + sinf(drop_t * 8.0f) * 20.0f;
                float x3d = cosf(ang) * dist;
                float y3d = sinf(ang) * dist * 0.7f;
                float z3d = -200.0f + (float)s * 30.0f;

                int sx = 0, sy = 0; float depth = 0;
                project_3d_point(x3d, y3d, z3d, cam_pitch + drop_t * 0.8f, cam_yaw + drop_t * 1.2f, 750.0f, w, h, &sx, &sy, &depth);

                if (s > 1) {
                    uint32_t holo = (s % 3 == 0) ? 0xFF00FFFF : ((s % 3 == 1) ? 0xFFFF00FF : 0xFFFFFFFF);
                    draw_line_thick(fb, w, h, prev_x, prev_y, sx, sy, holo, 1);
                }
                prev_x = sx; prev_y = sy;
            }
        }

        draw_3d_vaesen_character(fb, w, h, 6, t, cam_yaw, cam_pitch);

        // Enhanced Demoscene Bubble Title: CRESCENDO (Holographic White -> Neon Magenta)
        draw_demoscene_bubble_text(fb, w, h, (int)cx - 200, 180, "CRESCENDO", 0xFFFFFFFF, 0xFFFF00FF, 0xFF4A148C, t * 5.0f);
        draw_text(fb, w, h, (int)cx - 240, 260, "CHORUS 3: 3D VOLUMETRIC BLAST & TEDDY BEAR", 0xFF00FFFF, 2);
    }
    // -------------------------------------------------------------------------
    // SCENE 7: OUTRO (80:00 - 90:00) | 3D Fibonacci Spiral & Demoscene Banner
    // -------------------------------------------------------------------------
    else {
        ctx->scene_index = 7;
        float outro_t = (t - 80.0f) / 10.0f;

        for (int y = 0; y < h; y++) {
            float mist = (1.0f - outro_t) * ((float)y / (float)h);
            uint32_t col = 0xFF000000 | ((uint32_t)(12.0f * mist) << 16) | ((uint32_t)(35.0f * mist + 8.0f) << 8) | (uint32_t)(22.0f * mist + 8.0f);
            for (int x = 0; x < w; x++) fb[y * w + x] = col;
        }

        int spiral_pts = 80;
        int prev_x = 0, prev_y = 0;
        for (int sp = 0; sp < spiral_pts; sp++) {
            float theta = (float)sp * 0.25f + t * 0.5f;
            float r = (float)sp * 4.5f * (1.0f - outro_t * 0.5f);
            float x3d = r * cosf(theta);
            float y3d = r * sinf(theta) * 0.7f;
            float z3d = -150.0f + (float)sp * 5.0f;

            int sx = 0, sy = 0; float depth = 0;
            project_3d_point(x3d, y3d, z3d, cam_pitch, cam_yaw, 800.0f, w, h, &sx, &sy, &depth);

            if (sp > 0) {
                draw_line_thick(fb, w, h, prev_x, prev_y, sx, sy, 0xFF689B77, 0);
            }
            if (sp % 6 == 0 && (1.0f - outro_t) > 0.1f) {
                if (sx >= 0 && sx < w && sy >= 0 && sy < h) fb[sy * w + sx] = 0xFFE0F7FA;
            }
            prev_x = sx; prev_y = sy;
        }

        // Enhanced Demoscene Bubble Title: SETTLED (Emerald Mint -> Sage Green)
        draw_demoscene_bubble_text(fb, w, h, (int)cx - 170, 180, "SETTLED", 0xFF69F0AE, 0xFF00BFA5, 0xFF004D40, t * 2.0f);
        draw_text(fb, w, h, (int)cx - 240, 260, "OUTRO: 3D FIBONACCI SEAL & CHANCERY PROOF", 0xFFE0E0E0, 2);
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
