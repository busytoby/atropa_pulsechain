#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char **argv) {
    const char *path = (argc > 1) ? argv[1] : "/tmp/cpm_tomie_extracted_paper.txt";
    FILE *f = fopen(path, "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = (char *)malloc(sz + 1);
    if (!buf) {
        fclose(f);
        return 1;
    }
    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);

    printf("=================================================================\n");
    printf("CP/M-TOMIE CLEAN-ROOM TEXT STREAM DUMP (%ld bytes)\n", sz);
    printf("=================================================================\n");

    int line_count = 0;
    char *p = buf;
    while (*p) {
        char *line_start = p;
        while (*p && *p != '\n' && *p != '\r') {
            p++;
        }
        char saved = *p;
        *p = '\0';

        if (strlen(line_start) > 5) {
            printf("[%04d] %s\n", ++line_count, line_start);
        }

        if (saved == '\0') break;
        p++;
        if (saved == '\r' && *p == '\n') p++;
    }

    free(buf);
    return 0;
}
