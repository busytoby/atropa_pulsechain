#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "tsfi_nadler_skeletonizer.h"
#include "tsfi_continuous_zero_power.h"
#include "tsfi_lowpower_mode.h"
#include "tsfi_lowpower_fet.h"
#include "tsfi_defcon_power_alarm.h"

#define DEMO_WIDTH 512
#define DEMO_HEIGHT 512
#define TOTAL_FRAMES 90

int main(void) {
    printf("====================================================================\n");
    printf("   MORTON NADLER LOW-POWER SUPER8 DEMO MP4 GENERATOR                \n");
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

    printf("[INFO] Rendering %d Super8 demoscene frames for MP4 encoder...\n", TOTAL_FRAMES);

    for (int frame = 0; frame < TOTAL_FRAMES; frame++) {
        memset(bitmap, 0, DEMO_WIDTH * DEMO_HEIGHT);
        float phase = (float)frame / 15.0f;

        // Draw animated Super8 character stroke ('A' morphing width)
        int stroke_w = 12 + (int)(6.0f * sinf(phase));
        for (int y = 100; y < 400; y++) {
            for (int w = -stroke_w; w <= stroke_w; w++) {
                int x1 = 256 - (y - 100) / 2 + w;
                if (x1 >= 0 && x1 < DEMO_WIDTH) bitmap[y * DEMO_WIDTH + x1] = 255;
                int x2 = 256 + (y - 100) / 2 + w;
                if (x2 >= 0 && x2 < DEMO_WIDTH) bitmap[y * DEMO_WIDTH + x2] = 255;
            }
        }
        for (int x = 186; x <= 326; x++) {
            for (int dy = -8; dy <= 8; dy++) {
                int y = 250 + dy;
                bitmap[y * DEMO_WIDTH + x] = 255;
            }
        }

        // Run Morton Nadler Topological Character Skeletonizer
        tsfi_nadler_skeleton_summary_t skel_summary;
        tsfi_nadler_skeletonize_bitmap(bitmap, DEMO_WIDTH, DEMO_HEIGHT, &skel_summary);

        // Render Super8 Warm Yellow/Amber Demoscene Overlay
        for (int i = 0; i < DEMO_WIDTH * DEMO_HEIGHT; i++) {
            uint8_t orig = bitmap[i];
            if (orig > 0) {
                rgb_frame[i * 3 + 0] = 240; // Warm Red
                rgb_frame[i * 3 + 1] = 180; // Amber Green
                rgb_frame[i * 3 + 2] = 40;  // Golden Blue
            } else {
                // Dark Super8 Vignette Background
                rgb_frame[i * 3 + 0] = 15;
                rgb_frame[i * 3 + 1] = 12;
                rgb_frame[i * 3 + 2] = 20;
            }
        }

        // Write PPM Frame
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

    printf("[INFO] Super8 PPM frames rendered to /tmp/nadler_super8_*.ppm\n");
    return 0;
}
