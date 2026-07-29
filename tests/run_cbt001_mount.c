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
        [0xC1] = 'A', [0xC2] = 'B', [0xC3] = 'C', [0xC4] = 'D', [0xC5] = 'E',
        [0xC6] = 'F', [0xC7] = 'G', [0xC8] = 'H', [0xC9] = 'I',
        [0xD1] = 'J', [0xD2] = 'K', [0xD3] = 'L', [0xD4] = 'M', [0xD5] = 'N',
        [0xD6] = 'O', [0xD7] = 'P', [0xD8] = 'Q', [0xD9] = 'R',
        [0xE2] = 'S', [0xE3] = 'T', [0xE4] = 'U', [0xE5] = 'V', [0xE6] = 'W',
        [0xE7] = 'X', [0xE8] = 'Y', [0xE9] = 'Z',
        [0xF0] = '0', [0xF1] = '1', [0xF2] = '2', [0xF3] = '3', [0xF4] = '4',
        [0xF5] = '5', [0xF6] = '6', [0xF7] = '7', [0xF8] = '8', [0xF9] = '9',
        [0x4B] = '.', [0x4D] = '<', [0x5B] = '$', [0x5C] = '*', [0x5D] = ')',
        [0x60] = '-', [0x61] = '/', [0x6B] = ',', [0x7D] = '\'', [0x7E] = '=',
        [0x4F] = '|', [0x50] = '&', [0x5A] = '!', [0x5E] = ';', [0x6C] = '%',
        [0x6D] = '_', [0x6E] = '>', [0x6F] = '?', [0x7A] = ':', [0x7B] = '#',
        [0x7C] = '@'
    };
    char c = ebcdic_map[ebcdic];
    return (c == '\0') ? '?' : c;
}

int main(void) {
    // Copy the ZIP file to a compliant .dat.bin extension per Rule 13
    system("cp /tmp/cbt001/CBT001.zip /tmp/cbt001/cbt001.dat.bin");

    printf("\n=== MOUNTING CBT001.zip AS VIRTUAL TAPE TAPE01.dat.bin ===\n");
    char members[16][128];
    int count = tsfi_tape_zip_list_members("/tmp/cbt001/cbt001.dat.bin", members, 16);
    if (count <= 0) {
        printf("Failed to list members.\n");
        return 1;
    }

    printf("Archive Structure / Member Directory:\n");
    for (int i = 0; i < count; i++) {
        printf("  - Member #%d: %s\n", i + 1, members[i]);
    }

    printf("\nStreaming and Decoding Contents of first member: %s (First 35 lines)\n", members[0]);
    printf("--------------------------------------------------------------------------------\n");

    FILE *f = tsfi_tape_open_guarded("/tmp/cbt001/cbt001.dat.bin", "rb", TAPE_SECURITY_UNCLASSIFIED);
    if (!f) {
        printf("Failed to open tape stream.\n");
        return 1;
    }

    uint8_t card[80];
    int lines = 0;
    while (fread(card, 1, 80, f) == 80 && lines < 35) {
        char ascii[81];
        for (int i = 0; i < 80; i++) {
            ascii[i] = ebcdic_to_ascii(card[i]);
        }
        ascii[80] = '\0';
        printf("%03d: %s\n", lines + 1, ascii);
        lines++;
    }

    pclose(f);
    remove("/tmp/cbt001/cbt001.dat.bin");
    return 0;
}
