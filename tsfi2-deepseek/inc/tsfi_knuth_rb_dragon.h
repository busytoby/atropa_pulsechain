#ifndef TSFI_KNUTH_RB_DRAGON_H
#define TSFI_KNUTH_RB_DRAGON_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    DRAGON_COLOR_RED   = 0, // Even Parity Digits (0, 2) -> Real Axis Shift
    DRAGON_COLOR_BLACK = 1  // Odd Parity Digits (1, 3)  -> Imaginary Axis Shift
} tsfi_dragon_color_t;

typedef struct {
    uint64_t node_address;
    tsfi_dragon_color_t color;
    uint32_t black_height;
    char dat_bin_filename[128]; // Enforces Rule 13 (.DAT.BIN)
} tsfi_knuth_rb_dragon_node_t;

/* Classify Knuth Base 2i Quadtree Node as Red or Black based on digit parity */
int tsfi_knuth_rb_dragon_classify(
    uint64_t base2i_word,
    tsfi_knuth_rb_dragon_node_t *out_node
);

#endif // TSFI_KNUTH_RB_DRAGON_H
