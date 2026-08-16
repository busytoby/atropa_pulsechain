#ifndef TSFI_MP4_RENDERER_H
#define TSFI_MP4_RENDERER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MP4_WIDTH 1920
#define MP4_HEIGHT 1038
#define MP4_FPS 60
#define MP4_DEFAULT_SECONDS 90
#define MP4_DEFAULT_FRAMES (MP4_FPS * MP4_DEFAULT_SECONDS) // 5400 frames

typedef struct {
    uint32_t frame_index;
    float timestamp_sec;
    float total_duration_sec;
    int scene_index; // 1 to 7 (or dynamic acts)
    uint32_t *framebuffer; // 1920x1038 ARGB
} TsfiRenderFrameContext;

typedef struct {
    int current_frame;
    int total_frames;
    float duration_seconds;
    char audio_wav_path[256];
    char output_mp4_path[256];
} TsfiMp4Pipeline;

void tsfi_mp4_pipeline_init(TsfiMp4Pipeline *pipe, const char *audio_wav, const char *output_mp4);
void tsfi_mp4_pipeline_init_custom(TsfiMp4Pipeline *pipe, float duration_seconds, const char *audio_wav, const char *output_mp4);
void tsfi_mp4_render_scene_frame(TsfiRenderFrameContext *ctx);
bool tsfi_mp4_compile_video_with_audio(TsfiMp4Pipeline *pipe);

#endif // TSFI_MP4_RENDERER_H
