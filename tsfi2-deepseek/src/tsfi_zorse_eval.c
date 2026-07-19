#include "tsfi_zorse_eval.h"
#include "tsfi_ai_core.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

int tsfi_zorse_query_llm(const char *prompt, const char *model_name, char *response_out, size_t max_resp_len) {
    if (!prompt || !model_name || !response_out || max_resp_len == 0) return -1;
    
    response_out[0] = '\0';
    
    size_t m_len = strlen(model_name);
    size_t p_len = strlen(prompt);
    char *json_payload = (char *)malloc(m_len + p_len + 256);
    if (!json_payload) return -1;
    
    snprintf(json_payload, m_len + p_len + 256, 
             "{\"model\": \"%s\", \"prompt\": \"%s\", \"stream\": false}", 
             model_name, prompt);
    
    // Query local Ollama service port (11434)
    char *resp = tsfi_ai_exec_post("127.0.0.1", "11434", "/api/generate", json_payload);
    free(json_payload);
    
    if (!resp) return -2;
    
    char *body = strstr(resp, "\r\n\r\n");
    int ret = -3;
    if (body) {
        char *resp_start = strstr(body, "\"response\":\"");
        if (resp_start) {
            resp_start += 12;
            char *resp_end = resp_start;
            while (*resp_end != '\0') {
                if (*resp_end == '"' && *(resp_end - 1) != '\\') break;
                resp_end++;
            }
            
            if (*resp_end == '"') {
                size_t len = resp_end - resp_start;
                if (len >= max_resp_len) len = max_resp_len - 1;
                strncpy(response_out, resp_start, len);
                response_out[len] = '\0';
                ret = 0;
            }
        }
    }
    
    free(resp);
    return ret;
}

int tsfi_zorse_validate_cobol_pic(const char *pic_clause, int *is_valid_out) {
    if (!pic_clause || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // PICTURE clause must contain "PIC " or "PICTURE "
    const char *p = strstr(pic_clause, "PIC ");
    if (!p) p = strstr(pic_clause, "PICTURE ");
    if (!p) return 0;
    
    // Skip to the pattern definition
    if (strstr(p, " X") || strstr(p, " 9") || strstr(p, " S9") || strstr(p, " A")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_pgm(const char *exec_statement, int *is_valid_out) {
    if (!exec_statement || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Statement must contain "EXEC PGM="
    const char *p = strstr(exec_statement, "EXEC PGM=");
    if (!p) return 0;
    
    p += 9; // Skip "EXEC PGM="
    
    // Parse the program name length and verify it is alphanumeric
    int pgm_len = 0;
    while (*p != '\0' && *p != ' ' && *p != ',' && *p != '\n') {
        char c = *p;
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
            return 0; // Non-alphanumeric character found
        }
        pgm_len++;
        p++;
    }
    
    if (pgm_len >= 1 && pgm_len <= 8) {
        *is_valid_out = 1;
    }
    
    return 0;
}
