#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void trim_whitespace(char *str) {
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[--len] = '\0';
    }
}

// Scans text line to extract procedure or table name for member boundary
int extract_boundary_name(const char *line, char *out_name) {
    char clean_line[256];
    strncpy(clean_line, line, sizeof(clean_line));
    clean_line[sizeof(clean_line) - 1] = '\0';
    trim_whitespace(clean_line);

    // Look for XPL Procedure pattern: "<name>: PROCEDURE" or "<name>: PROC"
    char *proc_ptr = strstr(clean_line, "PROCEDURE");
    if (!proc_ptr) proc_ptr = strstr(clean_line, "PROC");
    char *colon_ptr = strchr(clean_line, ':');

    if (proc_ptr && colon_ptr && colon_ptr < proc_ptr) {
        // Extract name before colon
        int name_len = 0;
        char *p = clean_line;
        while (p < colon_ptr && name_len < 8) {
            if (isalnum((unsigned char)*p)) {
                out_name[name_len++] = toupper((unsigned char)*p);
            }
            p++;
        }
        out_name[name_len] = '\0';
        return 1;
    }

    // Look for SQL Table pattern: "CREATE TABLE <name>"
    char *table_ptr = strstr(clean_line, "CREATE TABLE");
    if (table_ptr) {
        char name_buf[64];
        if (sscanf(table_ptr + 12, "%63s", name_buf) == 1) {
            int name_len = 0;
            char *p = name_buf;
            // Clean table name (remove brackets, backticks, quotes)
            while (*p && name_len < 8) {
                if (isalnum((unsigned char)*p)) {
                    out_name[name_len++] = toupper((unsigned char)*p);
                }
                p++;
            }
            out_name[name_len] = '\0';
            return 1;
        }
    }

    return 0;
}

int main(int argc, char **argv) {
    printf("=== CBTUPD: VIRTUAL MEMBER BOUNDARY CARD INJECTOR ===\n");

    if (argc < 3) {
        fprintf(stderr, "Usage: tsfi_cbtupd_terse <monolithic_input.txt> <output_partitioned.txt>\n");
        return 1;
    }

    const char *input_path = argv[1];
    const char *output_path = argv[2];

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

    char line[256];
    size_t lines_processed = 0;
    size_t cards_injected = 0;

    while (fgets(line, sizeof(line), f_in)) {
        lines_processed++;
        char member_name[16];
        memset(member_name, 0, sizeof(member_name));

        // If line is a logical compiler boundary, inject "./ ADD NAME=" card
        if (extract_boundary_name(line, member_name)) {
            fprintf(f_out, "./ ADD NAME=%s\n", member_name);
            cards_injected++;
            printf("[CBTUPD] Injected boundary card: ./ ADD NAME=%s\n", member_name);
        }
        fputs(line, f_out);
    }

    fclose(f_in);
    fclose(f_out);

    printf("\n=== CBTUPD COMPLETED ===\n");
    printf("[CBTUPD] Total lines processed: %lu\n", lines_processed);
    printf("[CBTUPD] Total cards injected : %lu\n", cards_injected);
    return 0;
}
