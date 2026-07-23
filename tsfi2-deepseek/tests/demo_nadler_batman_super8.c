#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "tsfi_nadler_skeletonizer.h"
#include "tsfi_nadler_syntactic_parser.h"
#include "tsfi_continuous_zero_power.h"
#include "tsfi_lowpower_mode.h"
#include "tsfi_lowpower_fet.h"
#include "tsfi_defcon_power_alarm.h"
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

#define DEMO_WIDTH 256
#define DEMO_HEIGHT 256
#define TOTAL_FRAMES 120

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper to write Batman retro chiptune arpeggios mono WAV file (brooding D-minor rhythm)
static void write_wav_soundtrack(const char *filename, uint32_t *freqs, int frame_count) {
    FILE *f = fopen(filename, "wb");
    if (!f) return;

    // WAV Header
    fwrite("RIFF", 1, 4, f);
    uint32_t file_size = 36 + frame_count * 1470 * 2;
    fwrite(&file_size, 4, 1, f);
    fwrite("WAVEfmt ", 1, 8, f);
    uint32_t fmt_chunk_sz = 16;
    fwrite(&fmt_chunk_sz, 4, 1, f);
    uint16_t audio_fmt = 1; // PCM
    fwrite(&audio_fmt, 2, 1, f);
    uint16_t num_channels = 1; // Mono
    fwrite(&num_channels, 2, 1, f);
    uint32_t sample_rate = 44100;
    fwrite(&sample_rate, 4, 1, f);
    uint32_t byte_rate = 44100 * 2;
    fwrite(&byte_rate, 4, 1, f);
    uint16_t block_align = 2;
    fwrite(&block_align, 2, 1, f);
    uint16_t bits_per_sample = 16;
    fwrite(&bits_per_sample, 2, 1, f);
    fwrite("data", 1, 4, f);
    uint32_t data_sz = frame_count * 1470 * 2;
    fwrite(&data_sz, 4, 1, f);

    double phase = 0.0;
    for (int frame = 0; frame < frame_count; frame++) {
        double freq = freqs[frame];
        for (int i = 0; i < 1470; i++) {
            // Brooding synth square wave
            int16_t val = (sin(phase) > 0) ? 7000 : -7000;
            fwrite(&val, 2, 1, f);
            phase += 2.0 * M_PI * freq / 44100.0;
            if (phase > 2.0 * M_PI) phase -= 2.0 * M_PI;
        }
    }
    fclose(f);
}

// Draw filled ellipse helper for Bat-Signal glow
static void draw_filled_ellipse(uint8_t *bitmap, int w, int h, int cx, int cy, int rx, int ry, uint8_t color) {
    for (int y = cy - ry; y <= cy + ry; y++) {
        for (int x = cx - rx; x <= cx + rx; x++) {
            if (x >= 0 && x < w && y >= 0 && y < h) {
                float dx = (float)(x - cx) / rx;
                float dy = (float)(y - cy) / ry;
                if (dx*dx + dy*dy <= 1.0f) {
                    bitmap[y * w + x] = color;
                }
            }
        }
    }
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("[INFO] Current working directory: %s\n", cwd);
    }
    printf("====================================================================\n");
    printf("   MORTON NADLER GOTHAM SCROLLING BATMAN SUPER8 DEMO GENERATOR      \n");
    printf("====================================================================\n");

    // Initialize ZMM VM & load hucSystem contract
    tsfi_wire_firmware_init();
    // TsfiZmmVmState vm;
    // tsfi_zmm_vm_init(&vm);
    // tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    uint8_t *bitmap = calloc(DEMO_WIDTH * DEMO_HEIGHT, 1);
    uint8_t *rgb_frame = malloc(DEMO_WIDTH * DEMO_HEIGHT * 3);
    uint32_t *soundtrack_freqs = malloc(TOTAL_FRAMES * sizeof(uint32_t));

    if (!bitmap || !rgb_frame || !soundtrack_freqs) {
        fprintf(stderr, "Failed to allocate demo frame buffers.\n");
        return 1;
    }

    int target_h = (int)(DEMO_WIDTH / 1.85f);
    int border_y = (DEMO_HEIGHT - target_h) / 2;

    printf("[INFO] Rendering %d Super8 Gotham Skyline frames...\n", TOTAL_FRAMES);

    for (int frame = 0; frame < TOTAL_FRAMES; frame++) {
        memset(bitmap, 0, DEMO_WIDTH * DEMO_HEIGHT);

        // --- 1. RENDER VECTOR BAT-SIGNAL & BAT LOGO SILHOUETTE ---
        int sig_cx = 128 + (int)(25.0f * sinf((float)frame * 0.04f));
        int sig_cy = 100 + (int)(15.0f * cosf((float)frame * 0.03f));
        int rx = 40, ry = 24;

        // Draw Bat-Signal Yellow Glow Ellipse
        draw_filled_ellipse(bitmap, DEMO_WIDTH, DEMO_HEIGHT, sig_cx, sig_cy, rx, ry, 255);

        // Subtract/draw black Bat silhouette inside the yellow ellipse
        // Simple wing coordinates relative to bat center
        for (int y_rel = -ry + 8; y_rel < ry - 8; y_rel++) {
            for (int x_rel = -rx + 10; x_rel < rx - 10; x_rel++) {
                int px = sig_cx + x_rel;
                int py = sig_cy + y_rel;
                if (px >= 0 && px < DEMO_WIDTH && py >= 0 && py < DEMO_HEIGHT) {
                    float norm_x = (float)x_rel / rx;
                    float norm_y = (float)y_rel / ry;

                    // Bat mathematical contour mapping
                    bool in_bat = false;
                    float ax = fabsf(norm_x);
                    if (ax < 0.15f && norm_y > 0.0f && norm_y < 0.5f) {
                        // Head / Ears
                        in_bat = (ax > 0.05f || norm_y < 0.4f);
                    } else if (ax >= 0.15f && ax < 0.8f) {
                        // Wings curve body
                        float wing_top = 0.35f - 0.2f * cosf(ax * M_PI * 1.5f);
                        float wing_bottom = -0.5f + 0.3f * sinf(ax * M_PI);
                        in_bat = (norm_y >= wing_bottom && norm_y <= wing_top);
                    } else if (ax >= 0.8f) {
                        // Wing Tips
                        in_bat = (norm_y >= -0.2f && norm_y <= 0.1f);
                    }

                    if (in_bat) {
                        bitmap[py * DEMO_WIDTH + px] = 0; // Black Bat Silhouette
                    }
                }
            }
        }

        // Run Morton Nadler Topological Character Skeletonizer once to initialize metric checks
        if (frame == 0) {
            tsfi_nadler_skeleton_summary_t skel_summary;
            tsfi_nadler_skeletonize_bitmap(bitmap, DEMO_WIDTH, DEMO_HEIGHT, &skel_summary);
        }

        // Fetch Ocean Loader border color
        uint32_t expected_colors[3] = {255, 65535, 16711680}; // Red, Cyan, Blue
        uint32_t border_color = expected_colors[frame % 3];
        
        // Chiptune soundtrack: Brooding Batman Theme (D -> D# -> D -> C# bass loop)
        uint32_t base_freqs[4] = {147, 155, 147, 138}; // D3, D#3, D3, C#3
        uint32_t note_freq = base_freqs[(frame / 15) % 4];
        soundtrack_freqs[frame] = note_freq;

        uint32_t scsi_seed = (uint32_t)frame * 9876543ULL;

        uint8_t br = 0, bg = 0, bb = 0;
        if (border_color == 255) { br = 200; bg = 10; bb = 10; }
        else if (border_color == 65535) { br = 10; bg = 180; bb = 180; }
        else if (border_color == 16711680) { br = 10; bg = 10; bb = 200; }

        srand(scsi_seed);

        float bob = sinf((float)frame * 0.12f) * 1.5f;

        // Render viewport with Gotham skyline layers (Parallax Scroll)
        for (int y = 0; y < DEMO_HEIGHT; y++) {
            bool in_active_crop = (y >= border_y && y < DEMO_HEIGHT - border_y);
            bool in_sprocket = false;

            if (in_active_crop) {
                int sy_rel = (y - border_y) % 64;
                int active_sy = 32 + (int)bob;
                if (abs(sy_rel - active_sy) <= 12) {
                    in_sprocket = true;
                }
            }

            for (int x = 0; x < DEMO_WIDTH; x++) {
                int pixel_idx = y * DEMO_WIDTH + x;

                if (!in_active_crop) {
                    // Flashing tape loader borders
                    rgb_frame[pixel_idx * 3 + 0] = br;
                    rgb_frame[pixel_idx * 3 + 1] = bg;
                    rgb_frame[pixel_idx * 3 + 2] = bb;
                } else if (in_sprocket && (x >= 12 && x <= 28)) {
                    rgb_frame[pixel_idx * 3 + 0] = 8;
                    rgb_frame[pixel_idx * 3 + 1] = 8;
                    rgb_frame[pixel_idx * 3 + 2] = 8;
                } else {
                    // Inside Viewport: Multi-layered Gotham Skyline Scroll
                    int relative_y = y - border_y;

                    // Feature 1: Scroll Offsets (Parallax Scrolling)
                    int sky_scroll = (x + frame * 1) % DEMO_WIDTH;
                    int mid_scroll = (x + frame * 2) % DEMO_WIDTH;
                    int fore_scroll = (x + frame * 4) % DEMO_WIDTH;

                    // Feature 2: Sky Gradient (VCE palette emulation)
                    float sky_grad = (float)relative_y / target_h;
                    int sky_r = 10 + (int)(15.0f * (1.0f - sky_grad));
                    int sky_g = 8 + (int)(10.0f * (1.0f - sky_grad));
                    int sky_b = 24 + (int)(25.0f * sky_grad);

                    int pixel_r = sky_r;
                    int pixel_g = sky_g;
                    int pixel_b = sky_b;

                    // Distant skyline buildings (Slow scroll)
                    int dist_building_id = sky_scroll / 64;
                    int dist_building_h = 60 + (dist_building_id % 3) * 25;
                    if (relative_y >= target_h - dist_building_h) {
                        pixel_r = 15; pixel_g = 12; pixel_b = 28;
                    }

                    // Midground skyline (Cathedral, spires, gargoyles - Medium scroll)
                    int mid_building_id = mid_scroll / 48;
                    int mid_building_h = 100 + (mid_building_id % 5) * 20;
                    int offset_in_building = mid_scroll % 48;
                    if (offset_in_building > 20 && offset_in_building < 28) {
                        mid_building_h += 30;
                    }
                    if (relative_y >= target_h - mid_building_h) {
                        pixel_r = 10; pixel_g = 8; pixel_b = 18;
                    }

                    // Foreground skyline (Rooftops, fire escapes, water towers - Fast scroll)
                    int fore_building_id = fore_scroll / 80;
                    int fore_building_h = 140 + (fore_building_id % 2) * 30;
                    bool on_water_tower = false;
                    int fore_offset = fore_scroll % 80;
                    if (fore_offset > 30 && fore_offset < 50) {
                        if (relative_y >= target_h - fore_building_h - 25 && relative_y < target_h - fore_building_h) {
                            on_water_tower = true;
                        }
                    }
                    if (relative_y >= target_h - fore_building_h || on_water_tower) {
                        pixel_r = 5; pixel_g = 5; pixel_b = 10;
                    }

                    // Overlay Yellow Bat-Signal projected on sky / buildings
                    uint8_t sig_val = bitmap[pixel_idx];
                    if (sig_val > 0) {
                        pixel_r = 245;
                        pixel_g = 190;
                        pixel_b = 30;
                    }

                    uint32_t ns = (uint32_t)(y * 12345 + x * 67890 + frame * 98765);
                    ns ^= ns << 13; ns ^= ns >> 17; ns ^= ns << 5;
                    float noise = ((float)(ns % 100) / 100.0f - 0.5f) * 12.0f;
                    int r = pixel_r + (int)noise;
                    int g = pixel_g + (int)noise;
                    int b = pixel_b + (int)noise;

                    if (r < 0) r = 0;
                    if (r > 255) r = 255;
                    if (g < 0) g = 0;
                    if (g > 255) g = 255;
                    if (b < 0) b = 0;
                    if (b > 255) b = 255;

                    rgb_frame[pixel_idx * 3 + 0] = r;
                    rgb_frame[pixel_idx * 3 + 1] = g;
                    rgb_frame[pixel_idx * 3 + 2] = b;
                }
            }
        }

        // Draw SCSI film scratches
        if (rand() % 14 == 0) {
            int sx = rand() % DEMO_WIDTH;
            int sy = border_y + (rand() % target_h);
            int len = rand() % 40 + 10;
            for (int i = 0; i < len; i++) {
                int px = sx;
                int py = sy + i;
                if (px >= 0 && px < DEMO_WIDTH && py >= border_y && py < DEMO_HEIGHT - border_y) {
                    rgb_frame[(py * DEMO_WIDTH + px) * 3 + 0] = 5;
                    rgb_frame[(py * DEMO_WIDTH + px) * 3 + 1] = 5;
                    rgb_frame[(py * DEMO_WIDTH + px) * 3 + 2] = 5;
                }
            }
        }

        // Save PPM frame to local workspace batman_frames folder
        char ppm_path[256];
        snprintf(ppm_path, sizeof(ppm_path), "batman_frames/frame_%04d.ppm", frame);
        FILE *fp = fopen(ppm_path, "wb");
        if (fp) {
            fprintf(fp, "P6\n%d %d\n255\n", DEMO_WIDTH, DEMO_HEIGHT);
            fwrite(rgb_frame, 1, DEMO_WIDTH * DEMO_HEIGHT * 3, fp);
            fclose(fp);
        } else {
            perror("fopen ppm failed");
            fprintf(stderr, "Failed path: %s\n", ppm_path);
            return 1;
        }
    }

    // Write WAV soundtrack to local workspace batman_frames folder
    const char *wav_path = "batman_frames/chiptune.wav";
    write_wav_soundtrack(wav_path, soundtrack_freqs, TOTAL_FRAMES);

    free(bitmap);
    free(rgb_frame);
    free(soundtrack_freqs);
    // tsfi_zmm_vm_destroy(&vm);

    printf("[INFO] AUNCIENT Super8 Batman Gotham demo successfully generated.\n");
    _exit(0);
}
