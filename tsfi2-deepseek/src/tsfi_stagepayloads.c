#include "tsfi_stagepayloads.h"
#include <string.h>

void tsfi_stagepayloads_init(TSFiStagePayloadsTable *table) {
    if (!table) return;
    
    table->payload_count = 0;
    memset(table->payloads, 0, sizeof(table->payloads));
}

bool tsfi_stagepayloads_add(TSFiStagePayloadsTable *table, const char *prim_path, const char *payload_path) {
    if (!table || !prim_path || !payload_path || table->payload_count >= TSFI_MAX_STAGE_PAYLOADS) return false;
    
    // Check duplicates
    for (int i = 0; i < table->payload_count; i++) {
        if (strcmp(table->payloads[i].prim_path, prim_path) == 0) {
            return true;
        }
    }
    
    TSFiStagePayload *p = &table->payloads[table->payload_count];
    strncpy(p->prim_path, prim_path, sizeof(p->prim_path) - 1);
    strncpy(p->payload_path, payload_path, sizeof(p->payload_path) - 1);
    p->is_loaded = false; // Default unloaded to save memory
    
    table->payload_count++;
    return true;
}

bool tsfi_stagepayloads_load(TSFiStagePayloadsTable *table, const char *prim_path) {
    if (!table || !prim_path) return false;
    
    for (int i = 0; i < table->payload_count; i++) {
        if (strcmp(table->payloads[i].prim_path, prim_path) == 0) {
            table->payloads[i].is_loaded = true;
            return true;
        }
    }
    return false;
}

bool tsfi_stagepayloads_unload(TSFiStagePayloadsTable *table, const char *prim_path) {
    if (!table || !prim_path) return false;
    
    for (int i = 0; i < table->payload_count; i++) {
        if (strcmp(table->payloads[i].prim_path, prim_path) == 0) {
            table->payloads[i].is_loaded = false;
            return true;
        }
    }
    return false;
}

bool tsfi_stagepayloads_is_loaded(const TSFiStagePayloadsTable *table, const char *prim_path) {
    if (!table || !prim_path) return false;
    
    for (int i = 0; i < table->payload_count; i++) {
        if (strcmp(table->payloads[i].prim_path, prim_path) == 0) {
            return table->payloads[i].is_loaded;
        }
    }
    return false;
}
