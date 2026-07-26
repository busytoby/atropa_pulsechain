#include "tsfi_stagemetadata.h"
#include <string.h>

void tsfi_stagemetadata_init(TSFiStageMetadata *meta, const char *path) {
    if (!meta) return;
    
    if (path) {
        strncpy(meta->object_path, path, sizeof(meta->object_path) - 1);
    } else {
        strcpy(meta->object_path, "/World");
    }
    
    meta->entry_count = 0;
    memset(meta->entries, 0, sizeof(meta->entries));
}

bool tsfi_stagemetadata_set(TSFiStageMetadata *meta, const char *key, const char *value) {
    if (!meta || !key || !value) return false;
    
    // Update if key exists
    for (int i = 0; i < meta->entry_count; i++) {
        if (strcmp(meta->entries[i].key, key) == 0) {
            strncpy(meta->entries[i].value, value, sizeof(meta->entries[i].value) - 1);
            return true;
        }
    }
    
    if (meta->entry_count >= TSFI_MAX_METADATA_ENTRIES) return false;
    
    TSFiMetadataEntry *e = &meta->entries[meta->entry_count];
    strncpy(e->key, key, sizeof(e->key) - 1);
    strncpy(e->value, value, sizeof(e->value) - 1);
    
    meta->entry_count++;
    return true;
}

const char* tsfi_stagemetadata_get(const TSFiStageMetadata *meta, const char *key, const char *default_val) {
    if (!meta || !key) return default_val;
    
    for (int i = 0; i < meta->entry_count; i++) {
        if (strcmp(meta->entries[i].key, key) == 0) {
            return meta->entries[i].value;
        }
    }
    
    return default_val;
}
