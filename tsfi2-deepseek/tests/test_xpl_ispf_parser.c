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

// 3.5. ISPF Panel default variable loader (INIT section parser)
static bool load_panel_defaults(const char *filepath, const char *var_name, char *val_out, size_t max_val_len) {
    FILE *f = fopen(filepath, "r");
    if (!f) return false;

    char line[128];
    bool in_init = false;
    bool found = false;

    while (fgets(line, sizeof(line), f)) {
        // Strip trailing newline/whitespace
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n' || line[len - 1] == ' ')) {
            line[len - 1] = '\0';
            len--;
        }

        if (strcmp(line, ")INIT") == 0) {
            in_init = true;
            continue;
        }
        if (line[0] == ')' && strcmp(line, ")INIT") != 0) {
            in_init = false;
        }

        if (in_init) {
            char name[64] = {0};
            char val[64] = {0};
            // Match &NAME = 'VAL'
            if (sscanf(line, "  &%63[^ ] = '%63[^']'", name, val) == 2) {
                if (strcmp(name, var_name) == 0) {
                    snprintf(val_out, max_val_len, "%s", val);
                    found = true;
                    break;
                }
            }
        }
    }
    fclose(f);
    return found;
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

// 5. Dynamic Panel Renderer and Dialog Manager
static void render_ispf_panel(
    const char *panel_path,
    const char *msg_lib_path,
    const char *msg_code,
    const char *reg_val,
    const char *val_val,
    const char *tok_val,
    const char *zcmd
) {
    FILE *f = fopen(panel_path, "r");
    if (!f) return;

    char line[128];
    bool in_body = false;
    printf("\n=== ISPF TERMINAL SCREEN START ===\n");

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
            // Replace variables
            char rendered[256] = {0};
            char *src = line;
            char *dst = rendered;
            
            while (*src) {
                if (strncmp(src, "_PANEL_REG", 10) == 0) {
                    dst += sprintf(dst, "%-10s", reg_val);
                    src += 10;
                } else if (strncmp(src, "_PANEL_VAL", 10) == 0) {
                    dst += sprintf(dst, "%-10s", val_val);
                    src += 10;
                } else if (strncmp(src, "_OP_TOKEN", 9) == 0) {
                    dst += sprintf(dst, "%-9s", tok_val);
                    src += 9;
                } else if (strncmp(src, "_ZCMD", 5) == 0) {
                    dst += sprintf(dst, "%-5s", zcmd);
                    src += 5;
                } else {
                    *dst++ = *src++;
                }
            }
            *dst = '\0';
            printf("%s\n", rendered);
        }
    }
    fclose(f);

    if (msg_code && msg_code[0]) {
        ispf_message_t msg;
        if (lookup_ispf_message(msg_lib_path, msg_code, &msg)) {
            printf("\n*** %s: %s ***\n", msg.code, msg.short_text);
            printf("Message Description: %s\n", msg.long_text);
            if (msg.alarm) {
                printf("* BEEP * [System Alarm Activated]\n");
            }
        }
    }
    printf("=== ISPF TERMINAL SCREEN END ===\n\n");
}

// 6. ISPF Dialog option selection and dispatcher
static bool select_ispf_option(const char *option_cmd, char *msg_out) {
    if (strcmp(option_cmd, "1") == 0) {
        // Option 1: Compile task
        return true;
    } else if (strcmp(option_cmd, "X") == 0) {
        // Option X: Exit
        return true;
    } else {
        // Invalid option selected: load TSSO004E message
        snprintf(msg_out, 15, "TSSO004E");
        return false;
    }
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

    // Test 5: Verify Panel Initialization Defaults (ISPPLIB INIT)
    printf("[TEST] Testing panel default variable loading (INIT block)...\n");
    char default_val[64] = {0};
    
    assert(load_panel_defaults(panel_lib, "PANEL_REG", default_val, sizeof(default_val)) == true);
    assert(strcmp(default_val, "1") == 0);

    assert(load_panel_defaults(panel_lib, "PANEL_VAL", default_val, sizeof(default_val)) == true);
    assert(strcmp(default_val, "0") == 0);

    assert(load_panel_defaults(panel_lib, "OP_TOKEN", default_val, sizeof(default_val)) == true);
    assert(strcmp(default_val, "000") == 0);

    assert(load_panel_defaults(panel_lib, "NON_EXIST", default_val, sizeof(default_val)) == false);

    printf("   ✓ Panel default initialization parameters verified successfully.\n");

    // Test 6: Verify Panel Rendering & Message Display
    printf("[TEST] Testing panel dynamic terminal rendering and warning messages...\n");
    // Render normal successful state
    render_ispf_panel(panel_lib, msg_lib, "", "1", "1234", "999", "1");
    // Render error state with alarm
    render_ispf_panel(panel_lib, msg_lib, "TSSO001E", "abc", "1234", "999", "X");

    printf("   ✓ Panel renderer execution verified.\n");

    // Test 7: Verify Panel Option Selection & Dispatch (ISPPLIB ZCMD)
    printf("[TEST] Testing panel command option selection (ZCMD dispatch)...\n");
    char opt_msg[16] = {0};
    
    assert(select_ispf_option("1", opt_msg) == true);
    assert(select_ispf_option("X", opt_msg) == true);
    
    assert(select_ispf_option("INVALID", opt_msg) == false);
    assert(strcmp(opt_msg, "TSSO004E") == 0);

    // Look up and display looked up invalid option message
    render_ispf_panel(panel_lib, msg_lib, opt_msg, "1", "0", "000", "BAD");

    printf("   ✓ Panel option dispatcher verified successfully.\n");

    printf("=============================================================\n");
    printf("ALL ISPF LIBRARIES TESTS COMPLETED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
