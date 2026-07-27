#include "auncient_usd_stl_bridge.h"
#include <stdio.h>
#include <stdlib.h>

bool auncient_bridge_stl_to_usda(const AuncientStlMesh *stl_mesh, const char *usda_filepath) {
    if (!stl_mesh || !usda_filepath) {
        return false;
    }

    FILE *file = fopen(usda_filepath, "w");
    if (!file) {
        printf("[USD_STL_BRIDGE ERROR] Failed to create output file: %s\n", usda_filepath);
        return false;
    }

    // Write USDA Header
    fprintf(file, "#usda 1.0\n(\n    upAxis = \"Y\"\n)\n\n");
    fprintf(file, "def Mesh \"TeddyBearMesh\"\n{\n");

    // 1. Write unique points list
    fprintf(file, "    point3f[] points = [\n");
    for (uint32_t i = 0; i < stl_mesh->facet_count; i++) {
        const AuncientStlFacet *f = &stl_mesh->facets[i];
        for (int v = 0; v < 3; v++) {
            fprintf(file, "        (%.6f, %.6f, %.6f),\n", 
                    f->vertices[v][0], f->vertices[v][1], f->vertices[v][2]);
        }
    }
    fprintf(file, "    ]\n");

    // 2. Write face vertex counts (STL is strictly triangles, so 3 vertices per face)
    fprintf(file, "    int[] faceVertexCounts = [\n");
    for (uint32_t i = 0; i < stl_mesh->facet_count; i++) {
        fprintf(file, "        3,\n");
    }
    fprintf(file, "    ]\n");

    // 3. Write face vertex indices sequential order
    fprintf(file, "    int[] faceVertexIndices = [\n");
    for (uint32_t i = 0; i < stl_mesh->facet_count * 3; i += 3) {
        fprintf(file, "        %u, %u, %u,\n", i, i + 1, i + 2);
    }
    fprintf(file, "    ]\n");

    // 4. Write normals
    fprintf(file, "    vector3f[] normals = [\n");
    for (uint32_t i = 0; i < stl_mesh->facet_count; i++) {
        const AuncientStlFacet *f = &stl_mesh->facets[i];
        fprintf(file, "        (%.6f, %.6f, %.6f),\n", 
                f->normal[0], f->normal[1], f->normal[2]);
    }
    fprintf(file, "    ]\n");

    fprintf(file, "}\n");
    fclose(file);

    printf("[USD_STL_BRIDGE SUCCESS] Successfully bridged %u STL facets to USDA file: %s\n", 
           stl_mesh->facet_count, usda_filepath);
    return true;
}
