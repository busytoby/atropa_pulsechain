#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>

// Basic structures to represent JCL statements
typedef enum {
    JCL_COMMENT,
    JCL_JOB,
    JCL_EXEC,
    JCL_DD,
    JCL_SYSIN,
    JCL_INVALID
} JclType;

typedef struct {
    char name[64];
    JclType type;
    char op[16];
    char params[256];
} JclCard;

// Function to parse a single JCL line
bool parse_jcl_line(const char *line, JclCard *card) {
    if (!line || !card) return false;
    memset(card, 0, sizeof(JclCard));

    // Trim trailing newline/carriage returns
    char clean_line[512];
    strncpy(clean_line, line, sizeof(clean_line) - 1);
    clean_line[sizeof(clean_line) - 1] = '\0';
    size_t len = strlen(clean_line);
    while (len > 0 && (clean_line[len - 1] == '\r' || clean_line[len - 1] == '\n')) {
        clean_line[len - 1] = '\0';
        len--;
    }

    // Must start with '//'
    if (strncmp(clean_line, "//", 2) != 0) {
        card->type = JCL_INVALID;
        return false;
    }

    // Check if it is a comment card: '//*'
    if (clean_line[2] == '*') {
        card->type = JCL_COMMENT;
        return true;
    }

    // Skip leading '//'
    const char *ptr = clean_line + 2;

    // Read Name field (up to next space)
    size_t idx = 0;
    while (*ptr && !isspace((unsigned char)*ptr) && idx < sizeof(card->name) - 1) {
        card->name[idx++] = *ptr++;
    }
    card->name[idx] = '\0';

    // Skip spaces to locate Operation field
    while (*ptr && isspace((unsigned char)*ptr)) {
        ptr++;
    }

    // Read Operation field
    idx = 0;
    while (*ptr && !isspace((unsigned char)*ptr) && idx < sizeof(card->op) - 1) {
        card->op[idx++] = *ptr++;
    }
    card->op[idx] = '\0';

    // Skip spaces to locate Parameters field
    while (*ptr && isspace((unsigned char)*ptr)) {
        ptr++;
    }

    // Read Parameters field
    strncpy(card->params, ptr, sizeof(card->params) - 1);
    card->params[sizeof(card->params) - 1] = '\0';

    // Classify card type based on operation
    if (strcmp(card->op, "JOB") == 0) {
        card->type = JCL_JOB;
    } else if (strcmp(card->op, "EXEC") == 0) {
        card->type = JCL_EXEC;
    } else if (strcmp(card->op, "DD") == 0) {
        card->type = JCL_DD;
    } else if (strcmp(card->name, "SYSIN") == 0 || strcmp(card->op, "*") == 0) {
        card->type = JCL_SYSIN;
    } else {
        card->type = JCL_INVALID;
    }

    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XPLOS JCL SYNTACTIC PARSER TEST SUITE\n");
    printf("=============================================================\n");

    const char *jcl_path = "solidity/domain/jcl/cbt003_load_tape.jcl";
    FILE *f_jcl = fopen(jcl_path, "r");
    if (!f_jcl) {
        printf("[JCL PARSER ERROR] Could not open file: %s\n", jcl_path);
        return 1;
    }

    char line_buf[512];
    int line_num = 0;
    int parsed_cards = 0;
    int parsed_comments = 0;

    printf("[JCL PARSER] Parsing control cards from %s:\n\n", jcl_path);
    while (fgets(line_buf, sizeof(line_buf), f_jcl)) {
        line_num++;
        JclCard card;
        if (parse_jcl_line(line_buf, &card)) {
            if (card.type == JCL_COMMENT) {
                parsed_comments++;
            } else if (card.type != JCL_INVALID) {
                parsed_cards++;
                printf("Line %04d: TYPE=%-5s NAME=%-10s OP=%-5s PARAMS='%s'\n",
                       line_num,
                       (card.type == JCL_JOB) ? "JOB" :
                       (card.type == JCL_EXEC) ? "EXEC" :
                       (card.type == JCL_DD) ? "DD" : "SYSIN",
                       card.name, card.op, card.params);
            } else {
                // If it starts with '//' but operation is blank or invalid, it might be instream data or trailing delimiter
                if (strlen(card.name) == 0 && strlen(card.op) == 0) {
                    printf("Line %04d: Null/Delimiter Card (//)\n", line_num);
                } else {
                    printf("Line %04d: Instream data or delimiter card\n", line_num);
                }
            }
        }
    }
    fclose(f_jcl);

    printf("\n[JCL PARSER SUCCESS] Parsing metrics:\n");
    printf("   - Total Lines Processed  : %d\n", line_num);
    printf("   - Valid Control Cards    : %d\n", parsed_cards);
    printf("   - Comment Cards (//*)    : %d\n", parsed_comments);

    assert(parsed_cards > 0);
    assert(parsed_comments > 0);

    printf("=============================================================\n");
    printf("JCL PARSER TESTS COMPLETED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
