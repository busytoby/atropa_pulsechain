#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_stagecache.h"

int main(void) {
    printf("=== RUNNING AUNCIENT STAGECACHE EVALUATION TESTS ===\n");

    TSFiStageCache cache;
    tsfi_stagecache_init(&cache);

    TSFiStageRoot stage1;
    tsfi_stageroot_init(&stage1, "/World/Stage1");

    // Insert stage into cache
    uint32_t id1 = tsfi_stagecache_insert(&cache, &stage1);
    assert(id1 > 0);

    // Retrieve stage from cache and verify path matching
    TSFiStageRoot *found = tsfi_stagecache_find(&cache, id1);
    assert(found != NULL);
    assert(strcmp(found->stage_path, "/World/Stage1") == 0);

    // Invalidate the cache ID and verify eviction
    assert(tsfi_stagecache_invalidate(&cache, id1));
    assert(tsfi_stagecache_find(&cache, id1) == NULL);

    printf("   ✓ StageCache stage registrations and index retrievals verified.\n");
    printf("   ✓ Cache invalidations and evictions verified successfully.\n");
    printf("=== AUNCIENT STAGECACHE EVALUATION TESTS COMPLETE (PASS) ===\n");
    return 0;
}
