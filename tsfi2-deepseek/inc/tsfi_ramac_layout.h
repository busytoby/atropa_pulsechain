#ifndef TSFI_RAMAC_LAYOUT_H
#define TSFI_RAMAC_LAYOUT_H

#include "tsfi_dat.h"
#include <stdint.h>

// IBM 305 RAMAC disk geometry parameters
#define RAMAC_CYLINDERS 100
#define RAMAC_HEADS      50  // 50 disks, 1 head per surface
#define RAMAC_SECTORS    20  // 20 sectors per track
#define RAMAC_WORDS      8   // 8 words (32 bytes) per sector

typedef struct {
    int cylinder;
    int head;
    int sector;
    int word_offset;
} tsfi_ramac_chs;

// RAMAC-style cylinder overflow hash record layout
typedef struct {
    char key[32];
    char value[32];
    int next_overflow_index; // Index of next record in case of collision
    int is_active;
} tsfi_ramac_record;

// Translates a flat index to CHS coordinates
tsfi_ramac_chs tsfi_ramac_index_to_chs(int index);

// Translates CHS coordinates back to a flat index
int tsfi_ramac_chs_to_index(tsfi_ramac_chs chs);

// Calculates the estimated seek latency (in microseconds) between two flat indices
double tsfi_ramac_calculate_seek(int from_index, int to_index);

// Optimizes and serializes a Double-Array Trie (DAT) to disk using cylindrical layout
int tsfi_ramac_layout_optimize(tsfi_dat *dat, const char *filepath);

// Hashes a key to a primary sector index within a given cylinder
int tsfi_ramac_hash_key(const char *key, int cylinder);

// Inserts a record into the cylinder-overflow disk layout, keeping overflows within the same cylinder
int tsfi_ramac_insert_record(tsfi_ramac_record *disk, const char *key, const char *value, int cylinder, double *out_total_seek_us);

// Searches a record in the cylinder-overflow layout
const char* tsfi_ramac_search_record(tsfi_ramac_record *disk, const char *key, int cylinder, double *out_total_seek_us);

// IBM 305 RAMAC plugboard wiring control panel emulator
// Parses wiring rules like "0..7->8..15" and routes source buffer sections to destination
int tsfi_ramac_plugboard_route(const char *wiring, const uint8_t *src, uint8_t *dest, int max_len);

// Read-after-write verification loop (Double-Read verification)
// Returns 0 if verified, -1 if parity or content mismatch occurs
int tsfi_ramac_write_verified(tsfi_ramac_record *disk, const char *key, const char *value, int cylinder);

#endif // TSFI_RAMAC_LAYOUT_H
