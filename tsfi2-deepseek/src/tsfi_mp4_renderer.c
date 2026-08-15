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
// Formal Proving HUD & ZMM Code Structure Overlay
// -----------------------------------------------------------------------------
static void draw_proving_hud_overlay(uint32_t *fb, int w, int h, int scene, float t, uint64_t merkle_proof) {
    (void)h;
    // Draw top-left Chancery Prover Status Box
    int box_x = 40, box_y = 40;
    int box_w = 460, box_h = 100;
    uint32_t hud_border = 0xFF00FFCC; // Neon Cyan

    draw_line_thick(fb, w, h, box_x, box_y, box_x + box_w, box_y, hud_border, 1);
    draw_line_thick(fb, w, h, box_x + box_w, box_y, box_x + box_w, box_y + box_h, hud_border, 1);
    draw_line_thick(fb, w, h, box_x + box_w, box_y + box_h, box_x, box_y + box_h, hud_border, 1);
    draw_line_thick(fb, w, h, box_x, box_y + box_h, box_x, box_y, hud_border, 1);

    // Draw ZMM Register Telemetry bars inside HUD
    (void)scene;
    int bar_y = box_y + 30;
    int score_len = (int)((t / 90.0f) * 380.0f);
    if (score_len > 380) score_len = 380;



    draw_line_thick(fb, w, h, box_x + 20, bar_y, box_x + 20 + score_len, bar_y, 0xFF00FF7F, 2);
    draw_line_thick(fb, w, h, box_x + 20, bar_y + 25, box_x + 20 + 380, bar_y + 25, 0xFF333333, 1);

    // Draw Merkle root anchor hash tick marks in bottom right
    int rx = w - 400, ry = h - 60;
    for (int b = 0; b < 16; b++) {
        uint32_t bit_col = ((merkle_proof >> (b * 4)) & 0x1) ? 0xFF00FFCC : 0xFF555555;
        draw_line_thick(fb, w, h, rx + b * 22, ry, rx + b * 22, ry + 15, bit_col, 1);
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
    // SCENE 1: VERSE 1 (00:00 - 15:00) | Photorealistic Obsidian Silk Weave
    // -------------------------------------------------------------------------
    if (t < 15.0f) {
        ctx->scene_index = 1;
        for (int y = 0; y < h; y++) {
            float v = (float)y / (float)h;
            uint32_t r = (uint32_t)(20.0f * (1.0f - v));
            uint32_t g = (uint32_t)(14.0f * (1.0f - v));
            uint32_t b = (uint32_t)(28.0f * v + 6.0f);
            uint32_t col = 0xFF000000 | (r << 16) | (g << 8) | b;
            for (int x = 0; x < w; x++) fb[y * w + x] = col;
        }

        int cols = 40, rows = 24;
        float gw = (float)w * 0.70f, gh = (float)h * 0.70f;
        float sx = cx - gw * 0.5f, sy = cy - gh * 0.5f;

        for (int r = 0; r < rows - 1; r++) {
            for (int c = 0; c < cols - 1; c++) {
                float u0 = (float)c / (float)(cols - 1);
                float v0 = (float)r / (float)(rows - 1);
                float u1 = (float)(c + 1) / (float)(cols - 1);
                float v1 = (float)(r + 1) / (float)(rows - 1);

                float w00 = (coords19[c % 19] * cosf(u0 * 5.0f + t * 2.0f) + coords19[r % 19] * sinf(v0 * 3.0f + t)) * 50.0f;
                float w10 = (coords19[(c+1) % 19] * cosf(u1 * 5.0f + t * 2.0f) + coords19[r % 19] * sinf(v0 * 3.0f + t)) * 50.0f;
                float w01 = (coords19[c % 19] * cosf(u0 * 5.0f + t * 2.0f) + coords19[(r+1) % 19] * sinf(v1 * 3.0f + t)) * 50.0f;
                float w11 = (coords19[(c+1) % 19] * cosf(u1 * 5.0f + t * 2.0f) + coords19[(r+1) % 19] * sinf(v1 * 3.0f + t)) * 50.0f;

                int x00 = (int)(sx + u0 * gw), y00 = (int)(sy + v0 * gh + w00);
                int x10 = (int)(sx + u1 * gw), y10 = (int)(sy + v0 * gh + w10);
                int x01 = (int)(sx + u0 * gw), y01 = (int)(sy + v1 * gh + w01);
                int x11 = (int)(sx + u1 * gw), y11 = (int)(sy + v1 * gh + w11);

                uint32_t col = ((r + c) % 2 == 0) ? 0xFF4A3B5C : 0xFF6D5A85;
                draw_line_thick(fb, w, h, x00, y00, x10, y10, col, 0);
                draw_line_thick(fb, w, h, x00, y00, x01, y01, col, 0);
                if (c == cols - 2) draw_line_thick(fb, w, h, x10, y10, x11, y11, col, 0);
                if (r == rows - 2) draw_line_thick(fb, w, h, x01, y01, x11, y11, col, 0);
            }
        }
    }
    // -------------------------------------------------------------------------
    // SCENE 2: CHORUS 1 (15:00 - 25:00) | Emerald Velvet & 19D Quartz Conduits
    // -------------------------------------------------------------------------
    else if (t < 25.0f) {
        ctx->scene_index = 2;
        for (int i = 0; i < w * h; i++) fb[i] = 0xFF021008;

        float beat = fmodf(t, 0.5f) / 0.5f;
        float pulse = 1.0f + 0.25f * expf(-beat * 8.0f);

        for (int d = 0; d < 19; d++) {
            float rad_base = (90.0f + (float)d * 22.0f) * pulse;
            int pts = 48;
            int lx = 0, ly = 0, fx = 0, fy = 0;

            for (int p = 0; p < pts; p++) {
                float ang = (float)p * (2.0f * (float)M_PI / (float)pts);
                float ripple = sinf(ang * 8.0f + t * 4.0f) * (coords19[d] * 25.0f);
                int kx = (int)(cx + cosf(ang) * (rad_base + ripple) * 1.1f);
                int ky = (int)(cy + sinf(ang) * (rad_base + ripple) * 0.7f);

                if (p == 0) { fx = kx; fy = ky; }
                else {
                    uint32_t c_col = (d % 2 == 0) ? 0xFF00FF88 : 0xFF00E5FF;
                    draw_line_thick(fb, w, h, lx, ly, kx, ky, c_col, (d % 3 == 0) ? 1 : 0);
                }
                lx = kx; ly = ky;
            }
            draw_line_thick(fb, w, h, lx, ly, fx, fy, 0xFF00FF88, 0);
        }
    }
    // -------------------------------------------------------------------------
    // SCENE 3: VERSE 2 (25:00 - 38:00) | Two-Tone Golden Damask & EDO-22 Map
    // -------------------------------------------------------------------------
    else if (t < 38.0f) {
        ctx->scene_index = 3;
        for (int y = 0; y < h; y++) {
            uint32_t col = (y % 4 == 0) ? 0xFF221808 : 0xFF100C04;
            for (int x = 0; x < w; x++) fb[y * w + x] = col;
        }

        for (int d = 0; d < 19; d++) {
            float frac = (float)d / 19.0f;
            int x0 = (int)(cx + coords19[d] * (float)w * 0.48f);
            int y0 = (int)(80.0f + frac * (float)(h - 160));
            int x1 = (int)(cx - coords19[(d + 7) % 19] * (float)w * 0.48f);
            int y1 = (int)((float)h - 80.0f - frac * (float)(h - 160));

            uint32_t gold_col = (d % 2 == 0) ? 0xFFFFD700 : 0xFFE5C158;
            draw_line_thick(fb, w, h, x0, y0, x1, y1, gold_col, 1);

            if (x0 >= 3 && x0 < w - 3 && y0 >= 3 && y0 < h - 3) {
                fb[y0 * w + x0] = 0xFFFFFFFF;
                fb[(y0+1) * w + x0] = 0xFFFFF8DC;
            }
        }
    }
    // -------------------------------------------------------------------------
    // SCENE 4: CHORUS 2 (38:00 - 50:00) | 19D Cobalt Satin & Copper Manifolds
    // -------------------------------------------------------------------------
    else if (t < 50.0f) {
        ctx->scene_index = 4;
        for (int i = 0; i < w * h; i++) fb[i] = 0xFF050B1E;

        for (int d = 0; d < 19; d++) {
            float phi = (float)d * (2.0f * (float)M_PI / 19.0f) + t * 0.9f;
            int pts = 40;
            int lx_cobalt = 0, ly_cobalt = 0, lx_copper = 0, ly_copper = 0;

            for (int i = 0; i < pts; i++) {
                float theta = (float)i * (2.0f * (float)M_PI / (float)pts);
                float r1 = 280.0f + coords19[d] * 90.0f;
                int kx1 = (int)(cx + r1 * cosf(theta + phi) * 0.95f);
                int ky1 = (int)(cy + r1 * sinf(theta + phi) * 0.65f);

                float r2 = 230.0f - coords19[(d + 4) % 19] * 80.0f;
                int kx2 = (int)(cx + r2 * cosf(-theta - phi) * 0.95f);
                int ky2 = (int)(cy + r2 * sinf(-theta - phi) * 0.65f);

                if (i > 0) {
                    draw_line_thick(fb, w, h, lx_cobalt, ly_cobalt, kx1, ky1, 0xFF4D79FF, 1);
                    draw_line_thick(fb, w, h, lx_copper, ly_copper, kx2, ky2, 0xFFD27D2D, 1);
                }
                lx_cobalt = kx1; ly_cobalt = ky1;
                lx_copper = kx2; ly_copper = ky2;
            }
        }
    }
    // -------------------------------------------------------------------------
    // SCENE 5: VERSE 3 (50:00 - 62:00) | Strained Carbon-Kevlar Singularity
    // -------------------------------------------------------------------------
    else if (t < 62.0f) {
        ctx->scene_index = 5;
        for (int i = 0; i < w * h; i++) fb[i] = 0xFF140602;

        float tension = (t - 50.0f) / 12.0f;
        float focal_r = 320.0f * (1.0f - tension * 0.92f);

        int spokes = 38;
        for (int s = 0; s < spokes; s++) {
            float ang = (float)s * (2.0f * (float)M_PI / (float)spokes) + t * (1.0f + tension * 5.0f);
            int x0 = (int)(cx + cosf(ang) * (float)w * 0.55f);
            int y0 = (int)(cy + sinf(ang) * (float)h * 0.55f);
            int x1 = (int)(cx + cosf(ang) * (focal_r + coords19[s % 19] * 25.0f));
            int y1 = (int)(cy + sinf(ang) * (focal_r + coords19[s % 19] * 25.0f));

            uint32_t spoke_col = (s % 2 == 0) ? 0xFFFF4500 : 0xFFFF8C00;
            draw_line_thick(fb, w, h, x0, y0, x1, y1, spoke_col, 1);
        }

        int core_r = (int)(15.0f + sinf(t * 35.0f) * 10.0f);
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
                uint32_t r = (uint32_t)(25.0f + 20.0f * sinf(drop_t * 2.0f + (float)x * 0.01f));
                uint32_t g = (uint32_t)(18.0f + 18.0f * cosf(drop_t * 2.5f + (float)y * 0.01f));
                uint32_t b = (uint32_t)(40.0f + 25.0f * sinf(drop_t * 3.0f));
                fb[y * w + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
            }
        }

        for (int d = 0; d < 19; d++) {
            float base_ang = (float)d * (2.0f * (float)M_PI / 19.0f) + drop_t * 1.4f;
            int last_x = (int)cx, last_y = (int)cy;

            for (int s = 1; s <= 28; s++) {
                float dist = (float)s * 30.0f;
                float spiral = base_ang + (float)s * 0.14f;
                float shock = sinf(drop_t * 8.0f - (float)s * 0.3f) * 40.0f;

                int cur_x = (int)(cx + cosf(spiral) * (dist + shock + coords19[d] * 25.0f));
                int cur_y = (int)(cy + sinf(spiral) * (dist + shock + coords19[d] * 25.0f) * 0.75f);

                uint32_t holo = (s % 3 == 0) ? 0xFF00FFFF : ((s % 3 == 1) ? 0xFFFF00FF : 0xFFFFFFFF);
                draw_line_thick(fb, w, h, last_x, last_y, cur_x, cur_y, holo, 1);
                last_x = cur_x; last_y = cur_y;
            }
        }
    }
    // -------------------------------------------------------------------------
    // SCENE 7: OUTRO (80:00 - 90:00) | 19D Gossamer Lace & Dewdrops
    // -------------------------------------------------------------------------
    else {
        ctx->scene_index = 7;
        float outro_t = (t - 80.0f) / 10.0f;

        for (int y = 0; y < h; y++) {
            float mist = (1.0f - outro_t) * ((float)y / (float)h);
            uint32_t col = 0xFF000000 | ((uint32_t)(10.0f * mist) << 16) | ((uint32_t)(30.0f * mist + 6.0f) << 8) | (uint32_t)(18.0f * mist + 6.0f);
            for (int x = 0; x < w; x++) fb[y * w + x] = col;
        }

        for (int d = 0; d < 19; d++) {
            float rad = (70.0f + (float)d * 24.0f);
            int pts = 32;
            int lx = 0, ly = 0, fx = 0, fy = 0;

            for (int p = 0; p < pts; p++) {
                float ang = (float)p * (2.0f * (float)M_PI / (float)pts) + coords19[d] * 0.4f;
                int wx = (int)(cx + cosf(ang) * rad * 1.05f);
                int wy = (int)(cy + sinf(ang) * rad * 0.65f);

                if (p == 0) { fx = wx; fy = wy; }
                else {
                    draw_line_thick(fb, w, h, lx, ly, wx, wy, 0xFF5B8C68, 0);
                }

                if (p % 4 == 0 && (1.0f - outro_t) > 0.1f) {
                    fb[wy * w + wx] = 0xFFE0F7FA;
                    if (wy+1 < h) fb[(wy+1) * w + wx] = 0xFFFFFFFF;
                }
                lx = wx; ly = wy;
            }
            draw_line_thick(fb, w, h, lx, ly, fx, fy, 0xFF5B8C68, 0);
        }
    }

    // Draw Formal Proving HUD & ZMM Telemetry Overlay
    draw_proving_hud_overlay(fb, w, h, ctx->scene_index, t, 0x0d4e0757de528828ULL);
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
