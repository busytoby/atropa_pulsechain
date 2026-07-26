#ifndef TSFI_STAGEARC_H
#define TSFI_STAGEARC_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_COMPOSITION_ARCS 8

typedef enum {
    TSFI_ARC_REFERENCE,
    TSFI_ARC_INHERIT,
    TSFI_ARC_PAYLOAD,
    TSFI_ARC_SPECIALIZES
} TSFiArcType;

typedef struct {
    char source_path[64];
    char target_path[64];
    TSFiArcType type;
    bool is_active;
} TSFiCompositionArc;

typedef struct {
    TSFiCompositionArc arcs[TSFI_MAX_COMPOSITION_ARCS];
    int arc_count;
} TSFiStageArcTable;

// Initialize the StageArc table context
void tsfi_stagearc_init(TSFiStageArcTable *table);

// Adds a composition arc reference link between paths
bool tsfi_stagearc_add(TSFiStageArcTable *table, const char *src, const char *target, TSFiArcType type);

// Evaluates and resolves target coordinates or properties through the active references stack
const char* tsfi_stagearc_resolve_target(const TSFiStageArcTable *table, const char *src, TSFiArcType type);

#endif // TSFI_STAGEARC_H
