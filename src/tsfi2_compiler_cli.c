#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/tsfi2_compiler.h"
#include "inc/tsfi2_compiler_bin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 4 || strcmp(argv[2], "-o") != 0) {
        fprintf(stderr, "Usage: %s <input.c> -o <output.dat.bin>\n", argv[0]);
        return 1;
    }

    const char *input_path = argv[1];
    const char *output_path = argv[3];

    FILE *f = fopen(input_path, "r");
    if (!f) {
        fprintf(stderr, "Error: Could not open input file %s\n", input_path);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *source_code = malloc(size + 1);
    if (!source_code) {
        fclose(f);
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 1;
    }

    size_t read_bytes = fread(source_code, 1, size, f);
    source_code[read_bytes] = '\0';
    fclose(f);

    uint8_t bytecode[4096];
    size_t bytecode_len = 0;

    bool ok = tsfi2_compile(source_code, bytecode, sizeof(bytecode), &bytecode_len);
    free(source_code);

    if (!ok) {
        fprintf(stderr, "Error: TSFi2 compilation failed\n");
        return 1;
    }

    ok = tsfi2_compile_to_dat_bin(output_path, 0x1000, 1, bytecode, bytecode_len);
    if (!ok) {
        fprintf(stderr, "Error: Packaging to dat.bin failed (ensure file extension is .dat.bin)\n");
        return 1;
    }

    printf("TSFi2 compilation successful. Output written to %s\n", output_path);
    return 0;
}
