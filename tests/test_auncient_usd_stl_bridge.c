#include "../src/auncient_usd_stl_bridge.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT USD-STL COOPERATIVE INTERFACE TEST SUITE\n");
    printf("=============================================================\n");

    // Initialize mock STL mesh
    AuncientStlFacet facet = {
        .normal = { 0.0f, 1.0f, 0.0f },
        .vertices = {
            { 0.0f, 0.0f, 0.0f },
            { 1.0f, 0.0f, 0.0f },
            { 0.5f, 1.0f, 0.0f }
        },
        .attribute_byte_count = 0
    };
    AuncientStlMesh mock_stl = {
        .facet_count = 1,
        .facets = &facet
    };

    const char *usda_output_path = "tests/test_bridged.usda";

    // Bridge STL to USDA
    bool success = auncient_bridge_stl_to_usda(&mock_stl, usda_output_path);
    assert(success == true);

    // Read back USDA file to verify structure contains Mesh token
    FILE *file = fopen(usda_output_path, "r");
    assert(file != NULL);
    char buf[128];
    bool found_mesh = false;
    while (fgets(buf, sizeof(buf), file)) {
        if (strstr(buf, "def Mesh \"TeddyBearMesh\"") != NULL) {
            found_mesh = true;
            break;
        }
    }
    fclose(file);
    assert(found_mesh == true);

    // Clean up
    remove(usda_output_path);

    // 2. Test reverse conversion USDA back to ASCII STL
    printf("[TEST] Converting USDA mesh structure back to ASCII STL...\n");
    float points[9] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };
    uint32_t indices[3] = { 0, 1, 2 };
    const char *stl_output_path = "tests/test_reverse.stl";

    bool rev_success = auncient_bridge_usda_to_stl(points, 3, indices, 3, stl_output_path);
    assert(rev_success == true);

    FILE *stl_file = fopen(stl_output_path, "r");
    assert(stl_file != NULL);
    char stl_buf[128];
    bool found_solid = false;
    while (fgets(stl_buf, sizeof(stl_buf), stl_file)) {
        if (strstr(stl_buf, "solid GeneratedMesh") != NULL) {
            found_solid = true;
            break;
        }
    }
    fclose(stl_file);
    assert(found_solid == true);
    remove(stl_output_path);
    printf("   ✓ Reverse conversion back to ASCII STL verified.\n");

    printf("=============================================================\n");
    printf("ALL AUNCIENT USD-STL BRIDGE TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
