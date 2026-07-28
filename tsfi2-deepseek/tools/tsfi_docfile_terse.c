#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    printf("=== DOCFILE: POSITIONAL COLUMN SEQUENCER ===\n");

    if (argc < 3) {
        fprintf(stderr, "Usage: tsfi_docfile_terse <input.txt> <output_sequenced.txt> [prefix]\n");
        return 1;
    }

    const char *input_path = argv[1];
    const char *output_path = argv[2];
    const char *prefix = (argc >= 4) ? argv[3] : "AUNC";

    FILE *f_in = fopen(input_path, "r");
    if (!f_in) {
        fprintf(stderr, "Error: Cannot open input file %s\n", input_path);
        return 1;
    }

    FILE *f_out = fopen(output_path, "w");
    if (!f_out) {
        fprintf(stderr, "Error: Cannot create output file %s\n", output_path);
        fclose(f_in);
        return 1;
    }

    char line[512];
    size_t line_count = 0;

    while (fgets(line, sizeof(line), f_in)) {
        line_count++;
        // Remove trailing newlines/carriage returns
        line[strcspn(line, "\r\n")] = '\0';

        // Truncate to 72 characters if longer
        size_t len = strlen(line);
        if (len > 72) {
            line[72] = '\0';
            len = 72;
        }

        // Write line content
        fputs(line, f_out);

        // Pad with spaces up to column 72
        for (size_t i = len; i < 72; i++) {
            fputc(' ', f_out);
        }

        // Format 8-character sequence: 4-char prefix + 4-digit line number * 10
        char seq[9];
        snprintf(seq, sizeof(seq), "%-4.4s%04lu", prefix, (unsigned long)(line_count * 10));
        fputs(seq, f_out);
        fputc('\n', f_out);
    }

    fclose(f_in);
    fclose(f_out);

    printf("\n=== DOCFILE COMPLETED ===\n");
    printf("[DOCFILE] Sequenced %lu lines with prefix '%s'\n", line_count, prefix);
    return 0;
}
