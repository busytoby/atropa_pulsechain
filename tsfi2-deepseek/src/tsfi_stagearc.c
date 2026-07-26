#include "tsfi_stagearc.h"
#include <string.h>

void tsfi_stagearc_init(TSFiStageArcTable *table) {
    if (!table) return;
    
    table->arc_count = 0;
    memset(table->arcs, 0, sizeof(table->arcs));
}

bool tsfi_stagearc_add(TSFiStageArcTable *table, const char *src, const char *target, TSFiArcType type) {
    if (!table || !src || !target || table->arc_count >= TSFI_MAX_COMPOSITION_ARCS) return false;
    
    // Avoid duplicate arc definitions
    for (int i = 0; i < table->arc_count; i++) {
        if (strcmp(table->arcs[i].source_path, src) == 0 &&
            strcmp(table->arcs[i].target_path, target) == 0 &&
            table->arcs[i].type == type) {
            return true;
        }
    }
    
    TSFiCompositionArc *arc = &table->arcs[table->arc_count];
    strncpy(arc->source_path, src, sizeof(arc->source_path) - 1);
    strncpy(arc->target_path, target, sizeof(arc->target_path) - 1);
    arc->type = type;
    arc->is_active = true;
    
    table->arc_count++;
    return true;
}

const char* tsfi_stagearc_resolve_target(const TSFiStageArcTable *table, const char *src, TSFiArcType type) {
    if (!table || !src) return NULL;
    
    // Scan composition table in reverse order for strongest active opinion overrides
    for (int i = table->arc_count - 1; i >= 0; i--) {
        const TSFiCompositionArc *arc = &table->arcs[i];
        if (arc->is_active && arc->type == type && strcmp(arc->source_path, src) == 0) {
            return arc->target_path;
        }
    }
    
    return NULL;
}
