#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "tsfi_terse_fieldata.h"

#define BLOCK_SIZE 1024

void print_label_string(const char *name, const char *data, int len) {
    printf("  %-16s: '%.*s'\n", name, len, data);
}

// Dynamic inspector for decompressed payload content
void inspect_decompressed_payload(const char *dataset_name, const uint8_t *payload, size_t len) {
    printf("\n--- PAYLOAD CONTENT INSPECTION ---\n");

    // Case 1: EVM/ZMM Contract Bytecode Inspection
    if (strstr(dataset_name, "CONTRACT") != NULL || payload[0] == 0x60 || payload[0] == 0x00) {
        printf("  Detected Type: ZMM/EVM Deployable Code Object\n");
        size_t push_count = 0;
        size_t sstore_count = 0;
        size_t log_count = 0;

        for (size_t i = 0; i < len && i < 128; i++) {
            if (payload[i] >= 0x60 && payload[i] <= 0x7F) push_count++;
            else if (payload[i] == 0x55) sstore_count++;
            else if (payload[i] >= 0xA0 && payload[i] <= 0xA4) log_count++;
        }
        printf("  [Bytecode Analysis - First 128 Bytes]:\n");
        printf("    PUSH Instructions : %lu\n", push_count);
        printf("    SSTORE Operations : %lu\n", sstore_count);
        printf("    LOG/Emit Events   : %lu\n", log_count);
    } 
    // Case 2: RDBMS Schema and Token Database Inspection
    else if (strstr(dataset_name, "RDBMS") != NULL || strstr(dataset_name, "FILE") != NULL) {
        printf("  Detected Type: Relational Database Table Slice\n");
        
        // Scan for readable symbols or tokens
        size_t valid_printable = 0;
        for (size_t i = 0; i < len && i < 256; i++) {
            if (payload[i] >= 32 && payload[i] <= 126) {
                valid_printable++;
            }
        }
        printf("  [Database Analysis - Sample Records]:\n");
        printf("    Relational Tuples : %lu\n", len / 5); // 5 symbols per record in CAMMA layout
        printf("    Printable symbols : %lu/256 bytes\n", valid_printable);
    } else {
        printf("  Detected Type: Undefined Binary Media Asset\n");
    }
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

    char dataset_name[17];
    memset(dataset_name, 0, sizeof(dataset_name));
    if (strncmp(hdr1, "HDR1", 4) == 0) {
        memcpy(dataset_name, hdr1 + 4, 16);
        // Trim trailing spaces
        for (int i = 15; i >= 0; i--) {
            if (dataset_name[i] == ' ') dataset_name[i] = '\0';
            else break;
        }
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
    uint8_t first_block_payload[BLOCK_SIZE];
    memset(first_block_payload, 0, sizeof(first_block_payload));

    while (fread(meta, 1, 2, f) == 2) {
        uint16_t comp_len = ((uint16_t)meta[0] << 8) | meta[1];
        total_compressed_bytes += comp_len + 2;

        if (fread(comp_buf, 1, comp_len, f) != comp_len) {
            fprintf(stderr, "  [ERROR] Premature end of file on block #%lu\n", block_count + 1);
            break;
        }

        // Decompress to measure content output size
        int res = fieldata_terse_decompress(comp_buf, comp_len, decomp_buf, BLOCK_SIZE);
        if (res == 0) {
            total_decompressed_bytes += BLOCK_SIZE;
            if (block_count == 0) {
                memcpy(first_block_payload, decomp_buf, BLOCK_SIZE);
            }
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

    // 3. Dynamic Payload Inspection
    if (block_count > 0) {
        inspect_decompressed_payload(dataset_name, first_block_payload, BLOCK_SIZE);
    }

    fclose(f);
    return 0;
}
