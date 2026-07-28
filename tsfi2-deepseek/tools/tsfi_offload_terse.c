#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <time.h>
#include "tsfi_terse_fieldata.h"

#define BLOCK_SIZE 1024

// Generates an 80-byte IBM Standard Label (SL) record buffer
void generate_sl_record(char *buf, const char *label_type, const char *dataset_name) {
    memset(buf, ' ', 80);
    memcpy(buf, label_type, 4);

    if (strcmp(label_type, "VOL1") == 0) {
        memcpy(buf + 4, "CBTCOV", 6);
        memcpy(buf + 37, "AUNCIENTVM", 10);
    } else if (strcmp(label_type, "HDR1") == 0) {
        char dsname_padded[17];
        snprintf(dsname_padded, sizeof(dsname_padded), "%-16.16s", dataset_name);
        memcpy(buf + 4, dsname_padded, 16);
        memcpy(buf + 21, "CBTCOV", 6);
        memcpy(buf + 27, "0001", 4);
        memcpy(buf + 31, "0008", 4); // File sequence 8 for load modules

        time_t t = time(NULL);
        struct tm *lt = localtime(&t);
        char date_str[256];
        snprintf(date_str, sizeof(date_str), " %02d%03d", lt->tm_year % 100, lt->tm_yday + 1);
        memcpy(buf + 41, date_str, 6);
    } else if (strcmp(label_type, "HDR2") == 0) {
        buf[4] = 'V';
        memcpy(buf + 5, "01024", 5);
        memcpy(buf + 10, "01024", 5);
    }
}

int main(int argc, char **argv) {
    printf("=== OFFLOAD: VIRTUAL PARTITIONED OBJECT PACKER ===\n");

    if (argc < 3) {
        fprintf(stderr, "Usage: tsfi_offload_terse <input_dir> <output_tape.dat.bin>\n");
        return 1;
    }

    const char *in_dir_path = argv[1];
    const char *tape_path = argv[2];

    DIR *dir = opendir(in_dir_path);
    if (!dir) {
        fprintf(stderr, "Error: Cannot open input directory %s\n", in_dir_path);
        return 1;
    }

    FILE *f_tape = fopen(tape_path, "wb");
    if (!f_tape) {
        fprintf(stderr, "Error: Cannot create output tape file %s\n", tape_path);
        closedir(dir);
        return 1;
    }

    // 1. Write IBM Standard Tape Labels (VOL1, HDR1, HDR2)
    char label_vol1[80];
    char label_hdr1[80];
    char label_hdr2[80];
    generate_sl_record(label_vol1, "VOL1", "");
    generate_sl_record(label_hdr1, "HDR1", "CBTCOV.FILE008");
    generate_sl_record(label_hdr2, "HDR2", "");

    fwrite(label_vol1, 1, 80, f_tape);
    fwrite(label_hdr1, 1, 80, f_tape);
    fwrite(label_hdr2, 1, 80, f_tape);
    printf("[OFFLOAD] Injected Standard Tape Label Headers.\n");

    uint8_t block_buf[BLOCK_SIZE];
    size_t block_idx = 0;
    size_t total_blocks_written = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip current/parent directories and hidden files
        if (entry->d_name[0] == '.') continue;

        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", in_dir_path, entry->d_name);

        FILE *f_mem = fopen(filepath, "rb");
        if (!f_mem) {
            fprintf(stderr, "[WARN] Cannot open member file %s, skipping.\n", filepath);
            continue;
        }

        // Extract member name (strip extension)
        char member_name[9];
        memset(member_name, 0, sizeof(member_name));
        char *dot = strchr(entry->d_name, '.');
        if (dot) {
            int len = (int)(dot - entry->d_name);
            if (len > 8) len = 8;
            snprintf(member_name, sizeof(member_name), "%.*s", len, entry->d_name);
        } else {
            snprintf(member_name, sizeof(member_name), "%.8s", entry->d_name);
        }

        printf("[OFFLOAD] Packing Member: %-8s from %s\n", member_name, filepath);

        // 2. Generate and write the "./ ADD NAME=member " card header to block buffer
        char add_card_str[64];
        snprintf(add_card_str, sizeof(add_card_str), "./ ADD NAME=%s ", member_name);
        size_t card_len = strlen(add_card_str);

        for (size_t i = 0; i < card_len; i++) {
            block_buf[block_idx++] = (uint8_t)(add_card_str[i] & 0x3F);

            // If block is full, compress and write it
            if (block_idx == BLOCK_SIZE) {
                uint8_t comp_buf[BLOCK_SIZE * 2];
                size_t comp_len = 0;
                fieldata_terse_compress(block_buf, BLOCK_SIZE, comp_buf, &comp_len);

                uint8_t meta[2];
                meta[0] = (uint8_t)(comp_len >> 8);
                meta[1] = (uint8_t)(comp_len & 0xFF);
                fwrite(meta, 1, 2, f_tape);
                fwrite(comp_buf, 1, comp_len, f_tape);

                block_idx = 0;
                total_blocks_written++;
            }
        }

        // 3. Write member file contents to block buffer
        int c;
        while ((c = fgetc(f_mem)) != EOF) {
            block_buf[block_idx++] = (uint8_t)(c & 0x3F);

            if (block_idx == BLOCK_SIZE) {
                uint8_t comp_buf[BLOCK_SIZE * 2];
                size_t comp_len = 0;
                fieldata_terse_compress(block_buf, BLOCK_SIZE, comp_buf, &comp_len);

                uint8_t meta[2];
                meta[0] = (uint8_t)(comp_len >> 8);
                meta[1] = (uint8_t)(comp_len & 0xFF);
                fwrite(meta, 1, 2, f_tape);
                fwrite(comp_buf, 1, comp_len, f_tape);

                block_idx = 0;
                total_blocks_written++;
            }
        }
        fclose(f_mem);
    }

    // 4. Flush remaining buffer with padding
    if (block_idx > 0) {
        // Pad the rest of the block with zeros (neutral filler symbols)
        memset(block_buf + block_idx, 0, BLOCK_SIZE - block_idx);
        
        uint8_t comp_buf[BLOCK_SIZE * 2];
        size_t comp_len = 0;
        fieldata_terse_compress(block_buf, BLOCK_SIZE, comp_buf, &comp_len);

        uint8_t meta[2];
        meta[0] = (uint8_t)(comp_len >> 8);
        meta[1] = (uint8_t)(comp_len & 0xFF);
        fwrite(meta, 1, 2, f_tape);
        fwrite(comp_buf, 1, comp_len, f_tape);
        total_blocks_written++;
    }

    fclose(f_tape);
    closedir(dir);

    printf("\n=== OFFLOAD TAPE PACKING COMPLETED ===\n");
    printf("[OFFLOAD] Total Data Blocks Written: %lu\n", total_blocks_written);
    return 0;
}
