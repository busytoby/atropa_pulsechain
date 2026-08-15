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

void tsfi_mp4_render_scene_frame(TsfiRenderFrameContext *ctx) {
    if (!ctx || !ctx->framebuffer) return;

    float t = ctx->timestamp_sec;
    int w = MP4_WIDTH;
    int h = MP4_HEIGHT;

    // Determine Scene index (1 to 7)
    if (t < 15.0f) ctx->scene_index = 1;
    else if (t < 25.0f) ctx->scene_index = 2;
    else if (t < 38.0f) ctx->scene_index = 3;
    else if (t < 50.0f) ctx->scene_index = 4;
    else if (t < 62.0f) ctx->scene_index = 5;
    else if (t < 80.0f) ctx->scene_index = 6;
    else ctx->scene_index = 7;

    // Procedural 1.85:1 RenderMan Shader Simulation
    uint32_t bg_color = 0xFF050508; // Deep obsidian

    if (ctx->scene_index == 1) { // VERSE 1: Obsidian Silk Drape
        bg_color = 0xFF0A0A10;
    } else if (ctx->scene_index == 2) { // CHORUS 1: Emerald Crushed Velvet
        bg_color = 0xFF041208;
    } else if (ctx->scene_index == 3) { // VERSE 2: Golden Damask
        bg_color = 0xFF141005;
    } else if (ctx->scene_index == 4) { // CHORUS 2: Cobalt Satin & Copper Wire
        bg_color = 0xFF060B18;
    } else if (ctx->scene_index == 5) { // VERSE 3: Strained Carbon-Kevlar Singularity
        bg_color = 0xFF150804;
    } else if (ctx->scene_index == 6) { // CHORUS 3: BASS DROP Iridescent Shockwave
        bg_color = 0xFF181525;
    } else if (ctx->scene_index == 7) { // OUTRO: Gossamer Lace & Dew
        bg_color = 0xFF081510;
    }

    // Render Procedural Lissajous Quaternion Wave in 1.85:1 Space
    float phase = t * 2.0f;
    float center_x = (float)w * 0.5f;
    float center_y = (float)h * 0.5f;

    // Fill background
    for (int i = 0; i < w * h; i++) {
        ctx->framebuffer[i] = bg_color;
    }

    // Draw Lissajous curves and cloth lattice
    int num_points = 200;
    for (int p = 0; p < num_points; p++) {
        float prog = (float)p / (float)num_points;
        float qx = sinf(phase + prog * 4.0f * (float)M_PI) * ((float)w * 0.35f);
        float qy = cosf(phase * 1.5f + prog * 3.0f * (float)M_PI) * ((float)h * 0.35f);

        int px = (int)(center_x + qx);
        int py = (int)(center_y + qy);

        if (px >= 2 && px < w - 2 && py >= 2 && py < h - 2) {
            uint32_t pt_col = 0xFF00FFCC; // Cyan-green
            if (ctx->scene_index == 6) pt_col = 0xFFFFFFFF; // Max white
            else if (ctx->scene_index == 3) pt_col = 0xFFFFD700; // Gold

            for (int dy = -2; dy <= 2; dy++) {
                for (int dx = -2; dx <= 2; dx++) {
                    ctx->framebuffer[(py + dy) * w + (px + dx)] = pt_col;
                }
            }
        }
    }
}

bool tsfi_mp4_compile_video_with_audio(TsfiMp4Pipeline *pipe) {
    if (!pipe) return false;

    // Generate video using ffmpeg pipe from raw ARGB scanout buffer and mix with WAV audio
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

        // Write ARGB buffer to ffmpeg rawvideo pipe
        fwrite(frame, sizeof(uint32_t), (size_t)(MP4_WIDTH * MP4_HEIGHT), ffmpeg_pipe);
    }

    free(frame);
    int ret = pclose(ffmpeg_pipe);
    return (ret == 0);
}
