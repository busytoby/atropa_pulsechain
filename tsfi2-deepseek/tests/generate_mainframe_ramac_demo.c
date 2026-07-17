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

struct wav_header {
    char chunk_id[4];
    uint32_t chunk_size;
    char format[4];
    char subchunk1_id[4];
    uint32_t subchunk1_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char subchunk2_id[4];
    uint32_t subchunk2_size;
};

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

// 1. Drum Machine: Kick Drum
float get_kick(float age) {
    if (age < 0.0f || age > 0.4f) return 0.0f;
    float freq = 150.0f * expf(-20.0f * age) + 45.0f;
    float env = expf(-8.0f * age);
    return sinf(2.0f * M_PI * freq * age) * env;
}

// 2. Drum Machine: Snare Drum
float get_snare(float age) {
    if (age < 0.0f || age > 0.3f) return 0.0f;
    float noise = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
    float env = expf(-12.0f * age);
    float tone = sinf(2.0f * M_PI * 180.0f * age) * expf(-30.0f * age);
    return (0.7f * noise + 0.3f * tone) * env;
}

// 3. Drum Machine: Hi-Hat
float get_hat(float age) {
    if (age < 0.0f || age > 0.05f) return 0.0f;
    float noise = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
    float env = expf(-80.0f * age);
    return noise * env;
}

// 4. Sub-Bass Synth (Sine)
float get_sub_bass(float age, float freq) {
    if (age < 0.0f || age > 0.8f) return 0.0f;
    float env = expf(-4.0f * age);
    return sinf(2.0f * M_PI * freq * age) * env;
}

// 5. Lead Synth (Sawtooth wave)
float get_lead(float age, float freq) {
    if (age < 0.0f || age > 0.4f) return 0.0f;
    float env = expf(-6.0f * age);
    float phase = freq * age;
    float saw = 2.0f * (phase - floorf(phase + 0.5f));
    return saw * env;
}

// 6. Pad Synthesizer (FM)
float get_pad(double t, float freq) {
    float mod = sinf(2.0f * M_PI * 2.0f * t) * 8.0f;
    return sinf(2.0f * M_PI * freq * t + mod) * 0.25f;
}

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
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;
    while (y >= x) {
        draw_pixel(y_plane, width, height, xm + x, ym + y, color);
        draw_pixel(y_plane, width, height, xm - x, ym + y, color);
        draw_pixel(y_plane, width, height, xm + x, ym - y, color);
        draw_pixel(y_plane, width, height, xm - x, ym - y, color);
        draw_pixel(y_plane, width, height, xm + y, ym + x, color);
        draw_pixel(y_plane, width, height, xm - y, ym + x, color);
        draw_pixel(y_plane, width, height, xm + y, ym - x, color);
        draw_pixel(y_plane, width, height, xm - y, ym - x, color);
        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
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
    
    FILE *audio_out = fopen("mainframe_ramac_90s_audio.wav", "wb");
    FILE *video_out = fopen("mainframe_ramac_90s_video.yuv", "wb");
    
    if (!audio_out || !video_out) {
        printf("Error opening output files!\n");
        if (audio_out) fclose(audio_out);
        if (video_out) fclose(video_out);
        return 1;
    }

    // Write WAV header
    struct wav_header header;
    memcpy(header.chunk_id, "RIFF", 4);
    header.chunk_size = 36 + TOTAL_FRAMES * SAMPLES_PER_FRAME * sizeof(int16_t);
    memcpy(header.format, "WAVE", 4);
    memcpy(header.subchunk1_id, "fmt ", 4);
    header.subchunk1_size = 16;
    header.audio_format = 1;
    header.num_channels = 1;
    header.sample_rate = SAMPLE_RATE;
    header.byte_rate = SAMPLE_RATE * sizeof(int16_t);
    header.block_align = sizeof(int16_t);
    header.bits_per_sample = 16;
    memcpy(header.subchunk2_id, "data", 4);
    header.subchunk2_size = TOTAL_FRAMES * SAMPLES_PER_FRAME * sizeof(int16_t);

    fwrite(&header, sizeof(header), 1, audio_out);

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
            conv.state = 0;
            cmsctpn(&conv, "BOOT_TP", NULL);
        } else if (time_sec < 35.0f) {
            scene_name = "II RAMAC GEOMETRY SEEK";
            conv.state = 1;
            cmsctpn(&conv, "RAMAC_SEEK_TP", NULL);
            tsfi_appc_pacing_adjust(&conv, 0);
        } else if (time_sec < 55.0f) {
            scene_name = "III APPC SECURITY AUTH";
            conv.state = 4;
            cmsctpn(&conv, "SECURE_AUTH_TP", NULL);
            tsfi_appc_security_validate(&conv, "USER_ADMIN", "SECURE123");
        } else if (time_sec < 75.0f) {
            scene_name = "IV DECNET OVER SNA CHAINING";
            conv.state = 2;
            cmsctpn(&conv, "CHAINING_TP", NULL);
            tsfi_appc_set_conversation_type(&conv, 1);
        } else {
            scene_name = "V FET DISCHARGE AND DACTLU";
            conv.state = 3;
            cmsctpn(&conv, "TEARDOWN_TP", NULL);
        }

        tsfi_appc_update_vulkan_telemetry(&conv, &telem);

        // --- 2. Multi-Instrument Synthesizer Engine (Drums + 5 Synths) ---
        float bpm = 120.0f;
        float beat_dur = 60.0f / bpm; // 0.5 seconds
        
        for (int s = 0; s < SAMPLES_PER_FRAME; s++) {
            float t = time_sec + ((float)s / (float)SAMPLE_RATE);
            
            // Calculate beat parameters
            int beat_idx = (int)(t / beat_dur);
            float last_beat = (float)beat_idx * beat_dur;
            float age_beat = t - last_beat;
            
            float mix = 0.0f;

            // Instrument 1: Drum Channel (Kick, Snare, Hi-hat)
            float kick_val = 0.0f;
            float snare_val = 0.0f;
            float hat_val = 0.0f;
            
            // Kick on 1 and 3 beats
            if (beat_idx % 2 == 0) {
                kick_val = get_kick(age_beat);
            }
            // Snare on 2 and 4 beats
            if (beat_idx % 2 == 1) {
                snare_val = get_snare(age_beat);
            }
            // Hi-hat on every half beat (8th notes)
            int hat_idx = (int)(t / (beat_dur / 2.0f));
            float last_hat = (float)hat_idx * (beat_dur / 2.0f);
            hat_val = get_hat(t - last_hat);
            
            float drums = kick_val * 0.5f + snare_val * 0.35f + hat_val * 0.15f;
            mix += drums * 0.4f;

            // Instrument 2: Sub-Bass (Deep Sine)
            float bass_freq = 55.0f; // A1 note
            if (beat_idx % 4 == 1) bass_freq = 65.41f; // C2 note
            if (beat_idx % 4 == 3) bass_freq = 48.99f; // G1 note
            mix += get_sub_bass(age_beat, bass_freq) * 0.35f;

            // Instrument 3: Sawtooth Lead (Arpeggiator)
            float melody_freqs[4] = {440.0f, 523.25f, 659.25f, 783.99f}; // Am7 arpeggio
            int note_idx = (int)(t / (beat_dur / 2.0f));
            float last_note = (float)note_idx * (beat_dur / 2.0f);
            float lead_freq = melody_freqs[note_idx % 4];
            mix += get_lead(t - last_note, lead_freq) * 0.15f;

            // Instrument 4: FM Pad (Chords)
            float pad_freq = 220.0f;
            if (time_sec >= 35.0f && time_sec < 75.0f) {
                pad_freq = 261.63f; // C4 chord base
            }
            mix += get_pad(t, pad_freq) * 0.15f;

            // Instrument 5: DTMF Chord Generator (Active during SNA/DECnet phase)
            if (time_sec >= 55.0f && time_sec < 75.0f) {
                float dtmf_tone = 0.5f * sinf(2.0f * M_PI * DTMF_R1 * t) + 0.5f * sinf(2.0f * M_PI * DTMF_C1 * t);
                mix += dtmf_tone * 0.1f;
            }

            audio_buf[s] = (int16_t)(mix * 16384.0f);
        }
        fwrite(audio_buf, sizeof(int16_t), SAMPLES_PER_FRAME, audio_out);

        // --- 3. Video Frame Generation ---
        memset(y_plane, 16, y_size);
        memset(u_plane, 128, uv_size);
        memset(v_plane, 128, uv_size);

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

        int cx = 640;
        int cy = 360;
        draw_circle(y_plane, width, height, cx, cy, 100, 150);
        draw_circle(y_plane, width, height, cx, cy, 180, 150);
        draw_circle(y_plane, width, height, cx, cy, 260, 150);

        float angle = time_sec * 0.5f;
        int target_x = cx + (int)(260.0f * cosf(angle));
        int target_y = cy + (int)(260.0f * sinf(angle));
        draw_line(y_plane, width, height, cx, cy, target_x, target_y, 235);
        draw_circle(y_plane, width, height, target_x, target_y, 10, 255);

        char label_buf[128];
        snprintf(label_buf, sizeof(label_buf), "SCENE %s", scene_name);
        draw_string(y_plane, width, height, 50, 50, label_buf, 235);

        snprintf(label_buf, sizeof(label_buf), "TELEMETRY %s", telem.last_directive_str);
        draw_string(y_plane, width, height, 50, 100, label_buf, 235);

        snprintf(label_buf, sizeof(label_buf), "STATE %d PACING %d", conv.state, conv.pacing_window);
        draw_string(y_plane, width, height, 50, 150, label_buf, 235);

        if (conv.security_active == 0 && time_sec >= 35.0f && time_sec < 55.0f) {
            for (int r_row = 0; r_row < 60; r_row++) {
                for (int r_col = 0; r_col < 60; r_col++) {
                    y_plane[r_row * width + r_col] = 120;
                    u_plane[(r_row / 2) * (width / 2) + (r_col / 2)] = 240;
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
    printf("  Audio: mainframe_ramac_90s_audio.wav (16-bit standard RIFF WAVE)\n");
    printf("  Video: mainframe_ramac_90s_video.yuv (1280x720 YUV420p)\n");
    return 0;
}
