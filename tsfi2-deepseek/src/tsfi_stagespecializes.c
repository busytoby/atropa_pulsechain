#include "tsfi_stagespecializes.h"
#include <string.h>

void tsfi_stagespecializes_init(TSFiStageSpecializesTable *table) {
    if (!table) return;
    
    table->link_count = 0;
    memset(table->links, 0, sizeof(table->links));
}

bool tsfi_stagespecializes_add(TSFiStageSpecializesTable *table, const char *prim_path, const char *spec_path) {
    if (!table || !prim_path || !spec_path || table->link_count >= TSFI_MAX_SPECIALIZES_LINKS) return false;
    
    // Check duplicates
    for (int i = 0; i < table->link_count; i++) {
        if (strcmp(table->links[i].prim_path, prim_path) == 0 &&
            strcmp(table->links[i].specializes_path, spec_path) == 0) {
            return true;
        }
    }
    
    TSFiStageSpecializesLink *link = &table->links[table->link_count];
    strncpy(link->prim_path, prim_path, sizeof(link->prim_path) - 1);
    strncpy(link->specializes_path, spec_path, sizeof(link->specializes_path) - 1);
    link->is_active = true;
    
    table->link_count++;
    return true;
}

const char* tsfi_stagespecializes_resolve_spec(const TSFiStageSpecializesTable *table, const char *prim_path) {
    if (!table || !prim_path) return NULL;
    
    // Scan in reverse order for strongest specializes opinion override matching
    for (int i = table->link_count - 1; i >= 0; i--) {
        const TSFiStageSpecializesLink *link = &table->links[i];
        if (link->is_active && strcmp(link->prim_path, prim_path) == 0) {
            return link->specializes_path;
        }
    }
    
    return NULL;
}
