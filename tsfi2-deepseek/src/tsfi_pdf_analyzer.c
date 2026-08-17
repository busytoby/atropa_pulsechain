#include "tsfi_pdf_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    size_t len = 0;
    char *text = tsfi_pdf_extract_text(argv[1], &len);
    if (!text) return 1;

    for (int sec = 1; sec <= 9; sec++) {
        char pattern[16];
        snprintf(pattern, sizeof(pattern), "\n%d ", sec);
        char *p = strstr(text, pattern);
        if (!p && sec == 1) p = strstr(text, "1 Introduction");
        if (p) {
            printf("\n=======================================================\n");
            printf("Found Section #%d at offset %td:\n", sec, p - text);
            for (int i = 0; i < 400 && (p - text + i) < (long)len; i++) {
                unsigned char c = (unsigned char)p[i];
                putchar(isprint(c) || c == '\n' ? c : ' ');
            }
            printf("\n=======================================================\n");
        }
    }

    free(text);
    return 0;
}
