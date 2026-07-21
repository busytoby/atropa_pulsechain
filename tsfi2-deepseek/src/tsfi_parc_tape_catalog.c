#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include "tsfi_parc_tape_catalog.h"
#include "tsfi_parc_tape_label_yul.h"

static int g_hdl_counter = 37;
static int g_rdb_counter = 12;
static int g_uni_counter = 14;
static int g_ast_counter = 8;
static int g_sys_counter = 12;
static int g_vmx_counter = 10;
static int g_vol_counter = 30;

// Generate a domain-prefixed unique 6-character Volume ID
static void generate_unique_vol_id(const char *filename, int index, char *vol_id_out) {
    unsigned int idx = (unsigned int)(index > 999 ? 999 : index);
    if (strstr(filename, "holders_")) {
        snprintf(vol_id_out, 7, "HDL%03u", idx);
    } else if (strstr(filename, "rdbms_") || strstr(filename, "ledger")) {
        snprintf(vol_id_out, 7, "RDB%03u", idx);
    } else if (strstr(filename, "uniservo") || strstr(filename, "mmap")) {
        snprintf(vol_id_out, 7, "UNI%03u", idx);
    } else if (strstr(filename, "graph")) {
        snprintf(vol_id_out, 7, "AST%03u", idx);
    } else if (strstr(filename, "hogan") || strstr(filename, "lfs")) {
        snprintf(vol_id_out, 7, "SYS%03u", idx);
    } else if (strstr(filename, "evm") || strstr(filename, "vm")) {
        snprintf(vol_id_out, 7, "VMX%03u", idx);
    } else {
        snprintf(vol_id_out, 7, "VOL%03u", idx);
    }
}

// Format a meaningful 17-character File ID ending strictly with .dat.bin (Rule 13)
static void generate_meaningful_file_id(const char *filename, char *file_id_out) {
    char clean_name[64] = {0};
    const char *dot = strstr(filename, ".dat.bin");
    int name_len = dot ? (int)(dot - filename) : (int)strlen(filename);
    if (name_len > 9) name_len = 9; // Leave room for .DAT.BIN (8 chars)

    strncpy(clean_name, filename, name_len);
    clean_name[name_len] = '\0';

    // Convert to uppercase
    for (int i = 0; i < name_len; i++) {
        if (clean_name[i] >= 'a' && clean_name[i] <= 'z') {
            clean_name[i] -= 32;
        }
    }

    snprintf(file_id_out, 18, "%s.DAT.BIN", clean_name);
}

int tsfi_tape_catalog_next_id(const char *domain_prefix, char *out_vol_id) {
    if (!domain_prefix || !out_vol_id) return -1;

    int next_val = 1;
    if (strcmp(domain_prefix, "HDL") == 0) next_val = ++g_hdl_counter;
    else if (strcmp(domain_prefix, "RDB") == 0) next_val = ++g_rdb_counter;
    else if (strcmp(domain_prefix, "UNI") == 0) next_val = ++g_uni_counter;
    else if (strcmp(domain_prefix, "AST") == 0) next_val = ++g_ast_counter;
    else if (strcmp(domain_prefix, "SYS") == 0) next_val = ++g_sys_counter;
    else if (strcmp(domain_prefix, "VMX") == 0) next_val = ++g_vmx_counter;
    else next_val = ++g_vol_counter;

    snprintf(out_vol_id, 7, "%.3s%03u", domain_prefix, (unsigned int)(next_val > 999 ? 999 : next_val));
    return 0;
}

FILE *tsfi_tape_open_guarded(const char *file_path, const char *mode, uint8_t process_clearance) {
    if (!file_path || !mode) return NULL;

    // Rule 13: Must end strictly with .dat.bin
    if (!strstr(file_path, ".dat.bin") && !strstr(file_path, ".DAT.BIN")) {
        printf("[SECURITY] Access Denied: File %s does not satisfy .dat.bin Rule 13\n", file_path);
        return NULL;
    }

    FILE *f = fopen(file_path, mode);
    if (!f) return NULL;

    // If writing a new file, inscribe 720-byte Yul DDL header block
    if (strchr(mode, 'w') || strchr(mode, 'a')) {
        uint8_t header_buf[720];
        char vol_id[7];
        char file_id[18];
        tsfi_tape_catalog_next_id("VOL", vol_id);
        generate_meaningful_file_id(file_path, file_id);

        tsfi_tape_label_yul_format_full_header(
            header_buf, vol_id, file_id, TAPE_SECURITY_UNCLASSIFIED,
            0.0f, 0.0f, 1024.0f, 1024.0f, NULL, NULL,
            0.0f, 1, 1, 1, 32, 30
        );
        fwrite(header_buf, 1, 720, f);
        fflush(f);
    } else {
        // If reading existing file, validate 720-byte sequence & governance clearance
        uint8_t header_buf[720];
        size_t nread = fread(header_buf, 1, 720, f);
        fseek(f, 0, SEEK_SET);

        if (nread == 720) {
            int gov_res = tsfi_tape_label_yul_check_governance(header_buf, process_clearance);
            if (gov_res != 0) {
                printf("[SECURITY] Access Denied: Governance check failed for %s (Code %d)\n", file_path, gov_res);
                fclose(f);
                return NULL;
            }
        }
    }

    return f;
}

int tsfi_tape_catalog_process_all(const char *dir_path, tsfi_tape_catalog_entry_t *entries_out, int max_entries) {
    if (!dir_path || !entries_out || max_entries <= 0) return -1;

    DIR *dir = opendir(dir_path);
    if (!dir) return -2;

    struct dirent *entry;
    int count = 0;

    while ((entry = readdir(dir)) != NULL && count < max_entries) {
        if (strstr(entry->d_name, ".dat.bin") != NULL) {
            int written = snprintf(entries_out[count].file_path, sizeof(entries_out[count].file_path), "%s/%s", dir_path, entry->d_name);
            if (written < 0 || written >= (int)sizeof(entries_out[count].file_path)) {
                entries_out[count].file_path[sizeof(entries_out[count].file_path) - 1] = '\0';
            }
            generate_unique_vol_id(entry->d_name, count + 1, entries_out[count].volume_id);
            generate_meaningful_file_id(entry->d_name, entries_out[count].file_id);
            entries_out[count].security_level = TAPE_SECURITY_UNCLASSIFIED;
            entries_out[count].is_valid = 1;

            // Inscribe 720-byte VOL1..HDR8 full header onto disk file
            FILE *f = fopen(entries_out[count].file_path, "r+b");
            if (f) {
                uint8_t header_buf[720];
                tsfi_tape_label_yul_format_full_header(
                    header_buf, entries_out[count].volume_id, entries_out[count].file_id,
                    TAPE_SECURITY_UNCLASSIFIED, 0.0f, 0.0f, 1024.0f, 1024.0f, NULL, NULL,
                    0.0f, 1, 1, 1, 32, 30
                );
                fwrite(header_buf, 1, 720, f);
                fclose(f);
            }

            count++;
        }
    }

    closedir(dir);
    return count;
}
