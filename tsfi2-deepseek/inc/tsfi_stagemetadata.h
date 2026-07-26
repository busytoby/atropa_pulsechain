#ifndef TSFI_STAGEMETADATA_H
#define TSFI_STAGEMETADATA_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_METADATA_ENTRIES 8

typedef struct {
    char key[32];
    char value[64];
} TSFiMetadataEntry;

typedef struct {
    char object_path[64];
    TSFiMetadataEntry entries[TSFI_MAX_METADATA_ENTRIES];
    int entry_count;
} TSFiStageMetadata;

// Initialize the StageMetadata context
void tsfi_stagemetadata_init(TSFiStageMetadata *meta, const char *path);

// Sets a custom metadata key-value string on the object
bool tsfi_stagemetadata_set(TSFiStageMetadata *meta, const char *key, const char *value);

// Returns the metadata value associated with a key, or default if not found
const char* tsfi_stagemetadata_get(const TSFiStageMetadata *meta, const char *key, const char *default_val);

#endif // TSFI_STAGEMETADATA_H
