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

int tsfi_zorse_read_punch_card(const char *b64_card_img, const char *model_name, char *text_out, size_t max_len) {
    if (!b64_card_img || !model_name || !text_out || max_len == 0) return -1;
    
    text_out[0] = '\0';
    
    const char *prompt = "Read the punched hole patterns in this 80-column IBM punch card image and output the decoded alphanumeric text line.";
    
    return tsfi_ai_evaluate_vlm(b64_card_img, prompt, text_out, max_len);
}

int tsfi_zorse_audit_tape_mount(const char *b64_tape_img, const char *expected_tape_id, const char *model_name, int *is_mounted_out) {
    if (!b64_tape_img || !expected_tape_id || !model_name || !is_mounted_out) return -1;
    
    *is_mounted_out = 0;
    
    char ans[64];
    size_t e_len = strlen(expected_tape_id);
    char *prompt = (char *)malloc(e_len + 128);
    if (!prompt) return -1;
    
    snprintf(prompt, e_len + 128, 
             "Is the tape reel labeled with the ID %s correctly mounted on the tape drive unit in this image? Answer only Yes or No.", 
             expected_tape_id);
             
    int ret = tsfi_ai_evaluate_vlm(b64_tape_img, prompt, ans, sizeof(ans));
    free(prompt);
    
    if (ret == 0) {
        if (strstr(ans, "Yes") || strstr(ans, "yes") || strstr(ans, "YES")) {
            *is_mounted_out = 1;
        }
    }
    
    return ret;
}

int tsfi_zorse_parse_cabling_topology(const char *b64_topology_img, const char *model_name, char *sna_config_out, size_t max_len) {
    if (!b64_topology_img || !model_name || !sna_config_out || max_len == 0) return -1;
    
    sna_config_out[0] = '\0';
    
    const char *prompt = "Extract the node clustering layout and controller paths from this cabling topology diagram and generate corresponding SNA network definition statements.";
    
    return tsfi_ai_evaluate_vlm(b64_topology_img, prompt, sna_config_out, max_len);
}

int tsfi_zorse_audit_thermal_graph(const char *b64_thermal_img, const char *model_name, char *hotspot_info_out, size_t max_len) {
    if (!b64_thermal_img || !model_name || !hotspot_info_out || max_len == 0) return -1;
    
    hotspot_info_out[0] = '\0';
    
    const char *prompt = "Audit this cabinet thermal thermogram image to find overheating hot spot components. Output the coordinates or component labels.";
    
    return tsfi_ai_evaluate_vlm(b64_thermal_img, prompt, hotspot_info_out, max_len);
}

int tsfi_zorse_audit_job_stream(const char *jcl_source, const char *cobol_source, const char *model_name, int *is_valid_out, char *report_out, size_t max_report_len) {
    if (!jcl_source || !cobol_source || !model_name || !is_valid_out || !report_out || max_report_len == 0) return -1;
    
    *is_valid_out = 0;
    report_out[0] = '\0';
    
    int jcl_ok = 0;
    char jcl_err[128] = {0};
    int r = tsfi_zorse_validate_jcl(jcl_source, &jcl_ok, jcl_err, sizeof(jcl_err));
    if (r < 0) return -2;
    
    int cobol_ok = 0;
    r = tsfi_zorse_validate_cobol(cobol_source, &cobol_ok);
    if (r < 0) return -3;
    
    if (jcl_ok && cobol_ok) {
        *is_valid_out = 1;
    }
    
    snprintf(report_out, max_report_len, 
             "Zorse Audit Report:\n- JCL Validation: %s%s%s\n- COBOL Validation: %s\n- Overall Job Stream Status: %s\n",
             jcl_ok ? "PASS" : "FAIL",
             jcl_ok ? "" : " (",
             jcl_ok ? "" : jcl_err,
             cobol_ok ? "PASS" : "FAIL",
             (*is_valid_out) ? "APPROVED" : "REJECTED");
             
    return 0;
}

int tsfi_zorse_transpile_cobol_to_c(const char *cobol_data_div, const char *model_name, char *c_struct_out, size_t max_len) {
    if (!cobol_data_div || !model_name || !c_struct_out || max_len == 0) return -1;
    
    c_struct_out[0] = '\0';
    
    size_t c_len = strlen(cobol_data_div);
    char *prompt = (char *)malloc(c_len + 128);
    if (!prompt) return -1;
    
    snprintf(prompt, c_len + 128, 
             "Translate this COBOL DATA DIVISION variable layout into a binary-equivalent C struct. Preserve exact byte offsets: %s", 
             cobol_data_div);
             
    int ret = tsfi_zorse_query_llm(prompt, model_name, c_struct_out, max_len);
    free(prompt);
    
    return ret;
}

int tsfi_zorse_audit_sna_session(const char *sna_bind_hex, int *is_valid_out) {
    if (!sna_bind_hex || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Check for standard SNA session request unit prefixes like "31" or "01" (bind sequence hex boundaries)
    if (strncmp(sna_bind_hex, "31", 2) == 0 || strncmp(sna_bind_hex, "01", 2) == 0) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_audit_fan_spectrogram(const char *b64_spectrogram_img, const char *model_name, char *audit_result_out, size_t max_len) {
    if (!b64_spectrogram_img || !model_name || !audit_result_out || max_len == 0) return -1;
    
    audit_result_out[0] = '\0';
    
    const char *prompt = "Analyze this fan acoustic spectrogram image to identify bearing failures or operational speed anomalies.";
    
    return tsfi_ai_evaluate_vlm(b64_spectrogram_img, prompt, audit_result_out, max_len);
}

int tsfi_zorse_optimize_jcl_space(const char *cobol_src, const char *model_name, char *space_opt_out, size_t max_len) {
    if (!cobol_src || !model_name || !space_opt_out || max_len == 0) return -1;
    
    space_opt_out[0] = '\0';
    
    size_t c_len = strlen(cobol_src);
    char *prompt = (char *)malloc(c_len + 128);
    if (!prompt) return -1;
    
    snprintf(prompt, c_len + 128, 
             "Based on the variable sizes in this COBOL code, calculate and optimize the JCL SPACE allocation parameter: %s", 
             cobol_src);
             
    int ret = tsfi_zorse_query_llm(prompt, model_name, space_opt_out, max_len);
    free(prompt);
    
    return ret;
}

int tsfi_zorse_resolve_copybooks(const char *cobol_src, char *dependencies_out, size_t max_len) {
    if (!cobol_src || !dependencies_out || max_len == 0) return -1;
    
    dependencies_out[0] = '\0';
    
    const char *p = strstr(cobol_src, "COPY ");
    if (!p) return 0;
    
    p += 5; // Skip "COPY "
    
    // Extract copybook name (alphanumeric, -, or _)
    size_t len = 0;
    while (*p != '\0' && *p != ' ' && *p != '.' && *p != '\n') {
        if (len < max_len - 1) {
            dependencies_out[len] = *p;
            len++;
        }
        p++;
    }
    dependencies_out[len] = '\0';
    
    return 0;
}

int tsfi_zorse_validate_jcl_disp(const char *dd_statement, int *is_valid_out) {
    if (!dd_statement || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Check if statement contains DISP=
    const char *p = strstr(dd_statement, "DISP=");
    if (!p) return 0;
    
    p += 5; // Skip "DISP="
    
    // Match disposition status operators
    if (strncmp(p, "NEW", 3) == 0 || strncmp(p, "OLD", 3) == 0 ||
        strncmp(p, "SHR", 3) == 0 || strncmp(p, "MOD", 3) == 0 ||
        strncmp(p, "CATLG", 5) == 0 || strncmp(p, "KEEP", 4) == 0 ||
        strncmp(p, "DELETE", 6) == 0 || strncmp(p, "PASS", 4) == 0 ||
        *p == '(') {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_hlasm_macro(const char *macro_src, int *is_valid_out) {
    if (!macro_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Check if macro_src contains "         MACRO" and "         MEND"
    const char *mac = strstr(macro_src, "         MACRO");
    const char *mend = strstr(macro_src, "         MEND");
    
    if (mac && mend && mac < mend) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_generate_flow_prompt(const char *jcl_src, const char *model_name, char *prompt_out, size_t max_len) {
    if (!jcl_src || !model_name || !prompt_out || max_len == 0) return -1;
    
    prompt_out[0] = '\0';
    
    size_t j_len = strlen(jcl_src);
    char *prompt = (char *)malloc(j_len + 128);
    if (!prompt) return -1;
    
    snprintf(prompt, j_len + 128, 
             "Generate a flowchart diagram prompt describing the step execution sequence for this JCL: %s", 
             jcl_src);
             
    int ret = tsfi_zorse_query_llm(prompt, model_name, prompt_out, max_len);
    free(prompt);
    
    return ret;
}

int tsfi_zorse_validate_cobol_file_status(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Check if COBOL source specifies FILE STATUS clause
    if (strstr(cobol_src, "FILE STATUS IS ") || strstr(cobol_src, "FILE STATUS ")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_sysout(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    const char *p = strstr(jcl_line, "SYSOUT=");
    if (!p) return 0;
    
    p += 7; // Skip "SYSOUT="
    
    // Verify it is followed by a valid single alphanumeric character or '*'
    char c = *p;
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '*') {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_hlasm_register(const char *hlasm_line, int *is_valid_out) {
    if (!hlasm_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Check for USING or DROP statements
    const char *p = strstr(hlasm_line, " USING ");
    if (!p) p = strstr(hlasm_line, " DROP ");
    if (!p) return 0;
    
    // Find the register (skip space and directive)
    p = strchr(p + 1, ' ');
    if (!p) return 0;
    while (*p == ' ' || *p == '\t') p++;
    
    // Accept standard register formats like R0..R15 or numeric 0..15
    if (*p == 'R' || *p == 'r') p++;
    int reg = atoi(p);
    if (reg >= 0 && reg <= 15) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_stepname(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Check if statement contains JCL step name indicator (e.g. starts with //)
    if (strncmp(jcl_line, "//", 2) != 0) return 0;
    
    const char *p = jcl_line + 2;
    
    // Verify name length is between 1 and 8 characters and entirely alphanumeric
    int name_len = 0;
    while (*p != '\0' && *p != ' ' && *p != '\n' && *p != '\t') {
        char c = *p;
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
            return 0;
        }
        name_len++;
        p++;
    }
    
    if (name_len >= 1 && name_len <= 8) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_estimate_vsam_size(const char *cobol_pic_clause, int *record_len_out) {
    if (!cobol_pic_clause || !record_len_out) return -1;
    
    *record_len_out = 0;
    
    // Find PIC or PICTURE
    const char *p = strstr(cobol_pic_clause, "PIC ");
    if (!p) p = strstr(cobol_pic_clause, "PICTURE ");
    if (!p) return 0;
    
    // Find opening parenthesis
    p = strchr(p, '(');
    if (p) {
        p++;
        *record_len_out = atoi(p);
    } else {
        // Look for literal pattern length like "X" or "999"
        p = strchr(cobol_pic_clause, ' ');
        while (*p == ' ') p++;
        p = strchr(p, ' ');
        while (*p == ' ') p++;
        int len = 0;
        while (*p != '\0' && *p != '.' && *p != ' ' && *p != '\n') {
            if (*p == 'X' || *p == '9' || *p == 'A') {
                len++;
            }
            p++;
        }
        *record_len_out = len;
    }
    
    return 0;
}

int tsfi_zorse_evaluate_step_cond(int step_rc, const char *cond_expr, int *should_run_out) {
    if (!cond_expr || !should_run_out) return -1;
    
    *should_run_out = 1; // Default to run step
    
    // Parse target COND comparison block, e.g. COND=(4,LT)
    const char *p = strstr(cond_expr, "COND=");
    if (!p) return 0;
    
    p += 5; // Skip "COND="
    
    if (*p == '(') {
        p++;
        int val = atoi(p);
        p = strchr(p, ',');
        if (p) {
            p++;
            // Check condition comparison modes:
            // If the condition is true, the step is bypassed (should_run_out = 0)
            if (strncmp(p, "EQ", 2) == 0 && step_rc == val) *should_run_out = 0;
            if (strncmp(p, "NE", 2) == 0 && step_rc != val) *should_run_out = 0;
            if (strncmp(p, "GT", 2) == 0 && step_rc > val) *should_run_out = 0;
            if (strncmp(p, "LT", 2) == 0 && step_rc < val) *should_run_out = 0;
            if (strncmp(p, "GE", 2) == 0 && step_rc >= val) *should_run_out = 0;
            if (strncmp(p, "LE", 2) == 0 && step_rc <= val) *should_run_out = 0;
        }
    }
    
    return 0;
}

const char* tsfi_zorse_explain_file_status(int status_code) {
    switch (status_code) {
        case 0:  return "Success";
        case 10: return "End of file";
        case 23: return "Key not found";
        case 30: return "Permanent I/O error";
        case 35: return "File not found";
        case 39: return "Attribute conflict mismatch";
        case 46: return "Read failed: file not opened";
        default: return "Unknown status code";
    }
}

int tsfi_zorse_validate_jcl_region(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    const char *p = strstr(jcl_line, "REGION=");
    if (!p) return 0;
    
    p += 7; // Skip "REGION="
    
    // Verify it is followed by a number and optionally a unit (K or M)
    int val = atoi(p);
    if (val > 0) {
        while (*p >= '0' && *p <= '9') p++;
        if (*p == 'K' || *p == 'M' || *p == 'k' || *p == 'm' || *p == ' ' || *p == '\0' || *p == '\n' || *p == ',') {
            *is_valid_out = 1;
        }
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_occurs_depending(const char *occurs_clause, int *is_valid_out) {
    if (!occurs_clause || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Match "OCCURS" and "DEPENDING ON" anywhere inside the clause statement bounds
    if (strstr(occurs_clause, "OCCURS ") && strstr(occurs_clause, "DEPENDING ON")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_time(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    const char *p = strstr(jcl_line, "TIME=");
    if (!p) return 0;
    
    p += 5; // Skip "TIME="
    
    // Check for standard formats like TIME=1440, TIME=N, or TIME=(mins,secs)
    if (*p == '(' || (*p >= '0' && *p <= '9')) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_evaluate(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Check for matching EVALUATE and END-EVALUATE blocks
    if (strstr(cobol_src, "EVALUATE ") && strstr(cobol_src, "END-EVALUATE")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_msgclass(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    const char *p = strstr(jcl_line, "MSGCLASS=");
    if (!p) return 0;
    
    p += 9; // Skip "MSGCLASS="
    
    char c = *p;
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '*') {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_perform(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Check if statement contains PERFORM
    if (strstr(cobol_src, "PERFORM")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_typrun(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    const char *p = strstr(jcl_line, "TYPRUN=");
    if (!p) return 0;
    
    p += 7; // Skip "TYPRUN="
    
    if (strncmp(p, "HOLD", 4) == 0 || strncmp(p, "SCAN", 4) == 0 || strncmp(p, "COPY", 4) == 0) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_search(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Check for matching SEARCH and END-SEARCH blocks
    if (strstr(cobol_src, "SEARCH ") && strstr(cobol_src, "END-SEARCH")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_addrspc(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    const char *p = strstr(jcl_line, "ADDRSPC=");
    if (!p) return 0;
    
    p += 8; // Skip "ADDRSPC="
    
    if (strncmp(p, "VIRT", 4) == 0 || strncmp(p, "REAL", 4) == 0) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_audit_render_artifacts(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len) {
    if (!b64_render_img || !model_name || !analysis_out || max_len == 0) return -1;
    
    analysis_out[0] = '\0';
    
    const char *prompt = "Analyze this ray-traced render output to check for reflection noise, shadow artifacts, or surface clipping.";
    
    return tsfi_ai_evaluate_vlm(b64_render_img, prompt, analysis_out, max_len);
}

int tsfi_zorse_optimize_camera_matrix(const char *b64_render_img, const char *model_name, char *matrix_out, size_t max_len) {
    if (!b64_render_img || !model_name || !matrix_out || max_len == 0) return -1;
    
    matrix_out[0] = '\0';
    
    const char *prompt = "Evaluate the perspective projection in this rendering. Recommend corrections for translation offsets and field of view camera parameters.";
    
    return tsfi_ai_evaluate_vlm(b64_render_img, prompt, matrix_out, max_len);
}

int tsfi_zorse_audit_material_properties(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len) {
    if (!b64_render_img || !model_name || !analysis_out || max_len == 0) return -1;
    
    analysis_out[0] = '\0';
    
    const char *prompt = "Analyze this rendering to audit material textures, specularity, and roughness realism.";
    
    return tsfi_ai_evaluate_vlm(b64_render_img, prompt, analysis_out, max_len);
}

int tsfi_zorse_optimize_bounding_collision(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len) {
    if (!b64_render_img || !model_name || !analysis_out || max_len == 0) return -1;
    
    analysis_out[0] = '\0';
    
    const char *prompt = "Inspect the rendered scene geometry in this frame to identify and optimize bounding box overlaps or visual collision anomalies.";
    
    return tsfi_ai_evaluate_vlm(b64_render_img, prompt, analysis_out, max_len);
}

int tsfi_zorse_audit_icon_transparency(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len) {
    if (!b64_render_img || !model_name || !analysis_out || max_len == 0) return -1;
    
    analysis_out[0] = '\0';
    
    const char *prompt = "Verify icon transparency boundaries, anti-aliasing quality, and color contrast limits across interface background themes.";
    
    return tsfi_ai_evaluate_vlm(b64_render_img, prompt, analysis_out, max_len);
}

int tsfi_zorse_audit_icon_style(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len) {
    if (!b64_render_img || !model_name || !analysis_out || max_len == 0) return -1;
    
    analysis_out[0] = '\0';
    
    const char *prompt = "Check this rendered icon to ensure visual design consistency (including lighting perspective, stroke width, and scale) matches style guides.";
    
    return tsfi_ai_evaluate_vlm(b64_render_img, prompt, analysis_out, max_len);
}

int tsfi_zorse_validate_cobol_initialize(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "INITIALIZE ")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_dsntype(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    const char *p = strstr(jcl_line, "DSNTYPE=");
    if (!p) return 0;
    
    p += 8; // Skip "DSNTYPE="
    
    if (strncmp(p, "LIBRARY", 7) == 0 || strncmp(p, "PDS", 3) == 0 ||
        strncmp(p, "LARGE", 5) == 0 || strncmp(p, "EXTREQ", 6) == 0) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_goto(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "GO TO") || strstr(cobol_src, "GOTO")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_sms(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "DATACLAS=") || strstr(jcl_line, "STORCLAS=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_string(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "STRING ") && strstr(cobol_src, "DELIMITED BY")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_recorg(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    const char *p = strstr(jcl_line, "RECORG=");
    if (!p) return 0;
    
    p += 7; // Skip "RECORG="
    
    if (strncmp(p, "KS", 2) == 0 || strncmp(p, "ES", 2) == 0 ||
        strncmp(p, "RR", 2) == 0 || strncmp(p, "LS", 2) == 0) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_inspect(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "INSPECT ") && (strstr(cobol_src, "TALLYING") || strstr(cobol_src, "REPLACING"))) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_avgrec(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    const char *p = strstr(jcl_line, "AVGREC=");
    if (!p) return 0;
    
    p += 7; // Skip "AVGREC="
    
    if (*p == 'U' || *p == 'K' || *p == 'M' || *p == 'u' || *p == 'k' || *p == 'm') {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_unstring(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "UNSTRING ") && strstr(cobol_src, " INTO ")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_secmodel(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "SECMODEL=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_size_error(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "ADD ") && strstr(cobol_src, "SIZE ERROR")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_dsorg(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    const char *p = strstr(jcl_line, "DSORG=");
    if (!p) return 0;
    
    p += 6; // Skip "DSORG="
    
    if (strncmp(p, "PS", 2) == 0 || strncmp(p, "PO", 2) == 0 ||
        strncmp(p, "DA", 2) == 0 || strncmp(p, "IS", 2) == 0 ||
        strncmp(p, "CX", 2) == 0) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_audit_icon_balance(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len) {
    if (!b64_render_img || !model_name || !analysis_out || max_len == 0) return -1;
    
    analysis_out[0] = '\0';
    
    const char *prompt = "Verify the optical center of gravity, visual balance, and padding margins of this rendered icon.";
    
    return tsfi_ai_evaluate_vlm(b64_render_img, prompt, analysis_out, max_len);
}

int tsfi_zorse_audit_icon_palette(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len) {
    if (!b64_render_img || !model_name || !analysis_out || max_len == 0) return -1;
    
    analysis_out[0] = '\0';
    
    const char *prompt = "Inspect the color values and lighting shadows in this icon render to confirm compliance with brand palette specifications.";
    
    return tsfi_ai_evaluate_vlm(b64_render_img, prompt, analysis_out, max_len);
}

int tsfi_zorse_validate_cobol_multiply_error(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "MULTIPLY ") && strstr(cobol_src, "SIZE ERROR")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_label(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "LABEL=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_audit_light_bloom(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len) {
    if (!b64_render_img || !model_name || !analysis_out || max_len == 0) return -1;
    
    analysis_out[0] = '\0';
    
    const char *prompt = "Analyze this rendering to evaluate visual light bloom, glare intensity levels, and lens flare artifacts.";
    
    return tsfi_ai_evaluate_vlm(b64_render_img, prompt, analysis_out, max_len);
}

int tsfi_zorse_audit_denoising_clarity(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len) {
    if (!b64_render_img || !model_name || !analysis_out || max_len == 0) return -1;
    
    analysis_out[0] = '\0';
    
    const char *prompt = "Verify this ray-traced rendering to identify visual detail loss, blur, or smudging artifacts caused by denoising filters.";
    
    return tsfi_ai_evaluate_vlm(b64_render_img, prompt, analysis_out, max_len);
}

int tsfi_zorse_validate_cobol_divide_error(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "DIVIDE ") && strstr(cobol_src, "SIZE ERROR")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_keylen(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    const char *p = strstr(jcl_line, "KEYLEN=");
    if (!p) return 0;
    
    p += 7; // Skip "KEYLEN="
    
    int val = 0;
    while (*p >= '0' && *p <= '9') {
        val = val * 10 + (*p - '0');
        p++;
    }
    
    if (val >= 0 && val <= 255) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_subtract_error(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "SUBTRACT ") && strstr(cobol_src, "SIZE ERROR")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_expdt(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "EXPDT=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_compute_error(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "COMPUTE ") && strstr(cobol_src, "SIZE ERROR")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_retpd(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "RETPD=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_invalid_key(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "WRITE ") && strstr(cobol_src, "INVALID KEY")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_ucs(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "UCS=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_start_key(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "START ") && strstr(cobol_src, "INVALID KEY")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_fcb(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "FCB=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}
