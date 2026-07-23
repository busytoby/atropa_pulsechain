#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_nadler_skeletonizer.h"
#include "tsfi_lowpower_fet.h"

// Zhang-Suen thinning iteration helper
static int zhang_suen_iteration(uint8_t *bitmap, int width, int height, int step) {
    uint8_t *temp = malloc(width * height);
    if (!temp) return 0;
    memcpy(temp, bitmap, width * height);
    int changed = 0;

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            if (bitmap[y * width + x] == 0) continue;

            // Neighbors mapping:
            // p9 p2 p3
            // p8 p1 p4
            // p7 p6 p5
            uint8_t p2 = bitmap[(y - 1) * width + x] > 128 ? 1 : 0;
            uint8_t p3 = bitmap[(y - 1) * width + (x + 1)] > 128 ? 1 : 0;
            uint8_t p4 = bitmap[y * width + (x + 1)] > 128 ? 1 : 0;
            uint8_t p5 = bitmap[(y + 1) * width + (x + 1)] > 128 ? 1 : 0;
            uint8_t p6 = bitmap[(y + 1) * width + x] > 128 ? 1 : 0;
            uint8_t p7 = bitmap[(y + 1) * width + (x - 1)] > 128 ? 1 : 0;
            uint8_t p8 = bitmap[y * width + (x - 1)] > 128 ? 1 : 0;
            uint8_t p9 = bitmap[(y - 1) * width + (x - 1)] > 128 ? 1 : 0;

            int B = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
            
            // Count transitions A (0-to-1 transitions in sequence p2 -> p3 -> p4 -> p5 -> p6 -> p7 -> p8 -> p9 -> p2)
            int A = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) + 
                    (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) + 
                    (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) + 
                    (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);

            if (B >= 2 && B <= 6 && A == 1) {
                int cond1 = (step == 1) ? (p2 * p4 * p6 == 0) : (p2 * p4 * p8 == 0);
                int cond2 = (step == 1) ? (p4 * p6 * p8 == 0) : (p2 * p6 * p8 == 0);
                if (cond1 && cond2) {
                    temp[y * width + x] = 0;
                    changed = 1;
                }
            }
        }
    }

    if (changed) {
        memcpy(bitmap, temp, width * height);
    }
    free(temp);
    return changed;
}

int tsfi_nadler_skeletonize_bitmap(
    const uint8_t *bitmap,
    int width,
    int height,
    tsfi_nadler_skeleton_summary_t *out_summary
) {
    if (!out_summary) return -1;
    memset(out_summary, 0, sizeof(tsfi_nadler_skeleton_summary_t));

    if (!bitmap || width <= 0 || height <= 0) {
        return -1;
    }

    // Allocate local work buffer
    uint8_t *work_map = malloc(width * height);
    if (!work_map) return -1;
    memcpy(work_map, bitmap, width * height);

    // Iterative Zhang-Suen thinning loops
    int changed = 1;
    int iterations = 0;
    while (changed && iterations < 32) {
        changed = 0;
        changed |= zhang_suen_iteration(work_map, width, height, 1);
        changed |= zhang_suen_iteration(work_map, width, height, 2);
        iterations++;
    }

    // Single-pass endpoint & junction extraction on the thinned skeleton
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            if (work_map[y * width + x] > 128) {
                int neighbors = 0;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        if (work_map[(y + dy) * width + (x + dx)] > 128) {
                            neighbors++;
                        }
                    }
                }
                if (neighbors == 1) {
                    out_summary->endpoints_count++;
                } else if (neighbors >= 3) {
                    out_summary->junctions_count++;
                }
                out_summary->total_skeleton_pixels++;
            }
        }
    }

    free(work_map);
    out_summary->evm_gas_cost = 1500;

    // FET Soft-Body Discharge Physics (Rule 10)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    out_summary->fet_power_watts = (double)fet_metrics.optimized_power_watts;

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(out_summary->tape_dat_bin, sizeof(out_summary->tape_dat_bin),
             "NADLER_SKELETON_%08X.DAT.BIN", (unsigned int)out_summary->total_skeleton_pixels);

    return 0; // Skeletonization complete
}
