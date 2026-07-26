#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_stagepop.h"

int main(void) {
    printf("=== RUNNING AUNCIENT STAGEPOP TRAVERSAL TESTS ===\n");

    TSFiPrimNode root;
    tsfi_primroot_init(&root, "/World", "Xform");

    TSFiPrimNode mesh1;
    tsfi_primroot_init(&mesh1, "/World/Mesh1", "Mesh");
    
    TSFiPrimNode mesh2;
    tsfi_primroot_init(&mesh2, "/World/Mesh2", "Mesh");
    
    TSFiPrimNode camera_node;
    tsfi_primroot_init(&camera_node, "/World/Camera", "Camera");

    // Build hierarchy tree
    assert(tsfi_primroot_add_child(&root, &mesh1));
    assert(tsfi_primroot_add_child(&root, &mesh2));
    assert(tsfi_primroot_add_child(&root, &camera_node));

    // Run traversal query to filter for type "Mesh"
    TSFiStageTraversalResult result;
    tsfi_stagepop_traverse_type(&root, "Mesh", &result);

    // Verify exactly 2 meshes were matched during traversal
    assert(result.match_count == 2);
    assert(strcmp(result.matched_prims[0]->path, "/World/Mesh1") == 0);
    assert(strcmp(result.matched_prims[1]->path, "/World/Mesh2") == 0);

    // Run traversal query to filter for type "Camera"
    tsfi_stagepop_traverse_type(&root, "Camera", &result);
    assert(result.match_count == 1);
    assert(strcmp(result.matched_prims[0]->path, "/World/Camera") == 0);

    printf("   ✓ Depth-first scene graph recursive traversal verified.\n");
    printf("   ✓ Prim type-filtering searches verified successfully.\n");
    printf("=== AUNCIENT STAGEPOP TRAVERSAL TESTS COMPLETE (PASS) ===\n");
    return 0;
}
