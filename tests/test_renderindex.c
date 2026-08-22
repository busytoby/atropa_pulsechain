#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_renderindex.h"

int main(void) {
    printf("=== RUNNING AUNCIENT HYDRA RENDER INDEX (RENDERINDEX) TESTS ===\n");

    TSFiHydraScene scene;
    tsfi_hydrascene_init(&scene);

    TSFiRenderDelegate render;
    tsfi_renderdelegate_init(&render);

    TSFiRenderIndex index;
    tsfi_renderindex_init(&index, &scene, &render);

    assert(index.prim_count == 0);

    // 1. Register a primitive
    bool ok = tsfi_renderindex_register_prim(&index, "/auncient/assets/teddy_bear");
    assert(ok == true);
    assert(index.prim_count == 1);
    assert(index.primitives[0].is_dirty == true);

    // 2. Synchronize delegates
    ok = tsfi_renderindex_sync(&index);
    assert(ok == true); // Sync occurred
    assert(index.primitives[0].is_dirty == false);
    assert(render.drawn_primitive_count == 8);

    // 3. Sync again (should do nothing because dirty bit is clean)
    ok = tsfi_renderindex_sync(&index);
    assert(ok == false); // No sync occurred

    // 4. Dirty the primitive and sync again
    ok = tsfi_renderindex_register_prim(&index, "/auncient/assets/teddy_bear");
    assert(ok == true);
    assert(index.primitives[0].is_dirty == true);
    ok = tsfi_renderindex_sync(&index);
    assert(ok == true);

    printf("   ✓ Render index registration and delegate sync verified successfully.\n");
    printf("=== AUNCIENT HYDRA RENDER INDEX (RENDERINDEX) TESTS COMPLETE (PASS) ===\n");
    return 0;
}
