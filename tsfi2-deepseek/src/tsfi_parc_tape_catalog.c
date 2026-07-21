#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include "tsfi_parc_tape_catalog.h"
#include "tsfi_parc_tape_label_yul.h"

// Generate a domain-prefixed unique 6-character Volume ID
static void generate_unique_vol_id(const char *filename, int index, char *vol_id_out) {
    unsigned int idx = (unsigned int)(index > 999 ? 999 : index);
    if (strstr(filename, "holders_")) {
        snprintf(vol_id_out, 7, "HDL%03u", idx);
    } else if (strstr(filename, "rdbms_") || strstr(filename, "ledger")) {
        snprintf(vol_id_out, 7, "RDB%03u", idx);
    } else if (strstr(filename, "uniservo") || strstr(filename, "mmap")) {
        snprintf(vol_id_out, 7, "UNI%03u", idx);
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

            // Inscribe 240-byte VOL1+HDR1+HDR2 header onto disk file if needed
            FILE *f = fopen(entries_out[count].file_path, "r+b");
            if (f) {
                uint8_t header_buf[240];
                tsfi_tape_label_yul_format_header(header_buf, entries_out[count].volume_id, entries_out[count].file_id, TAPE_SECURITY_UNCLASSIFIED);
                fwrite(header_buf, 1, 240, f);
                fclose(f);
            }

            count++;
        }
    }

    closedir(dir);
    return count;
}
