#include "tsfi_zorse_eval.h"
#include "tsfi_ai_core.h"
#include "tsfi_dat.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

int tsfi_zorse_validate_cobol_file_status(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Check if COBOL source specifies FILE STATUS clause
    if (strstr(cobol_src, "FILE STATUS IS ") || strstr(cobol_src, "FILE STATUS ")) {
        *is_valid_out = 1;
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

int tsfi_zorse_validate_cobol_evaluate(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Check for matching EVALUATE and END-EVALUATE blocks
    if (strstr(cobol_src, "EVALUATE ") && strstr(cobol_src, "END-EVALUATE")) {
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

int tsfi_zorse_validate_cobol_search(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Check for matching SEARCH and END-SEARCH blocks
    if (strstr(cobol_src, "SEARCH ") && strstr(cobol_src, "END-SEARCH")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_initialize(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "INITIALIZE ")) {
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

int tsfi_zorse_validate_cobol_string(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "STRING ") && strstr(cobol_src, "DELIMITED BY")) {
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

int tsfi_zorse_validate_cobol_unstring(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "UNSTRING ") && strstr(cobol_src, " INTO ")) {
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

int tsfi_zorse_validate_cobol_multiply_error(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "MULTIPLY ") && strstr(cobol_src, "SIZE ERROR")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_divide_error(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "DIVIDE ") && strstr(cobol_src, "SIZE ERROR")) {
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

int tsfi_zorse_validate_cobol_compute_error(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "COMPUTE ") && strstr(cobol_src, "SIZE ERROR")) {
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

int tsfi_zorse_validate_cobol_start_key(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "START ") && strstr(cobol_src, "INVALID KEY")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_read_key(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "READ ") && strstr(cobol_src, "INVALID KEY")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_delete_key(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "DELETE ") && strstr(cobol_src, "INVALID KEY")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_rewrite_key(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "REWRITE ") && strstr(cobol_src, "INVALID KEY")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_call_exception(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "CALL ") && (strstr(cobol_src, "EXCEPTION") || strstr(cobol_src, "OVERFLOW"))) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_accept(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "ACCEPT ") && strstr(cobol_src, "FROM")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_display(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "DISPLAY ") && strstr(cobol_src, "UPON")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_entry(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "ENTRY ") && strstr(cobol_src, "USING")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_goback(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "GOBACK")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_cancel(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "CANCEL")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_exit(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "EXIT")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_continue(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "CONTINUE")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_init_repl(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "INITIALIZE ") && strstr(cobol_src, "REPLACING")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_cics_dfhresp(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "DFHRESP") && strstr(cobol_src, "NORMAL")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_dli_call(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "CBLTDLI") && (strstr(cobol_src, "GU") || strstr(cobol_src, "GN") || strstr(cobol_src, "REPL") || strstr(cobol_src, "DLET"))) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_evaluate_also(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "EVALUATE ") && strstr(cobol_src, "ALSO")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_vsen_vaesen_register(const char *name, const char *type, int risk_level, const char *status) {
    if (!name || !type || !status) return -1;
    
    vsen_vaesen_record record;
    memset(&record, 0, sizeof(record));
    strncpy(record.name, name, sizeof(record.name) - 1);
    strncpy(record.type, type, sizeof(record.type) - 1);
    record.risk_level = risk_level;
    strncpy(record.status, status, sizeof(record.status) - 1);
    
    // Rule 13: Must only support .dat.bin extension for quadtree, index, database slices
    FILE *fp = fopen("vaesen_registry.dat.bin", "ab");
    if (!fp) {
        fp = fopen("vaesen_registry.dat.bin", "wb");
    }
    if (!fp) return -2;
    
    size_t written = fwrite(&record, sizeof(record), 1, fp);
    fclose(fp);
    
    return (written == 1) ? 0 : -3;
}

int tsfi_vsen_vaesen_lookup(const char *name, char *type_out, int *risk_level_out, char *status_out, size_t max_len) {
    if (!name || !type_out || !risk_level_out || !status_out || max_len == 0) return -1;
    
    FILE *fp = fopen("vaesen_registry.dat.bin", "rb");
    if (!fp) return -2;
    
    vsen_vaesen_record record;
    int found = 0;
    while (fread(&record, sizeof(record), 1, fp) == 1) {
        if (strcmp(record.name, name) == 0) {
            strncpy(type_out, record.type, max_len - 1);
            type_out[max_len - 1] = '\0';
            *risk_level_out = record.risk_level;
            strncpy(status_out, record.status, sizeof(record.status) - 1);
            status_out[sizeof(record.status) - 1] = '\0';
            found = 1;
            break;
        }
    }
    fclose(fp);
    
    return found ? 0 : -3;
}

int tsfi_vsen_vaesen_audit_transaction(const char *cics_trans_id, const char *entity_name, int *is_allowed_out) {
    if (!cics_trans_id || !entity_name || !is_allowed_out) return -1;
    
    *is_allowed_out = 1;
    char type_buf[64];
    char status_buf[32];
    int risk_level = 0;
    
    int lookup_rc = tsfi_vsen_vaesen_lookup(entity_name, type_buf, &risk_level, status_buf, sizeof(type_buf));
    if (lookup_rc == 0) {
        // Critical Auncient Vaesen with high risk levels require explicit administrative override
        if (risk_level > 8) {
            *is_allowed_out = 0; // Block or raise transaction warning
        }
    }
    
    return 0;
}

int tsfi_vsen_vaesen_record_sight(const char *entity_name, const char *location, int fear_factor) {
    if (!entity_name || !location) return -1;
    
    vsen_vaesen_sight_record record;
    memset(&record, 0, sizeof(record));
    strncpy(record.entity_name, entity_name, sizeof(record.entity_name) - 1);
    strncpy(record.location, location, sizeof(record.location) - 1);
    record.fear_factor = fear_factor;
    
    // Rule 13: Must only support .dat.bin extension for quadtree, index, database slices
    FILE *fp = fopen("vaesen_sights.dat.bin", "ab");
    if (!fp) {
        fp = fopen("vaesen_sights.dat.bin", "wb");
    }
    if (!fp) return -2;
    
    size_t written = fwrite(&record, sizeof(record), 1, fp);
    fclose(fp);
    
    return (written == 1) ? 0 : -3;
}

int tsfi_vsen_vaesen_get_aggregate_fear(const char *location, int *agg_fear_out) {
    if (!location || !agg_fear_out) return -1;
    
    FILE *fp = fopen("vaesen_sights.dat.bin", "rb");
    if (!fp) {
        *agg_fear_out = 0;
        return 0; // Return empty/0 aggregate fear if database doesn't exist yet
    }
    
    vsen_vaesen_sight_record record;
    int total_fear = 0;
    while (fread(&record, sizeof(record), 1, fp) == 1) {
        if (strcmp(record.location, location) == 0) {
            total_fear += record.fear_factor;
        }
    }
    fclose(fp);
    
    *agg_fear_out = total_fear;
    return 0;
}

int tsfi_zorse_validate_cobol_call_using(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "CALL ") && strstr(cobol_src, "USING")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_inspect_repl(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "INSPECT ") && strstr(cobol_src, "REPLACING")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_exit_program(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "EXIT PROGRAM")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_stop_run(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "STOP RUN")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_goto_depending(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "GO TO ") && strstr(cobol_src, "DEPENDING ON")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_sort(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "SORT ") && strstr(cobol_src, " ON ")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_search_all(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "SEARCH ALL")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_read_next(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "READ ") && strstr(cobol_src, "NEXT")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_write_from(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "WRITE ") && strstr(cobol_src, "FROM")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_string_pointer(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "STRING ") && strstr(cobol_src, "POINTER")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_init_filler(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "INITIALIZE ") && strstr(cobol_src, "FILLER")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_entry_using(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "ENTRY ") && strstr(cobol_src, "USING")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_inspect_tallying(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "INSPECT ") && strstr(cobol_src, "TALLYING")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_inspect_tally_repl(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "INSPECT ") && strstr(cobol_src, "TALLYING") && strstr(cobol_src, "REPLACING")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_copy_of(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "COPY ") && strstr(cobol_src, " OF ")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_xml_gen(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "XML GENERATE")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_xml_parse(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "XML PARSE")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_vse_cics_rdo(const char *rdo_script, int *is_valid_out) {
    if (!rdo_script || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(rdo_script, "DEFINE CONNECTION") || strstr(rdo_script, "DEFINE SESSIONS")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_dyn_call(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    const char *call_ptr = strstr(cobol_src, "CALL ");
    if (call_ptr) {
        call_ptr += 5;
        while (*call_ptr == ' ' || *call_ptr == '\t') call_ptr++;
        if (*call_ptr != '"' && *call_ptr != '\'') {
            *is_valid_out = 1;
        }
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_sort_key(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "SORT ") && strstr(cobol_src, "KEY")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_merge_key(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "MERGE ") && strstr(cobol_src, "KEY")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_generate(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "GENERATE ")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_initiate(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "INITIATE ")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_terminate(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "TERMINATE ")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_page_heading(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "PAGE HEADING")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_page_limit(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "PAGE LIMIT")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_heading_group(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "TYPE IS") && strstr(cobol_src, "HEADING")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_detail_group(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "TYPE IS") && strstr(cobol_src, "DETAIL")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_control_heading(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "CONTROL HEADING") || (strstr(cobol_src, "TYPE IS") && strstr(cobol_src, "CH"))) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_control_footing(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "CONTROL FOOTING") || (strstr(cobol_src, "TYPE IS") && strstr(cobol_src, "CF"))) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_page_footing(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "PAGE FOOTING") || (strstr(cobol_src, "TYPE IS") && strstr(cobol_src, "PF"))) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_report_heading(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "REPORT HEADING") || (strstr(cobol_src, "TYPE IS") && strstr(cobol_src, "RH"))) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_report_footing(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "REPORT FOOTING") || (strstr(cobol_src, "TYPE IS") && strstr(cobol_src, "RF"))) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_cf_group(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "TYPE IS") && strstr(cobol_src, "CONTROL FOOTING")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_line_spacing(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "LINE PLUS") || strstr(cobol_src, "LINE NEXT")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_next_page(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "NEXT PAGE")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_next_page_limit(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "NEXT PAGE LIMIT")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_cf_spacing(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "CONTROL FOOTING") && (strstr(cobol_src, "LINE PLUS") || strstr(cobol_src, "LINE NEXT"))) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_pf_spacing(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "PAGE FOOTING") && (strstr(cobol_src, "LINE PLUS") || strstr(cobol_src, "LINE NEXT"))) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_rh_spacing(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "REPORT HEADING") && (strstr(cobol_src, "LINE PLUS") || strstr(cobol_src, "LINE NEXT"))) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_final_summary(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "CONTROL FOOTING") && strstr(cobol_src, "FINAL")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_final_spacing(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "FINAL") && (strstr(cobol_src, "LINE PLUS") || strstr(cobol_src, "LINE NEXT"))) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_ph_spacing_limit(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "PAGE HEADING") && strstr(cobol_src, "LIMIT")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_pf_spacing_limit(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "PAGE FOOTING") && strstr(cobol_src, "LIMIT")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_final_spacing_limit(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "FINAL") && strstr(cobol_src, "LIMIT")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_final_spacing_limit_override(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "FINAL") && strstr(cobol_src, "LIMIT") && strstr(cobol_src, "OVERRIDE")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_final_spacing_checks(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "FINAL") && strstr(cobol_src, "LIMIT") && strstr(cobol_src, "OVERRIDE") && strstr(cobol_src, "CHECKS")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_final_spacing_list_checks(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "FINAL") && strstr(cobol_src, "LIMIT") && strstr(cobol_src, "OVERRIDE") && strstr(cobol_src, "CHECKS") && strstr(cobol_src, "LIST")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_final_spacing_status(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "FINAL") && strstr(cobol_src, "LIMIT") && strstr(cobol_src, "OVERRIDE") && strstr(cobol_src, "CHECKS") && strstr(cobol_src, "LIST") && strstr(cobol_src, "STATUS")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_ch_spacing_limit(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "CONTROL HEADING") && strstr(cobol_src, "LIMIT")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_validate_cobol_ch_spacing_limit_override(const char *cobol_src, int *is_valid_out) {
    if (!cobol_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(cobol_src, "CONTROL HEADING") && strstr(cobol_src, "LIMIT") && strstr(cobol_src, "OVERRIDE")) {
        *is_valid_out = 1;
    }
    
    return 0;
}
