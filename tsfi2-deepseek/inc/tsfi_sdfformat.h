#ifndef TSFI_SDFFORMAT_H
#define TSFI_SDFFORMAT_H

#include <stdint.h>
#include <stdbool.h>

// Sdf File Format Plugin representation
typedef struct {
    char format_name[32];
    char target_extension[16];
} TSFiSdfFormat;

// Initialize the file format plugin context
void tsfi_sdfformat_init(TSFiSdfFormat *sdf);

// Verifies if the file path complies with the required .dat.bin extension constraint
bool tsfi_sdfformat_verify_extension(const TSFiSdfFormat *sdf, const char *file_path);

// Simulates parsing the binary quadtree index file into USD primitive count
int tsfi_sdfformat_parse_primitives(const TSFiSdfFormat *sdf, const char *file_path);

#endif // TSFI_SDFFORMAT_H
