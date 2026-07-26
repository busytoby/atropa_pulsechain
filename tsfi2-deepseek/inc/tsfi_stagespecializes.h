#ifndef TSFI_STAGESPECIALIZES_H
#define TSFI_STAGESPECIALIZES_H

#include "tsfi_primroot.h"
#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_SPECIALIZES_LINKS 8

typedef struct {
    char prim_path[64];
    char specializes_path[64];
    bool is_active;
} TSFiStageSpecializesLink;

typedef struct {
    TSFiStageSpecializesLink links[TSFI_MAX_SPECIALIZES_LINKS];
    int link_count;
} TSFiStageSpecializesTable;

// Initialize the StageSpecializes context
void tsfi_stagespecializes_init(TSFiStageSpecializesTable *table);

// Adds a specializes link between a prim path and an ancestral specialized template path
bool tsfi_stagespecializes_add(TSFiStageSpecializesTable *table, const char *prim_path, const char *spec_path);

// Resolves and returns the specialized template path for a given prim path
const char* tsfi_stagespecializes_resolve_spec(const TSFiStageSpecializesTable *table, const char *prim_path);

#endif // TSFI_STAGESPECIALIZES_H
