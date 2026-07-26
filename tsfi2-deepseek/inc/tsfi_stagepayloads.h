#ifndef TSFI_STAGEPAYLOADS_H
#define TSFI_STAGEPAYLOADS_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_STAGE_PAYLOADS 8

typedef struct {
    char prim_path[64];
    char payload_path[64];
    bool is_loaded;
} TSFiStagePayload;

typedef struct {
    TSFiStagePayload payloads[TSFI_MAX_STAGE_PAYLOADS];
    int payload_count;
} TSFiStagePayloadsTable;

// Initialize the StagePayloads table context
void tsfi_stagepayloads_init(TSFiStagePayloadsTable *table);

// Adds a new payload reference link to a specific path
bool tsfi_stagepayloads_add(TSFiStagePayloadsTable *table, const char *prim_path, const char *payload_path);

// Loads a payload reference, making its coordinates available
bool tsfi_stagepayloads_load(TSFiStagePayloadsTable *table, const char *prim_path);

// Unloads a payload reference to save memory
bool tsfi_stagepayloads_unload(TSFiStagePayloadsTable *table, const char *prim_path);

// Checks if a payload is active and loaded
bool tsfi_stagepayloads_is_loaded(const TSFiStagePayloadsTable *table, const char *prim_path);

#endif // TSFI_STAGEPAYLOADS_H
