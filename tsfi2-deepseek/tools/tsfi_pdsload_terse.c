#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "tsfi_terse_fieldata.h"

#define BLOCK_SIZE 1024

// 6-bit FIELDATA symbols for "./ ADD NAME="
const uint8_t PDS_ADD_CARD[] = { 46, 47, 32, 1, 4, 4, 32, 14, 1, 13, 5, 61 };
#define CARD_LEN 12

// Converts a 6-bit symbol back to printable ASCII char for file names
char symbol_to_char(uint8_t sym) {
    // Map basic alphabet ('A'-'Z' are masked to 1-26)
    if (sym >= 1 && sym <= 26) return (char)('A' + sym - 1);
    // Map numbers ('0'-'9' are masked to 48-57)
    if (sym >= 48 && sym <= 57) return (char)('0' + sym - 48);
    // Map space
    if (sym == 32) return ' ';
    return '?'; // Return '?' for non-alphanumeric symbols (like newlines/tabs/etc)
}

int main(int argc, char **argv) {
    printf("=== PDSLOAD: VIRTUAL PARTITIONED OBJECT LOADER ===\n");

    if (argc < 3) {
        fprintf(stderr, "Usage: tsfi_pdsload_terse <input_tape.dat.bin> <output_dir>\n");
        return 1;
    }

    const char *tape_path = argv[1];
    const char *out_dir = argv[2];

    // Create target directory if it doesn't exist
#ifdef _WIN32
    mkdir(out_dir);
#else
    mkdir(out_dir, 0777);
#endif

    FILE *f_tape = fopen(tape_path, "rb");
    if (!f_tape) {
        fprintf(stderr, "Error: Cannot open virtual tape %s\n", tape_path);
        return 1;
    }

    // Skip the 240-byte IBM Standard Tape Label Headers
    if (fseek(f_tape, 240, SEEK_SET) != 0) {
        fprintf(stderr, "Error: Failed to seek past tape labels.\n");
        fclose(f_tape);
        return 1;
    }

    uint8_t meta[2];
    uint8_t comp_buf[4096];
    uint8_t decomp_buf[BLOCK_SIZE];

    FILE *current_member_file = NULL;
    size_t extracted_members_count = 0;
    size_t total_blocks_processed = 0;

    while (fread(meta, 1, 2, f_tape) == 2) {
        uint16_t comp_len = ((uint16_t)meta[0] << 8) | meta[1];
        if (fread(comp_buf, 1, comp_len, f_tape) != comp_len) {
            fprintf(stderr, "[WARN] Premature tape end during block read.\n");
            break;
        }

        // Decompress content block
        int res = fieldata_terse_decompress(comp_buf, comp_len, decomp_buf, BLOCK_SIZE);
        if (res != 0) {
            fprintf(stderr, "[ERROR] Decompression failed on block #%lu\n", total_blocks_processed + 1);
            break;
        }
        total_blocks_processed++;

        // Parse decompressed buffer for PDS ./ ADD NAME= member card
        size_t idx = 0;
        while (idx < BLOCK_SIZE) {
            // Check for "./ ADD NAME=" boundary pattern
            if (BLOCK_SIZE - idx >= CARD_LEN && memcmp(decomp_buf + idx, PDS_ADD_CARD, CARD_LEN) == 0) {
                // Close current member file if open
                if (current_member_file) {
                    fclose(current_member_file);
                    current_member_file = NULL;
                }

                idx += CARD_LEN;
                
                // Read member name (up to 8 symbols)
                char member_name[9];
                memset(member_name, 0, sizeof(member_name));
                for (int m = 0; m < 8 && idx < BLOCK_SIZE; m++) {
                    uint8_t sym = decomp_buf[idx++];
                    char c = symbol_to_char(sym);
                    if (c == '?' || c == ' ') break; // Terminate on non-alphanumeric or space
                    member_name[m] = c;
                }

                // Open new output member file
                char member_path[512];
                snprintf(member_path, sizeof(member_path), "%s/%s.dat.bin", out_dir, member_name);
                current_member_file = fopen(member_path, "wb");
                if (!current_member_file) {
                    fprintf(stderr, "Error: Cannot create member output file %s\n", member_path);
                    fclose(f_tape);
                    return 1;
                }

                printf("[PDSLOAD] Extracting Member: %-8s -> %s\n", member_name, member_path);
                extracted_members_count++;
            } else {
                // Write standard data payload symbols to active member file
                if (current_member_file) {
                    fputc(decomp_buf[idx], current_member_file);
                }
                idx++;
            }
        }
    }

    if (current_member_file) {
        fclose(current_member_file);
    }
    fclose(f_tape);

    printf("\n=== PDSLOAD UNPACK COMPLETED ===\n");
    printf("[PDSLOAD] Total Data Blocks Processed: %lu\n", total_blocks_processed);
    printf("[PDSLOAD] Total Members Unpacked      : %lu\n", extracted_members_count);

    return 0;
}
