#include <stdint.h>
#include "tsfi_parc_rle.h"

int tsfi_parc_rle_compress(const uint8_t *src, int src_len, uint8_t *dest, int max_dest_len) {
    if (!src || !dest || src_len <= 0 || max_dest_len <= 0) return -1;

    int src_idx = 0;
    int dest_idx = 0;

    while (src_idx < src_len) {
        uint8_t val = src[src_idx];
        int run_len = 1;

        // Count sequential matching values (limit to 127 to fit signed byte run-length)
        while (src_idx + run_len < src_len && src[src_idx + run_len] == val && run_len < 127) {
            run_len++;
        }

        if (dest_idx + 2 > max_dest_len) {
            return -2; // destination buffer overflow
        }

        dest[dest_idx++] = (uint8_t)run_len;
        dest[dest_idx++] = val;
        src_idx += run_len;
    }

    return dest_idx;
}

int tsfi_parc_rle_decompress(const uint8_t *src, int src_len, uint8_t *dest, int max_dest_len) {
    if (!src || !dest || src_len <= 0 || max_dest_len <= 0) return -1;

    int src_idx = 0;
    int dest_idx = 0;

    while (src_idx < src_len) {
        int run_len = (int)src[src_idx++];
        if (src_idx >= src_len) {
            return -3; // incomplete RLE stream
        }
        uint8_t val = src[src_idx++];

        if (dest_idx + run_len > max_dest_len) {
            return -2; // destination buffer overflow
        }

        for (int i = 0; i < run_len; i++) {
            dest[dest_idx++] = val;
        }
    }

    return dest_idx;
}
