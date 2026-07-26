#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_stagemetadata.h"

int main(void) {
    printf("=== RUNNING AUNCIENT STAGEMETADATA LOOKUP TESTS ===\n");

    TSFiStageMetadata meta;
    tsfi_stagemetadata_init(&meta, "/World/Camera");

    assert(strcmp(meta.object_path, "/World/Camera") == 0);
    assert(meta.entry_count == 0);

    // Set custom metadata strings
    assert(tsfi_stagemetadata_set(&meta, "comment", "Main Cinematic Camera"));
    assert(tsfi_stagemetadata_set(&meta, "renderTarget", "hdr_viewport"));
    assert(meta.entry_count == 2);

    // Retrieve and verify metadata values
    const char *val1 = tsfi_stagemetadata_get(&meta, "comment", "none");
    assert(strcmp(val1, "Main Cinematic Camera") == 0);

    const char *val2 = tsfi_stagemetadata_get(&meta, "renderTarget", "none");
    assert(strcmp(val2, "hdr_viewport") == 0);

    // Verify default fallback
    const char *fallback = tsfi_stagemetadata_get(&meta, "author", "unknown");
    assert(strcmp(fallback, "unknown") == 0);

    printf("   ✓ StageMetadata string updates verified.\n");
    printf("   ✓ Key-value dictionary query lookups verified successfully.\n");
    printf("=== AUNCIENT STAGEMETADATA LOOKUP TESTS COMPLETE (PASS) ===\n");
    return 0;
}
