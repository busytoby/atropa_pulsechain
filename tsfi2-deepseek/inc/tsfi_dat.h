#ifndef TSFI_DAT_H
#define TSFI_DAT_H

#include "tsfi_trie.h"

typedef struct {
    int *base;
    int *check;
    char **values;
    int capacity;
    char *tail;
    int tail_size;
} tsfi_dat;

// Compile a standard Trie into a Double-Array Trie (DAT) structure
tsfi_dat* tsfi_dat_compile(tsfi_trie_node *trie_root);

// Search for a key in the compiled DAT
const char* tsfi_dat_search(tsfi_dat *dat, const char *key);

// Free all resources allocated in the DAT
void tsfi_dat_destroy(tsfi_dat *dat);

// Initialize Font Ligature DAT router
tsfi_dat* tsfi_dat_init_ligature_router(void);

// Resolve font ligature using DAT
int tsfi_dat_resolve_ligature(tsfi_dat *router, const char *sequence);

// Initialize SCSI command DAT router
tsfi_dat* tsfi_dat_init_scsi_router(void);

// Resolve SCSI command using DAT prefix match
const char* tsfi_dat_resolve_scsi(tsfi_dat *router, const char *scsi_cmd);

// Save Double-Array Trie to a raw binary file
int tsfi_dat_save_bin(tsfi_dat *dat, const char *filepath);

// Load Double-Array Trie from a raw binary file
tsfi_dat* tsfi_dat_load_bin(const char *filepath);

// Generate a Bitcoin Script validating the multi-step DAT traversal path for a key
int tsfi_dat_generate_btc_script(tsfi_dat *dat, const char *key, int expected_final_state, uint8_t *script_out);

#endif // TSFI_DAT_H
