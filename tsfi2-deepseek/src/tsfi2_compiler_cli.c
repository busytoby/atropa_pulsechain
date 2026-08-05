#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/tsfi2_compiler.h"
#include "inc/tsfi2_compiler_bin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool run_replace_utility(const char *target, const char *replacement, const char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "Error: Could not open file %s for reading\n", filepath);
        return false;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(size + 1);
    if (!buf) {
        fclose(f);
        return false;
    }
    size_t read_bytes = fread(buf, 1, size, f);
    buf[read_bytes] = '\0';
    fclose(f);

    size_t target_len = strlen(target);
    size_t replacement_len = strlen(replacement);
    size_t alloc_size = size * 2 + 1; // Over-allocate to handle expansions
    char *out_buf = malloc(alloc_size);
    if (!out_buf) {
        free(buf);
        return false;
    }

    char *src_ptr = buf;
    char *dst_ptr = out_buf;
    while (*src_ptr) {
        if (strncmp(src_ptr, target, target_len) == 0) {
            strcpy(dst_ptr, replacement);
            dst_ptr += replacement_len;
            src_ptr += target_len;
        } else {
            *dst_ptr++ = *src_ptr++;
        }
    }
    *dst_ptr = '\0';

    f = fopen(filepath, "w");
    if (!f) {
        fprintf(stderr, "Error: Could not open file %s for writing\n", filepath);
        free(buf);
        free(out_buf);
        return false;
    }
    fwrite(out_buf, 1, dst_ptr - out_buf, f);
    fclose(f);

    free(buf);
    free(out_buf);
    return true;
}

static bool run_split_utility(const char *source, const char *split_indicator, const char *file1, const char *file2) {
    FILE *f = fopen(source, "r");
    if (!f) {
        fprintf(stderr, "Error: Could not open source %s\n", source);
        return false;
    }
    FILE *f1 = fopen(file1, "w");
    FILE *f2 = fopen(file2, "w");
    if (!f1 || !f2) {
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        fclose(f);
        return false;
    }

    char line[2048];
    bool past_split = false;
    while (fgets(line, sizeof(line), f)) {
        if (!past_split && strstr(line, split_indicator)) {
            past_split = true;
        }
        if (!past_split) {
            fputs(line, f1);
        } else {
            fputs(line, f2);
        }
    }

    fclose(f);
    fclose(f1);
    fclose(f2);
    return true;
}

int main(int argc, char *argv[]) {
    if (argc >= 2 && strcmp(argv[1], "--replace") == 0) {
        if (argc < 5) {
            fprintf(stderr, "Usage: %s --replace <target> <replacement> <file>\n", argv[0]);
            return 1;
        }
        if (run_replace_utility(argv[2], argv[3], argv[4])) {
            printf("Replacement utility executed successfully on %s\n", argv[4]);
            return 0;
        }
        return 1;
    }

    if (argc >= 2 && strcmp(argv[1], "--split-header") == 0) {
        if (argc < 6) {
            fprintf(stderr, "Usage: %s --split-header <source> <indicator> <file1> <file2>\n", argv[0]);
            return 1;
        }
        if (run_split_utility(argv[2], argv[3], argv[4], argv[5])) {
            printf("Split utility executed successfully on %s\n", argv[2]);
            return 0;
        }
        return 1;
    }

    if (argc < 4 || strcmp(argv[2], "-o") != 0) {
        fprintf(stderr, "Usage: %s <input.c> -o <output.dat.bin>\n", argv[0]);
        fprintf(stderr, "   or: %s --replace <target> <replacement> <file>\n", argv[0]);
        fprintf(stderr, "   or: %s --split-header <source> <indicator> <file1> <file2>\n", argv[0]);
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

    const char *mount_keys = NULL;
    const char *mount_vals = NULL;
    if (strstr(source_code, "// wmq_mount STANAG")) {
        mount_keys = "NetworkMount";
        mount_vals = "STANAG";
    } else if (strstr(source_code, "// wmq_mount DECNET")) {
        mount_keys = "NetworkMount";
        mount_vals = "DECNET";
    }

    uint8_t bytecode[4096];
    size_t bytecode_len = 0;

    bool ok = tsfi2_compile(source_code, bytecode, sizeof(bytecode), &bytecode_len);

    if (!ok) {
        free(source_code);
        fprintf(stderr, "Error: TSFi2 compilation failed\n");
        return 1;
    }

    if (mount_keys && mount_vals) {
        ok = tsfi2_compile_to_dat_bin_ext(output_path, 0x1000, 1, mount_keys, mount_vals, bytecode, bytecode_len);
    } else {
        ok = tsfi2_compile_to_dat_bin(output_path, 0x1000, 1, bytecode, bytecode_len);
    }
    free(source_code);
    if (!ok) {
        fprintf(stderr, "Error: Packaging to dat.bin failed (ensure file extension is .dat.bin)\n");
        return 1;
    }

    printf("TSFi2 compilation successful. Output written to %s\n", output_path);
    return 0;
}
