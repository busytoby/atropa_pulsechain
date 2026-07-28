#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "tsfi_terse_fieldata.h"

// Candidate block sizes to evaluate
const int BLOCK_SIZES[] = { 256, 512, 1024, 2048, 4096 };
#define NUM_CANDIDATES 5

int evaluate_block_size(const char *filepath, int block_size, size_t *out_total_bytes, double *out_waste_pct) {
    FILE *f = fopen(filepath, "rb");
    if (!f) return -1;

    uint8_t *in_buf = malloc(block_size);
    uint8_t *comp_buf = malloc(block_size * 2);
    if (!in_buf || !comp_buf) {
        free(in_buf);
        free(comp_buf);
        fclose(f);
        return -2;
    }

    size_t total_written = 240; // Starts with 240 bytes standard label headers
    size_t total_padding = 0;
    size_t total_payload = 0;

    while (1) {
        size_t read_bytes = fread(in_buf, 1, block_size, f);
        if (read_bytes == 0) break;

        size_t compressed_len = 0;
        // Compress block using TERSE
        int res = fieldata_terse_compress(in_buf, read_bytes, comp_buf, &compressed_len);
        if (res != 0) {
            free(in_buf);
            free(comp_buf);
            fclose(f);
            return -3;
        }

        // Add 2 bytes of block metadata
        total_written += (compressed_len + 2);
        total_payload += compressed_len;

        // If block is partially filled, count padding waste
        if (read_bytes < (size_t)block_size) {
            total_padding += (block_size - read_bytes);
        }
    }

    free(in_buf);
    free(comp_buf);
    fclose(f);

    *out_total_bytes = total_written;
    *out_waste_pct = (double)total_padding / (total_payload + total_padding) * 100.0;
    return 0;
}

int main(int argc, char **argv) {
    printf("=== BLKDISK: DATASET BLOCK SIZE OPTIMIZATION UTILITY ===\n");

    if (argc < 2) {
        fprintf(stderr, "Usage: tsfi_blkdisk_terse <input_data_file>\n");
        return 1;
    }

    const char *filepath = argv[1];

    printf("[BLKDISK] Evaluating optimal block size configurations for dataset: %s\n\n", filepath);
    printf("  %-10s | %-18s | %-16s\n", "Block Size", "Total Tape Footprint", "Padding Waste %");
    printf("  -----------+--------------------+-----------------\n");

    int optimal_idx = -1;
    size_t min_footprint = 0xFFFFFFFF;

    for (int i = 0; i < NUM_CANDIDATES; i++) {
        size_t total_bytes = 0;
        double waste_pct = 0.0;
        int res = evaluate_block_size(filepath, BLOCK_SIZES[i], &total_bytes, &waste_pct);
        if (res == 0) {
            printf("  %-10d | %-14lu bytes | %-14.2f%%\n", BLOCK_SIZES[i], total_bytes, waste_pct);
            if (total_bytes < min_footprint) {
                min_footprint = total_bytes;
                optimal_idx = i;
            }
        } else {
            fprintf(stderr, "  [ERROR] Failed to evaluate configuration for block size %d\n", BLOCK_SIZES[i]);
        }
    }

    if (optimal_idx != -1) {
        printf("\n[BLKDISK] Recommendation: Use block size of %d bytes (minimizes disk footprint to %lu bytes).\n", 
               BLOCK_SIZES[optimal_idx], min_footprint);
    }

    return 0;
}
