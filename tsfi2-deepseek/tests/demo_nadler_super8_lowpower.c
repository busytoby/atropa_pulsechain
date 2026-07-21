#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "tsfi_nadler_skeletonizer.h"
#include "tsfi_nadler_syntactic_parser.h"
#include "tsfi_continuous_zero_power.h"
#include "tsfi_lowpower_mode.h"
#include "tsfi_lowpower_fet.h"
#include "tsfi_defcon_power_alarm.h"

#define DEMO_WIDTH 512
#define DEMO_HEIGHT 512
#define TOTAL_FRAMES 120

int main(void) {
    printf("====================================================================\n");
    printf("   MORTON NADLER MULTI-CHARACTER SUPER8 DEMO MP4 GENERATOR           \n");
    printf("====================================================================\n");

    // 1. Initialize Power Sentinel
    tsfi_continuous_power_status_t p_status;
    tsfi_continuous_power_init(&p_status);
    tsfi_continuous_power_enforce_lowest(&p_status);

    tsfi_defcon_alarm_status_t defcon_status;
    tsfi_defcon_power_alarm_eval(p_status.active_power_watts, &defcon_status);

    uint8_t *bitmap = calloc(DEMO_WIDTH * DEMO_HEIGHT, 1);
    uint8_t *rgb_frame = malloc(DEMO_WIDTH * DEMO_HEIGHT * 3);
    if (!bitmap || !rgb_frame) {
        fprintf(stderr, "Failed to allocate demo frame buffers.\n");
        return 1;
    }

    printf("[INFO] Rendering %d Super8 multi-character demoscene frames...\n", TOTAL_FRAMES);

    for (int frame = 0; frame < TOTAL_FRAMES; frame++) {
        memset(bitmap, 0, DEMO_WIDTH * DEMO_HEIGHT);
        int char_phase = (frame / 30) % 4; // Cycle through 'A', 'B', 'R', '8'

        if (char_phase == 0) {
            // Draw 'A'
            for (int y = 100; y < 400; y++) {
                for (int w = -10; w <= 10; w++) {
                    int x1 = 256 - (y - 100) / 2 + w;
                    if (x1 >= 0 && x1 < DEMO_WIDTH) bitmap[y * DEMO_WIDTH + x1] = 255;
                    int x2 = 256 + (y - 100) / 2 + w;
                    if (x2 >= 0 && x2 < DEMO_WIDTH) bitmap[y * DEMO_WIDTH + x2] = 255;
                }
            }
            for (int x = 186; x <= 326; x++) {
                for (int dy = -8; dy <= 8; dy++) {
                    bitmap[(250 + dy) * DEMO_WIDTH + x] = 255;
                }
            }
        } else if (char_phase == 1) {
            // Draw 'B'
            for (int y = 100; y < 400; y++) {
                for (int w = 0; w <= 20; w++) {
                    bitmap[y * DEMO_WIDTH + (150 + w)] = 255;
                }
            }
            // Upper and lower loops
            for (int theta = 0; theta < 180; theta++) {
                float rad = (float)theta * 3.14159f / 180.0f;
                int x1 = 160 + (int)(100.0f * sinf(rad));
                int y1 = 175 - (int)(75.0f * cosf(rad));
                int y2 = 325 - (int)(75.0f * cosf(rad));
                if (x1 >= 0 && x1 < DEMO_WIDTH && y1 >= 0 && y1 < DEMO_HEIGHT) bitmap[y1 * DEMO_WIDTH + x1] = 255;
                if (x1 >= 0 && x1 < DEMO_WIDTH && y2 >= 0 && y2 < DEMO_HEIGHT) bitmap[y2 * DEMO_WIDTH + x1] = 255;
            }
        } else if (char_phase == 2) {
            // Draw 'R'
            for (int y = 100; y < 400; y++) {
                for (int w = 0; w <= 20; w++) {
                    bitmap[y * DEMO_WIDTH + (150 + w)] = 255;
                }
            }
            for (int theta = 0; theta < 180; theta++) {
                float rad = (float)theta * 3.14159f / 180.0f;
                int x1 = 160 + (int)(100.0f * sinf(rad));
                int y1 = 175 - (int)(75.0f * cosf(rad));
                if (x1 >= 0 && x1 < DEMO_WIDTH && y1 >= 0 && y1 < DEMO_HEIGHT) bitmap[y1 * DEMO_WIDTH + x1] = 255;
            }
            // Right leg diagonal
            for (int i = 0; i < 150; i++) {
                int x = 170 + i;
                int y = 250 + i;
                if (x < DEMO_WIDTH && y < DEMO_HEIGHT) bitmap[y * DEMO_WIDTH + x] = 255;
            }
        } else {
            // Draw '8'
            for (int theta = 0; theta < 360; theta++) {
                float rad = (float)theta * 3.14159f / 180.0f;
                int x1 = 256 + (int)(80.0f * sinf(rad));
                int y1 = 180 - (int)(70.0f * cosf(rad));
                int y2 = 330 - (int)(70.0f * cosf(rad));
                if (x1 >= 0 && x1 < DEMO_WIDTH && y1 >= 0 && y1 < DEMO_HEIGHT) bitmap[y1 * DEMO_WIDTH + x1] = 255;
                if (x1 >= 0 && x1 < DEMO_WIDTH && y2 >= 0 && y2 < DEMO_HEIGHT) bitmap[y2 * DEMO_WIDTH + x1] = 255;
            }
        }

        // Run Morton Nadler Skeletonizer Thinning
        tsfi_nadler_skeleton_summary_t skel_summary;
        tsfi_nadler_skeletonize_bitmap(bitmap, DEMO_WIDTH, DEMO_HEIGHT, &skel_summary);

        // Render Super8 Golden/Amber Demoscene Overlay
        for (int i = 0; i < DEMO_WIDTH * DEMO_HEIGHT; i++) {
            uint8_t orig = bitmap[i];
            if (orig > 0) {
                rgb_frame[i * 3 + 0] = 240;
                rgb_frame[i * 3 + 1] = 180;
                rgb_frame[i * 3 + 2] = 40;
            } else {
                rgb_frame[i * 3 + 0] = 15;
                rgb_frame[i * 3 + 1] = 12;
                rgb_frame[i * 3 + 2] = 20;
            }
        }

        char ppm_path[256];
        snprintf(ppm_path, sizeof(ppm_path), "/tmp/nadler_super8_%04d.ppm", frame);
        FILE *fp = fopen(ppm_path, "wb");
        if (fp) {
            fprintf(fp, "P6\n%d %d\n255\n", DEMO_WIDTH, DEMO_HEIGHT);
            fwrite(rgb_frame, 1, DEMO_WIDTH * DEMO_HEIGHT * 3, fp);
            fclose(fp);
        }
    }

    free(bitmap);
    free(rgb_frame);

    printf("[INFO] Multi-character Super8 PPM frames rendered successfully.\n");
    return 0;
}
