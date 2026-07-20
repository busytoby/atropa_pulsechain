#include "tsfi_parc_bitblt.h"

int tsfi_parc_bitblt_transfer(tsfi_parc_bitblt_t *bb, int dest_x, int dest_y, int src_x, int src_y, int w, int h, BitBltOp op) {
    if (!bb || !bb->dest_bits || w <= 0 || h <= 0) return -1;

    for (int dy = 0; dy < h; dy++) {
        int cur_dest_y = dest_y + dy;
        if (cur_dest_y < 0 || cur_dest_y >= bb->dest_height) continue;
        int cur_src_y = src_y + dy;

        for (int dx = 0; dx < w; dx++) {
            int cur_dest_x = dest_x + dx;
            if (cur_dest_x < 0 || cur_dest_x >= bb->dest_width) continue;
            int cur_src_x = src_x + dx;

            uint32_t src_val = 0xFFFFFFFF; // Default color if no source image
            if (bb->src_bits && cur_src_y >= 0 && cur_src_y < bb->src_height && cur_src_x >= 0 && cur_src_x < bb->src_width) {
                src_val = bb->src_bits[cur_src_y * bb->src_width + cur_src_x];
            }

            // Apply halftone pattern mask (16x16 pixels template tiling)
            if (bb->has_halftone) {
                uint16_t row = bb->halftone_pattern[cur_dest_y % 16];
                int bit = (row >> (cur_dest_x % 16)) & 1;
                if (!bit) {
                    src_val = 0; // Mask out color if pattern bit is zero
                }
            }

            int dest_idx = cur_dest_y * bb->dest_width + cur_dest_x;
            uint32_t dest_val = bb->dest_bits[dest_idx];

            uint32_t out_val = src_val;
            switch (op) {
                case BITBLT_COPY:
                    out_val = src_val;
                    break;
                case BITBLT_OR:
                    out_val = dest_val | src_val;
                    break;
                case BITBLT_AND:
                    out_val = dest_val & src_val;
                    break;
                case BITBLT_XOR:
                    out_val = dest_val ^ src_val;
                    break;
                case BITBLT_PAINT:
                    // Transparent copy: only copy if source is not dark/black
                    if ((src_val & 0x00FFFFFF) != 0) {
                        out_val = src_val;
                    } else {
                        out_val = dest_val;
                    }
                    break;
            }
            bb->dest_bits[dest_idx] = out_val;
        }
    }
    return 0;
}
