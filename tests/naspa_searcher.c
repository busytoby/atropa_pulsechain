#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tsfi_parc_tape_catalog.h"
#include "tsfi_parc_tape_label_yul.h"

static char ebcdic_to_ascii(uint8_t ebcdic) {
    static const char ebcdic_map[256] = {
        [' '] = ' ',
        [0xF0] = '0', [0xF1] = '1', [0xF2] = '2', [0xF3] = '3', [0xF4] = '4',
        [0xF5] = '5', [0xF6] = '6', [0xF7] = '7', [0xF8] = '8', [0xF9] = '9',
        [0xC1] = 'A', [0xC2] = 'B', [0xC3] = 'C', [0xC4] = 'D', [0xC5] = 'E',
        [0xC6] = 'F', [0xC7] = 'G', [0xC8] = 'H', [0xC9] = 'I',
        [0xD1] = 'J', [0xD2] = 'K', [0xD3] = 'L', [0xD4] = 'M', [0xD5] = 'N',
        [0xD6] = 'O', [0xD7] = 'P', [0xD8] = 'Q', [0xD9] = 'R',
        [0xE2] = 'S', [0xE3] = 'T', [0xE4] = 'U', [0xE5] = 'V', [0xE6] = 'W',
        [0xE7] = 'X', [0xE8] = 'Y', [0xE9] = 'Z',
        [0x81] = 'a', [0x82] = 'b', [0x83] = 'c', [0x84] = 'd', [0x85] = 'e',
        [0x86] = 'f', [0x87] = 'g', [0x88] = 'h', [0x89] = 'i',
        [0x91] = 'j', [0x92] = 'k', [0x93] = 'l', [0x94] = 'm', [0x95] = 'n',
        [0x96] = 'o', [0x97] = 'p', [0x98] = 'q', [0x99] = 'r',
        [0xA2] = 's', [0xA3] = 't', [0xA4] = 'u', [0xA5] = 'v', [0xA6] = 'w',
        [0xA7] = 'x', [0xA8] = 'y', [0xA9] = 'z',
        [0x4B] = '.', [0x4C] = '<', [0x4D] = '(', [0x5B] = '$', [0x5C] = '*', 
        [0x5D] = ')', [0x4E] = '+', [0x50] = '&', [0x5E] = ';',
        [0x60] = '-', [0x61] = '/', [0x6B] = ',', [0x6C] = '%', [0x6D] = '_',
        [0x6E] = '>', [0x6F] = '?', [0x7A] = ':', [0x7B] = '#', [0x7C] = '@', 
        [0x7D] = '\'', [0x7E] = '=', [0x7F] = '"'
    };
    char c = ebcdic_map[ebcdic];
    return (c == '\0') ? '?' : c;
}

// Convert string to lowercase in-place
static void to_lowercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

int main(void) {
    system("cp /tmp/cbt001/CBT001.zip /tmp/cbt001/cbt001_search.dat.bin");

    FILE *f = tsfi_tape_open_guarded("/tmp/cbt001/cbt001_search.dat.bin", "rb", TAPE_SECURITY_UNCLASSIFIED);
    if (!f) {
        printf("Failed to open tape.\n");
        return 1;
    }

    uint8_t card[80];
    char ascii[81];
    char search_buf[81];
    int line_num = 0;

    printf("\n=== SEARCHING FOR 'NASPA' ENTRIES IN MOUNTED TAPE ===\n");

    while (fread(card, 1, 80, f) == 80) {
        line_num++;
        for (int i = 0; i < 80; i++) {
            ascii[i] = ebcdic_to_ascii(card[i]);
        }
        ascii[80] = '\0';

        strcpy(search_buf, ascii);
        to_lowercase(search_buf);

        if (strstr(search_buf, "naspa") != NULL) {
            char *end = strstr(ascii, "   ");
            if (end) *end = '\0';
            printf("Line %d: %s\n", line_num, ascii);
        }
    }

    pclose(f);
    remove("/tmp/cbt001/cbt001_search.dat.bin");
    return 0;
}
