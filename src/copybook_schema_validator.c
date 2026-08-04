#define _DEFAULT_SOURCE
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    int level;
    char name[64];
    char picture[64];
} CopybookField;

// Parse a copybook line like "05  ISBN-VAL   PIC X(13)."
bool parse_copybook_line(const char *line, CopybookField *field) {
    if (!line || !field) return false;
    memset(field, 0, sizeof(CopybookField));

    // Simple tokenization
    char buf[256];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tok1 = strtok(buf, " \t.");
    if (!tok1) return false;
    field->level = atoi(tok1);
    if (field->level == 0) return false;

    char *tok2 = strtok(NULL, " \t.");
    if (!tok2) return false;
    strncpy(field->name, tok2, sizeof(field->name) - 1);

    char *tok3 = strtok(NULL, " \t.");
    if (tok3 && strcmp(tok3, "PIC") == 0) {
        char *tok4 = strtok(NULL, " \t.");
        if (tok4) {
            strncpy(field->picture, tok4, sizeof(field->picture) - 1);
        }
    }

    return true;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <copybook_file> <tsv_data_file>\n", argv[0]);
        return 1;
    }

    FILE *cf = fopen(argv[1], "r");
    if (!cf) {
        perror("Failed to open copybook file");
        return 1;
    }

    CopybookField fields[64];
    int field_count = 0;

    char line[256];
    while (fgets(line, sizeof(line), cf) && field_count < 64) {
        if (parse_copybook_line(line, &fields[field_count])) {
            field_count++;
        }
    }
    fclose(cf);

    printf("[Validator] Loaded %d copybook fields from %s\n", field_count, argv[1]);

    FILE *df = fopen(argv[2], "r");
    if (!df) {
        perror("Failed to open data file");
        return 1;
    }

    int line_num = 1;
    char data_line[4096];
    bool all_valid = true;

    while (fgets(data_line, sizeof(data_line), df)) {
        // Strip newline
        size_t len = strlen(data_line);
        while (len > 0 && (data_line[len - 1] == '\n' || data_line[len - 1] == '\r')) {
            data_line[len - 1] = '\0';
            len--;
        }

        // Tokenize and count columns
        char *curr = data_line;
        char *token;
        int col_index = 0;
        while ((token = strsep(&curr, "\t")) != NULL) {
            if (col_index < field_count) {
                // Validate if field picture specifies alphanumeric length
                const char *pic = fields[col_index].picture;
                if (strncmp(pic, "X(", 2) == 0) {
                    int max_len = atoi(pic + 2);
                    if ((int)strlen(token) > max_len) {
                        printf("[SCHEMA ERROR] Line %d, Column %d (%s) exceeds copybook size %d (Value: '%s')\n",
                               line_num, col_index + 1, fields[col_index].name, max_len, token);
                        all_valid = false;
                    }
                }
            }
            col_index++;
        }

        if (col_index != field_count) {
            printf("[SCHEMA WARNING] Line %d contains %d columns, copybook expects %d fields.\n",
                   line_num, col_index, field_count);
            all_valid = false;
        }

        line_num++;
    }

    fclose(df);

    if (all_valid) {
        printf("[Validator] Verification SUCCESS: All data matches copybook specifications.\n");
        return 0;
    } else {
        printf("[Validator] Verification FAILED due to schema mismatches.\n");
        return 1;
    }
}
