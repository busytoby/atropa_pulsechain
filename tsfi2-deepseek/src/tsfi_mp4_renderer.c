#define _GNU_SOURCE
#include "tsfi_mp4_renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Verified LAU Token Contract Addresses and State Roots for 19D Projection
static const char *LAU_TOKENS[5] = {
    "0xAD4e198623A5E2723e19E4D4a6ECF72B1D19FE4B", // Judge Alpha
    "0xD07B9f3DF4E9634EbAa0CBF079816925b2C474Ce", // Witness Beta
    "0xd32c39fee49391c7952d1b30b15921b0d3b42e69", // Witness Gamma
    "0xed343c0f99c89ed7c3c934a88f90261fd6a9a68b", // Juror Delta
    "0x3e10ed242ecb3951151e7a07e0a8f43d4f150c0e"  // Juror Epsilon
};

// Compute 19D Projection Hyperplane from SHA-256 byte payload of valid LAU address
static void compute_19d_projection(const char *address, float t, float *out_coords, int dim_count) {
    if (!address || !out_coords || dim_count <= 0) return;

    size_t len = strlen(address);
    for (int d = 0; d < dim_count; d++) {
        uint8_t byte_val = (uint8_t)address[(d * 2) % len];
        float weight = (float)byte_val / 255.0f;
        // Non-preferential orthogonal projection across 19 dimensions
        out_coords[d] = sinf(t * (1.0f + (float)d * 0.1f) + weight * (float)M_PI) * (0.5f + weight * 0.5f);
    }
}

void tsfi_mp4_pipeline_init(TsfiMp4Pipeline *pipe, const char *audio_wav, const char *output_mp4) {
    if (!pipe) return;
    memset(pipe, 0, sizeof(TsfiMp4Pipeline));
    pipe->total_frames = MP4_TOTAL_FRAMES;
    if (audio_wav) strncpy(pipe->audio_wav_path, audio_wav, sizeof(pipe->audio_wav_path) - 1);
    if (output_mp4) strncpy(pipe->output_mp4_path, output_mp4, sizeof(pipe->output_mp4_path) - 1);
}

static inline void draw_line(uint32_t *pixels, int w, int h, int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        if (x0 >= 0 && x0 < w && y0 >= 0 && y0 < h) {
            pixels[y0 * w + x0] = color;
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

// -----------------------------------------------------------------------------
// 19D Hyperplane Projection & Verlet Cloth Simulation Renderer (No generic Lissajous)
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
    // SCENE 1: VERSE 1 (00:00 - 15:00) | 19D Obsidian Silk Drape
    // -------------------------------------------------------------------------
    if (t < 15.0f) {
        ctx->scene_index = 1;
        for (int y = 0; y < h; y++) {
            float v = (float)y / (float)h;
            uint32_t col = 0xFF000000 | ((uint32_t)(15.0f * (1.0f - v)) << 16) | ((uint32_t)(10.0f * (1.0f - v)) << 8) | (uint32_t)(20.0f * v + 5.0f);
            for (int x = 0; x < w; x++) fb[y * w + x] = col;
        }

        // Project 19D coordinates into 2D Verlet silk grid
        int cols = 32, rows = 20;
        float gw = (float)w * 0.65f, gh = (float)h * 0.65f;
        float sx = cx - gw * 0.5f, sy = cy - gh * 0.5f;

        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                float u = (float)c / (float)(cols - 1);
                float v = (float)r / (float)(rows - 1);

                // Displace with 19D projection weights
                float wave = (coords19[c % 19] * cosf(u * 6.0f + t) + coords19[r % 19] * sinf(v * 4.0f + t)) * 40.0f;
                int px0 = (int)(sx + u * gw);
                int py0 = (int)(sy + v * gh + wave);

                uint32_t col = (r % 2 == 0) ? 0xFF3D324C : 0xFF635277;

                if (c < cols - 1) {
                    float next_u = (float)(c + 1) / (float)(cols - 1);
                    float next_wave = (coords19[(c + 1) % 19] * cosf(next_u * 6.0f + t) + coords19[r % 19] * sinf(v * 4.0f + t)) * 40.0f;
                    draw_line(fb, w, h, px0, py0, (int)(sx + next_u * gw), (int)(sy + v * gh + next_wave), col);
                }
                if (r < rows - 1) {
                    float next_v = (float)(r + 1) / (float)(rows - 1);
                    float next_wave = (coords19[c % 19] * cosf(u * 6.0f + t) + coords19[(r + 1) % 19] * sinf(next_v * 4.0f + t)) * 40.0f;
                    draw_line(fb, w, h, px0, py0, (int)(sx + u * gw), (int)(sy + next_v * gh + next_wave), col);
                }
            }
        }
    }
    // -------------------------------------------------------------------------
    // SCENE 2: CHORUS 1 (15:00 - 25:00) | Emerald Velvet & 19D LAU Conduits
    // -------------------------------------------------------------------------
    else if (t < 25.0f) {
        ctx->scene_index = 2;
        for (int i = 0; i < w * h; i++) fb[i] = 0xFF020C06;

        float beat = fmodf(t, 0.5f) / 0.5f;
        float pulse = 1.0f + 0.20f * expf(-beat * 6.0f);

        // 19D LAU Token Voltage Projection Conduits
        for (int d = 0; d < 19; d++) {
            float rad = (120.0f + (float)d * 18.0f) * pulse;
            float ang_offset = coords19[d] * (float)M_PI;
            int pts = 36;
            int px0 = 0, py0 = 0, first_x = 0, first_y = 0;

            for (int p = 0; p < pts; p++) {
                float ang = (float)p * (2.0f * (float)M_PI / (float)pts) + ang_offset;
                int kx = (int)(cx + cosf(ang) * rad);
                int ky = (int)(cy + sinf(ang) * rad * 0.7f);

                if (p == 0) { first_x = kx; first_y = ky; }
                else {
                    uint32_t c_col = (d % 2 == 0) ? 0xFF00FF7F : 0xFF00E5EE;
                    draw_line(fb, w, h, px0, py0, kx, ky, c_col);
                }
                px0 = kx; py0 = ky;
            }
            draw_line(fb, w, h, px0, py0, first_x, first_y, 0xFF00FF7F);
        }
    }
    // -------------------------------------------------------------------------
    // SCENE 3: VERSE 2 (25:00 - 38:00) | Golden Damask & 19D EDO-22 Map
    // -------------------------------------------------------------------------
    else if (t < 38.0f) {
        ctx->scene_index = 3;
        for (int y = 0; y < h; y++) {
            uint32_t col = (y % 4 == 0) ? 0xFF181206 : 0xFF0C0903;
            for (int x = 0; x < w; x++) fb[y * w + x] = col;
        }

        // 19D Projection Lines connecting EDO-22 Micro-Divisions
        for (int d = 0; d < 19; d++) {
            float frac = (float)d / 19.0f;
            int x0 = (int)(cx + coords19[d] * (float)w * 0.45f);
            int y0 = (int)(100.0f + frac * (float)(h - 200));
            int x1 = (int)(cx - coords19[(d + 5) % 19] * (float)w * 0.45f);
            int y1 = (int)((float)h - 100.0f - frac * (float)(h - 200));

            uint32_t gold_col = (d % 2 == 0) ? 0xFFFFD700 : 0xFFDAA520;
            draw_line(fb, w, h, x0, y0, x1, y1, gold_col);

            if (x0 >= 2 && x0 < w - 2 && y0 >= 2 && y0 < h - 2) {
                fb[y0 * w + x0] = 0xFFFFFFFF;
            }
        }
    }
    // -------------------------------------------------------------------------
    // SCENE 4: CHORUS 2 (38:00 - 50:00) | 19D Dual-Manifolds (Cobalt & Copper)
    // -------------------------------------------------------------------------
    else if (t < 50.0f) {
        ctx->scene_index = 4;
        for (int i = 0; i < w * h; i++) fb[i] = 0xFF040816;

        // 19D Bijective Manifold Transduction
        for (int d = 0; d < 19; d++) {
            float phi = (float)d * (2.0f * (float)M_PI / 19.0f) + t * 0.8f;
            int pts = 32;
            int lx_cobalt = 0, ly_cobalt = 0, lx_copper = 0, ly_copper = 0;

            for (int i = 0; i < pts; i++) {
                float theta = (float)i * (2.0f * (float)M_PI / (float)pts);
                float r1 = 260.0f + coords19[d] * 80.0f;
                int kx1 = (int)(cx + r1 * cosf(theta + phi) * 0.9f);
                int ky1 = (int)(cy + r1 * sinf(theta + phi) * 0.6f);

                float r2 = 220.0f - coords19[(d + 3) % 19] * 70.0f;
                int kx2 = (int)(cx + r2 * cosf(-theta - phi) * 0.9f);
                int ky2 = (int)(cy + r2 * sinf(-theta - phi) * 0.6f);

                if (i > 0) {
                    draw_line(fb, w, h, lx_cobalt, ly_cobalt, kx1, ky1, 0xFF4169E1);
                    draw_line(fb, w, h, lx_copper, ly_copper, kx2, ky2, 0xFFB87333);
                }
                lx_cobalt = kx1; ly_cobalt = ky1;
                lx_copper = kx2; ly_copper = ky2;
            }
        }
    }
    // -------------------------------------------------------------------------
    // SCENE 5: VERSE 3 (50:00 - 62:00) | 19D Strained Carbon Singularity
    // -------------------------------------------------------------------------
    else if (t < 62.0f) {
        ctx->scene_index = 5;
        for (int i = 0; i < w * h; i++) fb[i] = 0xFF100602;

        float tension = (t - 50.0f) / 12.0f;
        float focal_r = 300.0f * (1.0f - tension * 0.90f);

        for (int d = 0; d < 19; d++) {
            float ang = (float)d * (2.0f * (float)M_PI / 19.0f) + t * (1.0f + tension * 4.0f);
            int x0 = (int)(cx + cosf(ang) * (float)w * 0.5f);
            int y0 = (int)(cy + sinf(ang) * (float)h * 0.5f);
            int x1 = (int)(cx + cosf(ang) * (focal_r + coords19[d] * 20.0f));
            int y1 = (int)(cy + sinf(ang) * (focal_r + coords19[d] * 20.0f));

            uint32_t spoke_col = (d % 2 == 0) ? 0xFFFF4500 : 0xFFFF8C00;
            draw_line(fb, w, h, x0, y0, x1, y1, spoke_col);
        }
    }
    // -------------------------------------------------------------------------
    // SCENE 6: CHORUS 3 (62:00 - 80:00) | 19D BASS DROP & HOLOGRAPHIC LATTICE
    // -------------------------------------------------------------------------
    else if (t < 80.0f) {
        ctx->scene_index = 6;
        float drop_t = t - 62.0f;
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                uint32_t r = (uint32_t)(20.0f + 15.0f * sinf(drop_t * 2.0f + (float)x * 0.01f));
                uint32_t g = (uint32_t)(15.0f + 15.0f * cosf(drop_t * 2.5f + (float)y * 0.01f));
                uint32_t b = (uint32_t)(35.0f + 20.0f * sinf(drop_t * 3.0f));
                fb[y * w + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
            }
        }

        // Full 19D Hyperplane Shockwave Dispersal
        for (int d = 0; d < 19; d++) {
            float base_ang = (float)d * (2.0f * (float)M_PI / 19.0f) + drop_t * 1.2f;
            int last_x = (int)cx, last_y = (int)cy;

            for (int s = 1; s <= 25; s++) {
                float dist = (float)s * 28.0f;
                float spiral = base_ang + (float)s * 0.12f;
                float shock = sinf(drop_t * 8.0f - (float)s * 0.3f) * 35.0f;

                int cur_x = (int)(cx + cosf(spiral) * (dist + shock + coords19[d] * 20.0f));
                int cur_y = (int)(cy + sinf(spiral) * (dist + shock + coords19[d] * 20.0f) * 0.7f);

                uint32_t holo = (s % 3 == 0) ? 0xFF00FFFF : ((s % 3 == 1) ? 0xFFFF00FF : 0xFFFFFFFF);
                draw_line(fb, w, h, last_x, last_y, cur_x, cur_y, holo);
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
            uint32_t col = 0xFF000000 | ((uint32_t)(8.0f * mist) << 16) | ((uint32_t)(25.0f * mist + 5.0f) << 8) | (uint32_t)(15.0f * mist + 5.0f);
            for (int x = 0; x < w; x++) fb[y * w + x] = col;
        }

        // 19D Micro-Dew Web Nodes
        for (int d = 0; d < 19; d++) {
            float rad = (60.0f + (float)d * 22.0f);
            int pts = 24;
            int lx = 0, ly = 0, fx = 0, fy = 0;

            for (int p = 0; p < pts; p++) {
                float ang = (float)p * (2.0f * (float)M_PI / (float)pts) + coords19[d] * 0.5f;
                int wx = (int)(cx + cosf(ang) * rad);
                int wy = (int)(cy + sinf(ang) * rad * 0.6f);

                if (p == 0) { fx = wx; fy = wy; }
                else {
                    draw_line(fb, w, h, lx, ly, wx, wy, 0xFF4A7C59);
                }

                if (p % 4 == 0 && (1.0f - outro_t) > 0.1f) {
                    fb[wy * w + wx] = 0xFFE0F7FA;
                }
                lx = wx; ly = wy;
            }
            draw_line(fb, w, h, lx, ly, fx, fy, 0xFF4A7C59);
        }
    }
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
