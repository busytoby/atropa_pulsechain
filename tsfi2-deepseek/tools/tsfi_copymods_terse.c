#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "tsfi_terse_fieldata.h"

#define BLOCK_SIZE 1024

int main(int argc, char **argv) {
    printf("=== COPYMODS HIGH-SPEED MULTI-DESTINATION STREAMER ===\n");

    if (argc < 4) {
        fprintf(stderr, "Usage: tsfi_copymods_terse <input_file> <output_dest1> [output_dest2] ... [output_destN]\n");
        return 1;
    }

    const char *input_path = argv[1];
    int dest_count = argc - 2;
    if (dest_count > 16) dest_count = 16; // Limit to 16 outputs like COPYMODS

    FILE *in_file = fopen(input_path, "rb");
    if (!in_file) {
        fprintf(stderr, "Error: Cannot open input file %s\n", input_path);
        return 1;
    }

    FILE *out_files[16];
    for (int i = 0; i < dest_count; i++) {
        out_files[i] = fopen(argv[2 + i], "wb");
        if (!out_files[i]) {
            fprintf(stderr, "Error: Cannot open output destination %s\n", argv[2 + i]);
            // Close already opened outputs
            for (int j = 0; j < i; j++) fclose(out_files[j]);
            fclose(in_file);
            return 1;
        }
        printf("[COPYMODS] Registered Output Destination #%d: %s\n", i + 1, argv[2 + i]);
    }

    uint8_t input_buf[BLOCK_SIZE];
    uint8_t compressed_buf[BLOCK_SIZE * 2];
    size_t total_input_bytes = 0;
    size_t total_output_bytes = 0;

    while (1) {
        size_t read_bytes = fread(input_buf, 1, BLOCK_SIZE, in_file);
        if (read_bytes == 0) break;
        total_input_bytes += read_bytes;

        // Compress using optimized TERSE Arithmetic Range Coding
        size_t compressed_len = 0;
        int res = fieldata_terse_compress(input_buf, read_bytes, compressed_buf, &compressed_len);
        if (res != 0) {
            fprintf(stderr, "Error: Arithmetic compression failed on block\n");
            break;
        }

        // Write block metadata (2 bytes for compressed length)
        uint8_t meta[2];
        meta[0] = (uint8_t)(compressed_len >> 8);
        meta[1] = (uint8_t)(compressed_len & 0xFF);

        // Write simultaneously to all targets (COPYMODS carbon copy replication)
        for (int i = 0; i < dest_count; i++) {
            fwrite(meta, 1, 2, out_files[i]);
            fwrite(compressed_buf, 1, compressed_len, out_files[i]);
        }
        total_output_bytes += (compressed_len + 2);
    }

    printf("\n=== STREAM COMPLETED SUCCESSFULLY ===\n");
    printf("[COPYMODS] Total Input FIELDATA Symbols: %lu\n", total_input_bytes);
    printf("[COPYMODS] Total Output Compressed Block Bytes: %lu\n", total_output_bytes);
    printf("[COPYMODS] Compression Ratio: %.2f%%\n", (double)total_output_bytes / total_input_bytes * 100.0);

    fclose(in_file);
    for (int i = 0; i < dest_count; i++) {
        fclose(out_files[i]);
    }
    return 0;
}
