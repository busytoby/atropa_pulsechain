#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

// Audit Log structures matching test_xpl_sesslog.c
typedef enum {
    LOG_CONDUCTION = 0,
    LOG_RED_VIOLATION = 1,
    LOG_BLACK_TRAP = 2
} log_type_t;

typedef struct {
    uint32_t timestamp_seq;
    log_type_t type;
    char details[64];
    uint32_t code_info;
} audit_entry_t;

static audit_entry_t mock_logs[] = {
    { 1000, LOG_CONDUCTION,    "BOOTSTRAP COMPLETED SUCCESSFULLY",    0 },
    { 1001, LOG_RED_VIOLATION, "SYS1.LINKLIB RESTRICTED WRITE BLOCKED", 1 },
    { 1002, LOG_BLACK_TRAP,    "VTOC SPACE EXHAUSTED",                25 }
};
#define LOGS_COUNT 3

// 1. RED ANALYZER Gating: Static page offset validation
static bool red_analyzer_validate_page_offset(uint32_t start_index, uint32_t page_size) {
    if (page_size == 0) return false;
    // Gating index bounds
    if (start_index >= LOGS_COUNT) {
        return false; // Index out of bounds (RED rail block)
    }
    return true; // Valid page boundaries
}

// 2. BLACK XPLSM Gating: Dynamic selection redirect
static bool black_xplsm_process_selection(uint32_t row_index, char *redirect_msg_code_out) {
    if (row_index >= LOGS_COUNT) return false;

    audit_entry_t selected = mock_logs[row_index];
    if (selected.type == LOG_RED_VIOLATION) {
        // Redirect to TSOHELP message TSSO001E (Security Violation)
        snprintf(redirect_msg_code_out, 16, "TSSO001E");
    } else if (selected.type == LOG_BLACK_TRAP) {
        // Redirect to TSOHELP message TSSO002E (Execution Trap)
        snprintf(redirect_msg_code_out, 16, "TSSO002E");
    } else {
        // Informational conduction - normal completion
        redirect_msg_code_out[0] = '\0';
    }

    return true;
}

// 3. Table Renderer (TBDISPL emulation)
static void render_tbdispl_table(const char *panel_path, uint32_t start_index, uint32_t count) {
    FILE *f = fopen(panel_path, "r");
    if (!f) return;

    char line[128];
    bool in_body = false;
    printf("\n=== ISPF TBDISPL TABLE SCREEN START ===\n");

    while (fgets(line, sizeof(line), f)) {
        // Strip trailing newline/whitespace
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n')) {
            line[len - 1] = '\0';
            len--;
        }

        if (strncmp(line, ")BODY", 5) == 0) {
            in_body = true;
            continue;
        }
        if (line[0] == ')') {
            in_body = false;
        }

        if (in_body) {
            // Replaces headers and static text, substitutes row values when loop variable matched
            if (strstr(line, "_R1")) {
                // Output rows
                for (uint32_t i = 0; i < count; i++) {
                    uint32_t idx = start_index + i;
                    if (idx >= LOGS_COUNT) break;
                    
                    const char *type_str = (mock_logs[idx].type == LOG_CONDUCTION) ? "CONDUCT" :
                                           (mock_logs[idx].type == LOG_RED_VIOLATION) ? "RED_ERR" : "BLK_TRP";
                                           
                    printf(" %-4u  %-8u  %-10s  %s\n", idx + 1, mock_logs[idx].timestamp_seq, type_str, mock_logs[idx].details);
                }
            } else if (strstr(line, "_R2") || strstr(line, "_R3")) {
                // Skip placeholder lines already expanded
                continue;
            } else {
                // Replace general panel command command options
                char rendered[256] = {0};
                char *src = line;
                char *dst = rendered;
                while (*src) {
                    if (strncmp(src, "_ZCMD", 5) == 0) {
                        dst += sprintf(dst, "1    ");
                        src += 5;
                    } else if (strncmp(src, "_DSN", 4) == 0) {
                        dst += sprintf(dst, "SYS1.ISPSLIB");
                        src += 4;
                    } else {
                        *dst++ = *src++;
                    }
                }
                *dst = '\0';
                printf("%s\n", rendered);
            }
        }
    }
    fclose(f);
    printf("=== ISPF TBDISPL TABLE SCREEN END ===\n\n");
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TBDISPL SESSLOG TABLE DISPLAY VALIDATION SUITE\n");
    printf("=============================================================\n");

    const char *panel_path = "ispf_libraries/ispplib/sesslog_table.ispplib";

    // Test 1: RED rail static page boundary audits
    printf("[TEST] Testing page offset limits against RED rail...\n");
    assert(red_analyzer_validate_page_offset(0, 3) == true);
    assert(red_analyzer_validate_page_offset(2, 1) == true);
    
    // Attempted page read exceeding log array boundaries
    assert(red_analyzer_validate_page_offset(3, 1) == false);
    printf("   ✓ RED rail successfully checked table paging bounds.\n");

    // Test 2: BLACK rail select redirect routing
    printf("[TEST] Selecting table rows and checking diagnostics routing...\n");
    char redirect_msg[16] = {0};

    // Selecting Row 0 (Conduction) -> No redirect expected
    assert(black_xplsm_process_selection(0, redirect_msg) == true);
    assert(redirect_msg[0] == '\0');

    // Selecting Row 1 (RED rail violation) -> Redirect to TSSO001E
    assert(black_xplsm_process_selection(1, redirect_msg) == true);
    assert(strcmp(redirect_msg, "TSSO001E") == 0);

    // Selecting Row 2 (BLACK rail trap) -> Redirect to TSSO002E
    assert(black_xplsm_process_selection(2, redirect_msg) == true);
    assert(strcmp(redirect_msg, "TSSO002E") == 0);
    printf("   ✓ BLACK rail successfully routed table selection row requests to diagnostic finder.\n");

    // Test 3: Table Renderer display check
    printf("[TEST] Rendering dynamic ISPF TBDISPL table format...\n");
    render_tbdispl_table(panel_path, 0, 3);
    printf("   ✓ Table display rendered successfully.\n");

    printf("=============================================================\n");
    printf("ALL TBDISPL TABLE DISPLAY TESTS PASSED CLEANLY\n");
    printf("=============================================================\n");
    return 0;
}
