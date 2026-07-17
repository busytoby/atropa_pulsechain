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

// Simple 3x5 Pixel Font for drawing labels on the YUV buffer
static const uint8_t char_glyphs[36][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}  // Z
};

void draw_pixel(uint8_t *y_plane, int width, int height, int x, int y, uint8_t color) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        y_plane[y * width + x] = color;
    }
}

void draw_line(uint8_t *y_plane, int width, int height, int x0, int y0, int x1, int y1, uint8_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        draw_pixel(y_plane, width, height, x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void draw_circle(uint8_t *y_plane, int width, int height, int xm, int ym, int r, uint8_t color) {
    int x = -r, y = 0, err = 2 - 2 * r;
    do {
        draw_pixel(y_plane, width, height, xm - x, ym + y, color);
        draw_pixel(y_plane, width, height, xm - y, ym - x, color);
        draw_pixel(y_plane, width, height, xm + x, ym - y, color);
        draw_pixel(y_plane, width, height, xm + y, ym + x, color);
        r = err;
        if (r <= y) err += ++y * 2 + 1;
        if (r > x || err > y) err += ++x * 2 + 1;
    } while (x < 0);
}

void draw_char(uint8_t *y_plane, int width, int height, int x, int y, char c, uint8_t color) {
    int idx = -1;
    if (c >= '0' && c <= '9') idx = c - '0';
    else if (c >= 'A' && c <= 'Z') idx = c - 'A' + 10;
    else if (c >= 'a' && c <= 'z') idx = c - 'a' + 10;

    if (idx != -1) {
        for (int col = 0; col < 5; col++) {
            uint8_t line = char_glyphs[idx][col];
            for (int row = 0; row < 8; row++) {
                if (line & (1 << row)) {
                    // Draw 2x2 pixels per font dot for visibility
                    draw_pixel(y_plane, width, height, x + col * 2, y + row * 2, color);
                    draw_pixel(y_plane, width, height, x + col * 2 + 1, y + row * 2, color);
                    draw_pixel(y_plane, width, height, x + col * 2, y + row * 2 + 1, color);
                    draw_pixel(y_plane, width, height, x + col * 2 + 1, y + row * 2 + 1, color);
                }
            }
        }
    }
}

void draw_string(uint8_t *y_plane, int width, int height, int x, int y, const char *str, uint8_t color) {
    while (*str) {
        if (*str == ' ') {
            x += 8;
        } else {
            draw_char(y_plane, width, height, x, y, *str, color);
            x += 14;
        }
        str++;
    }
}

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
        const char *scene_name = "NONE";
        if (time_sec < 15.0f) {
            scene_name = "I SYSTEM BOOT";
            conv.state = 0; // Allocated
            cmsctpn(&conv, "BOOT_TP", NULL);
        } else if (time_sec < 35.0f) {
            scene_name = "II RAMAC GEOMETRY SEEK";
            conv.state = 1; // Send
            cmsctpn(&conv, "RAMAC_SEEK_TP", NULL);
            tsfi_appc_pacing_adjust(&conv, 0); // Increase pacing
        } else if (time_sec < 55.0f) {
            scene_name = "III APPC SECURITY AUTH";
            conv.state = 4; // Confirming
            cmsctpn(&conv, "SECURE_AUTH_TP", NULL);
            tsfi_appc_security_validate(&conv, "USER_ADMIN", "SECURE123");
        } else if (time_sec < 75.0f) {
            scene_name = "IV DECNET OVER SNA CHAINING";
            conv.state = 2; // Receive
            cmsctpn(&conv, "CHAINING_TP", NULL);
            tsfi_appc_set_conversation_type(&conv, 1); // Basic mode
        } else {
            scene_name = "V FET DISCHARGE AND DACTLU";
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
                float freq = 2000.0f + 1000.0f * sinf(2.0f * M_PI * 0.5f * t);
                val = sinf(2.0f * M_PI * freq * t);
            } else if (time_sec < 35.0f) {
                float track_id = time_sec - 15.0f;
                float freq = 300.0f + track_id * 50.0f;
                val = sinf(2.0f * M_PI * freq * t);
            } else if (time_sec < 55.0f) {
                val = sinf(2.0f * M_PI * 55.0f * t) * (1.0f - fmodf(t, 1.0f));
            } else if (time_sec < 75.0f) {
                val = 0.5f * sinf(2.0f * M_PI * DTMF_R1 * t) + 0.5f * sinf(2.0f * M_PI * DTMF_C1 * t);
            } else {
                float noise = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
                val = noise * expf(-5.0f * fmodf(t, 2.0f));
            }

            audio_buf[s] = (int16_t)(val * 16384.0f);
        }
        fwrite(audio_buf, sizeof(int16_t), SAMPLES_PER_FRAME, audio_out);

        // --- 3. Video Frame Generation (CAD Layout YUV420p) ---
        memset(y_plane, 16, y_size); // Dark background
        memset(u_plane, 128, uv_size);
        memset(v_plane, 128, uv_size);

        // Draw grey reference grid lines
        for (int gy = 0; gy < height; gy += 80) {
            for (int gx = 0; gx < width; gx++) {
                y_plane[gy * width + gx] = 40;
            }
        }
        for (int gx = 0; gx < width; gx += 80) {
            for (int gy = 0; gy < height; gy++) {
                y_plane[gy * width + gx] = 40;
            }
        }

        // Draw Concentric CAD Circles (RAMAC cylinders)
        int cx = 640;
        int cy = 360;
        draw_circle(y_plane, width, height, cx, cy, 100, 150);
        draw_circle(y_plane, width, height, cx, cy, 180, 150);
        draw_circle(y_plane, width, height, cx, cy, 260, 150);

        // Draw mechanical Access Arm vector line based on timeline
        float angle = time_sec * 0.5f;
        int target_x = cx + (int)(260.0f * cosf(angle));
        int target_y = cy + (int)(260.0f * sinf(angle));
        draw_line(y_plane, width, height, cx, cy, target_x, target_y, 235);
        draw_circle(y_plane, width, height, target_x, target_y, 10, 255); // Head pointer

        // Render CAD Text Overlays
        char label_buf[128];
        snprintf(label_buf, sizeof(label_buf), "SCENE %s", scene_name);
        draw_string(y_plane, width, height, 50, 50, label_buf, 235);

        snprintf(label_buf, sizeof(label_buf), "TELEMETRY %s", telem.last_directive_str);
        draw_string(y_plane, width, height, 50, 100, label_buf, 235);

        snprintf(label_buf, sizeof(label_buf), "STATE %d PACING %d", conv.state, conv.pacing_window);
        draw_string(y_plane, width, height, 50, 150, label_buf, 235);

        // Apply visual corner flash for security warnings / alerts
        if (conv.security_active == 0 && time_sec >= 35.0f && time_sec < 55.0f) {
            // Draw red flashing boundaries in YUV representation
            for (int r_row = 0; r_row < 60; r_row++) {
                for (int r_col = 0; r_col < 60; r_col++) {
                    y_plane[r_row * width + r_col] = 120;
                    u_plane[(r_row / 2) * (width / 2) + (r_col / 2)] = 240; // Red chroma
                }
            }
            draw_string(y_plane, width, height, 80, 50, "SECURITY WARNING", 255);
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
