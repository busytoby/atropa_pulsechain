#include "tsfi_stagerelationship.h"
#include <string.h>

void tsfi_stagerelationship_init(TSFiPrimRelationships *pr, const char *path) {
    if (!pr) return;
    
    if (path) {
        strncpy(pr->prim_path, path, sizeof(pr->prim_path) - 1);
    } else {
        strcpy(pr->prim_path, "/World");
    }
    
    pr->relationship_count = 0;
    memset(pr->relationships, 0, sizeof(pr->relationships));
}

bool tsfi_stagerelationship_add_target(TSFiPrimRelationships *pr, const char *rel_name, const char *target_path) {
    if (!pr || !rel_name || !target_path) return false;
    
    // Find or create relationship
    TSFiStageRelationship *rel = NULL;
    for (int i = 0; i < pr->relationship_count; i++) {
        if (strcmp(pr->relationships[i].name, rel_name) == 0) {
            rel = &pr->relationships[i];
            break;
        }
    }
    
    if (!rel) {
        if (pr->relationship_count >= TSFI_MAX_RELATIONSHIPS) return false;
        rel = &pr->relationships[pr->relationship_count];
        strncpy(rel->name, rel_name, sizeof(rel->name) - 1);
        rel->target_count = 0;
        pr->relationship_count++;
    }
    
    // Add target if not duplicate
    for (int j = 0; j < rel->target_count; j++) {
        if (strcmp(rel->targets[j], target_path) == 0) {
            return true;
        }
    }
    
    if (rel->target_count >= TSFI_MAX_RELATIONSHIP_TARGETS) return false;
    strncpy(rel->targets[rel->target_count], target_path, sizeof(rel->targets[rel->target_count]) - 1);
    rel->target_count++;
    
    return true;
}

const char* tsfi_stagerelationship_get_first_target(const TSFiPrimRelationships *pr, const char *rel_name) {
    if (!pr || !rel_name) return NULL;
    
    for (int i = 0; i < pr->relationship_count; i++) {
        if (strcmp(pr->relationships[i].name, rel_name) == 0) {
            if (pr->relationships[i].target_count > 0) {
                return pr->relationships[i].targets[0];
            }
            return NULL;
        }
    }
    
    return NULL;
}
