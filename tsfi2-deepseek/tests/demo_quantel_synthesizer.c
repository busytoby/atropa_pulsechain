#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "tsfi_paint.h"
#include "tsfi_computel_blue_box.h"

#define WIDTH 512
#define HEIGHT 512
#define FRAMES 2700 // 90 seconds @ 30 FPS
#define SAMPLE_RATE 44100

// WAV Header struct
#pragma pack(push, 1)
struct WavHeader {
    char riff[4];
    int32_t overall_size;
    char wave[4];
    char fmt_chunk_marker[4];
    int32_t fmt_length;
    int16_t audio_format;
    int16_t num_channels;
    int32_t sample_rate;
    int32_t byte_rate;
    int16_t block_align;
    int16_t bits_per_sample;
    char data_chunk_header[4];
    int32_t data_size;
};
#pragma pack(pop)

// Generate 90s synthesizer soundtrack
void generate_synth_soundtrack(const char *filename) {
    FILE *f = fopen(filename, "wb");
    if (!f) return;

    int total_samples = SAMPLE_RATE * 90;
    int data_size = total_samples * sizeof(int16_t);

    struct WavHeader hdr;
    memcpy(hdr.riff, "RIFF", 4);
    hdr.overall_size = 36 + data_size;
    memcpy(hdr.wave, "WAVE", 4);
    memcpy(hdr.fmt_chunk_marker, "fmt ", 4);
    hdr.fmt_length = 16;
    hdr.audio_format = 1;
    hdr.num_channels = 1;
    hdr.sample_rate = SAMPLE_RATE;
    hdr.byte_rate = SAMPLE_RATE * sizeof(int16_t);
    hdr.block_align = sizeof(int16_t);
    hdr.bits_per_sample = 16;
    memcpy(hdr.data_chunk_header, "data", 4);
    hdr.data_size = data_size;

    fwrite(&hdr, 1, sizeof(hdr), f);

    // Simple FM/additive synthesizer melody generator with Lissajous-driven drums
    float phase_bass = 0.0f;
    float phase_lead = 0.0f;
    float phase_pad = 0.0f;
    float phase_drum_kick = 0.0f;

    for (int s = 0; s < total_samples; s++) {
        float t = (float)s / SAMPLE_RATE;

        // Lissajous modulation signals
        float X = sinf(t * 2.0f * (float)M_PI * 1.5f);
        float Y = cosf(t * 2.0f * (float)M_PI * 2.5f);

        // Dynamic chord/melody changes
        int chord_idx = ((int)t / 4) % 4;
        float chord_freqs[4] = {110.0f, 130.81f, 146.83f, 165.0f}; // A2, C3, D3, E3
        float base_f = chord_freqs[chord_idx];

        // Lead note changes every 0.25 seconds
        int note_idx = ((int)(t * 4)) % 8;
        float scales[8] = {1.0f, 1.2f, 1.25f, 1.5f, 1.6f, 1.8f, 1.875f, 2.0f};
        float lead_freq = base_f * scales[note_idx] * 4.0f;

        // Synthesize waves
        phase_bass += (2.0f * (float)M_PI * base_f) / SAMPLE_RATE;
        phase_pad += (2.0f * (float)M_PI * base_f * 1.5f) / SAMPLE_RATE;
        phase_lead += (2.0f * (float)M_PI * lead_freq) / SAMPLE_RATE;

        if (phase_bass > 2.0f * (float)M_PI) { phase_bass -= 2.0f * (float)M_PI; }
        if (phase_pad > 2.0f * (float)M_PI) { phase_pad -= 2.0f * (float)M_PI; }
        if (phase_lead > 2.0f * (float)M_PI) { phase_lead -= 2.0f * (float)M_PI; }

        float bass_sample = sinf(phase_bass) + 0.2f * sinf(phase_bass * 2.0f);
        float pad_sample = sinf(phase_pad) * 0.4f;
        float lead_sample = sinf(phase_lead + 0.3f * sinf(phase_lead * 2.5f)) * 0.3f;

        // Envelope modulations
        float lead_env = 1.0f - fmodf(t * 4.0f, 1.0f);
        lead_sample *= lead_env;

        // --- Lissajous Drums synthesis ---
        float drum_sample = 0.0f;

        // Kick Drum (120 BPM, every 0.5 seconds)
        float t_kick = fmodf(t, 0.5f);
        float kick_env = expf(-t_kick * 30.0f);
        float kick_freq = 150.0f * kick_env * (1.0f + 0.1f * X * Y);
        phase_drum_kick += (2.0f * (float)M_PI * kick_freq) / SAMPLE_RATE;
        if (phase_drum_kick > 2.0f * (float)M_PI) { phase_drum_kick -= 2.0f * (float)M_PI; }
        float kick_sample = sinf(phase_drum_kick) * kick_env * 1.0f;

        // Snare Drum (triggered on off beats: offset by 0.25 seconds)
        float t_snare = fmodf(t + 0.25f, 0.5f);
        float snare_env = expf(-t_snare * 15.0f);
        float noise = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
        // Resonant modulation using Lissajous phase
        float snare_sample = noise * snare_env * (0.6f + 0.2f * sinf(X * Y * (float)M_PI)) * 0.5f;

        // Hi-Hats (every 0.125 seconds)
        float t_hat = fmodf(t, 0.125f);
        float hat_env = expf(-t_hat * 80.0f);
        float hat_sample = noise * hat_env * 0.15f;

        drum_sample = kick_sample + snare_sample + hat_sample;

        float combined = (bass_sample + pad_sample + lead_sample + drum_sample) * 0.4f;
        if (combined > 1.0f) { combined = 1.0f; }
        if (combined < -1.0f) { combined = -1.0f; }

        int16_t sample_16 = (int16_t)(combined * 32767.0f);
        fwrite(&sample_16, 1, sizeof(int16_t), f);
    }

    fclose(f);
}

// Draw sprocket holes and black borders for Super8 crop (1.85:1)
void apply_super8_crop(uint32_t *pixels, int w, int h, int f_idx) {
    // 1.85:1 aspect crop inside 512x512
    int target_h = (int)(w / 1.85f);
    int border_y = (h - target_h) / 2;

    // Top border
    for (int y = 0; y < border_y; y++) {
        memset(pixels + y * w, 0, w * sizeof(uint32_t));
    }
    // Bottom border
    for (int y = h - border_y; y < h; y++) {
        memset(pixels + y * w, 0, w * sizeof(uint32_t));
    }

    float bob = sinf(f_idx * 0.1f) * 2.0f;

    // Sprocket holes on the left-hand margin (every 64 pixels vertically)
    for (int sy = border_y + 16; sy < h - border_y - 16; sy += 64) {
        int active_sy = sy + (int)bob;
        for (int y = active_sy - 12; y <= active_sy + 12; y++) {
            if (y < border_y || y >= h - border_y) { continue; }
            for (int x = 12; x <= 28; x++) {
                int dy = abs(y - active_sy);
                int dx = abs(x - 20);
                if (dy > 9 && dx > 6) { continue; }
                pixels[y * w + x] = 0xFF080808;
            }
        }
    }

    // Film grain overlay
    float noise = ((float)rand() / RAND_MAX - 0.5f) * 8.0f;
    for (int y = border_y; y < h - border_y; y++) {
        for (int x = 0; x < w; x++) {
            uint32_t pix = pixels[y * w + x];
            int r = ((pix >> 16) & 0xFF) + (int)noise;
            int g = ((pix >> 8) & 0xFF) + (int)noise;
            int b = (pix & 0xFF) + (int)noise;

            if (r < 0) { r = 0; }
            if (r > 255) { r = 255; }
            if (g < 0) { g = 0; }
            if (g > 255) { g = 255; }
            if (b < 0) { b = 0; }
            if (b > 255) { b = 255; }

            pixels[y * w + x] = (pix & 0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }

    if (f_idx % 25 == 0) {
        int sx = rand() % (w - 60) + 30;
        int sy = rand() % (h - 2 * border_y) + border_y;
        int len = rand() % 20 + 10;
        for (int i = 0; i < len; i++) {
            int px = sx + i;
            int py = sy + (int)(sinf(i * 0.2f) * 3.0f);
            if (px >= 0 && px < w && py >= border_y && py < h - border_y) {
                pixels[py * w + px] = 0xFF101010;
            }
        }
    }
}

int main() {
    srand((unsigned int)time(NULL));
    const char *audio_file = "/tmp/tsfi_synth_demo.wav";
    
    printf("[INFO] Synthesizing soundtrack...\n");
    generate_synth_soundtrack(audio_file);

    printf("[INFO] Launching FFmpeg encoder...\n");
    FILE *ffmpeg_pipe = popen("ffmpeg -y -f rawvideo -vcodec rawvideo -s 512x512 -pix_fmt rgb24 -r 30 -i - -i /tmp/tsfi_synth_demo.wav -c:v libx264 -preset ultrafast -c:a aac -b:a 192k -pix_fmt yuv420p bin/quantel_synthesis_demo.mp4 2>/dev/null", "w");
    if (!ffmpeg_pipe) {
        printf("[ERROR] Failed to open FFmpeg pipe.\n");
        return 1;
    }

    uint32_t *canvas = calloc(WIDTH * HEIGHT, sizeof(uint32_t));
    uint32_t *canvas_b = calloc(WIDTH * HEIGHT, sizeof(uint32_t));
    uint32_t *dst_buffer = calloc(WIDTH * HEIGHT, sizeof(uint32_t));
    uint8_t *rgb_out = malloc(WIDTH * HEIGHT * 3);

    printf("[INFO] Rendering 90-second Quantel Paintbox / Mirage / Harry showcase...\n");

    for (int f = 0; f < FRAMES; f++) {
        float t = (float)f / 30.0f;

        // Clear canvas with base background gradient
        for (int y = 0; y < HEIGHT; y++) {
            uint32_t color = (0xFF000000) | ((int)(15.0f * (1.0f + sinf(t + y * 0.01f))) << 16) |
                             ((int)(20.0f * (1.0f + cosf(t * 0.5f))) << 8) |
                             ((int)(40.0f * (1.0f + sinf(t * 0.3f))));
            for (int x = 0; x < WIDTH; x++) {
                canvas[y * WIDTH + x] = color;
            }
        }

        // Draw animated spirals using dynamic Paintbox tools
        float px = WIDTH / 2.0f + 160.0f * cosf(t * 2.5f) * sinf(t * 0.3f);
        float py = HEIGHT / 2.0f + 120.0f * sinf(t * 2.5f) * cosf(t * 0.3f);
        uint32_t paint_color = 0xFFFF00FF; // Magenta base
        
        tsfi_quantel_paintbox_pressure_jitter_hue(canvas, WIDTH, HEIGHT, (int)px, (int)py, 20, 0.8f, 0.2f, paint_color);
        tsfi_quantel_paintbox_pressure_jitter_size(canvas, WIDTH, HEIGHT, (int)(WIDTH - px), (int)(HEIGHT - py), 25, 0.7f, 0.3f, 0xFF00FFFF);

        // Draw a dry chalk textured path
        float chalk_x = WIDTH / 2.0f + 140.0f * cosf(t * 1.5f);
        float chalk_y = HEIGHT / 2.0f + 100.0f * sinf(t * 1.5f);
        tsfi_quantel_paintbox_chalk_pressure_texture(canvas, WIDTH, HEIGHT, (int)chalk_x, (int)chalk_y, 18, 0.9f, 2.0f, 0xFFFFA500);

        // Draw path with shifting opacity, saturation, brightness value, thickness and drawing angle orientation
        float op_sat_x = WIDTH / 2.0f + 100.0f * cosf(t * 3.1f);
        float op_sat_y = HEIGHT / 2.0f + 80.0f * sinf(t * 3.1f);
        tsfi_quantel_paintbox_pressure_jitter_opacity_saturation_value_width_angle(canvas, WIDTH, HEIGHT, (int)op_sat_x, (int)op_sat_y, 22, 0.85f, 0.25f, 0xFFADFF2F, t * 2.0f);

        // Periodically trigger a Mirage 3D Warp on chord hits (every 4 seconds)
        memcpy(canvas_b, canvas, WIDTH * HEIGHT * sizeof(uint32_t));
        int phase_cycle = ((int)t) % 4;
        if (phase_cycle == 0) {
            float pulse = 1.0f - fmodf(t, 1.0f);
            tsfi_quantel_mirage_sphere_warp(canvas_b, WIDTH, HEIGHT, dst_buffer, WIDTH, HEIGHT, 0.5f * pulse, 150.0f * (1.0f + pulse));
            memcpy(canvas_b, dst_buffer, WIDTH * HEIGHT * sizeof(uint32_t));
        } else if (phase_cycle == 1) {
            // Apply ribbon wave trapeze warp
            tsfi_quantel_mirage_ribbon_wave_warp(canvas, WIDTH, HEIGHT, canvas_b, WIDTH, HEIGHT, 0.05f, 20.0f * sinf(t));
        } else if (phase_cycle == 2) {
            // Spherical perspective twist
            tsfi_quantel_mirage_spherical_coordinate_zoom_twist_warp(canvas, WIDTH, HEIGHT, canvas_b, WIDTH, HEIGHT, 1.0f, 1.0f, 1.1f, 0.2f * sinf(t));
        } else if (phase_cycle == 3) {
            // 3D perspective page curl warp with active rotational perspective and shear
            tsfi_quantel_mirage_page_curl_perspective_rotation_shear_warp(canvas, WIDTH, HEIGHT, canvas_b, WIDTH, HEIGHT, 40.0f, 0.5f * sinf(t), 1.0f + 0.1f * sinf(t), 0.05f, t * 0.1f, 0.05f * sinf(t), 0.0f);
        }

        // Periodically trigger a Harry Wipe Transition (every 15 seconds)
        int transition_stage = ((int)t) % 15;
        if (transition_stage == 0) {
            float wipe_p = fmodf(t, 1.0f);
            uint32_t *matrix_canvas = calloc(WIDTH * HEIGHT, sizeof(uint32_t));
            for (int y = 0; y < HEIGHT; y++) {
                for (int x = 0; x < WIDTH; x++) {
                    matrix_canvas[y * WIDTH + x] = ((x / 16 + y / 16) % 2 == 0) ? 0xFF002244 : 0xFF004488;
                }
            }
            // Use vertical-scale-aspect field split color offset with rotation for transition
            tsfi_quantel_harry_blend_fields_color_offset_vertical_scale_aspect_rotation(canvas_b, matrix_canvas, dst_buffer, WIDTH, HEIGHT, wipe_p, 4, 0xFF00FF00, 3.0f * sinf(t), 1.0f + 0.05f * cosf(t), 1.0f + 0.02f * sinf(t), t * 0.05f);
            memcpy(canvas_b, dst_buffer, WIDTH * HEIGHT * sizeof(uint32_t));
            free(matrix_canvas);

            // Draw Concentric double outer storyboard highlights with custom margins, thicknesses and position offsets
            tsfi_quantel_storyboard_border_highlights_concentric_double_outer_width_offset(canvas_b, WIDTH, HEIGHT, 32, 120, WIDTH - 64, HEIGHT - 240, 4, 2, 3, 0xFFFFD700, 0xFFFF00FF, 10, 2, (int)(3.0f * sinf(t)), (int)(3.0f * cosf(t)));
        }

        // Enforce Super8 crop aspect ratio and sprocket holes
        apply_super8_crop(canvas_b, WIDTH, HEIGHT, f);

        // Convert to RGB24 and write to FFmpeg
        for (int i = 0; i < WIDTH * HEIGHT; i++) {
            uint32_t pix = canvas_b[i];
            rgb_out[i * 3 + 0] = (pix >> 16) & 0xFF;
            rgb_out[i * 3 + 1] = (pix >> 8) & 0xFF;
            rgb_out[i * 3 + 2] = pix & 0xFF;
        }

        fwrite(rgb_out, 1, WIDTH * HEIGHT * 3, ffmpeg_pipe);

        if (f % 150 == 0) {
            printf("  -> Synthesis loop: Rendered %d/%d frames (%.1f%%)...\n", f, FRAMES, (float)f / FRAMES * 100.0f);
        }
    }

    pclose(ffmpeg_pipe);
    free(canvas);
    free(canvas_b);
    free(dst_buffer);
    free(rgb_out);
    remove(audio_file);

    printf("\n[SUCCESS] 90-Second Synthesizer-Driven Showcase rendered to quantel_synthesis_demo.mp4\n");
    return 0;
}
