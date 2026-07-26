#ifndef TSFI_STAGECACHE_H
#define TSFI_STAGECACHE_H

#include "tsfi_stageroot.h"
#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_CACHED_STAGES 8

typedef struct {
    uint32_t cache_id;
    TSFiStageRoot stage;
    bool is_valid;
} TSFiCachedStageSlot;

typedef struct {
    TSFiCachedStageSlot slots[TSFI_MAX_CACHED_STAGES];
    uint32_t next_id;
} TSFiStageCache;

// Initialize the StageCache context
void tsfi_stagecache_init(TSFiStageCache *cache);

// Inserts an active StageRoot into the cache and returns its assigned cache ID
uint32_t tsfi_stagecache_insert(TSFiStageCache *cache, const TSFiStageRoot *stage);

// Finds and retrieves a pointer to a cached StageRoot based on its cache ID
TSFiStageRoot* tsfi_stagecache_find(TSFiStageCache *cache, uint32_t cache_id);

// Invalidates and evicts a cached stage from the slot list
bool tsfi_stagecache_invalidate(TSFiStageCache *cache, uint32_t cache_id);

#endif // TSFI_STAGECACHE_H
