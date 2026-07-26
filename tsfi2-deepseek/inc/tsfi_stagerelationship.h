#ifndef TSFI_STAGERELATIONSHIP_H
#define TSFI_STAGERELATIONSHIP_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_RELATIONSHIP_TARGETS 4
#define TSFI_MAX_RELATIONSHIPS 8

typedef struct {
    char name[32];
    char targets[TSFI_MAX_RELATIONSHIP_TARGETS][64];
    int target_count;
} TSFiStageRelationship;

typedef struct {
    char prim_path[64];
    TSFiStageRelationship relationships[TSFI_MAX_RELATIONSHIPS];
    int relationship_count;
} TSFiPrimRelationships;

// Initialize the PrimRelationships context
void tsfi_stagerelationship_init(TSFiPrimRelationships *pr, const char *path);

// Adds a target path to a relationship name on the primitive
bool tsfi_stagerelationship_add_target(TSFiPrimRelationships *pr, const char *rel_name, const char *target_path);

// Returns the first target path of a relationship, or NULL if not found
const char* tsfi_stagerelationship_get_first_target(const TSFiPrimRelationships *pr, const char *rel_name);

#endif // TSFI_STAGERELATIONSHIP_H
