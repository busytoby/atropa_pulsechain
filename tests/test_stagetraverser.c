#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_stagetraverser.h"

int main(void) {
    printf("=== RUNNING AUNCIENT STAGETRAVERSER VALIDATION TESTS ===\n");

    TSFiPrimNode root;
    tsfi_primroot_init(&root, "/World", "Xform");

    TSFiPrimNode mesh1;
    tsfi_primroot_init(&mesh1, "/World/Mesh1", "Mesh");

    TSFiPrimNode mesh2;
    tsfi_primroot_init(&mesh2, "/World/Mesh2", "Mesh");
    
    // Set mesh2 inactive
    assert(tsfi_primroot_set_attribute(&mesh2, "active", 0.0f));

    // Connect child links
    assert(tsfi_primroot_add_child(&root, &mesh1));
    assert(tsfi_primroot_add_child(&root, &mesh2));

    // Case 1: Traverse with include_inactive = false
    TSFiTraversalPredicate pred1;
    tsfi_stagetraverser_init_predicate(&pred1, false, "Mesh");
    
    TSFiTraversalRange range;
    tsfi_stagetraverser_traverse(&root, &pred1, &range);
    
    // Should prune mesh2 and match only mesh1
    assert(range.count == 1);
    assert(strcmp(range.nodes[0]->path, "/World/Mesh1") == 0);

    // Case 2: Traverse with include_inactive = true
    TSFiTraversalPredicate pred2;
    tsfi_stagetraverser_init_predicate(&pred2, true, "Mesh");
    
    tsfi_stagetraverser_traverse(&root, &pred2, &range);
    
    // Should match both meshes
    assert(range.count == 2);

    printf("   ✓ StageTraverser active filtering and pruning verified.\n");
    printf("   ✓ Conditional traversal predicate checks verified successfully.\n");
    printf("=== AUNCIENT STAGETRAVERSER VALIDATION TESTS COMPLETE (PASS) ===\n");
    return 0;
}
