#include "tsfi_stagereferences.h"
#include <string.h>

void tsfi_stagereferences_init(TSFiStageReferencesTable *table) {
    if (!table) return;
    
    table->ref_count = 0;
    memset(table->refs, 0, sizeof(table->refs));
}

bool tsfi_stagereferences_add(TSFiStageReferencesTable *table, const char *prim_path, const char *asset_path, const char *target_prim) {
    if (!table || !prim_path || !asset_path || !target_prim || table->ref_count >= TSFI_MAX_STAGE_REFERENCES) return false;
    
    // Check duplicates
    for (int i = 0; i < table->ref_count; i++) {
        if (strcmp(table->refs[i].prim_path, prim_path) == 0 &&
            strcmp(table->refs[i].asset_path, asset_path) == 0 &&
            strcmp(table->refs[i].target_prim_path, target_prim) == 0) {
            return true;
        }
    }
    
    TSFiStageReference *r = &table->refs[table->ref_count];
    strncpy(r->prim_path, prim_path, sizeof(r->prim_path) - 1);
    strncpy(r->asset_path, asset_path, sizeof(r->asset_path) - 1);
    strncpy(r->target_prim_path, target_prim, sizeof(r->target_prim_path) - 1);
    r->is_active = true;
    
    table->ref_count++;
    return true;
}

const char* tsfi_stagereferences_resolve_asset(const TSFiStageReferencesTable *table, const char *prim_path) {
    if (!table || !prim_path) return NULL;
    
    // Scan in reverse order for strongest override opinion matching
    for (int i = table->ref_count - 1; i >= 0; i--) {
        const TSFiStageReference *r = &table->refs[i];
        if (r->is_active && strcmp(r->prim_path, prim_path) == 0) {
            return r->asset_path;
        }
    }
    
    return NULL;
}
