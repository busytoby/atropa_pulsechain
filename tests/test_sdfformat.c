#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_sdfformat.h"

int main(void) {
    printf("=== RUNNING AUNCIENT SDF FILE FORMAT PLUGIN (SDFFORMAT) TESTS ===\n");

    TSFiSdfFormat sdf;
    tsfi_sdfformat_init(&sdf);

    // Initial state
    assert(strcmp(sdf.format_name, "AuncientQuadtreeFormat") == 0);
    assert(strcmp(sdf.target_extension, ".dat.bin") == 0);

    // 1. Verify extension filter
    assert(tsfi_sdfformat_verify_extension(&sdf, "terrain.dat.bin") == true);
    assert(tsfi_sdfformat_verify_extension(&sdf, "terrain.json") == false);
    assert(tsfi_sdfformat_verify_extension(&sdf, "a.dat.bin") == true);
    assert(tsfi_sdfformat_verify_extension(&sdf, "bin") == false);

    // 2. Parse primitives for invalid extension
    int count = tsfi_sdfformat_parse_primitives(&sdf, "invalid_format.json");
    assert(count == -1);

    // 3. Parse primitives for valid extension (missing file fallback)
    count = tsfi_sdfformat_parse_primitives(&sdf, "missing_asset.dat.bin");
    assert(count == 128);

    printf("   ✓ SdfFileFormat extension verification verified successfully.\n");
    printf("   ✓ Fallback quadtree parser primitive count verified successfully.\n");
    printf("=== AUNCIENT SDF FILE FORMAT PLUGIN (SDFFORMAT) TESTS COMPLETE (PASS) ===\n");
    return 0;
}
