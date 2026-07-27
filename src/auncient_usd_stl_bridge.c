#include "auncient_usd_stl_bridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

    // Compute bounding box for automatic centering and scaling
    float min_x = 99999.0f, max_x = -99999.0f;
    float min_y = 99999.0f, max_y = -99999.0f;
    float min_z = 99999.0f, max_z = -99999.0f;
    
    for (uint32_t i = 0; i < stl_mesh->facet_count; i++) {
        const AuncientStlFacet *f = &stl_mesh->facets[i];
        for (int v = 0; v < 3; v++) {
            float x = f->vertices[v][0];
            float y = f->vertices[v][1];
            float z = f->vertices[v][2];
            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
            if (y < min_y) min_y = y;
            if (y > max_y) max_y = y;
            if (z < min_z) min_z = z;
            if (z > max_z) max_z = z;
        }
    }
    
    float center_x = (min_x + max_x) * 0.5f;
    float center_y = (min_y + max_y) * 0.5f;
    float center_z = (min_z + max_z) * 0.5f;
    
    float ext_x = max_x - min_x;
    float ext_y = max_y - min_y;
    float ext_z = max_z - min_z;
    float max_extent = ext_x;
    if (ext_y > max_extent) max_extent = ext_y;
    if (ext_z > max_extent) max_extent = ext_z;
    if (max_extent == 0.0f) max_extent = 1.0f;
    
    float scale = 1.0f / max_extent;

    // Weld vertices to construct shared points topology
    uint32_t max_verts = stl_mesh->facet_count * 3;
    float (*unique_verts)[3] = (float (*)[3])malloc(sizeof(float) * 3 * max_verts);
    uint32_t *face_indices = (uint32_t *)malloc(sizeof(uint32_t) * max_verts);
    if (!unique_verts || !face_indices) {
        if (unique_verts) free(unique_verts);
        if (face_indices) free(face_indices);
        fclose(file);
        return false;
    }
    uint32_t unique_count = 0;

    for (uint32_t i = 0; i < stl_mesh->facet_count; i++) {
        const AuncientStlFacet *f = &stl_mesh->facets[i];
        for (int v = 0; v < 3; v++) {
            float vx = (f->vertices[v][0] - center_x) * scale;
            float vy = (f->vertices[v][1] - center_y) * scale;
            float vz = (f->vertices[v][2] - center_z) * scale;
            
            int found_idx = -1;
            for (uint32_t u = 0; u < unique_count; u++) {
                float dx = unique_verts[u][0] - vx;
                float dy = unique_verts[u][1] - vy;
                float dz = unique_verts[u][2] - vz;
                float dist_sq = dx*dx + dy*dy + dz*dz;
                if (dist_sq < 1e-10f) {
                    found_idx = (int)u;
                    break;
                }
            }
            
            if (found_idx == -1) {
                unique_verts[unique_count][0] = vx;
                unique_verts[unique_count][1] = vy;
                unique_verts[unique_count][2] = vz;
                face_indices[i * 3 + v] = unique_count;
                unique_count++;
            } else {
                face_indices[i * 3 + v] = (uint32_t)found_idx;
            }
        }
    }

    // 1. Write unique points list (centered and normalized)
    fprintf(file, "    point3f[] points = [\n");
    for (uint32_t i = 0; i < unique_count; i++) {
        fprintf(file, "        (%.6f, %.6f, %.6f),\n", 
                unique_verts[i][0], unique_verts[i][1], unique_verts[i][2]);
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
    for (uint32_t i = 0; i < stl_mesh->facet_count; i++) {
        fprintf(file, "        %u, %u, %u,\n", 
                face_indices[i * 3 + 0], face_indices[i * 3 + 1], face_indices[i * 3 + 2]);
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

    free(unique_verts);
    free(face_indices);

    printf("[USD_STL_BRIDGE SUCCESS] Successfully bridged %u STL facets to USDA file: %s (welded to %u unique vertices)\n", 
           stl_mesh->facet_count, usda_filepath, unique_count);
    return true;
}

bool auncient_bridge_usda_to_stl(const float *points, uint32_t point_count, const uint32_t *indices, uint32_t index_count, const char *stl_filepath) {
    if (!points || !indices || !stl_filepath || (index_count % 3) != 0) {
        return false;
    }

    FILE *file = fopen(stl_filepath, "w");
    if (!file) {
        printf("[USD_STL_BRIDGE ERROR] Failed to create output STL: %s\n", stl_filepath);
        return false;
    }

    fprintf(file, "solid GeneratedMesh\n");

    for (uint32_t i = 0; i < index_count; i += 3) {
        uint32_t idx0 = indices[i];
        uint32_t idx1 = indices[i + 1];
        uint32_t idx2 = indices[i + 2];

        if (idx0 >= point_count || idx1 >= point_count || idx2 >= point_count) {
            fclose(file);
            return false;
        }

        float v0[3] = { points[idx0*3], points[idx0*3+1], points[idx0*3+2] };
        float v1[3] = { points[idx1*3], points[idx1*3+1], points[idx1*3+2] };
        float v2[3] = { points[idx2*3], points[idx2*3+1], points[idx2*3+2] };

        float u[3] = { v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2] };
        float w[3] = { v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2] };

        float nx = u[1] * w[2] - u[2] * w[1];
        float ny = u[2] * w[0] - u[0] * w[2];
        float nz = u[0] * w[1] - u[1] * w[0];

        float len = sqrtf(nx*nx + ny*ny + nz*nz);
        if (len > 0.0f) {
            nx /= len;
            ny /= len;
            nz /= len;
        }

        fprintf(file, "  facet normal %.6f %.6f %.6f\n", nx, ny, nz);
        fprintf(file, "    outer loop\n");
        fprintf(file, "      vertex %.6f %.6f %.6f\n", v0[0], v0[1], v0[2]);
        fprintf(file, "      vertex %.6f %.6f %.6f\n", v1[0], v1[1], v1[2]);
        fprintf(file, "      vertex %.6f %.6f %.6f\n", v2[0], v2[1], v2[2]);
        fprintf(file, "    endloop\n");
        fprintf(file, "  endfacet\n");
    }

    fprintf(file, "endsolid GeneratedMesh\n");
    fclose(file);
    printf("[USD_STL_BRIDGE SUCCESS] Successfully converted USDA mesh to ASCII STL: %s\n", stl_filepath);
    return true;
}
