#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi2-deepseek/inc/tsfi_quadtree_ksds.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <input_file> [output_separator_char]\n", argv[0]);
        printf("Example: %s data.tsv ','\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror("Failed to open input file");
        return 1;
    }

    char target_sep = ',';
    if (argc >= 3) {
        target_sep = argv[2][0];
    }

    printf("[Converter] Converting %s using separator '%c'\n", argv[1], target_sep);

    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        size_t len = strlen(line);
        for (size_t i = 0; i < len; i++) {
            // Replace our tab separation variable or tabs with the target separator
            if (line[i] == tsfi_tab_separation_variable || line[i] == '\t') {
                putchar(target_sep);
            } else {
                putchar(line[i]);
            }
        }
    }

    fclose(f);
    return 0;
}
