#include "tsfi_zorse_eval.h"
#include <string.h>
#include <stdio.h>

int tsfi_zorse_validate_jcl(const char *jcl_string, int *is_valid_out, char *error_msg_out, int max_err_len) {
    if (!jcl_string || !is_valid_out || !error_msg_out || max_err_len <= 0) return -1;
    
    *is_valid_out = 0;
    error_msg_out[0] = '\0';
    
    // Check JCL Prefix: Must start with comment or statement boundary
    if (strncmp(jcl_string, "//", 2) != 0 && strncmp(jcl_string, "/*", 2) != 0) {
        snprintf(error_msg_out, max_err_len, "Invalid prefix: JCL must start with '//' or '/*'");
        return 0;
    }
    
    // Check for JOB card presence
    if (!strstr(jcl_string, " JOB ")) {
        snprintf(error_msg_out, max_err_len, "Missing mandatory JCL JOB statement card");
        return 0;
    }
    
    *is_valid_out = 1;
    return 0;
}

int tsfi_zorse_validate_cobol(const char *cobol_string, int *is_valid_out) {
    if (!cobol_string || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Validate Division Sequences:
    // A standard COBOL source must contain IDENTIFICATION DIVISION and PROCEDURE DIVISION.
    const char *ident = strstr(cobol_string, "IDENTIFICATION DIVISION.");
    const char *proc = strstr(cobol_string, "PROCEDURE DIVISION.");
    
    if (ident && proc && ident < proc) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_dd(const char *dd_statement, int *is_valid_out) {
    if (!dd_statement || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // DD statement must contain " DD " parameter
    if (!strstr(dd_statement, " DD ")) return 0;
    
    // Check for standard properties: DSN/DSNAME and DISP
    if ((strstr(dd_statement, "DSN=") || strstr(dd_statement, "DSNAME=")) && strstr(dd_statement, "DISP=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_hlasm(const char *hlasm_instruction, int *is_valid_out) {
    if (!hlasm_instruction || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    size_t len = strlen(hlasm_instruction);
    if (len == 0 || hlasm_instruction[0] == '*') {
        // Comment lines are valid HLASM lines
        *is_valid_out = 1;
        return 0;
    }
    
    // Check if any comma is followed by a space (invalid operands spacing)
    const char *p = hlasm_instruction;
    while ((p = strchr(p, ',')) != NULL) {
        if (p[1] == ' ' || p[1] == '\t') {
            return 0;
        }
        p++;
    }
    
    *is_valid_out = 1;
    return 0;
}
