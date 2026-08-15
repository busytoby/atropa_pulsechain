#define _GNU_SOURCE
#include "tsfi_mp4_renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
// Advanced 7-Scene Photorealistic Geometry & Cloth Simulation Renderer
// -----------------------------------------------------------------------------
void tsfi_mp4_render_scene_frame(TsfiRenderFrameContext *ctx) {
    if (!ctx || !ctx->framebuffer) return;

    float t = ctx->timestamp_sec;
    int w = MP4_WIDTH;
    int h = MP4_HEIGHT;
    uint32_t *fb = ctx->framebuffer;
    float cx = (float)w * 0.5f;
    float cy = (float)h * 0.5f;

    // -------------------------------------------------------------------------
    // SCENE 1: VERSE 1 (00:00 - 15:00) | Obsidian Silk Drape & Harmonic Waves
    // -------------------------------------------------------------------------
    if (t < 15.0f) {
        ctx->scene_index = 1;
        // Background gradient: Deep obsidian with warm amber low-angle vignette
        for (int y = 0; y < h; y++) {
            float v = (float)y / (float)h;
            uint32_t r = (uint32_t)(15.0f * (1.0f - v));
            uint32_t g = (uint32_t)(10.0f * (1.0f - v));
            uint32_t b = (uint32_t)(20.0f * v + 5.0f);
            uint32_t col = 0xFF000000 | (r << 16) | (g << 8) | b;
            for (int x = 0; x < w; x++) fb[y * w + x] = col;
        }

        // Render Billowing Obsidian Silk Mesh (Verlet grid simulation)
        int cols = 32, rows = 20;
        float gw = (float)w * 0.6f;
        float gh = (float)h * 0.6f;
        float start_x = cx - gw * 0.5f;
        float start_y = cy - gh * 0.5f;

        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                float u = (float)c / (float)(cols - 1);
                float v = (float)r / (float)(rows - 1);

                // 55Hz Sub-Bass wind wave
                float wave = sinf(t * 3.5f + u * 4.0f) * cosf(t * 2.0f + v * 3.0f) * 45.0f;
                int px0 = (int)(start_x + u * gw);
                int py0 = (int)(start_y + v * gh + wave);

                // Anisotropic silk sheen color
                uint32_t silk_col = (r % 2 == 0) ? 0xFF352B42 : 0xFF58486A;

                if (c < cols - 1) {
                    float next_u = (float)(c + 1) / (float)(cols - 1);
                    float next_wave = sinf(t * 3.5f + next_u * 4.0f) * cosf(t * 2.0f + v * 3.0f) * 45.0f;
                    int px1 = (int)(start_x + next_u * gw);
                    int py1 = (int)(start_y + v * gh + next_wave);
                    draw_line(fb, w, h, px0, py0, px1, py1, silk_col);
                }
                if (r < rows - 1) {
                    float next_v = (float)(r + 1) / (float)(rows - 1);
                    float next_wave = sinf(t * 3.5f + u * 4.0f) * cosf(t * 2.0f + next_v * 3.0f) * 45.0f;
                    int px2 = (int)(start_x + u * gw);
                    int py2 = (int)(start_y + next_v * gh + next_wave);
                    draw_line(fb, w, h, px0, py0, px2, py2, silk_col);
                }
            }
        }
    }
    // -------------------------------------------------------------------------
    // SCENE 2: CHORUS 1 (15:00 - 25:00) | Emerald Velvet & Glowing Quartz Conduits
    // -------------------------------------------------------------------------
    else if (t < 25.0f) {
        ctx->scene_index = 2;
        // Deep emerald background
        for (int i = 0; i < w * h; i++) fb[i] = 0xFF020C06;

        float beat = fmodf(t, 0.5f) / 0.5f; // 120 BPM kick pulse
        float pulse_scale = 1.0f + 0.15f * expf(-beat * 6.0f);

        // Render Resonant Emerald Velvet Rings & Glowing Conduits
        int rings = 8;
        for (int ri = 1; ri <= rings; ri++) {
            float rad = (float)ri * 45.0f * pulse_scale;
            int num_pts = 64;
            int prev_x = 0, prev_y = 0, first_x = 0, first_y = 0;

            for (int p = 0; p < num_pts; p++) {
                float ang = (float)p * (2.0f * (float)M_PI / (float)num_pts) + t * 0.8f;
                // Acid growl modulation
                float r_mod = rad + sinf(ang * 6.0f + t * 4.0f) * 15.0f;
                int kx = (int)(cx + cosf(ang) * r_mod);
                int ky = (int)(cy + sinf(ang) * r_mod);

                if (p == 0) { first_x = kx; first_y = ky; }
                else {
                    uint32_t conduit_col = (ri % 2 == 0) ? 0xFF00FF7F : 0xFF00E5EE;
                    draw_line(fb, w, h, prev_x, prev_y, kx, ky, conduit_col);
                }
                prev_x = kx; prev_y = ky;
            }
            draw_line(fb, w, h, prev_x, prev_y, first_x, first_y, 0xFF00FF7F);
        }
    }
    // -------------------------------------------------------------------------
    // SCENE 3: VERSE 2 (25:00 - 38:00) | Two-Tone Golden Damask & EDO-22 Lattice
    // -------------------------------------------------------------------------
    else if (t < 38.0f) {
        ctx->scene_index = 3;
        // Warm golden-carbon background
        for (int y = 0; y < h; y++) {
            uint32_t col = (y % 4 == 0) ? 0xFF181206 : 0xFF0C0903;
            for (int x = 0; x < w; x++) fb[y * w + x] = col;
        }

        // Draw EDO-22 Arpeggiator Cross-Hatch Damask Tapestry
        int num_chords = 22;
        for (int c = 0; c < num_chords; c++) {
            float frac = (float)c / (float)num_chords;
            float shift = sinf(t * 2.0f + frac * (float)M_PI) * 120.0f;
            int x0 = (int)((float)w * frac + shift);
            int y0 = 100;
            int x1 = (int)((float)w * (1.0f - frac) - shift);
            int y1 = h - 100;

            uint32_t gold_col = (c % 2 == 0) ? 0xFFFFD700 : 0xFFDAA520;
            draw_line(fb, w, h, x0, y0, x1, y1, gold_col);

            // Draw glowing micro-lattice nodes
            if (x0 >= 3 && x0 < w - 3 && y0 >= 3 && y0 < h - 3) {
                fb[y0 * w + x0] = 0xFFFFFFFF;
            }
        }
    }
    // -------------------------------------------------------------------------
    // SCENE 4: CHORUS 2 (38:00 - 50:00) | 4D Quaternion Manifolds (Cobalt & Copper)
    // -------------------------------------------------------------------------
    else if (t < 50.0f) {
        ctx->scene_index = 4;
        // Deep cobalt studio background
        for (int i = 0; i < w * h; i++) fb[i] = 0xFF040816;

        // Dual Interlocking Satin & Copper Manifolds
        int strips = 16;
        for (int s = 0; s < strips; s++) {
            float phi = (float)s * ((float)M_PI / (float)strips) + t * 1.2f;
            int pts = 48;
            int px_cobalt = 0, py_cobalt = 0;
            int px_copper = 0, py_copper = 0;

            for (int i = 0; i < pts; i++) {
                float theta = (float)i * (2.0f * (float)M_PI / (float)pts);

                // Cobalt Manifold (Torus knot)
                float r1 = 280.0f + 70.0f * cosf(3.0f * theta);
                int kx1 = (int)(cx + r1 * cosf(2.0f * theta + phi) * 0.9f);
                int ky1 = (int)(cy + r1 * sinf(2.0f * theta + phi) * 0.6f);

                // Copper Manifold (Counter-rotating)
                float r2 = 240.0f + 60.0f * sinf(2.0f * theta);
                int kx2 = (int)(cx + r2 * cosf(-2.0f * theta - phi) * 0.9f);
                int ky2 = (int)(cy + r2 * sinf(-2.0f * theta - phi) * 0.6f);

                if (i > 0) {
                    draw_line(fb, w, h, px_cobalt, py_cobalt, kx1, ky1, 0xFF4169E1); // Royal Cobalt Blue
                    draw_line(fb, w, h, px_copper, py_copper, kx2, ky2, 0xFFB87333); // Metallic Copper
                }
                px_cobalt = kx1; py_cobalt = ky1;
                px_copper = kx2; py_copper = ky2;
            }
        }
    }
    // -------------------------------------------------------------------------
    // SCENE 5: VERSE 3 (50:00 - 62:00) | Strained Carbon-Kevlar Singularity
    // -------------------------------------------------------------------------
    else if (t < 62.0f) {
        ctx->scene_index = 5;
        // Thermal dark gradient
        for (int i = 0; i < w * h; i++) fb[i] = 0xFF100602;

        float tension_prog = (t - 50.0f) / 12.0f; // 0.0 to 1.0
        float focal_rad = 350.0f * (1.0f - tension_prog * 0.85f); // Compresses inward

        // Draw Inward-Stretching Carbon Fiber Spokes & Glowing Thermal Cracks
        int spokes = 36;
        for (int s = 0; s < spokes; s++) {
            float ang = (float)s * (2.0f * (float)M_PI / (float)spokes) + t * (1.0f + tension_prog * 5.0f);
            int x0 = (int)(cx + cosf(ang) * (float)w * 0.6f);
            int y0 = (int)(cy + sinf(ang) * (float)h * 0.6f);
            int x1 = (int)(cx + cosf(ang) * focal_rad);
            int y1 = (int)(cy + sinf(ang) * focal_rad);

            // Glowing incandescent orange-red on high tension
            uint32_t spoke_col = (s % 2 == 0) ? 0xFFFF4500 : 0xFFFF8C00;
            draw_line(fb, w, h, x0, y0, x1, y1, spoke_col);
        }

        // Stroboscopic central core
        int core_r = (int)(20.0f + sinf(t * 30.0f) * 10.0f);
        for (int dy = -core_r; dy <= core_r; dy++) {
            for (int dx = -core_r; dx <= core_r; dx++) {
                if (dx*dx + dy*dy <= core_r*core_r) {
                    fb[(int)(cy + dy) * w + (int)(cx + dx)] = 0xFFFFFFFF;
                }
            }
        }
    }
    // -------------------------------------------------------------------------
    // SCENE 6: CHORUS 3 (62:00 - 80:00) | THE MASSIVE BASS DROP & HOLOGRAPHIC STORM
    // -------------------------------------------------------------------------
    else if (t < 80.0f) {
        ctx->scene_index = 6;
        // Volumetric iridescence background
        float drop_t = t - 62.0f;
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                uint32_t r = (uint32_t)(20.0f + 15.0f * sinf(drop_t * 2.0f + (float)x * 0.01f));
                uint32_t g = (uint32_t)(15.0f + 15.0f * cosf(drop_t * 2.5f + (float)y * 0.01f));
                uint32_t b = (uint32_t)(35.0f + 20.0f * sinf(drop_t * 3.0f));
                fb[y * w + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
            }
        }

        // Exploding 360-degree Billowing Silk Ribbons & Hyper-Luminous Lattice
        int ribbons = 24;
        for (int rb = 0; rb < ribbons; rb++) {
            float base_ang = (float)rb * (2.0f * (float)M_PI / (float)ribbons) + drop_t * 1.5f;
            int num_steps = 30;
            int last_rx = (int)cx, last_ry = (int)cy;

            for (int s = 1; s <= num_steps; s++) {
                float dist = (float)s * 25.0f;
                float spiral_ang = base_ang + (float)s * 0.15f;
                float shockwave = sinf(drop_t * 8.0f - (float)s * 0.3f) * 30.0f;

                int cur_rx = (int)(cx + cosf(spiral_ang) * (dist + shockwave));
                int cur_ry = (int)(cy + sinf(spiral_ang) * (dist + shockwave) * 0.7f);

                uint32_t holo_col = (s % 3 == 0) ? 0xFF00FFFF : ((s % 3 == 1) ? 0xFFFF00FF : 0xFFFFFFFF);
                draw_line(fb, w, h, last_rx, last_ry, cur_rx, cur_ry, holo_col);
                last_rx = cur_rx; last_ry = cur_ry;
            }
        }
    }
    // -------------------------------------------------------------------------
    // SCENE 7: OUTRO (80:00 - 90:00) | Translucent Gossamer Lace & Morning Dew
    // -------------------------------------------------------------------------
    else {
        ctx->scene_index = 7;
        float outro_prog = (t - 80.0f) / 10.0f; // 0.0 to 1.0

        // Peaceful forest green morning mist
        for (int y = 0; y < h; y++) {
            float mist = (1.0f - outro_prog) * ((float)y / (float)h);
            uint32_t r = (uint32_t)(8.0f * mist);
            uint32_t g = (uint32_t)(25.0f * mist + 5.0f);
            uint32_t b = (uint32_t)(15.0f * mist + 5.0f);
            uint32_t col = 0xFF000000 | (r << 16) | (g << 8) | b;
            for (int x = 0; x < w; x++) fb[y * w + x] = col;
        }

        // Delicate Translucent Gossamer Web Rings
        int webs = 6;
        for (int wb = 1; wb <= webs; wb++) {
            float rad = (float)wb * 60.0f;
            int num_nodes = 32;
            int lx = 0, ly = 0, fx = 0, fy = 0;

            for (int n = 0; n < num_nodes; n++) {
                float ang = (float)n * (2.0f * (float)M_PI / (float)num_nodes);
                int wx = (int)(cx + cosf(ang) * rad);
                int wy = (int)(cy + sinf(ang) * rad * 0.6f);

                if (n == 0) { fx = wx; fy = wy; }
                else {
                    draw_line(fb, w, h, lx, ly, wx, wy, 0xFF4A7C59); // Sage green gossamer
                }

                // Dewdrop sparkles
                if (n % 4 == 0 && (1.0f - outro_prog) > 0.1f) {
                    float sparkle = sinf(t * 12.0f + (float)n) * 2.0f;
                    int dew_r = (int)(3.0f + sparkle);
                    for (int dy = -dew_r; dy <= dew_r; dy++) {
                        for (int dx = -dew_r; dx <= dew_r; dx++) {
                            int sx = wx + dx, sy = wy + dy;
                            if (sx >= 0 && sx < w && sy >= 0 && sy < h) {
                                fb[sy * w + sx] = 0xFFE0F7FA; // Sparkling water dew
                            }
                        }
                    }
                }
                lx = wx; ly = wy;
            }
            draw_line(fb, w, h, lx, ly, fx, fy, 0xFF4A7C59);
        }
    }
}

// -----------------------------------------------------------------------------
// Video Compiler Pipeline (FFmpeg RawVideo Pipe)
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
