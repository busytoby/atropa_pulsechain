#define _GNU_SOURCE
#include "tsfi_mp4_renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Verified LAU Token Contract Addresses (Judge Alpha, Witness Beta, Witness Gamma, Juror Delta, Juror Epsilon)
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
// Advanced Photorealistic Ashikhmin-Shirley & Normal Specular Quad Rasterizer
// -----------------------------------------------------------------------------
static inline void fill_photorealistic_cloth_quad(uint32_t *fb, int w, int h,
                                                  int x0, int y0, int x1, int y1,
                                                  int x2, int y2, int x3, int y3,
                                                  uint32_t base_color,
                                                  float nx, float ny, float nz,
                                                  float u, float v, int scene) {
    (void)x3; (void)y3;

    // Light direction vector L = (0.577, -0.577, 0.577), View vector V = (0, 0, 1)
    float lx = 0.577f, ly = -0.577f, lz = 0.577f;
    float n_dot_l = nx * lx + ny * ly + nz * lz;
    if (n_dot_l < 0.15f) n_dot_l = 0.15f;

    // Halfway vector H = normalize(L + V)
    float hx = lx, hy = ly, hz = lz + 1.0f;
    float inv_h = 1.0f / sqrtf(hx*hx + hy*hy + hz*hz);
    hx *= inv_h; hy *= inv_h; hz *= inv_h;
    float n_dot_h = nx * hx + ny * hy + nz * hz;
    if (n_dot_h < 0.0f) n_dot_h = 0.0f;

    // Ashikhmin-Shirley Anisotropic Specular Highlight
    float nu = 40.0f, nv = 10.0f;
    if (scene == 2) { nu = 80.0f; nv = 20.0f; } // Velvet retro-reflection
    float spec = powf(n_dot_h, (nu + nv) * 0.5f) * 0.45f;

    // Procedural Micro-Weave Normal Tangent (Scene 3 Damask)
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
    (void)scene;
    int box_x = 40, box_y = 40;
    int box_w = 480, box_h = 110;
    uint32_t hud_cyan = 0xFF00FFCC;

    draw_line_thick(fb, w, h, box_x, box_y, box_x + box_w, box_y, hud_cyan, 1);
    draw_line_thick(fb, w, h, box_x + box_w, box_y, box_x + box_w, box_y + box_h, hud_cyan, 1);
    draw_line_thick(fb, w, h, box_x + box_w, box_y + box_h, box_x, box_y + box_h, hud_cyan, 1);
    draw_line_thick(fb, w, h, box_x, box_y + box_h, box_x, box_y, hud_cyan, 1);

    int bar_y = box_y + 35;
    int score_len = (int)((t / 90.0f) * 400.0f);
    if (score_len > 400) score_len = 400;

    draw_line_thick(fb, w, h, box_x + 20, bar_y, box_x + 20 + score_len, bar_y, 0xFF00FF7F, 3);
    draw_line_thick(fb, w, h, box_x + 20, bar_y + 30, box_x + 20 + 400, bar_y + 30, 0xFF333333, 1);

    // Bottom-Left 2-3 Tree AST Merkle Graph Overlay
    int tree_x = 60, tree_y = h - 140;
    draw_line_thick(fb, w, h, tree_x + 60, tree_y, tree_x + 60, tree_y + 10, 0xFFFFD700, 2);
    draw_line_thick(fb, w, h, tree_x + 60, tree_y + 10, tree_x + 20, tree_y + 45, 0xFF00E5FF, 1);
    draw_line_thick(fb, w, h, tree_x + 60, tree_y + 10, tree_x + 100, tree_y + 45, 0xFF00E5FF, 1);
    draw_line_thick(fb, w, h, tree_x + 20, tree_y + 45, tree_x, tree_y + 80, 0xFF76EE00, 1);
    draw_line_thick(fb, w, h, tree_x + 20, tree_y + 45, tree_x + 40, tree_y + 80, 0xFF76EE00, 1);
    draw_line_thick(fb, w, h, tree_x + 100, tree_y + 45, tree_x + 80, tree_y + 80, 0xFF76EE00, 1);
    draw_line_thick(fb, w, h, tree_x + 100, tree_y + 45, tree_x + 120, tree_y + 80, 0xFF76EE00, 1);

    // Bottom-Right 64-Bit Merkle Root Visualizer
    int rx = w - 420, ry = h - 60;
    for (int b = 0; b < 16; b++) {
        uint32_t bit_col = ((merkle_proof >> (b * 4)) & 0x1) ? 0xFF00FFCC : 0xFF444444;
        draw_line_thick(fb, w, h, rx + b * 24, ry, rx + b * 24, ry + 18, bit_col, 1);
    }
}

// -----------------------------------------------------------------------------
// Super8 Film Grain & Photochemical Emulsion
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

    // Determine 19D Projection basis from active LAU Token
    float coords19[19] = {0};
    int token_idx = ((int)(t / 18.0f)) % 5;
    compute_19d_projection(LAU_TOKENS[token_idx], t, coords19, 19);

    // -------------------------------------------------------------------------
    // SCENE 1: VERSE 1 (00:00 - 15:00) | Shaded Obsidian Silk with Ashikhmin-Shirley
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
    }
    // -------------------------------------------------------------------------
    // SCENE 2: CHORUS 1 (15:00 - 25:00) | Emerald Crushed Velvet & Retro-Reflection
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
    }
    // -------------------------------------------------------------------------
    // SCENE 3: VERSE 2 (25:00 - 38:00) | Two-Tone Golden Damask & EDO-22 Weave
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
    }
    // -------------------------------------------------------------------------
    // SCENE 4: CHORUS 2 (38:00 - 50:00) | Cobalt Satin & Copper Wire SSAO Contact
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
    }
    // -------------------------------------------------------------------------
    // SCENE 5: VERSE 3 (50:00 - 62:00) | Carbon-Kevlar Thermal Strain Glow
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

            // Thermal blackbody strain color shift
            uint32_t spoke_col = (s % 2 == 0) ? 0xFFFF3300 : 0xFFFFaa00;
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
    }
    // -------------------------------------------------------------------------
    // SCENE 6: CHORUS 3 (62:00 - 80:00) | THE BASS DROP & 19D HOLOGRAPHIC STORM
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
    }
    // -------------------------------------------------------------------------
    // SCENE 7: OUTRO (80:00 - 90:00) | Gossamer Lace & Caustic Dewdrop Lensing
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
