#ifndef TSFI_NADLER_CONTOUR_CODE_H
#define TSFI_NADLER_CONTOUR_CODE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint64_t packed_chain_word;  // Packed 3-bit 8-directional codes
    uint32_t total_boundary_pts; // Total contour boundary points
    uint32_t evm_gas_cost;       // 800 Gas Cost per Contour Loop
    double fet_power_watts;      // Power in Watts under 78.2% FET Drop
    char tape_dat_bin[128];      // Rule 13 Filename (.DAT.BIN)
} tsfi_nadler_contour_summary_t;

/* Trace 2D bitmap boundary and pack into Morton Nadler 8-Directional Chain Code */
int tsfi_nadler_encode_contour(
    const uint8_t *bitmap,
    int width,
    int height,
    tsfi_nadler_contour_summary_t *out_summary
);

#endif // TSFI_NADLER_CONTOUR_CODE_H
