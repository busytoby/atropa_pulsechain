#ifndef TSFI_NADLER_HULL_AUTOMATON_H
#define TSFI_NADLER_HULL_AUTOMATON_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint64_t nadler_convex_hull_word; // Packed 4-Direction Convex Boundary Mask
    uint32_t feature_matches_count;   // Sequential-Scan Feature Matches
    uint32_t evm_gas_cost;            // 2,500 Gas Cost per Convex Mask Slot
    char tape_dat_bin[128];           // Rule 13 Filename (.DAT.BIN)
} tsfi_nadler_hull_summary_t;

/* Encode 2D Vector Bounding Box into 64-bit Nadler Convex Hull Word */
uint64_t tsfi_nadler_encode_convex_hull(
    uint16_t x_min,
    uint16_t y_min,
    uint16_t x_max,
    uint16_t y_max
);

/* Execute Nadler Sequential-Scan Cellular Automaton over Raster Buffer */
int tsfi_nadler_run_sequential_automaton(
    const uint8_t *raster_buffer,
    size_t length,
    tsfi_nadler_hull_summary_t *out_summary
);

#endif // TSFI_NADLER_HULL_AUTOMATON_H
