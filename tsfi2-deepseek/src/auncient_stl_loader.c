#include "auncient_stl_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool auncient_stl_load_binary(const char *filepath, AuncientStlMesh *mesh_out) {
    if (!filepath || !mesh_out) {
        return false;
    }

    FILE *file = fopen(filepath, "rb");
    if (!file) {
        printf("[STL LOAD ERROR] Failed to open file: %s\n", filepath);
        return false;
    }

    // Binary STL header is exactly 80 bytes
    uint8_t header[80];
    if (fread(header, 1, 80, file) != 80) {
        printf("[STL LOAD ERROR] Failed to read 80-byte header.\n");
        fclose(file);
        return false;
    }

    // Number of facets is a 32-bit unsigned integer
    uint32_t num_facets = 0;
    if (fread(&num_facets, 4, 1, file) != 1) {
        printf("[STL LOAD ERROR] Failed to read facet count.\n");
        fclose(file);
        return false;
    }

    mesh_out->facet_count = num_facets;
    mesh_out->facets = (AuncientStlFacet *)malloc(sizeof(AuncientStlFacet) * num_facets);
    if (!mesh_out->facets) {
        printf("[STL LOAD ERROR] Memory allocation failed for %u facets.\n", num_facets);
        fclose(file);
        return false;
    }

    // Read facets sequentially
    size_t read_count = fread(mesh_out->facets, sizeof(AuncientStlFacet), num_facets, file);
    if (read_count != num_facets) {
        printf("[STL LOAD ERROR] Read %zu facets, expected %u.\n", read_count, num_facets);
        free(mesh_out->facets);
        mesh_out->facets = NULL;
        mesh_out->facet_count = 0;
        fclose(file);
        return false;
    }

    fclose(file);
    printf("[STL LOAD SUCCESS] Loaded binary STL: %s (%u facets parsed)\n", filepath, num_facets);
    return true;
}

bool auncient_stl_load_ascii(const char *filepath, AuncientStlMesh *mesh_out) {
    FILE *file = fopen(filepath, "r");
    if (!file) return false;

    char line[256];
    uint32_t num_facets = 0;
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "facet normal") != NULL) {
            num_facets++;
        }
    }
    rewind(file);

    mesh_out->facet_count = num_facets;
    mesh_out->facets = (AuncientStlFacet *)malloc(sizeof(AuncientStlFacet) * num_facets);
    if (!mesh_out->facets) {
        fclose(file);
        return false;
    }

    uint32_t f_idx = 0;
    int v_idx = 0;
    while (fgets(line, sizeof(line), file) && f_idx < num_facets) {
        float nx, ny, nz;
        if (sscanf(line, " facet normal %f %f %f", &nx, &ny, &nz) == 3) {
            mesh_out->facets[f_idx].normal[0] = nx;
            mesh_out->facets[f_idx].normal[1] = ny;
            mesh_out->facets[f_idx].normal[2] = nz;
            v_idx = 0;
        } else {
            float vx, vy, vz;
            if (sscanf(line, " vertex %f %f %f", &vx, &vy, &vz) == 3 && v_idx < 3) {
                mesh_out->facets[f_idx].vertices[v_idx][0] = vx;
                mesh_out->facets[f_idx].vertices[v_idx][1] = vy;
                mesh_out->facets[f_idx].vertices[v_idx][2] = vz;
                v_idx++;
                if (v_idx == 3) {
                    mesh_out->facets[f_idx].attribute_byte_count = 0;
                    f_idx++;
                }
            }
        }
    }

    fclose(file);
    printf("[STL LOAD SUCCESS] Loaded ASCII STL: %s (%u facets parsed)\n", filepath, num_facets);
    return true;
}

bool auncient_stl_load(const char *filepath, AuncientStlMesh *mesh_out) {
    if (!filepath || !mesh_out) return false;

    FILE *file = fopen(filepath, "r");
    if (!file) return false;

    char check[8] = {0};
    size_t r = fread(check, 1, 5, file);
    fclose(file);

    if (r == 5 && strncmp(check, "solid", 5) == 0) {
        return auncient_stl_load_ascii(filepath, mesh_out);
    }
    return auncient_stl_load_binary(filepath, mesh_out);
}

void auncient_stl_free_mesh(AuncientStlMesh *mesh) {
    if (mesh && mesh->facets) {
        free(mesh->facets);
        mesh->facets = NULL;
        mesh->facet_count = 0;
    }
}
