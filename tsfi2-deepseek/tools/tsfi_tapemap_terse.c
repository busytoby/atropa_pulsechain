#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "tsfi_terse_fieldata.h"

#define BLOCK_SIZE 1024

void print_label_string(const char *name, const char *data, int len) {
    printf("  %-16s: '%.*s'\n", name, len, data);
}

int main(int argc, char **argv) {
    printf("=== TAPEMAP: COAXIAL RDBMS TAPE MAPPING UTILITY ===\n");

    if (argc < 2) {
        fprintf(stderr, "Usage: tsfi_tapemap_terse <input_labeled_file.dat.bin>\n");
        return 1;
    }

    const char *filepath = argv[1];
    FILE *f = fopen(filepath, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open target file %s\n", filepath);
        return 1;
    }

    // 1. Read and parse 240-byte IBM Standard Tape Label Headers
    char vol1[80];
    char hdr1[80];
    char hdr2[80];

    if (fread(vol1, 1, 80, f) != 80 ||
        fread(hdr1, 1, 80, f) != 80 ||
        fread(hdr2, 1, 80, f) != 80) {
        fprintf(stderr, "Error: Invalid tape file format. Missing 240-byte standard label block.\n");
        fclose(f);
        return 1;
    }

    printf("\n--- STANDARD TAPE LABELS ---\n");
    if (strncmp(vol1, "VOL1", 4) == 0) {
        print_label_string("Label Type", vol1, 4);
        print_label_string("VOLSER ID", vol1 + 4, 6);
        print_label_string("Owner ID", vol1 + 37, 10);
    } else {
        printf("  [WARN] VOL1 Label Identifier invalid: '%.4s'\n", vol1);
    }

    printf("\n");
    if (strncmp(hdr1, "HDR1", 4) == 0) {
        print_label_string("Label Type", hdr1, 4);
        print_label_string("Dataset Name", hdr1 + 4, 16);
        print_label_string("Vol Seq Number", hdr1 + 21, 6);
        print_label_string("File Seq Number", hdr1 + 31, 4);
        print_label_string("Creation Date", hdr1 + 41, 6);
    } else {
        printf("  [WARN] HDR1 Label Identifier invalid: '%.4s'\n", hdr1);
    }

    printf("\n");
    if (strncmp(hdr2, "HDR2", 4) == 0) {
        print_label_string("Label Type", hdr2, 4);
        print_label_string("Record Format", hdr2 + 4, 1);
        print_label_string("Block Length", hdr2 + 5, 5);
        print_label_string("Record Length", hdr2 + 10, 5);
    } else {
        printf("  [WARN] HDR2 Label Identifier invalid: '%.4s'\n", hdr2);
    }

    // 2. Scan data blocks and measure compression
    printf("\n--- DATA BLOCK SCAN ---\n");
    size_t block_count = 0;
    size_t total_compressed_bytes = 0;
    size_t total_decompressed_bytes = 0;
    
    uint8_t meta[2];
    uint8_t comp_buf[4096];
    uint8_t decomp_buf[BLOCK_SIZE];

    while (fread(meta, 1, 2, f) == 2) {
        uint16_t comp_len = ((uint16_t)meta[0] << 8) | meta[1];
        total_compressed_bytes += comp_len + 2; // block meta + compressed payload

        if (fread(comp_buf, 1, comp_len, f) != comp_len) {
            fprintf(stderr, "  [ERROR] Premature end of file on block #%lu\n", block_count + 1);
            break;
        }

        // Decompress to measure exact content output size
        int res = fieldata_terse_decompress(comp_buf, comp_len, decomp_buf, BLOCK_SIZE);
        if (res == 0) {
            total_decompressed_bytes += BLOCK_SIZE;
        } else {
            fprintf(stderr, "  [WARN] Failed to decompress block #%lu content\n", block_count + 1);
        }

        block_count++;
        printf("  Block #%-4lu: Compressed Size = %4u bytes | Content Size = %d bytes\n", 
               block_count, comp_len, BLOCK_SIZE);
    }

    printf("\n--- SUMMARY METRICS ---\n");
    printf("  Total Data Blocks  : %lu\n", block_count);
    printf("  Total Tape Bytes   : %lu bytes (including 240 bytes labels)\n", total_compressed_bytes + 240);
    printf("  Total Content Bytes: %lu bytes\n", total_decompressed_bytes);
    if (total_decompressed_bytes > 0) {
        printf("  Net Comp Ratio     : %.2f%%\n", 
               (double)(total_compressed_bytes) / total_decompressed_bytes * 100.0);
    }

    fclose(f);
    return 0;
}
