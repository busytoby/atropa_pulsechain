#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_knuth_rb_dragon.h"

int tsfi_knuth_rb_dragon_classify(
    uint64_t base2i_word,
    tsfi_knuth_rb_dragon_node_t *out_node
) {
    if (!out_node) return -1;
    memset(out_node, 0, sizeof(tsfi_knuth_rb_dragon_node_t));
    out_node->node_address = base2i_word;

    // 1. Inspect lowest digit parity to assign Red / Black color
    uint8_t lowest_digit = base2i_word & 3;
    out_node->color = (lowest_digit % 2 == 0) ? DRAGON_COLOR_RED : DRAGON_COLOR_BLACK;

    // 2. Count Black-height (number of odd-parity digits across the word)
    out_node->black_height = 0;
    for (int k = 0; k < 16; k++) {
        uint8_t d = (base2i_word >> (k * 4)) & 3;
        if (d % 2 != 0) {
            out_node->black_height++;
        }
    }

    // 3. Format .DAT.BIN filename enforcing Rule 13
    snprintf(out_node->dat_bin_filename, sizeof(out_node->dat_bin_filename),
             "RB_DRAGON_%s_BH%u_%016lX.DAT.BIN",
             (out_node->color == DRAGON_COLOR_RED) ? "RED" : "BLACK",
             out_node->black_height,
             (unsigned long)base2i_word);

    printf("[RED-BLACK DRAGON] Node 0x%016lX -> Color: %s | Black-Height: %u (%s)\n",
           (unsigned long)base2i_word,
           (out_node->color == DRAGON_COLOR_RED) ? "RED" : "BLACK",
           out_node->black_height,
           out_node->dat_bin_filename);

    return 0; // Classification complete
}
