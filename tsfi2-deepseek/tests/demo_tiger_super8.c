#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "tsfi_paint.h"
#include "tsfi_computel_blue_box.h"
#include "tsfi_hogan.h"

#define WIDTH 512
#define HEIGHT 512
#define FRAMES 2700 // 90 seconds @ 30 FPS
#define SAMPLE_RATE 8000

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

// Simple State Variable Filter
typedef struct {
    double f, q, low, band, high;
} Resonator;

static void resonator_init(Resonator *r, double freq, double Q) {
    double omega = 2.0 * M_PI * freq / SAMPLE_RATE;
    r->f = 2.0 * sin(omega / 2.0);
    r->q = 1.0 / Q;
    r->low = 0.0;
    r->band = 0.0;
    r->high = 0.0;
}

static double resonator_tick(Resonator *r, double input) {
    r->high = input - r->low - r->q * r->band;
    r->band += r->f * r->high;
    r->low += r->f * r->band;
    return r->band;
}

static double parse_note(const char *note) {
    if (strcmp(note, "REST") == 0 || strcmp(note, "rest") == 0) return 0.0;
    char char0 = note[0];
    int idx = 0;
    switch (char0) {
        case 'C': case 'c': idx = 0; break;
        case 'D': case 'd': idx = 2; break;
        case 'E': case 'e': idx = 4; break;
        case 'F': case 'f': idx = 5; break;
        case 'G': case 'g': idx = 7; break;
        case 'A': case 'a': idx = 9; break;
        case 'B': case 'b': idx = 11; break;
        default: return 0.0;
    }
    char char1 = note[1];
    int octave_char = char1;
    if (char1 == '#' || char1 == 'b') {
        octave_char = note[2];
        if (char1 == '#') idx += 1;
        else idx -= 1;
    }
    int octave = octave_char - '0';
    double freq = 0.0;
    switch (idx) {
        case 0:  freq = 261.63; break; // C4
        case 1:  freq = 277.18; break;
        case 2:  freq = 293.66; break;
        case 3:  freq = 311.13; break;
        case 4:  freq = 329.63; break;
        case 5:  freq = 349.23; break;
        case 6:  freq = 369.99; break;
        case 7:  freq = 392.00; break;
        case 8:  freq = 415.30; break;
        case 9:  freq = 440.00; break;
        case 10: freq = 466.16; break; // Bb4
        case 11: freq = 493.88; break;
        default: return 0.0;
    }
    if (octave > 4) {
        for (int i = 4; i < octave; i++) freq *= 2.0;
    } else if (octave < 4) {
        for (int i = octave; i < 4; i++) freq /= 2.0;
    }
    return freq;
}

static void write_wav_header(FILE *fp, uint32_t data_size) {
    struct WavHeader header;
    memcpy(header.riff, "RIFF", 4);
    header.overall_size = 36 + data_size;
    memcpy(header.wave, "WAVE", 4);
    memcpy(header.fmt_chunk_marker, "fmt ", 4);
    header.fmt_length = 16;
    header.audio_format = 1;
    header.num_channels = 1;
    header.sample_rate = SAMPLE_RATE;
    header.byte_rate = SAMPLE_RATE * 2;
    header.block_align = 2;
    header.bits_per_sample = 16;
    memcpy(header.data_chunk_header, "data", 4);
    header.data_size = data_size;
    fwrite(&header, sizeof(struct WavHeader), 1, fp);
}

void generate_tiger_soundtrack(const char *filepath) {
    const char *bass_seq[] = {
        "C2", "C2", "C2", "C2", "C2", "C2", "C2", "C2",
        "C2", "C2", "Bb1", "C2", "C2", "C2", "C2", "C2",
        "C2", "C2", "Bb1", "C2", "C2", "C2", "C2", "C2",
        "C2", "C2", "Bb1", "C2", "C2", "Ab1", "Bb1", "C2"
    };
    const char *lead_seq[] = {
        "C4", "C4", "C4", "C4", "C4", "C4", "C4", "C4",
        "C4", "C4", "Bb3", "C4", "REST", "REST", "REST", "REST",
        "REST", "REST", "Bb3", "C4", "REST", "REST", "REST", "REST",
        "REST", "REST", "Bb3", "C4", "REST", "Ab3", "Bb3", "C4"
    };
    const char *arp_seq[] = {
        "C3", "Eb3", "G3", "C4", "Eb3", "G3", "C4", "Eb3",
        "C3", "Eb3", "G3", "C4", "Eb3", "G3", "C4", "Eb3",
        "C3", "Eb3", "G3", "C4", "Eb3", "G3", "C4", "Eb3",
        "C3", "Eb3", "G3", "C4", "Eb3", "G3", "C4", "Eb3"
    };
    int kick_seq[] = {
        1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
        1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0
    };
    int snare_seq[] = {
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0
    };
    int hh_seq[] = {
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0
    };
    int clap_seq[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0,
        1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1
    };

    int step_duration_ms = 136;
    int samples_per_step = (SAMPLE_RATE * step_duration_ms) / 1000;
    int loop_samples = samples_per_step * 32;
    int loops = (SAMPLE_RATE * 90) / loop_samples + 1;
    int total_samples = loop_samples * loops;

    FILE *f = fopen(filepath, "wb");
    if (!f) return;
    write_wav_header(f, total_samples * 2);

    double lead_phase = 0.0;
    double bass_phase = 0.0;
    double arp_phase = 0.0;

    double kick_env = 0.0;
    double kick_pitch = 150.0;
    double snare_env = 0.0;
    double hihat_env = 0.0;
    double clap_env = 0.0;

    Resonator snare_res, lead_res;
    resonator_init(&snare_res, 1000.0, 4.0);
    resonator_init(&lead_res, 800.0, 2.0);

    unsigned int seed = 0x54494752; // "TIGR"

    for (int l = 0; l < loops; l++) {
        for (int step = 0; step < 32; step++) {
            if (kick_seq[step]) { kick_env = 1.0; kick_pitch = 150.0; }
            if (snare_seq[step]) snare_env = 1.0;
            if (hh_seq[step]) hihat_env = 1.0;
            if (clap_seq[step]) clap_env = 1.0;

            double lead_freq = parse_note(lead_seq[step]);
            double bass_freq = parse_note(bass_seq[step]);
            double arp_freq = parse_note(arp_seq[step]);

            for (int s = 0; s < samples_per_step; s++) {
                double kick_out = 0.0;
                if (kick_env > 0.0) {
                    kick_out = sin(2.0 * M_PI * kick_pitch * s / SAMPLE_RATE) * kick_env;
                    kick_pitch *= 0.995;
                    kick_env *= 0.992;
                }

                double snare_out = 0.0;
                if (snare_env > 0.0) {
                    seed = seed * 1103515245 + 12345;
                    double noise = ((double)(seed & 0xFFFF) / 65535.0) - 0.5;
                    snare_out = resonator_tick(&snare_res, noise) * snare_env;
                    snare_env *= 0.995;
                }

                double hh_out = 0.0;
                if (hihat_env > 0.0) {
                    seed = seed * 1103515245 + 12345;
                    double noise = ((double)(seed & 0xFFFF) / 65535.0) - 0.5;
                    hh_out = noise * hihat_env * 0.5;
                    hihat_env *= 0.985;
                }

                double clap_out = 0.0;
                if (clap_env > 0.0) {
                    seed = seed * 1103515245 + 12345;
                    double noise = ((double)(seed & 0xFFFF) / 65535.0) - 0.5;
                    clap_out = noise * clap_env;
                    clap_env *= 0.991;
                }

                double lead_out = 0.0;
                if (lead_freq > 0.0) {
                    double raw_saw = (lead_phase / M_PI) - 1.0;
                    lead_out = resonator_tick(&lead_res, raw_saw) * 0.3;
                    lead_phase += (2.0 * M_PI * lead_freq) / SAMPLE_RATE;
                    if (lead_phase >= 2.0 * M_PI) lead_phase -= 2.0 * M_PI;
                }

                double bass_out = 0.0;
                if (bass_freq > 0.0) {
                    bass_out = (sin(bass_phase) >= 0.0) ? 0.25 : -0.25;
                    bass_phase += (2.0 * M_PI * bass_freq) / SAMPLE_RATE;
                    if (bass_phase >= 2.0 * M_PI) bass_phase -= 2.0 * M_PI;
                }

                double arp_out = 0.0;
                if (arp_freq > 0.0) {
                    double raw_tri = (arp_phase < M_PI) ? (arp_phase / M_PI * 2.0 - 1.0) : (3.0 - arp_phase / M_PI * 2.0);
                    arp_out = raw_tri * 0.15;
                    arp_phase += (2.0 * M_PI * arp_freq) / SAMPLE_RATE;
                    if (arp_phase >= 2.0 * M_PI) arp_phase -= 2.0 * M_PI;
                }

                double mixed = kick_out + snare_out + hh_out + clap_out + lead_out + bass_out + arp_out;
                if (mixed > 1.0) mixed = 1.0;
                if (mixed < -1.0) mixed = -1.0;

                int16_t sample_16 = (int16_t)(16384.0 * mixed);
                fwrite(&sample_16, 1, sizeof(int16_t), f);
            }
        }
    }
    fclose(f);
}

void apply_super8_crop(uint32_t *pixels, int w, int h, int f_idx) {
    int target_h = (int)(w / 1.85f);
    int border_y = (h - target_h) / 2;

    for (int y = 0; y < border_y; y++) {
        memset(pixels + y * w, 0, w * sizeof(uint32_t));
    }
    for (int y = h - border_y; y < h; y++) {
        memset(pixels + y * w, 0, w * sizeof(uint32_t));
    }

    float bob = sinf(f_idx * 0.1f) * 2.0f;

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

void draw_hogan_telemetry(uint32_t *pixels, int w, int h, const hogan_umbrella_system *sys, float t) {
    (void)h;
    int start_y = 398;
    int end_y = 508;
    int start_x = 35;
    int end_x = w - 35;

    for (int x = start_x; x <= end_x; x++) {
        pixels[start_y * w + x] = 0xFF00FF00;
        pixels[end_y * w + x] = 0xFF00FF00;
    }
    for (int y = start_y; y <= end_y; y++) {
        pixels[y * w + start_x] = 0xFF00FF00;
        pixels[y * w + end_x] = 0xFF00FF00;
    }

    for (int acc = 0; acc < 3; acc++) {
        if (!sys->accounts[acc].active) continue;
        uint64_t bal = sys->accounts[acc].balance;
        int bar_length = (int)(bal * 100 / 25000);
        if (bar_length > 120) bar_length = 120;
        if (bar_length < 0) bar_length = 0;
        int y_pos = start_y + 15 + acc * 18;

        uint32_t acc_color = (acc == 0) ? 0xFF00FFFF : ((acc == 1) ? 0xFF0080FF : 0xFFFFFF00);
        for (int dx = 0; dx < 8; dx++) {
            for (int dy = 0; dy < 8; dy++) {
                pixels[(y_pos + dy) * w + (start_x + 10 + dx)] = acc_color;
            }
        }

        for (int x = 0; x < bar_length; x++) {
            for (int dy = 0; dy < 6; dy++) {
                pixels[(y_pos + dy + 1) * w + (start_x + 25 + x)] = 0xFF00FF00;
            }
        }
    }

    int active_vm = ((int)(t * 2.0f)) % 3;
    int vm_x = start_x + 160;
    int vm_y = start_y + 20;

    for (int vm = 0; vm < 3; vm++) {
        uint32_t color = (vm == active_vm) ? 0xFF00FF00 : 0xFF003300;
        for (int dx = 0; dx < 25; dx++) {
            for (int dy = 0; dy < 12; dy++) {
                pixels[(vm_y + dy) * w + (vm_x + vm * 30 + dx)] = color;
            }
        }
    }

    int dot_y = start_y + 75;
    int dot_x = start_x + 160;
    int tx_limit = (sys->tx_count < 20) ? (int)sys->tx_count : 20;
    for (int i = 0; i < tx_limit; i++) {
        const hogan_transaction *tx = &sys->tx_log[sys->tx_count - tx_limit + i];
        uint32_t dot_color = tx->processed ? 0xFF00FF00 : 0xFFFF5500;
        int dot_width = (tx->amount > 150) ? 8 : 5;
        int dot_height = (tx->amount > 150) ? 12 : 8;

        for (int dx = 0; dx < dot_width; dx++) {
            for (int dy = 0; dy < dot_height; dy++) {
                int px_x = dot_x + i * 12 + dx;
                int px_y = dot_y + dy;
                if (px_x < end_x - 5) {
                    pixels[px_y * w + px_x] = dot_color;
                }
            }
        }
    }
}

int main() {
    srand((unsigned int)time(NULL));
    const char *audio_file = "/tmp/tsfi_tiger_demo.wav";
    
    printf("[INFO] Synthesizing Eye of the Tiger .bio arrangement...\n");
    generate_tiger_soundtrack(audio_file);

    printf("[INFO] Launching FFmpeg encoder...\n");
    FILE *ffmpeg_pipe = popen("ffmpeg -y -f rawvideo -vcodec rawvideo -s 512x512 -pix_fmt rgb24 -r 30 -i - -i /tmp/tsfi_tiger_demo.wav -c:v libx264 -preset ultrafast -c:a aac -b:a 192k -pix_fmt yuv420p bin/tiger_super8_demo.mp4 2>/dev/null", "w");
    if (!ffmpeg_pipe) {
        printf("[ERROR] Failed to open FFmpeg pipe.\n");
        return 1;
    }

    hogan_umbrella_system hogan_sys;
    tsfi_hogan_init(&hogan_sys);
    tsfi_hogan_register_account(&hogan_sys, 999, 1000000);
    tsfi_hogan_register_account(&hogan_sys, 1001, 10000);
    tsfi_hogan_register_account(&hogan_sys, 2002, 5000);
    tsfi_hogan_register_account(&hogan_sys, 3003, 2000);

    uint32_t *canvas = calloc(WIDTH * HEIGHT, sizeof(uint32_t));
    uint32_t *canvas_b = calloc(WIDTH * HEIGHT, sizeof(uint32_t));
    uint32_t *dst_buffer = calloc(WIDTH * HEIGHT, sizeof(uint32_t));
    uint8_t *rgb_out = malloc(WIDTH * HEIGHT * 3);

    printf("[INFO] Rendering 90-second Eye of the Tiger Super8 / Quantel demoscene showcase...\n");

    for (int f = 0; f < FRAMES; f++) {
        float t = (float)f / 30.0f;

        // Orange/Gold-tinted gradient background representing the Tiger glowing eye backdrop
        for (int y = 0; y < HEIGHT; y++) {
            uint32_t color = (0xFF000000) | 
                             ((int)(40.0f * (1.0f + sinf(t + y * 0.015f))) << 16) |
                             ((int)(20.0f * (1.0f + cosf(t * 0.4f))) << 8) |
                             ((int)(10.0f * (1.0f + sinf(t * 0.2f))));
            for (int x = 0; x < WIDTH; x++) {
                canvas[y * WIDTH + x] = color;
            }
        }

        // Draw animated Tiger eye glowing wireframe
        float center_x = WIDTH / 2.0f;
        float center_y = HEIGHT / 2.0f;
        float pulse = 1.0f + 0.15f * sinf(t * 8.0f); // Pulsing on simulated beat

        // Draw circular eye boundaries using Paintbox pressure tools
        for (int theta = 0; theta < 360; theta += 15) {
            float rad = (theta * (float)M_PI) / 180.0f;
            float ex = center_x + 90.0f * cosf(rad) * pulse;
            float ey = center_y + 50.0f * sinf(rad) * pulse;
            tsfi_quantel_paintbox_pressure_jitter_hue(canvas, WIDTH, HEIGHT, (int)ex, (int)ey, 14, 0.8f, 0.1f, 0xFFFF8C00); // Dark Orange
        }

        // Draw tiger pupil slit
        for (int dy = -40; dy <= 40; dy++) {
            int px = (int)center_x + (int)(sinf(t * 3.0f) * 2.0f);
            int py = (int)center_y + dy;
            if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                tsfi_quantel_paintbox_chalk_pressure_texture(canvas, WIDTH, HEIGHT, px, py, 10, 0.9f, 1.5f, 0xFF080808);
            }
        }

        // Draw dynamic tiger stripes sweeping across the screen using Paintbox pressure dynamics and path velocity shifts
        float stripe_x = center_x + 140.0f * cosf(t * 2.0f);
        float stripe_y = center_y + 80.0f * sinf(t * 3.5f);
        tsfi_quantel_paintbox_pressure_jitter_opacity_saturation_value_width_angle_shape_texture_path(canvas, WIDTH, HEIGHT, (int)stripe_x, (int)stripe_y, 16, 0.9f, 0.3f, 0xFFFF4500, t * 1.5f, 1.4f, 1.8f, 5.0f * cosf(t), 5.0f * sinf(t));

        memcpy(canvas_b, canvas, WIDTH * HEIGHT * sizeof(uint32_t));

        // Mirage 3D warps to simulate screen roll / optical distortions with depth offsets
        int phase_cycle = ((int)t) % 4;
        if (phase_cycle == 0) {
            float curl_pulse = 1.0f - fmodf(t, 1.0f);
            tsfi_quantel_mirage_sphere_warp(canvas_b, WIDTH, HEIGHT, dst_buffer, WIDTH, HEIGHT, 0.4f * curl_pulse, 120.0f * (1.0f + curl_pulse));
            memcpy(canvas_b, dst_buffer, WIDTH * HEIGHT * sizeof(uint32_t));
        } else if (phase_cycle == 3) {
            // Apply 3D page curl perspective rotation shear center light depth warp
            tsfi_quantel_mirage_page_curl_perspective_rotation_shear_center_light_depth_warp(canvas, WIDTH, HEIGHT, canvas_b, WIDTH, HEIGHT, 45.0f, 0.4f * sinf(t), 1.0f + 0.05f * sinf(t), 0.04f, t * 0.08f, 0.04f * sinf(t), 0.0f, center_x, center_y, 1.0f, 1.0f, 0.05f * cosf(t));
        }

        // Harry Wipe Transitions with Matte mask
        int transition_stage = ((int)t) % 12;
        if (transition_stage == 0) {
            float wipe_p = fmodf(t, 1.0f);
            uint32_t *mat = calloc(WIDTH * HEIGHT, sizeof(uint32_t));
            uint8_t *matte_mask = malloc(WIDTH * HEIGHT);
            for (int y = 0; y < HEIGHT; y++) {
                for (int x = 0; x < WIDTH; x++) {
                    mat[y * WIDTH + x] = ((x / 32 + y / 32) % 2 == 0) ? 0xFF3E2723 : 0xFF4E342E;
                    matte_mask[y * WIDTH + x] = (x > WIDTH / 2) ? 255 : 128;
                }
            }
            tsfi_quantel_harry_blend_fields_color_offset_vertical_scale_aspect_rotation_center_mirror_matte(canvas_b, mat, dst_buffer, WIDTH, HEIGHT, wipe_p, 4, 0xFFFF8C00, 2.0f * sinf(t), 1.0f + 0.03f * cosf(t), 1.0f + 0.01f * sinf(t), t * 0.03f, center_x, center_y, 0, 0, matte_mask);
            memcpy(canvas_b, dst_buffer, WIDTH * HEIGHT * sizeof(uint32_t));
            free(mat);
            free(matte_mask);

            // Double outline highlights with inner bevel overlay
            tsfi_quantel_storyboard_border_highlights_concentric_double_outer_width_offset_color_texture_bevel(canvas_b, WIDTH, HEIGHT, 32, 120, WIDTH - 64, HEIGHT - 240, 4, 2, 3, 0xFFFF8C00, 0xFFFF4500, 10, 2, (int)(2.0f * sinf(t)), (int)(2.0f * cosf(t)), 0xFF000000, 0.2f, 1);
        }

        // Dispatch CICS inputs to HOGAN
        if (f % 30 == 0) {
            float paint_val = sqrtf(stripe_x * stripe_x + stripe_y * stripe_y) * 0.4f;
            float mirage_val = 30.0f * (1.0f + 0.1f * sinf(t));
            float harry_val = (1.0f + 0.01f * sinf(t)) * 90.0f;

            tsfi_hogan_dispatch_tx(&hogan_sys, 999, 1001, (uint64_t)paint_val, VM_RAMAC);
            tsfi_hogan_dispatch_tx(&hogan_sys, 999, 2002, (uint64_t)mirage_val, VM_EVM);
            tsfi_hogan_dispatch_tx(&hogan_sys, 999, 3003, (uint64_t)harry_val, VM_ZMM);
        }
        if (f % 150 == 0) {
            tsfi_hogan_overnight_reconciliation(&hogan_sys, "demo_tiger_lfs.dat.bin");
        }

        // Enforce Super8 crop gate & sprocket holes
        apply_super8_crop(canvas_b, WIDTH, HEIGHT, f);

        // HOGAN telemetry layout overlay
        draw_hogan_telemetry(canvas_b, WIDTH, HEIGHT, &hogan_sys, t);

        // Convert to RGB24 for FFmpeg pipe output
        for (int i = 0; i < WIDTH * HEIGHT; i++) {
            uint32_t pix = canvas_b[i];
            rgb_out[i * 3 + 0] = (pix >> 16) & 0xFF;
            rgb_out[i * 3 + 1] = (pix >> 8) & 0xFF;
            rgb_out[i * 3 + 2] = pix & 0xFF;
        }

        fwrite(rgb_out, 1, WIDTH * HEIGHT * 3, ffmpeg_pipe);

        if (f % 150 == 0) {
            printf("  -> Eye of the Tiger loop: Rendered %d/%d frames (%.1f%%)...\n", f, FRAMES, (float)f / FRAMES * 100.0f);
        }
    }

    pclose(ffmpeg_pipe);
    free(canvas);
    free(canvas_b);
    free(dst_buffer);
    free(rgb_out);
    remove(audio_file);

    printf("\n[SUCCESS] 90-Second Eye of the Tiger demoscene rendered to bin/tiger_super8_demo.mp4\n");
}
