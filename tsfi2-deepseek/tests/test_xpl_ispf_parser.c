#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>

// Struct representing MVS ISPF Message Lookup
typedef struct {
    char code[16];
    char short_text[64];
    char long_text[128];
    bool alarm;
} ispf_message_t;

// Struct representing Operator entry in ISPF Table
typedef struct {
    uint32_t op_id;
    uint32_t clearance;
    char status[16];
} ispf_operator_t;

// 1. ISPF Message Parser Utility
static bool lookup_ispf_message(const char *filepath, const char *msg_code, ispf_message_t *out) {
    FILE *f = fopen(filepath, "r");
    if (!f) return false;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, msg_code, strlen(msg_code)) == 0) {
            snprintf(out->code, sizeof(out->code), "%s", msg_code);
            
            // Extract short text and alarm
            char alarm_str[8] = {0};
            int scanned = sscanf(line + strlen(msg_code), " '%63[^']' .ALARM=%7s", out->short_text, alarm_str);
            if (scanned >= 1) {
                out->alarm = (strcmp(alarm_str, "YES") == 0);
            }
            
            // Read next line for long text description
            if (fgets(line, sizeof(line), f)) {
                sscanf(line, "'%127[^']'", out->long_text);
            }
            fclose(f);
            return true;
        }
    }
    fclose(f);
    return false;
}

// 2. ISPF Table Operator Parser Utility
static bool lookup_ispf_operator(const char *filepath, uint32_t op_id, ispf_operator_t *out) {
    FILE *f = fopen(filepath, "r");
    if (!f) return false;

    char line[128];
    // Skip header line
    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        return false;
    }

    while (fgets(line, sizeof(line), f)) {
        uint32_t id = 0, clear = 0;
        char stat[32] = {0};
        if (sscanf(line, "%u %u %31s", &id, &clear, stat) == 3) {
            if (id == op_id) {
                out->op_id = id;
                out->clearance = clear;
                snprintf(out->status, sizeof(out->status), "%s", stat);
                fclose(f);
                return true;
            }
        }
    }
    fclose(f);
    return false;
}

// 3. ISPF Skeleton dynamic JCL Generator
static bool generate_xcom_from_skeleton(
    const char *skel_path,
    const char *out_path,
    uint32_t val,
    int a0, int a1, int a2, int a3
) {
    FILE *in = fopen(skel_path, "r");
    if (!in) return false;

    FILE *out = fopen(out_path, "w");
    if (!out) {
        fclose(in);
        return false;
    }

    char line[256];
    while (fgets(line, sizeof(line), in)) {
        // Simple token expansion for skeleton: replace &PANEL_VAL and &A0..&A3
        if (strstr(line, "WRITE_ABD")) {
            fprintf(out, "WRITE_ABD %u %d %d %d %d\n", val, a0, a1, a2, a3);
        } else {
            fputs(line, out);
        }
    }

    fclose(in);
    fclose(out);
    return true;
}

// 4. ISPF Panel input validation rule checker (VER rule parser)
static bool validate_panel_field_rules(const char *filepath, const char *field_name, const char *value_str, char *msg_out) {
    FILE *f = fopen(filepath, "r");
    if (!f) return false;

    char line[128];
    bool in_proc = false;
    bool checked = false;
    bool passed = true;

    while (fgets(line, sizeof(line), f)) {
        // Strip trailing newline/whitespace
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n' || line[len - 1] == ' ')) {
            line[len - 1] = '\0';
            len--;
        }

        if (strcmp(line, ")PROC") == 0) {
            in_proc = true;
            continue;
        }
        if (line[0] == ')') {
            in_proc = false;
        }

        if (in_proc) {
            char name[64] = {0};
            char type[32] = {0};
            char msg[16] = {0};
            // Match VER (&NAME,TYPE,MSG=CODE) or VER (&NAME,TYPE)
            if (sscanf(line, "  VER (&%63[^,],%31[^,],MSG=%15[^)])", name, type, msg) == 3 ||
                sscanf(line, "  VER (&%63[^,],%31[^)])", name, type) == 2) {
                if (strcmp(name, field_name) == 0) {
                    checked = true;
                    if (strcmp(type, "NUM") == 0) {
                        // Check if numeric
                        for (size_t i = 0; i < strlen(value_str); i++) {
                            if (value_str[i] < '0' || value_str[i] > '9') {
                                passed = false;
                                if (msg_out && msg[0]) {
                                    snprintf(msg_out, 15, "%s", msg);
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    fclose(f);
    return checked ? passed : true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ISPF FRONTEHD COMPILER LIBRARIES VALIDATION SUITE\n");
    printf("=============================================================\n");

    const char *msg_lib = "ispf_libraries/ispmlib/errors.ispmlib";
    const char *tbl_lib = "ispf_libraries/isptlib/operators.isptlib";
    const char *skel_lib = "ispf_libraries/ispslib/task_skeleton.ispslib";
    const char *output_jcl = "ispf_libraries/task_compiled.xpl";

    // Test 1: Verify Message Library Parsers (ISPMLIB)
    printf("[TEST] Parsing and verifying ISPF message notifications...\n");
    ispf_message_t msg;
    assert(lookup_ispf_message(msg_lib, "TSSO001E", &msg) == true);
    assert(strcmp(msg.short_text, "TSSO Opcode Violation") == 0);
    assert(msg.alarm == true);
    assert(strstr(msg.long_text, "RED rail activated") != NULL);

    assert(lookup_ispf_message(msg_lib, "TSSO003I", &msg) == true);
    assert(strcmp(msg.short_text, "TSSO Command Executed") == 0);
    assert(msg.alarm == false);

    printf("   ✓ Message lookup and parsed attributes validated successfully.\n");

    // Test 2: Verify Operator Table Database (ISPTLIB)
    printf("[TEST] Querying operator clearance credentials from table...\n");
    ispf_operator_t op;
    assert(lookup_ispf_operator(tbl_lib, 999, &op) == true);
    assert(op.clearance == 3);
    assert(strcmp(op.status, "ACTIVE") == 0);

    assert(lookup_ispf_operator(tbl_lib, 555, &op) == true);
    assert(op.clearance == 2);
    assert(strcmp(op.status, "SUSPENDED") == 0);

    assert(lookup_ispf_operator(tbl_lib, 222, &op) == false); // Non-existent

    printf("   ✓ Operator credentials and status limits validated successfully.\n");

    // Test 3: Dynamic JCL generation from Skeleton (ISPSLIB)
    printf("[TEST] Compiling skeleton dynamic JCL output via panel inputs...\n");
    assert(generate_xcom_from_skeleton(skel_lib, output_jcl, 8585, 1, 0, 1, 0) == true);

    // Verify generated JCL file output
    FILE *verify = fopen(output_jcl, "r");
    assert(verify != NULL);
    char line[128];
    bool found = false;
    while (fgets(line, sizeof(line), verify)) {
        if (strncmp(line, "WRITE_ABD", 9) == 0) {
            assert(strcmp(line, "WRITE_ABD 8585 1 0 1 0\n") == 0);
            found = true;
            break;
        }
    }
    assert(found == true);
    fclose(verify);
    remove(output_jcl);

    printf("   ✓ Dynamic JCL generation and variable substitution verified.\n");

    // Test 4: Verify Panel Input Validation Rules (ISPPLIB)
    printf("[TEST] Testing panel input validation rules (VER checks)...\n");
    const char *panel_lib = "ispf_libraries/ispplib/submit_task.ispplib";
    char err_msg[16] = {0};
    
    // Numeric checks on PANEL_REG, PANEL_VAL, and OP_TOKEN
    assert(validate_panel_field_rules(panel_lib, "PANEL_REG", "123", err_msg) == true);
    
    err_msg[0] = '\0';
    assert(validate_panel_field_rules(panel_lib, "PANEL_REG", "abc", err_msg) == false);
    assert(strcmp(err_msg, "TSSO001E") == 0);

    assert(validate_panel_field_rules(panel_lib, "PANEL_VAL", "9999", err_msg) == true);
    
    err_msg[0] = '\0';
    assert(validate_panel_field_rules(panel_lib, "PANEL_VAL", "12a34", err_msg) == false);
    assert(strcmp(err_msg, "TSSO001E") == 0);

    assert(validate_panel_field_rules(panel_lib, "OP_TOKEN", "999", err_msg) == true);
    
    err_msg[0] = '\0';
    assert(validate_panel_field_rules(panel_lib, "OP_TOKEN", "99a", err_msg) == false);
    assert(strcmp(err_msg, "TSSO002E") == 0);
    
    printf("   ✓ Panel validation rule processor and MSG mapping verified successfully.\n");

    printf("=============================================================\n");
    printf("ALL ISPF LIBRARIES TESTS COMPLETED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
