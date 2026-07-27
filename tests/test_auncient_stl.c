#include "../src/auncient_stl_loader.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT STL BINARY LOADER TEST SUITE\n");
    printf("=============================================================\n");

    // Write a dummy binary STL file to disk for testing
    const char *test_path = "tests/test_dummy.stl";
    FILE *file = fopen(test_path, "wb");
    assert(file != NULL);

    // Write 80-byte header
    uint8_t header[80] = {0};
    strcpy((char *)header, "Auncient Dummy STL Header");
    fwrite(header, 1, 80, file);

    // Write 1 facet count
    uint32_t count = 1;
    fwrite(&count, 4, 1, file);

    // Write 1 facet data
    AuncientStlFacet facet = {
        .normal = { 0.0f, 0.0f, 1.0f },
        .vertices = {
            { 0.0f, 0.0f, 0.0f },
            { 1.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f }
        },
        .attribute_byte_count = 0
    };
    fwrite(&facet, sizeof(AuncientStlFacet), 1, file);
    fclose(file);

    // Load and verify
    AuncientStlMesh mesh = {0};
    bool success = auncient_stl_load_binary(test_path, &mesh);
    assert(success == true);
    assert(mesh.facet_count == 1);
    assert(mesh.facets[0].normal[2] == 1.0f);
    assert(mesh.facets[0].vertices[1][0] == 1.0f);

    auncient_stl_free_mesh(&mesh);
    remove(test_path);

    printf("=============================================================\n");
    printf("ALL AUNCIENT STL BINARY LOADER TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
