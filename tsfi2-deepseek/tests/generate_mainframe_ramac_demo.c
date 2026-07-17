#include "tsfi_mainframe_decnet.h"
#include "tsfi_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define SAMPLE_RATE 44100
#define FPS 30
#define DURATION 90
#define TOTAL_FRAMES (DURATION * FPS)
#define SAMPLES_PER_FRAME (SAMPLE_RATE / FPS)

// DTMF Frequencies (Row/Col)
static const float DTMF_R1 = 697.0f;
static const float DTMF_C1 = 1209.0f;

int main(void) {
    printf("[Mainframe Demo] Starting C-based 90-second APPC & RAMAC Simulation...\n");

    // Initialize systems
    tsfi_appc_conversation conv;
    tsfi_appc_allocate(&conv, 1, 2);
    
    LauTelemetryState telem;
    memset(&telem, 0, sizeof(telem));
    
    FILE *audio_out = fopen("mainframe_ramac_90s_audio.pcm", "wb");
    FILE *video_out = fopen("mainframe_ramac_90s_video.yuv", "wb");
    
    if (!audio_out || !video_out) {
        printf("Error opening output streams!\n");
        if (audio_out) fclose(audio_out);
        if (video_out) fclose(video_out);
        return 1;
    }

    // Allocate frame buffers (YUV420p format)
    int width = 1280;
    int height = 720;
    size_t y_size = width * height;
    size_t uv_size = (width / 2) * (height / 2);
    uint8_t *y_plane = malloc(y_size);
    uint8_t *u_plane = malloc(uv_size);
    uint8_t *v_plane = malloc(uv_size);

    int16_t *audio_buf = malloc(SAMPLES_PER_FRAME * sizeof(int16_t));

    // Simulation loop
    for (int frame = 0; frame < TOTAL_FRAMES; frame++) {
        float time_sec = (float)frame / (float)FPS;

        // --- 1. APPC & SNA State Transitions (90s timeline) ---
        if (time_sec < 15.0f) {
            // Scene I: Boot
            conv.state = 0; // Allocated
            cmsctpn(&conv, "BOOT_TP", NULL);
        } else if (time_sec < 35.0f) {
            // Scene II: RAMAC Seek
            conv.state = 1; // Send
            cmsctpn(&conv, "RAMAC_SEEK_TP", NULL);
            tsfi_appc_pacing_adjust(&conv, 0); // Increase pacing
        } else if (time_sec < 55.0f) {
            // Scene III: Security Auth
            conv.state = 4; // Confirming
            cmsctpn(&conv, "SECURE_AUTH_TP", NULL);
            tsfi_appc_security_validate(&conv, "USER_ADMIN", "SECURE123");
        } else if (time_sec < 75.0f) {
            // Scene IV: Chaining
            conv.state = 2; // Receive
            cmsctpn(&conv, "CHAINING_TP", NULL);
            tsfi_appc_set_conversation_type(&conv, 1); // Basic mode
        } else {
            // Scene V: Teardown & FET discharge
            conv.state = 3; // Deallocated
            cmsctpn(&conv, "TEARDOWN_TP", NULL);
        }

        // Update Vulkan shared telemetry state
        tsfi_appc_update_vulkan_telemetry(&conv, &telem);

        // --- 2. Audio Synthesis (Synthesizer Engines) ---
        for (int s = 0; s < SAMPLES_PER_FRAME; s++) {
            float t = time_sec + ((float)s / (float)SAMPLE_RATE);
            float val = 0.0f;

            if (time_sec < 15.0f) {
                // High frequency sweeps (Autodyne Resonance)
                float freq = 2000.0f + 1000.0f * sinf(2.0f * M_PI * 0.5f * t);
                val = sinf(2.0f * M_PI * freq * t);
            } else if (time_sec < 35.0f) {
                // RAMAC seek slides
                float track_id = time_sec - 15.0f;
                float freq = 300.0f + track_id * 50.0f;
                val = sinf(2.0f * M_PI * freq * t);
            } else if (time_sec < 55.0f) {
                // Low-frequency security sub-bass pulses
                val = sinf(2.0f * M_PI * 55.0f * t) * (1.0f - fmodf(t, 1.0f));
            } else if (time_sec < 75.0f) {
                // DTMF chord loops
                val = 0.5f * sinf(2.0f * M_PI * DTMF_R1 * t) + 0.5f * sinf(2.0f * M_PI * DTMF_C1 * t);
            } else {
                // Decaying noise alert
                float noise = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
                val = noise * expf(-5.0f * fmodf(t, 2.0f));
            }

            audio_buf[s] = (int16_t)(val * 16384.0f);
        }
        fwrite(audio_buf, sizeof(int16_t), SAMPLES_PER_FRAME, audio_out);

        // --- 3. Video Frame Generation (YUV420p) ---
        // Render background luminance (grey grid lines)
        memset(y_plane, 16, y_size); // Dark background
        memset(u_plane, 128, uv_size);
        memset(v_plane, 128, uv_size);

        // Draw grid representation
        for (int gy = 0; gy < height; gy += 40) {
            for (int gx = 0; gx < width; gx++) {
                y_plane[gy * width + gx] = 40;
            }
        }
        for (int gx = 0; gx < width; gx += 40) {
            for (int gy = 0; gy < height; gy++) {
                y_plane[gy * width + gx] = 40;
            }
        }

        // Apply visual corner flash for security warnings / alerts
        if (conv.security_active == 0 && time_sec >= 35.0f && time_sec < 55.0f) {
            // Draw red flashing boundaries in YUV representation
            for (int r_row = 0; r_row < 100; r_row++) {
                for (int r_col = 0; r_col < 100; r_col++) {
                    y_plane[r_row * width + r_col] = 80;
                    u_plane[(r_row / 2) * (width / 2) + (r_col / 2)] = 240; // Red chroma
                }
            }
        }

        fwrite(y_plane, 1, y_size, video_out);
        fwrite(u_plane, 1, uv_size, video_out);
        fwrite(v_plane, 1, uv_size, video_out);
    }

    fclose(audio_out);
    fclose(video_out);
    free(y_plane);
    free(u_plane);
    free(v_plane);
    free(audio_buf);

    printf("[SUCCESS] Demo streams compiled to:\n");
    printf("  Audio: mainframe_ramac_90s_audio.pcm (16-bit raw PCM)\n");
    printf("  Video: mainframe_ramac_90s_video.yuv (1280x720 YUV420p)\n");
    return 0;
}
