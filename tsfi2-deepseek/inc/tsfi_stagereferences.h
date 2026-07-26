#ifndef TSFI_STAGEREFERENCES_H
#define TSFI_STAGEREFERENCES_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_STAGE_REFERENCES 8

typedef struct {
    char prim_path[64];
    char asset_path[128];
    char target_prim_path[64];
    bool is_active;
} TSFiStageReference;

typedef struct {
    TSFiStageReference refs[TSFI_MAX_STAGE_REFERENCES];
    int ref_count;
} TSFiStageReferencesTable;

// Initialize the StageReferences table context
void tsfi_stagereferences_init(TSFiStageReferencesTable *table);

// Adds a new reference linking a stage path to an external asset prim path
bool tsfi_stagereferences_add(TSFiStageReferencesTable *table, const char *prim_path, const char *asset_path, const char *target_prim);

// Resolves and returns the external asset path associated with a referenced prim path
const char* tsfi_stagereferences_resolve_asset(const TSFiStageReferencesTable *table, const char *prim_path);

#endif // TSFI_STAGEREFERENCES_H
