#ifndef TSFI_PARC_TAPE_CATALOG_H
#define TSFI_PARC_TAPE_CATALOG_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    char file_path[256];
    char volume_id[7];    // 6 chars + null terminator
    char file_id[18];     // 17 chars + null terminator (.dat.bin)
    uint8_t security_level;
    int is_valid;
} tsfi_tape_catalog_entry_t;

/* Scan directory and catalog all .dat.bin tapes, applying unique VOL ids and meaningful file_ids */
int tsfi_tape_catalog_process_all(const char *dir_path, tsfi_tape_catalog_entry_t *entries_out, int max_entries);

#endif // TSFI_PARC_TAPE_CATALOG_H
