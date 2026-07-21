#ifndef TSFI_NADLER_SKELETONIZER_H
#define TSFI_NADLER_SKELETONIZER_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t total_skeleton_pixels; // Count of 1-pixel wide skeleton points
    uint32_t endpoints_count;        // Count of Degree-1 endpoints
    uint32_t junctions_count;        // Count of Degree-3 T-junctions
    uint32_t evm_gas_cost;           // 1,500 Gas Cost per Skeleton Graph
    double fet_power_watts;          // Power in Watts under 78.2% FET Drop
    char tape_dat_bin[128];          // Rule 13 Filename (.DAT.BIN)
} tsfi_nadler_skeleton_summary_t;

/* Execute Morton Nadler topological skeletonization & thinning over bitmap */
int tsfi_nadler_skeletonize_bitmap(
    const uint8_t *bitmap,
    int width,
    int height,
    tsfi_nadler_skeleton_summary_t *out_summary
);

#endif // TSFI_NADLER_SKELETONIZER_H
