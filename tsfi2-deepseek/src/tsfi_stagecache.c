#include "tsfi_stagecache.h"
#include <string.h>

void tsfi_stagecache_init(TSFiStageCache *cache) {
    if (!cache) return;
    
    cache->next_id = 1001;
    memset(cache->slots, 0, sizeof(cache->slots));
}

uint32_t tsfi_stagecache_insert(TSFiStageCache *cache, const TSFiStageRoot *stage) {
    if (!cache || !stage) return 0;
    
    // Find an empty or invalid slot to insert
    for (int i = 0; i < TSFI_MAX_CACHED_STAGES; i++) {
        if (!cache->slots[i].is_valid) {
            TSFiCachedStageSlot *slot = &cache->slots[i];
            
            slot->cache_id = cache->next_id++;
            memcpy(&slot->stage, stage, sizeof(TSFiStageRoot));
            slot->is_valid = true;
            
            return slot->cache_id;
        }
    }
    
    return 0; // Cache full
}

TSFiStageRoot* tsfi_stagecache_find(TSFiStageCache *cache, uint32_t cache_id) {
    if (!cache || cache_id == 0) return NULL;
    
    for (int i = 0; i < TSFI_MAX_CACHED_STAGES; i++) {
        if (cache->slots[i].is_valid && cache->slots[i].cache_id == cache_id) {
            return &cache->slots[i].stage;
        }
    }
    
    return NULL;
}

bool tsfi_stagecache_invalidate(TSFiStageCache *cache, uint32_t cache_id) {
    if (!cache || cache_id == 0) return false;
    
    for (int i = 0; i < TSFI_MAX_CACHED_STAGES; i++) {
        if (cache->slots[i].is_valid && cache->slots[i].cache_id == cache_id) {
            cache->slots[i].is_valid = false;
            return true;
        }
    }
    
    return false;
}
