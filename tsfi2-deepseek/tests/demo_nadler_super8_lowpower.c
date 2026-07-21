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
#define TOTAL_FRAMES 240

int main(void) {
    printf("====================================================================\n");
    printf("   MORTON NADLER AUNCIENT CHARACTER SUPER8 DEMO MP4 GENERATOR        \n");
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

    const char *auncient_str = "AUNCIENT";
    size_t str_len = strlen(auncient_str);

    printf("[INFO] Rendering %d Super8 AUNCIENT character demoscene frames...\n", TOTAL_FRAMES);

    for (int frame = 0; frame < TOTAL_FRAMES; frame++) {
        memset(bitmap, 0, DEMO_WIDTH * DEMO_HEIGHT);
        char cur_char = auncient_str[(frame / 30) % str_len];

        if (cur_char == 'A') {
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
        } else if (cur_char == 'U') {
            for (int y = 100; y < 320; y++) {
                for (int w = 0; w <= 20; w++) {
                    bitmap[y * DEMO_WIDTH + (150 + w)] = 255;
                    bitmap[y * DEMO_WIDTH + (342 + w)] = 255;
                }
            }
            for (int theta = 0; theta <= 180; theta++) {
                float rad = (float)theta * 3.14159f / 180.0f;
                int x = 256 - (int)(100.0f * cosf(rad));
                int y = 320 + (int)(75.0f * sinf(rad));
                if (x >= 0 && x < DEMO_WIDTH && y >= 0 && y < DEMO_HEIGHT) bitmap[y * DEMO_WIDTH + x] = 255;
            }
        } else if (cur_char == 'N') {
            for (int y = 100; y < 400; y++) {
                for (int w = 0; w <= 20; w++) {
                    bitmap[y * DEMO_WIDTH + (150 + w)] = 255;
                    bitmap[y * DEMO_WIDTH + (342 + w)] = 255;
                }
            }
            for (int i = 0; i < 200; i++) {
                int x = 160 + i;
                int y = 100 + (i * 300) / 200;
                if (x < DEMO_WIDTH && y < DEMO_HEIGHT) bitmap[y * DEMO_WIDTH + x] = 255;
            }
        } else if (cur_char == 'C') {
            for (int theta = 45; theta <= 315; theta++) {
                float rad = (float)theta * 3.14159f / 180.0f;
                int x = 270 - (int)(110.0f * cosf(rad));
                int y = 250 - (int)(120.0f * sinf(rad));
                if (x >= 0 && x < DEMO_WIDTH && y >= 0 && y < DEMO_HEIGHT) bitmap[y * DEMO_WIDTH + x] = 255;
            }
        } else if (cur_char == 'I') {
            for (int y = 100; y < 400; y++) {
                for (int w = -10; w <= 10; w++) {
                    bitmap[y * DEMO_WIDTH + (256 + w)] = 255;
                }
            }
        } else if (cur_char == 'E') {
            for (int y = 100; y < 400; y++) {
                for (int w = 0; w <= 20; w++) {
                    bitmap[y * DEMO_WIDTH + (150 + w)] = 255;
                }
            }
            for (int x = 150; x <= 330; x++) {
                for (int dy = -8; dy <= 8; dy++) {
                    bitmap[(100 + dy) * DEMO_WIDTH + x] = 255;
                    bitmap[(250 + dy) * DEMO_WIDTH + x] = 255;
                    bitmap[(400 + dy) * DEMO_WIDTH + x] = 255;
                }
            }
        } else if (cur_char == 'T') {
            for (int y = 100; y < 400; y++) {
                for (int w = -10; w <= 10; w++) {
                    bitmap[y * DEMO_WIDTH + (256 + w)] = 255;
                }
            }
            for (int x = 150; x <= 362; x++) {
                for (int dy = -8; dy <= 8; dy++) {
                    bitmap[(100 + dy) * DEMO_WIDTH + x] = 255;
                }
            }
        }

        // Run Morton Nadler Topological Character Skeletonizer
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

    printf("[INFO] AUNCIENT Super8 PPM frames rendered successfully.\n");
    return 0;
}
