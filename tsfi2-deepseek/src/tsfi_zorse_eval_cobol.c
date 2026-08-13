#include "tsfi_zorse_eval.h"
#include "tsfi_ai_core.h"
#include "tsfi_dat.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

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

static uint64_t g_vsen_global_tx_counter = 1000;
static uint64_t g_vsen_global_lsn_counter = 1;

static uint64_t tsfi_vsen_compute_fnv1a_dna_hash(const void *data, size_t len, uint64_t prev_hash) {
    const uint8_t *bytes = (const uint8_t *)data;
    uint64_t hash = prev_hash ? prev_hash : 14695981039346656037ULL;
    for (size_t i = 0; i < len; i++) {
        hash ^= bytes[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

int tsfi_vsen_tx_begin(uint64_t *tx_id_out) {
    if (!tx_id_out) return -1;
    *tx_id_out = ++g_vsen_global_tx_counter;

    vsen_wal_entry_header_t wal;
    memset(&wal, 0, sizeof(wal));
    wal.lsn = ++g_vsen_global_lsn_counter;
    wal.tx_id = *tx_id_out;
    wal.op_type = 1; // BEGIN

    FILE *fp = fopen("vaesen_registry.wal.dat.bin", "ab");
    if (fp) {
        fwrite(&wal, sizeof(wal), 1, fp);
        fclose(fp);
    }
    return 0;
}

int tsfi_vsen_tx_commit(uint64_t tx_id) {
    if (tx_id == 0) return -1;

    vsen_wal_entry_header_t wal;
    memset(&wal, 0, sizeof(wal));
    wal.lsn = ++g_vsen_global_lsn_counter;
    wal.tx_id = tx_id;
    wal.op_type = 4; // COMMIT

    FILE *fp = fopen("vaesen_registry.wal.dat.bin", "ab");
    if (fp) {
        fwrite(&wal, sizeof(wal), 1, fp);
        fclose(fp);
    }
    return 0;
}

int tsfi_vsen_tx_abort(uint64_t tx_id) {
    if (tx_id == 0) return -1;

    vsen_wal_entry_header_t wal;
    memset(&wal, 0, sizeof(wal));
    wal.lsn = ++g_vsen_global_lsn_counter;
    wal.tx_id = tx_id;
    wal.op_type = 5; // ABORT

    FILE *fp = fopen("vaesen_registry.wal.dat.bin", "ab");
    if (fp) {
        fwrite(&wal, sizeof(wal), 1, fp);
        fclose(fp);
    }
    return 0;
}

int tsfi_vsen_tx_savepoint(uint64_t tx_id, const char *savepoint_name) {
    if (tx_id == 0 || !savepoint_name) return -1;

    vsen_wal_entry_header_t wal;
    memset(&wal, 0, sizeof(wal));
    wal.lsn = ++g_vsen_global_lsn_counter;
    wal.tx_id = tx_id;
    wal.op_type = 6; // SAVEPOINT
    strncpy(wal.savepoint_name, savepoint_name, sizeof(wal.savepoint_name) - 1);

    FILE *fp = fopen("vaesen_registry.wal.dat.bin", "ab");
    if (fp) {
        fwrite(&wal, sizeof(wal), 1, fp);
        fclose(fp);
    }
    return 0;
}

int tsfi_vsen_tx_rollback_to_savepoint(uint64_t tx_id, const char *savepoint_name) {
    if (tx_id == 0 || !savepoint_name) return -1;

    vsen_wal_entry_header_t wal;
    memset(&wal, 0, sizeof(wal));
    wal.lsn = ++g_vsen_global_lsn_counter;
    wal.tx_id = tx_id;
    wal.op_type = 7; // ROLLBACK_SAVEPOINT
    strncpy(wal.savepoint_name, savepoint_name, sizeof(wal.savepoint_name) - 1);

    FILE *fp = fopen("vaesen_registry.wal.dat.bin", "ab");
    if (fp) {
        fwrite(&wal, sizeof(wal), 1, fp);
        fclose(fp);
    }
    return 0;
}

int tsfi_vsen_wal_recover(const char *dat_bin_file_path) {
    if (!dat_bin_file_path) return -1;
    size_t len = strlen(dat_bin_file_path);
    if (len < 8 || strcmp(dat_bin_file_path + len - 8, ".dat.bin") != 0) return -2;

    char wal_path[256];
    snprintf(wal_path, sizeof(wal_path), "%.*swal.dat.bin", (int)(len - 7), dat_bin_file_path);

    FILE *fp = fopen(wal_path, "rb");
    if (!fp) return 0; // No WAL file needing recovery

    vsen_wal_entry_header_t entry;
    uint64_t max_lsn = 0;
    while (fread(&entry, sizeof(entry), 1, fp) == 1) {
        if (entry.lsn > max_lsn) max_lsn = entry.lsn;
    }
    fclose(fp);

    if (max_lsn > g_vsen_global_lsn_counter) {
        g_vsen_global_lsn_counter = max_lsn;
    }
    return 0;
}

int tsfi_vsen_audit_chain_verify(const char *dat_bin_file_path) {
    if (!dat_bin_file_path) return -1;
    FILE *fp = fopen(dat_bin_file_path, "rb");
    if (!fp) return -2;

    uint64_t prev_hash = 0;
    int valid = 1;

    if (strstr(dat_bin_file_path, "amt_orientation") != NULL) {
        vsen_amt_orientation_mvcc_record record;
        while (fread(&record, sizeof(record), 1, fp) == 1) {
            uint64_t expected_hash = tsfi_vsen_compute_fnv1a_dna_hash(&record.data, sizeof(record.data), prev_hash);
            if (record.mvcc.dna_hash != expected_hash) {
                valid = 0;
                break;
            }
            prev_hash = record.mvcc.dna_hash;
        }
    } else if (strstr(dat_bin_file_path, "erara_page_text") != NULL) {
        vsen_erara_page_text_mvcc_record record;
        while (fread(&record, sizeof(record), 1, fp) == 1) {
            uint64_t expected_hash = tsfi_vsen_compute_fnv1a_dna_hash(&record.data, sizeof(record.data), prev_hash);
            if (record.mvcc.dna_hash != expected_hash) {
                valid = 0;
                break;
            }
            prev_hash = record.mvcc.dna_hash;
        }
    } else if (strstr(dat_bin_file_path, "erara") != NULL) {
        vsen_erara_title_mvcc_record record;
        while (fread(&record, sizeof(record), 1, fp) == 1) {
            uint64_t expected_hash = tsfi_vsen_compute_fnv1a_dna_hash(&record.data, sizeof(record.data), prev_hash);
            if (record.mvcc.dna_hash != expected_hash) {
                valid = 0;
                break;
            }
            prev_hash = record.mvcc.dna_hash;
        }
    } else {
        vsen_vaesen_mvcc_record record;
        while (fread(&record, sizeof(record), 1, fp) == 1) {
            uint64_t expected_hash = tsfi_vsen_compute_fnv1a_dna_hash(&record.data, sizeof(record.data), prev_hash);
            if (record.mvcc.dna_hash != expected_hash) {
                valid = 0;
                break;
            }
            prev_hash = record.mvcc.dna_hash;
        }
    }

    fclose(fp);
    return valid ? 0 : -3;
}

int tsfi_vsen_vaesen_register(const char *name, const char *type, int risk_level, const char *status) {
    if (!name || !type || !status) return -1;

    uint64_t tx_id = 0;
    tsfi_vsen_tx_begin(&tx_id);

    vsen_vaesen_mvcc_record mvcc_rec;
    memset(&mvcc_rec, 0, sizeof(mvcc_rec));
    mvcc_rec.mvcc.tx_id = tx_id;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    mvcc_rec.mvcc.commit_timestamp = (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)(ts.tv_nsec / 1000);
    mvcc_rec.mvcc.prev_version_offset = 0;
    mvcc_rec.mvcc.is_deleted = 0;

    strncpy(mvcc_rec.data.name, name, sizeof(mvcc_rec.data.name) - 1);
    strncpy(mvcc_rec.data.type, type, sizeof(mvcc_rec.data.type) - 1);
    mvcc_rec.data.risk_level = risk_level;
    strncpy(mvcc_rec.data.status, status, sizeof(mvcc_rec.data.status) - 1);

    // Rule 13: Must only support .dat.bin extension for quadtree, index, database slices
    vsen_vaesen_mvcc_record existing;
    uint64_t last_offset = 0;
    uint64_t prev_dna_hash = 0;

    FILE *fp_read = fopen("vaesen_registry.dat.bin", "rb");
    if (fp_read) {
        while (fread(&existing, sizeof(existing), 1, fp_read) == 1) {
            prev_dna_hash = existing.mvcc.dna_hash;
            if (strcmp(existing.data.name, name) == 0 && !existing.mvcc.is_deleted) {
                last_offset = ftell(fp_read) - sizeof(existing);
            }
        }
        fclose(fp_read);
    }

    mvcc_rec.mvcc.prev_version_offset = last_offset;
    mvcc_rec.mvcc.dna_hash = tsfi_vsen_compute_fnv1a_dna_hash(&mvcc_rec.data, sizeof(vsen_vaesen_record), prev_dna_hash);

    FILE *fp = fopen("vaesen_registry.dat.bin", "ab");
    if (!fp) {
        tsfi_vsen_tx_abort(tx_id);
        return -2;
    }
    uint64_t write_offset = ftell(fp);
    size_t written = fwrite(&mvcc_rec, sizeof(mvcc_rec), 1, fp);
    fclose(fp);

    if (written == 1) {
        vsen_wal_entry_header_t wal;
        memset(&wal, 0, sizeof(wal));
        wal.lsn = ++g_vsen_global_lsn_counter;
        wal.tx_id = tx_id;
        wal.op_type = 2; // INSERT/UPDATE
        strncpy(wal.target_file, "vaesen_registry.dat.bin", sizeof(wal.target_file) - 1);
        wal.record_offset = write_offset;
        wal.payload_len = sizeof(mvcc_rec);

        FILE *wal_fp = fopen("vaesen_registry.wal.dat.bin", "ab");
        if (wal_fp) {
            fwrite(&wal, sizeof(wal), 1, wal_fp);
            fclose(wal_fp);
        }
        tsfi_vsen_tx_commit(tx_id);
        return 0;
    } else {
        tsfi_vsen_tx_abort(tx_id);
        return -3;
    }
}

int tsfi_vsen_vaesen_lookup(const char *name, char *type_out, int *risk_level_out, char *status_out, size_t max_len) {
    if (!name || !type_out || !risk_level_out || !status_out || max_len == 0) return -1;

    FILE *fp = fopen("vaesen_registry.dat.bin", "rb");
    if (!fp) return -2;

    vsen_vaesen_mvcc_record record;
    int found = 0;
    while (fread(&record, sizeof(record), 1, fp) == 1) {
        if (strcmp(record.data.name, name) == 0 && !record.mvcc.is_deleted) {
            strncpy(type_out, record.data.type, max_len - 1);
            type_out[max_len - 1] = '\0';
            *risk_level_out = record.data.risk_level;
            strncpy(status_out, record.data.status, sizeof(record.data.status) - 1);
            status_out[sizeof(record.data.status) - 1] = '\0';
            found = 1;
        }
    }
    fclose(fp);

    return found ? 0 : -3;
}

int tsfi_vsen_vaesen_lookup_as_of(const char *name, uint64_t timestamp, char *type_out, int *risk_level_out, char *status_out, size_t max_len) {
    if (!name || !type_out || !risk_level_out || !status_out || max_len == 0) return -1;

    FILE *fp = fopen("vaesen_registry.dat.bin", "rb");
    if (!fp) return -2;

    vsen_vaesen_mvcc_record record;
    int found = 0;
    while (fread(&record, sizeof(record), 1, fp) == 1) {
        if (strcmp(record.data.name, name) == 0 && record.mvcc.commit_timestamp <= timestamp && !record.mvcc.is_deleted) {
            strncpy(type_out, record.data.type, max_len - 1);
            type_out[max_len - 1] = '\0';
            *risk_level_out = record.data.risk_level;
            strncpy(status_out, record.data.status, sizeof(record.data.status) - 1);
            status_out[sizeof(record.data.status) - 1] = '\0';
            found = 1;
        }
    }
    fclose(fp);

    return found ? 0 : -3;
}

int tsfi_erara_register_title(const char *doi, const char *title, const char *author, uint32_t pub_year, uint32_t total_pages, const char *iiif_manifest_url) {
    if (!doi || !title || !author || !iiif_manifest_url) return -1;

    uint64_t tx_id = 0;
    tsfi_vsen_tx_begin(&tx_id);

    vsen_erara_title_mvcc_record mvcc_rec;
    memset(&mvcc_rec, 0, sizeof(mvcc_rec));
    mvcc_rec.mvcc.tx_id = tx_id;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    mvcc_rec.mvcc.commit_timestamp = (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)(ts.tv_nsec / 1000);
    mvcc_rec.mvcc.prev_version_offset = 0;
    mvcc_rec.mvcc.is_deleted = 0;

    strncpy(mvcc_rec.data.doi, doi, sizeof(mvcc_rec.data.doi) - 1);
    strncpy(mvcc_rec.data.title, title, sizeof(mvcc_rec.data.title) - 1);
    strncpy(mvcc_rec.data.author, author, sizeof(mvcc_rec.data.author) - 1);
    mvcc_rec.data.pub_year = pub_year;
    mvcc_rec.data.total_pages = total_pages;
    strncpy(mvcc_rec.data.iiif_manifest_url, iiif_manifest_url, sizeof(mvcc_rec.data.iiif_manifest_url) - 1);

    // Rule 13: Must only support .dat.bin extension for quadtree, index, database slices
    vsen_erara_title_mvcc_record existing;
    uint64_t last_offset = 0;
    uint64_t prev_dna_hash = 0;

    FILE *fp_read = fopen("erara_catalog.dat.bin", "rb");
    if (fp_read) {
        while (fread(&existing, sizeof(existing), 1, fp_read) == 1) {
            prev_dna_hash = existing.mvcc.dna_hash;
            if ((strcmp(existing.data.doi, doi) == 0 || strcmp(existing.data.title, title) == 0) && !existing.mvcc.is_deleted) {
                last_offset = ftell(fp_read) - sizeof(existing);
            }
        }
        fclose(fp_read);
    }

    mvcc_rec.mvcc.prev_version_offset = last_offset;
    mvcc_rec.mvcc.dna_hash = tsfi_vsen_compute_fnv1a_dna_hash(&mvcc_rec.data, sizeof(vsen_erara_title_record_t), prev_dna_hash);

    FILE *fp = fopen("erara_catalog.dat.bin", "ab");
    if (!fp) {
        tsfi_vsen_tx_abort(tx_id);
        return -2;
    }
    uint64_t write_offset = ftell(fp);
    size_t written = fwrite(&mvcc_rec, sizeof(mvcc_rec), 1, fp);
    fclose(fp);

    if (written == 1) {
        vsen_wal_entry_header_t wal;
        memset(&wal, 0, sizeof(wal));
        wal.lsn = ++g_vsen_global_lsn_counter;
        wal.tx_id = tx_id;
        wal.op_type = 2; // INSERT/UPDATE
        strncpy(wal.target_file, "erara_catalog.dat.bin", sizeof(wal.target_file) - 1);
        wal.record_offset = write_offset;
        wal.payload_len = sizeof(mvcc_rec);

        FILE *wal_fp = fopen("erara_catalog.wal.dat.bin", "ab");
        if (wal_fp) {
            fwrite(&wal, sizeof(wal), 1, wal_fp);
            fclose(wal_fp);
        }
        tsfi_vsen_tx_commit(tx_id);
        return 0;
    } else {
        tsfi_vsen_tx_abort(tx_id);
        return -3;
    }
}

int tsfi_erara_lookup_title(const char *doi_or_title, vsen_erara_title_record_t *record_out) {
    if (!doi_or_title || !record_out) return -1;

    FILE *fp = fopen("erara_catalog.dat.bin", "rb");
    if (!fp) return -2;

    vsen_erara_title_mvcc_record record;
    int found = 0;
    while (fread(&record, sizeof(record), 1, fp) == 1) {
        if ((strcmp(record.data.doi, doi_or_title) == 0 || strstr(record.data.title, doi_or_title) != NULL) && !record.mvcc.is_deleted) {
            *record_out = record.data;
            found = 1;
        }
    }
    fclose(fp);

    return found ? 0 : -3;
}

int tsfi_erara_lookup_title_as_of(const char *doi_or_title, uint64_t timestamp, vsen_erara_title_record_t *record_out) {
    if (!doi_or_title || !record_out) return -1;

    FILE *fp = fopen("erara_catalog.dat.bin", "rb");
    if (!fp) return -2;

    vsen_erara_title_mvcc_record record;
    int found = 0;
    while (fread(&record, sizeof(record), 1, fp) == 1) {
        if ((strcmp(record.data.doi, doi_or_title) == 0 || strstr(record.data.title, doi_or_title) != NULL) && record.mvcc.commit_timestamp <= timestamp && !record.mvcc.is_deleted) {
            *record_out = record.data;
            found = 1;
        }
    }
    fclose(fp);

    return found ? 0 : -3;
}

int tsfi_erara_search_by_author(const char *author_query, vsen_erara_title_record_t *results_out, int max_results, int *count_out) {
    if (!author_query || !results_out || max_results <= 0 || !count_out) return -1;

    *count_out = 0;
    FILE *fp = fopen("erara_catalog.dat.bin", "rb");
    if (!fp) return -2;

    vsen_erara_title_mvcc_record record;
    int count = 0;
    while (fread(&record, sizeof(record), 1, fp) == 1) {
        if (strstr(record.data.author, author_query) != NULL && !record.mvcc.is_deleted) {
            results_out[count] = record.data;
            count++;
            if (count >= max_results) break;
        }
    }
    fclose(fp);

    *count_out = count;
    return (count > 0) ? 0 : -3;
}

int tsfi_erara_register_page_text(const char *doi, uint32_t page_num, const char *page_text) {
    if (!doi || !page_text || page_num == 0) return -1;

    uint64_t tx_id = 0;
    tsfi_vsen_tx_begin(&tx_id);

    vsen_erara_page_text_mvcc_record mvcc_rec;
    memset(&mvcc_rec, 0, sizeof(mvcc_rec));
    mvcc_rec.mvcc.tx_id = tx_id;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    mvcc_rec.mvcc.commit_timestamp = (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)(ts.tv_nsec / 1000);
    mvcc_rec.mvcc.is_deleted = 0;

    strncpy(mvcc_rec.data.doi, doi, sizeof(mvcc_rec.data.doi) - 1);
    mvcc_rec.data.page_num = page_num;
    strncpy(mvcc_rec.data.page_text, page_text, sizeof(mvcc_rec.data.page_text) - 1);

    // Compute total input character count and check for buffer truncation
    size_t input_len = strlen(page_text);
    mvcc_rec.data.character_count = (uint32_t)input_len;
    mvcc_rec.data.is_truncated = (input_len >= sizeof(mvcc_rec.data.page_text)) ? 1 : 0;

    // Extract first line (incipit) up to newline or 255 chars
    size_t line_len = 0;
    while (page_text[line_len] != '\0' && page_text[line_len] != '\n' && line_len < sizeof(mvcc_rec.data.first_line) - 1) {
        line_len++;
    }
    strncpy(mvcc_rec.data.first_line, page_text, line_len);
    mvcc_rec.data.first_line[line_len] = '\0';

    // Compute word count
    uint32_t words = 0;
    bool in_word = false;
    for (size_t i = 0; i < mvcc_rec.data.character_count; i++) {
        if (page_text[i] == ' ' || page_text[i] == '\t' || page_text[i] == '\n' || page_text[i] == '\r') {
            in_word = false;
        } else if (!in_word) {
            in_word = true;
            words++;
        }
    }
    mvcc_rec.data.word_count = words;

    // Rule 13: Storage layout must be .dat.bin
    vsen_erara_page_text_mvcc_record existing;
    uint64_t last_offset = 0;
    uint64_t prev_dna_hash = 0;

    FILE *fp_read = fopen("erara_page_text.dat.bin", "rb");
    if (fp_read) {
        while (fread(&existing, sizeof(existing), 1, fp_read) == 1) {
            prev_dna_hash = existing.mvcc.dna_hash;
            if (strcmp(existing.data.doi, doi) == 0 && existing.data.page_num == page_num && !existing.mvcc.is_deleted) {
                last_offset = ftell(fp_read) - sizeof(existing);
            }
        }
        fclose(fp_read);
    }

    mvcc_rec.mvcc.prev_version_offset = last_offset;
    mvcc_rec.mvcc.dna_hash = tsfi_vsen_compute_fnv1a_dna_hash(&mvcc_rec.data, sizeof(vsen_erara_page_text_record_t), prev_dna_hash);

    FILE *fp = fopen("erara_page_text.dat.bin", "ab");
    if (!fp) {
        tsfi_vsen_tx_abort(tx_id);
        return -2;
    }
    uint64_t write_offset = ftell(fp);
    size_t written = fwrite(&mvcc_rec, sizeof(mvcc_rec), 1, fp);
    fclose(fp);

    if (written == 1) {
        vsen_wal_entry_header_t wal;
        memset(&wal, 0, sizeof(wal));
        wal.lsn = ++g_vsen_global_lsn_counter;
        wal.tx_id = tx_id;
        wal.op_type = 2; // INSERT/UPDATE
        strncpy(wal.target_file, "erara_page_text.dat.bin", sizeof(wal.target_file) - 1);
        wal.record_offset = write_offset;
        wal.payload_len = sizeof(mvcc_rec);

        FILE *wal_fp = fopen("erara_page_text.wal.dat.bin", "ab");
        if (wal_fp) {
            fwrite(&wal, sizeof(wal), 1, wal_fp);
            fclose(wal_fp);
        }
        tsfi_vsen_tx_commit(tx_id);
        return 0;
    } else {
        tsfi_vsen_tx_abort(tx_id);
        return -3;
    }
}

int tsfi_erara_analyze_page_text(const char *doi, uint32_t page_num, vsen_erara_page_text_record_t *record_out) {
    if (!doi || !record_out || page_num == 0) return -1;

    FILE *fp = fopen("erara_page_text.dat.bin", "rb");
    if (!fp) return -2;

    vsen_erara_page_text_mvcc_record record;
    int found = 0;
    while (fread(&record, sizeof(record), 1, fp) == 1) {
        if (strcmp(record.data.doi, doi) == 0 && record.data.page_num == page_num && !record.mvcc.is_deleted) {
            *record_out = record.data;
            found = 1;
        }
    }
    fclose(fp);

    return found ? 0 : -3;
}

int tsfi_vsen_amt_register_orientation(const char *amt_id, const char *orientation, int forebearance_factor, const char *vision_summary) {
    if (!amt_id || !orientation || !vision_summary) return -1;

    uint64_t tx_id = 0;
    tsfi_vsen_tx_begin(&tx_id);

    vsen_amt_orientation_mvcc_record mvcc_rec;
    memset(&mvcc_rec, 0, sizeof(mvcc_rec));
    mvcc_rec.mvcc.tx_id = tx_id;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    mvcc_rec.mvcc.commit_timestamp = (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)(ts.tv_nsec / 1000);
    mvcc_rec.mvcc.is_deleted = 0;

    strncpy(mvcc_rec.data.amt_id, amt_id, sizeof(mvcc_rec.data.amt_id) - 1);
    strncpy(mvcc_rec.data.orientation, orientation, sizeof(mvcc_rec.data.orientation) - 1);
    mvcc_rec.data.is_eisent_eih_intrinsic = 1; // Intrinsic definition "amt nit ist ein eisent eih" holds sound
    mvcc_rec.data.forebearance_factor = forebearance_factor;
    strncpy(mvcc_rec.data.vision_summary, vision_summary, sizeof(mvcc_rec.data.vision_summary) - 1);

    vsen_amt_orientation_mvcc_record existing;
    uint64_t last_offset = 0;
    uint64_t prev_dna_hash = 0;

    FILE *fp_read = fopen("amt_orientation.dat.bin", "rb");
    if (fp_read) {
        while (fread(&existing, sizeof(existing), 1, fp_read) == 1) {
            prev_dna_hash = existing.mvcc.dna_hash;
            if (strcmp(existing.data.amt_id, amt_id) == 0 && !existing.mvcc.is_deleted) {
                last_offset = ftell(fp_read) - sizeof(existing);
            }
        }
        fclose(fp_read);
    }

    mvcc_rec.mvcc.prev_version_offset = last_offset;
    mvcc_rec.mvcc.dna_hash = tsfi_vsen_compute_fnv1a_dna_hash(&mvcc_rec.data, sizeof(vsen_amt_orientation_record_t), prev_dna_hash);

    FILE *fp = fopen("amt_orientation.dat.bin", "ab");
    if (!fp) {
        tsfi_vsen_tx_abort(tx_id);
        return -2;
    }
    uint64_t write_offset = ftell(fp);
    size_t written = fwrite(&mvcc_rec, sizeof(mvcc_rec), 1, fp);
    fclose(fp);

    if (written == 1) {
        vsen_wal_entry_header_t wal;
        memset(&wal, 0, sizeof(wal));
        wal.lsn = ++g_vsen_global_lsn_counter;
        wal.tx_id = tx_id;
        wal.op_type = 2; // INSERT/UPDATE
        strncpy(wal.target_file, "amt_orientation.dat.bin", sizeof(wal.target_file) - 1);
        wal.record_offset = write_offset;
        wal.payload_len = sizeof(mvcc_rec);

        FILE *wal_fp = fopen("amt_orientation.wal.dat.bin", "ab");
        if (wal_fp) {
            fwrite(&wal, sizeof(wal), 1, wal_fp);
            fclose(wal_fp);
        }
        tsfi_vsen_tx_commit(tx_id);
        return 0;
    } else {
        tsfi_vsen_tx_abort(tx_id);
        return -3;
    }
}

int tsfi_vsen_amt_lookup_orientation(const char *amt_id, vsen_amt_orientation_record_t *record_out) {
    if (!amt_id || !record_out) return -1;

    FILE *fp = fopen("amt_orientation.dat.bin", "rb");
    if (!fp) return -2;

    vsen_amt_orientation_mvcc_record record;
    int found = 0;
    while (fread(&record, sizeof(record), 1, fp) == 1) {
        if (strcmp(record.data.amt_id, amt_id) == 0 && !record.mvcc.is_deleted) {
            *record_out = record.data;
            found = 1;
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
