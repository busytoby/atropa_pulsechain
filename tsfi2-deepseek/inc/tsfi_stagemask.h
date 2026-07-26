#ifndef TSFI_STAGEMASK_H
#define TSFI_STAGEMASK_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_MASK_PATHS 8

typedef struct {
    char allowed_prefixes[TSFI_MAX_MASK_PATHS][64];
    int prefix_count;
    bool is_enabled;
} TSFiStageMask;

// Initialize the StageMask context
void tsfi_stagemask_init(TSFiStageMask *mask);

// Adds an allowed namespace path prefix to the mask filter list
bool tsfi_stagemask_add_prefix(TSFiStageMask *mask, const char *prefix);

// Checks if a queried path is allowed by the active population mask filter
bool tsfi_stagemask_is_path_allowed(const TSFiStageMask *mask, const char *path);

#endif // TSFI_STAGEMASK_H
