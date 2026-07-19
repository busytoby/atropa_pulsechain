#include "tsfi_zorse_eval.h"
#include "tsfi_ai_core.h"
#include "tsfi_dat.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

int tsfi_zorse_generate_jcl_from_flowchart(const char *b64_flowchart_img, const char *model_name, char *jcl_out, size_t max_len) {
    if (!b64_flowchart_img || !model_name || !jcl_out || max_len == 0) return -1;
    
    jcl_out[0] = '\0';
    
    const char *prompt = "Extract the program execution sequence from this flowchart image and write a compliant JCL job stream with EXEC cards.";
    
    return tsfi_ai_evaluate_vlm(b64_flowchart_img, prompt, jcl_out, max_len);
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

int tsfi_zorse_validate_jcl_sms(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "DATACLAS=") || strstr(jcl_line, "STORCLAS=")) {
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

int tsfi_zorse_validate_jcl_secmodel(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "SECMODEL=")) {
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

int tsfi_zorse_validate_jcl_label(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "LABEL=")) {
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

int tsfi_zorse_validate_jcl_expdt(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "EXPDT=")) {
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

int tsfi_zorse_validate_jcl_ucs(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "UCS=")) {
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

int tsfi_zorse_validate_jcl_prty(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    const char *p = strstr(jcl_line, "PRTY=");
    if (!p) return 0;
    
    p += 5; // Skip "PRTY="
    
    int val = 0;
    while (*p >= '0' && *p <= '9') {
        val = val * 10 + (*p - '0');
        p++;
    }
    
    if (val >= 0 && val <= 15) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_burst(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "BURST=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_flash(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "FLASH=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_hold(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "HOLD=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_free(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "FREE=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_outlim(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "OUTLIM=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_steplib(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "STEPLIB") || strstr(jcl_line, "JOBLIB")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_dest(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "DEST=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_export(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "EXPORT SYMLIST=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_set(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "SET ")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_import(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "IMPORT ")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_jcllib(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "JCLLIB ORDER=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_spool(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$ ") && (strstr(jcl_line, "LST") || strstr(jcl_line, "PUN") || strstr(jcl_line, "JOB"))) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_vtam_mpc(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "MPCGRP") || strstr(jcl_line, "MPC")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_fba_vdisk(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "VDISK") && strstr(jcl_line, "UNIT=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_vtam_port(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "PORTNAME=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_symlist(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "SYMLIST=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_dlurvtam(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "DLURVTAM") || strstr(jcl_line, "DLUR")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_dest_router(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "DEST=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_dsid(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "DSID=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_dsname(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "DSNAME=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_path(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "PATH=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_gdg(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "(+") || strstr(jcl_line, "(-")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_pathmode(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "PATHMODE=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_gdg_version(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, ".G") && strstr(jcl_line, "V")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_triple_s(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "SSS=") || strstr(jcl_line, "SSSPGM=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_pathdisp(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "PATHDISP=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_filedata(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "FILEDATA=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_protect(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "PROTECT=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_subsys(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "SUBSYS=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_free_close(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "FREE=CLOSE")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_accode(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "ACCODE=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_dsntype_large(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "DSNTYPE=LARGE")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_eropt(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "EROPT=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_submit(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "POWER ") && strstr(jcl_line, "SUBMIT")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_dsname_ver(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "DSNAME=") && strstr(jcl_line, ".G") && strstr(jcl_line, "V")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_parm_override(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "PARM=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_jcl_sysout_route(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "SYSOUT=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_class(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "CLASS=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_prtclass(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "PRTCLASS=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_punclass(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "PUNCLASS=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_hold(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "HOLD=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_disp(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "DISP=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_prtclass_override(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "PRTCLASS=") && strstr(jcl_line, "(")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_copies(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "COPIES=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_pages(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "PAGES=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_pages_override(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "PAGES=") && strstr(jcl_line, "(")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_form(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "FORM=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_lines(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "LINES=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_pages_list(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "PAGES=") && strstr(jcl_line, "(") && strstr(jcl_line, ",")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_pages_limit(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "PAGES=") && !strstr(jcl_line, "(")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_node(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "NODE=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_flash(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "FLASH=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_flash_override(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "FLASH=") && strstr(jcl_line, "(")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_flash_status(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "FLASH=") && strstr(jcl_line, "STATUS")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_node_status(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "NODE=") && strstr(jcl_line, "STATUS")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_node_override(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "NODE=") && strstr(jcl_line, "(")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_pri(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "PRI=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_pri_override(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "PRI=") && strstr(jcl_line, "(")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_keep(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "KEEP=")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_keep_status(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "KEEP=") && strstr(jcl_line, "STATUS")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_class_override(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "CLASS=") && strstr(jcl_line, "(")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_class_status(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "CLASS=") && strstr(jcl_line, "STATUS")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_keep_override(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "KEEP=") && strstr(jcl_line, "(")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_keep_status_override(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "KEEP=") && strstr(jcl_line, "STATUS") && strstr(jcl_line, "(")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_pri_status_override(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "PRI=") && strstr(jcl_line, "STATUS") && strstr(jcl_line, "(")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_pri_override_list(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "PRI=") && strstr(jcl_line, "(") && strstr(jcl_line, ",")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_pri_status_checks(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "PRI=") && strstr(jcl_line, "STATUS") && strstr(jcl_line, "CHECKS") && strstr(jcl_line, "(")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_class_status_override(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "CLASS=") && strstr(jcl_line, "STATUS") && strstr(jcl_line, "(")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_power_class_status_override_list(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "* $$") && strstr(jcl_line, "CLASS=") && strstr(jcl_line, "STATUS") && strstr(jcl_line, "(") && strstr(jcl_line, ",")) {
        *is_valid_out = 1;
    }
    
    return 0;
}
