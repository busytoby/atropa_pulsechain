#ifndef TSFI_NADLER_OPTIMIZER_H
#define TSFI_NADLER_OPTIMIZER_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint64_t nadler_z_index;      // Morton Nadler Bit-Interleaved Z-Order Index
    double fet_switching_reduction_pct; // Percentage Reduction in Bit Toggles
    uint32_t evm_gas_consumed;     // EVM Gas Cost (5,000 gas vs 40,000 standard)
    char nadler_dat_bin[128];     // Rule 13 Filename (.DAT.BIN)
} tsfi_nadler_optimizer_t;

/* Initialize and execute Morton Nadler Positional Interleaving (Z-Order) */
int tsfi_nadler_interleave_2d(
    uint32_t x,
    uint32_t y,
    tsfi_nadler_optimizer_t *out_opt
);

#endif // TSFI_NADLER_OPTIMIZER_H
