#ifndef TSFI_PARC_TAPE_CATALOG_H
#define TSFI_PARC_TAPE_CATALOG_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

typedef struct {
    char file_path[256];
    char volume_id[7];    // 6 chars + null terminator
    char file_id[18];     // 17 chars + null terminator (.dat.bin)
    uint8_t security_level;
    int is_valid;
} tsfi_tape_catalog_entry_t;

/* Scan directory and catalog all .dat.bin tapes, applying unique VOL ids and meaningful file_ids */
int tsfi_tape_catalog_process_all(const char *dir_path, tsfi_tape_catalog_entry_t *entries_out, int max_entries);

/* Generate the next sequential Volume ID for a given domain prefix ("HDL", "RDB", "UNI", "AST", "SYS", "VMX") */
int tsfi_tape_catalog_next_id(const char *domain_prefix, char *out_vol_id);

/* Open a .dat.bin tape file with automated Yul DDL header inscription and security governance checks */
FILE *tsfi_tape_open_guarded(const char *file_path, const char *mode, uint8_t process_clearance);

#endif // TSFI_PARC_TAPE_CATALOG_H
