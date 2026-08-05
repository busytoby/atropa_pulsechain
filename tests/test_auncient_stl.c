#include "../tsfi2-deepseek/src/auncient_stl_loader.h"
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

    // Load and verify binary via unified loader
    AuncientStlMesh mesh = {0};
    bool success = auncient_stl_load(test_path, &mesh);
    assert(success == true);
    assert(mesh.facet_count == 1);
    assert(mesh.facets[0].normal[2] == 1.0f);
    assert(mesh.facets[0].vertices[1][0] == 1.0f);

    auncient_stl_free_mesh(&mesh);
    remove(test_path);

    // Write a dummy ASCII STL file for testing
    const char *test_ascii_path = "tests/test_dummy_ascii.stl";
    FILE *ascii_file = fopen(test_ascii_path, "w");
    assert(ascii_file != NULL);
    fprintf(ascii_file, "solid Dummy\n");
    fprintf(ascii_file, "  facet normal 0.0 0.0 1.0\n");
    fprintf(ascii_file, "    outer loop\n");
    fprintf(ascii_file, "      vertex 0.0 0.0 0.0\n");
    fprintf(ascii_file, "      vertex 1.0 0.0 0.0\n");
    fprintf(ascii_file, "      vertex 0.0 1.0 0.0\n");
    fprintf(ascii_file, "    endloop\n");
    fprintf(ascii_file, "  endfacet\n");
    fprintf(ascii_file, "endsolid Dummy\n");
    fclose(ascii_file);

    AuncientStlMesh mesh_ascii = {0};
    bool success_ascii = auncient_stl_load(test_ascii_path, &mesh_ascii);
    assert(success_ascii == true);
    assert(mesh_ascii.facet_count == 1);
    assert(mesh_ascii.facets[0].normal[2] == 1.0f);
    assert(mesh_ascii.facets[0].vertices[1][0] == 1.0f);

    auncient_stl_free_mesh(&mesh_ascii);
    remove(test_ascii_path);

    printf("=============================================================\n");
    printf("ALL AUNCIENT STL BINARY AND ASCII LOADER TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
