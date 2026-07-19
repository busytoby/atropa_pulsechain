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

int tsfi_zorse_validate_cobol_redefines(const char *redefines_clause, int *is_valid_out) {
    if (!redefines_clause || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Must contain " REDEFINES "
    const char *p = strstr(redefines_clause, " REDEFINES ");
    if (!p) return 0;
    
    p += 11; // Skip " REDEFINES "
    
    // Check that it's followed by a valid identifier (alphanumeric, -, or _) and ends with a period
    int id_len = 0;
    while (*p != '\0' && *p != ' ' && *p != '\n' && *p != '.') {
        char c = *p;
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_')) {
            return 0;
        }
        id_len++;
        p++;
    }
    
    if (id_len >= 1 && *p == '.') {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_symbolic(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Locate variable declaration or reference starter symbol '&'
    const char *p = strchr(jcl_line, '&');
    if (!p) return 0;
    
    p++; // Skip '&'
    
    // Variable name must start with alphabetical character
    char first = *p;
    if (!((first >= 'A' && first <= 'Z') || (first >= 'a' && first <= 'z'))) {
        return 0;
    }
    
    int var_len = 1;
    p++;
    while (*p != '\0' && *p != ' ' && *p != ',' && *p != '=' && *p != '.' && *p != '\'' && *p != '\n') {
        char c = *p;
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
            return 0;
        }
        var_len++;
        p++;
    }
    
    if (var_len <= 8) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_occurs(const char *occurs_clause, int *is_valid_out) {
    if (!occurs_clause || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Must contain " OCCURS "
    const char *p = strstr(occurs_clause, " OCCURS ");
    if (!p) return 0;
    
    p += 8; // Skip " OCCURS "
    
    // Verify it is followed by a number and optionally "TIMES" ending with period
    int occurs_limit = atoi(p);
    if (occurs_limit <= 0 || occurs_limit > 32767) {
        return 0; // Exceeds standard mainframe limit sizing bounds
    }
    
    *is_valid_out = 1;
    return 0;
}

int tsfi_zorse_validate_jcl_cond(const char *cond_parameter, int *is_valid_out) {
    if (!cond_parameter || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Find "COND="
    const char *p = strstr(cond_parameter, "COND=");
    if (!p) return 0;
    
    p += 5; // Skip "COND="
    
    // Check if it starts with parenthesized comparison block, e.g. COND=(4,LT)
    if (*p == '(') {
        p++;
        // Find digit return code
        int code = atoi(p);
        if (code < 0 || code > 4095) return 0;
        
        // Find comma
        p = strchr(p, ',');
        if (!p) return 0;
        p++; // Skip ','
        
        // Match operators: EQ, NE, GT, LT, GE, LE
        if (strncmp(p, "EQ", 2) == 0 || strncmp(p, "NE", 2) == 0 ||
            strncmp(p, "GT", 2) == 0 || strncmp(p, "LT", 2) == 0 ||
            strncmp(p, "GE", 2) == 0 || strncmp(p, "LE", 2) == 0) {
            *is_valid_out = 1;
        }
    }
    
    return 0;
}

int tsfi_zorse_autocorrect_source(const char *failed_source, const char *lang, const char *model_name, char *corrected_source_out, size_t max_len) {
    if (!failed_source || !lang || !model_name || !corrected_source_out || max_len == 0) return -1;
    
    corrected_source_out[0] = '\0';
    
    // Construct prompt
    size_t f_len = strlen(failed_source);
    size_t l_len = strlen(lang);
    char *prompt = (char *)malloc(f_len + l_len + 128);
    if (!prompt) return -1;
    
    snprintf(prompt, f_len + l_len + 128, 
             "The following %s code has a syntax error. Please output only the corrected code block: %s", 
             lang, failed_source);
    
    int ret = tsfi_zorse_query_llm(prompt, model_name, corrected_source_out, max_len);
    free(prompt);
    
    return ret;
}

int tsfi_zorse_resolve_dependencies(const char *cobol_src, const char *jcl_src, const char *model_name, char *mapping_out, size_t max_len) {
    if (!cobol_src || !jcl_src || !model_name || !mapping_out || max_len == 0) return -1;
    
    mapping_out[0] = '\0';
    
    // Construct prompt
    size_t c_len = strlen(cobol_src);
    size_t j_len = strlen(jcl_src);
    char *prompt = (char *)malloc(c_len + j_len + 256);
    if (!prompt) return -1;
    
    snprintf(prompt, c_len + j_len + 256, 
             "Given this COBOL code: %s and this JCL code: %s, map SELECT ASSIGN variables to JCL DD names. Format: VAR->DD.", 
             cobol_src, jcl_src);
    
    int ret = tsfi_zorse_query_llm(prompt, model_name, mapping_out, max_len);
    free(prompt);
    
    return ret;
}

int tsfi_zorse_audit_screen_visual(const char *b64_screen_img, const char *model_name, char *alert_level_out, size_t max_len) {
    if (!b64_screen_img || !model_name || !alert_level_out || max_len == 0) return -1;
    
    alert_level_out[0] = '\0';
    
    const char *prompt = "Is there an active SYSTEM DEFCON ALERT or security error visible in this terminal screen capture? If yes, output the DEFCON level number.";
    
    // We reuse tsfi_ai_evaluate_vlm which is specifically tailored for Ollama/SD VLM calls.
    // However, if the target VLM is Ollama's moondream, we call the public function:
    return tsfi_ai_evaluate_vlm(b64_screen_img, prompt, alert_level_out, max_len);
}

int tsfi_zorse_transpile_cobol_to_hlasm(const char *cobol_src, const char *model_name, char *hlasm_out, size_t max_len) {
    if (!cobol_src || !model_name || !hlasm_out || max_len == 0) return -1;
    
    hlasm_out[0] = '\0';
    
    size_t c_len = strlen(cobol_src);
    char *prompt = (char *)malloc(c_len + 128);
    if (!prompt) return -1;
    
    snprintf(prompt, c_len + 128, 
             "Translate this COBOL block into equivalent IBM HLASM instructions. Output only the HLASM instructions: %s", 
             cobol_src);
    
    int ret = tsfi_zorse_query_llm(prompt, model_name, hlasm_out, max_len);
    free(prompt);
    
    return ret;
}

int tsfi_zorse_generate_jcl_from_flowchart(const char *b64_flowchart_img, const char *model_name, char *jcl_out, size_t max_len) {
    if (!b64_flowchart_img || !model_name || !jcl_out || max_len == 0) return -1;
    
    jcl_out[0] = '\0';
    
    const char *prompt = "Extract the program execution sequence from this flowchart image and write a compliant JCL job stream with EXEC cards.";
    
    return tsfi_ai_evaluate_vlm(b64_flowchart_img, prompt, jcl_out, max_len);
}

int tsfi_zorse_transpile_hlasm_to_cobol(const char *hlasm_src, const char *model_name, char *cobol_out, size_t max_len) {
    if (!hlasm_src || !model_name || !cobol_out || max_len == 0) return -1;
    
    cobol_out[0] = '\0';
    
    size_t h_len = strlen(hlasm_src);
    char *prompt = (char *)malloc(h_len + 128);
    if (!prompt) return -1;
    
    snprintf(prompt, h_len + 128, 
             "Translate this HLASM assembly block into equivalent COBOL divisions and statements. Output only COBOL code: %s", 
             hlasm_src);
    
    int ret = tsfi_zorse_query_llm(prompt, model_name, cobol_out, max_len);
    free(prompt);
    
    return ret;
}

int tsfi_zorse_map_dasd_space(const char *b64_layout_img, const char *model_name, char *space_out, size_t max_len) {
    if (!b64_layout_img || !model_name || !space_out || max_len == 0) return -1;
    
    space_out[0] = '\0';
    
    const char *prompt = "Analyze this DASD cylinder volume layout diagram and output the corresponding JCL SPACE allocation parameter, e.g. SPACE=(CYL,(10,5)).";
    
    return tsfi_ai_evaluate_vlm(b64_layout_img, prompt, space_out, max_len);
}

int tsfi_zorse_explain_source(const char *source, const char *lang, const char *model_name, char *explanation_out, size_t max_len) {
    if (!source || !lang || !model_name || !explanation_out || max_len == 0) return -1;
    
    explanation_out[0] = '\0';
    
    size_t s_len = strlen(source);
    size_t l_len = strlen(lang);
    char *prompt = (char *)malloc(s_len + l_len + 128);
    if (!prompt) return -1;
    
    snprintf(prompt, s_len + l_len + 128, 
             "Explain the logic, variables, and execution steps of the following %s source block in detail: %s", 
             lang, source);
    
    int ret = tsfi_zorse_query_llm(prompt, model_name, explanation_out, max_len);
    free(prompt);
    
    return ret;
}
