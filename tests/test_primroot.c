#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_primroot.h"

int main(void) {
    printf("=== RUNNING AUNCIENT PRIMROOT SCHEMA TESTS ===\n");

    TSFiPrimNode root;
    tsfi_primroot_init(&root, "/World", "Xform");
    assert(strcmp(root.path, "/World") == 0);

    TSFiPrimNode camera_node;
    tsfi_primroot_init(&camera_node, "/World/Camera", "Camera");

    // Parent camera to World root
    assert(tsfi_primroot_add_child(&root, &camera_node));
    assert(root.child_count == 1);
    assert(camera_node.parent == &root);

    // Verify dynamic attribute property bindings
    assert(tsfi_primroot_set_attribute(&camera_node, "focalLength", 35.0f));
    assert(tsfi_primroot_set_attribute(&camera_node, "horizontalAperture", 36.0f));
    
    // Read and verify attribute updates
    float f = tsfi_primroot_get_attribute(&camera_node, "focalLength", 50.0f);
    assert(fabs(f - 35.0f) < 1e-5f);

    float default_val = tsfi_primroot_get_attribute(&camera_node, "clippingRange", 10.0f);
    assert(fabs(default_val - 10.0f) < 1e-5f);

    printf("   ✓ PrimRoot hierarchy child parenting links verified.\n");
    printf("   ✓ Dynamic attribute value lookup queries verified.\n");
    printf("=== AUNCIENT PRIMROOT SCHEMA TESTS COMPLETE (PASS) ===\n");
    return 0;
}
