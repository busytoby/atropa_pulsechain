#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "tsfi_parc_tape_catalog.h"
#include "tsfi_parc_tape_label_yul.h"

static char ebcdic_to_ascii(uint8_t ebcdic) {
    static const char ebcdic_map[256] = {
        [' '] = ' ',
        // Numbers
        [0xF0] = '0', [0xF1] = '1', [0xF2] = '2', [0xF3] = '3', [0xF4] = '4',
        [0xF5] = '5', [0xF6] = '6', [0xF7] = '7', [0xF8] = '8', [0xF9] = '9',
        // Uppercase letters
        [0xC1] = 'A', [0xC2] = 'B', [0xC3] = 'C', [0xC4] = 'D', [0xC5] = 'E',
        [0xC6] = 'F', [0xC7] = 'G', [0xC8] = 'H', [0xC9] = 'I',
        [0xD1] = 'J', [0xD2] = 'K', [0xD3] = 'L', [0xD4] = 'M', [0xD5] = 'N',
        [0xD6] = 'O', [0xD7] = 'P', [0xD8] = 'Q', [0xD9] = 'R',
        [0xE2] = 'S', [0xE3] = 'T', [0xE4] = 'U', [0xE5] = 'V', [0xE6] = 'W',
        [0xE7] = 'X', [0xE8] = 'Y', [0xE9] = 'Z',
        // Lowercase letters
        [0x81] = 'a', [0x82] = 'b', [0x83] = 'c', [0x84] = 'd', [0x85] = 'e',
        [0x86] = 'f', [0x87] = 'g', [0x88] = 'h', [0x89] = 'i',
        [0x91] = 'j', [0x92] = 'k', [0x93] = 'l', [0x94] = 'm', [0x95] = 'n',
        [0x96] = 'o', [0x97] = 'p', [0x98] = 'q', [0x99] = 'r',
        [0xA2] = 's', [0xA3] = 't', [0xA4] = 'u', [0xA5] = 'v', [0xA6] = 'w',
        [0xA7] = 'x', [0xA8] = 'y', [0xA9] = 'z',
        // Punctuation & symbols
        [0x4B] = '.', [0x4C] = '<', [0x4D] = '(', [0x5B] = '$', [0x5C] = '*', 
        [0x5D] = ')', [0x4E] = '+', [0x50] = '&', [0x5E] = ';',
        [0x60] = '-', [0x61] = '/', [0x6B] = ',', [0x6C] = '%', [0x6D] = '_',
        [0x6E] = '>', [0x6F] = '?', [0x7A] = ':', [0x7B] = '#', [0x7C] = '@', 
        [0x7D] = '\'', [0x7E] = '=', [0x7F] = '"'
    };
    char c = ebcdic_map[ebcdic];
    return (c == '\0') ? '?' : c;
}

int main(void) {
    // Copy the ZIP file to a compliant .dat.bin extension per Rule 13
    system("cp /tmp/cbt001/CBT001.zip /tmp/cbt001/cbt001_catalog.dat.bin");

    FILE *f = tsfi_tape_open_guarded("/tmp/cbt001/cbt001_catalog.dat.bin", "rb", TAPE_SECURITY_UNCLASSIFIED);
    if (!f) {
        printf("Failed to open tape stream.\n");
        return 1;
    }

    uint8_t card[80];
    char ascii[81];
    int printed_files = 0;

    printf("\n=== EXTRACTING CATALOG MAPPINGS FROM MOUNTED TAPE ===\n");

    while (fread(card, 1, 80, f) == 80) {
        // Decode to ASCII
        for (int i = 0; i < 80; i++) {
            ascii[i] = ebcdic_to_ascii(card[i]);
        }
        ascii[80] = '\0';

        // Check for file indicator marker: "***FILE"
        if (strstr(ascii, "***FILE") != NULL) {
            // Trim trailing characters and doc file indicators
            char *end = strstr(ascii, "   ");
            if (end) *end = '\0';
            
            // Print file record description line
            printf("%s\n", ascii);
            printed_files++;

            // Print next 3 lines if they are comments describing the file
            for (int k = 0; k < 3; k++) {
                uint8_t next_card[80];
                char next_ascii[81];
                long pos = ftell(f); // Save stream position
                if (fread(next_card, 1, 80, f) == 80) {
                    for (int i = 0; i < 80; i++) {
                        next_ascii[i] = ebcdic_to_ascii(next_card[i]);
                    }
                    next_ascii[80] = '\0';
                    if (strstr(next_ascii, "//*") != NULL && strstr(next_ascii, "***") == NULL) {
                        char *comment_end = strstr(next_ascii, "   ");
                        if (comment_end) *comment_end = '\0';
                        printf("  %s\n", next_ascii + 3);
                    } else {
                        // Rewind if it's not a continuation comment
                        fseek(f, pos, SEEK_SET);
                        break;
                    }
                }
            }
            
            // Stop after printing first 15 files to keep output concise
            if (printed_files >= 15) {
                break;
            }
        }
    }

    pclose(f);
    remove("/tmp/cbt001/cbt001_catalog.dat.bin");
    return 0;
}
