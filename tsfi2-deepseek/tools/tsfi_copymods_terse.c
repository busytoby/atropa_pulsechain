#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "tsfi_terse_fieldata.h"

#define BLOCK_SIZE 1024

// Generates an 80-byte IBM Standard Label (SL) record buffer
void generate_sl_record(char *buf, const char *label_type, const char *dataset_name, int file_seq) {
    memset(buf, ' ', 80);
    
    // Bytes 0-3: Label Identifier (e.g. VOL1, HDR1, HDR2)
    memcpy(buf, label_type, 4);

    if (strcmp(label_type, "VOL1") == 0) {
        // Bytes 4-9: Volume Serial Number (VOLSER)
        memcpy(buf + 4, "CBTCOV", 6);
        // Byte 10: Accessibility
        buf[10] = ' ';
        // Bytes 37-46: Owner Identifier
        memcpy(buf + 37, "AUNCIENTVM", 10);
    } else if (strcmp(label_type, "HDR1") == 0) {
        // Bytes 4-20: Dataset Identifier (File Name)
        char dsname_padded[17];
        snprintf(dsname_padded, sizeof(dsname_padded), "%-16.16s", dataset_name);
        memcpy(buf + 4, dsname_padded, 16);
        
        // Bytes 21-26: Volume Serial Number
        memcpy(buf + 21, "CBTCOV", 6);
        
        // Bytes 27-30: Volume Sequence Number
        memcpy(buf + 27, "0001", 4);
        
        // Bytes 31-34: File Sequence Number
        char seq_str[5];
        snprintf(seq_str, sizeof(seq_str), "%04d", file_seq);
        memcpy(buf + 31, seq_str, 4);

        // Bytes 41-47: Creation Date (Julian format yyddd)
        time_t t = time(NULL);
        struct tm *lt = localtime(&t);
        char date_str[7];
        snprintf(date_str, sizeof(date_str), " %02d%03d", lt->tm_year % 100, lt->tm_yday + 1);
        memcpy(buf + 41, date_str, 6);
    } else if (strcmp(label_type, "HDR2") == 0) {
        // Byte 4: Record Format (F = Fixed length, V = Variable, U = Undefined)
        buf[4] = 'V';
        // Bytes 5-9: Block Length
        memcpy(buf + 5, "01024", 5);
        // Bytes 10-14: Record Length
        memcpy(buf + 10, "01024", 5);
    }
}

int main(int argc, char **argv) {
    printf("=== COPYMODS ADAPTIVE MULTI-DESTINATION STREAMER ===\n");

    if (argc < 4) {
        fprintf(stderr, "Usage: tsfi_copymods_terse <input_file> <output_dest1> [output_dest2] ... [output_destN]\n");
        return 1;
    }

    const char *input_path = argv[1];
    int dest_count = argc - 2;
    if (dest_count > 16) dest_count = 16; // Limit to 16 outputs

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
            for (int j = 0; j < i; j++) fclose(out_files[j]);
            fclose(in_file);
            return 1;
        }
        printf("[COPYMODS] Registered Output Destination #%d: %s\n", i + 1, argv[2 + i]);
    }

    // 1. Write IBM Standard Labels (VOL1, HDR1, HDR2) to destinations
    char label_vol1[80];
    char label_hdr1[80];
    char label_hdr2[80];
    generate_sl_record(label_vol1, "VOL1", "", 0);
    generate_sl_record(label_hdr1, "HDR1", "CBTCOV.FILE006", 6);
    generate_sl_record(label_hdr2, "HDR2", "", 0);

    for (int i = 0; i < dest_count; i++) {
        fwrite(label_vol1, 1, 80, out_files[i]);
        fwrite(label_hdr1, 1, 80, out_files[i]);
        fwrite(label_hdr2, 1, 80, out_files[i]);
    }
    printf("[COPYMODS] Injected Standard Tape Label Headers (VOL1, HDR1, HDR2) into targets.\n");

    uint8_t input_buf[BLOCK_SIZE];
    uint8_t compressed_buf[BLOCK_SIZE * 2];
    size_t total_input_bytes = 0;
    size_t total_output_bytes = 240; // Including 240 bytes of standard label headers

    while (1) {
        size_t read_bytes = fread(input_buf, 1, BLOCK_SIZE, in_file);
        if (read_bytes == 0) break;
        total_input_bytes += read_bytes;

        // Compress using adaptive TERSE Arithmetic Range Coding
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

        // Write simultaneously to all targets
        for (int i = 0; i < dest_count; i++) {
            fwrite(meta, 1, 2, out_files[i]);
            fwrite(compressed_buf, 1, compressed_len, out_files[i]);
        }
        total_output_bytes += (compressed_len + 2);
    }

    printf("\n=== STREAM COMPLETED SUCCESSFULLY ===\n");
    printf("[COPYMODS] Total Input FIELDATA Symbols: %lu\n", total_input_bytes);
    printf("[COPYMODS] Total Output Compressed Bytes (inc Labels): %lu\n", total_output_bytes);
    printf("[COPYMODS] Compression Ratio: %.2f%%\n", (double)total_output_bytes / total_input_bytes * 100.0);

    fclose(in_file);
    for (int i = 0; i < dest_count; i++) {
        fclose(out_files[i]);
    }
    return 0;
}
