#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_stagepayloads.h"

int main(void) {
    printf("=== RUNNING AUNCIENT STAGEPAYLOADS LOAD TESTS ===\n");

    TSFiStagePayloadsTable table;
    tsfi_stagepayloads_init(&table);

    // Add payload reference: '/World/Torus' references heavy mesh file
    assert(tsfi_stagepayloads_add(&table, "/World/Torus", "/Asset/HeavyTorus.dat.bin"));
    assert(table.payload_count == 1);

    // Verify default state is unloaded
    assert(tsfi_stagepayloads_is_loaded(&table, "/World/Torus") == false);

    // Load payload
    assert(tsfi_stagepayloads_load(&table, "/World/Torus"));
    assert(tsfi_stagepayloads_is_loaded(&table, "/World/Torus") == true);

    // Unload payload
    assert(tsfi_stagepayloads_unload(&table, "/World/Torus"));
    assert(tsfi_stagepayloads_is_loaded(&table, "/World/Torus") == false);

    printf("   ✓ StagePayloads link registrations verified.\n");
    printf("   ✓ Dynamic coordinate buffer load and unload states verified.\n");
    printf("=== AUNCIENT STAGEPAYLOADS LOAD TESTS COMPLETE (PASS) ===\n");
    return 0;
}
