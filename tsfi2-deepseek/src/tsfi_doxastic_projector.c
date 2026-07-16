#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_doxastic_projector.h"

int tsfi_doxastic_project_region(tsfi_dat *dat, int x_start, int x_end, int y, int z, 
                                 TSFiDoxasticVertex *vertices_out, int max_vertices) {
    if (!dat || !vertices_out || max_vertices <= 0 || x_start > x_end) return 0;

    int written = 0;
    // Calculate global strain profile to map boundary gradients
    TSFiStrainClassification global_strain = tsfi_doxastic_classify_strain(dat, x_start, x_end, y, z);

    for (int x = x_start; x <= x_end; x++) {
        if (written >= max_vertices) break;

        char query[128];
        snprintf(query, sizeof(query), "svdag/%d/%d/%d", x, y, z);
        const char *val = tsfi_dat_search(dat, query);

        TSFiDoxasticVertex vertex = {
            .x = (float)x,
            .y = (float)y,
            .z = (float)z,
            .r = 0.5f, .g = 0.5f, .b = 0.5f, .a = 1.0f, // Default CLASS_UNKNOWN (Gray)
            .strain = 0.0f
        };

        if (val) {
            if (strcmp(val, "RELATION_TRUE") == 0) {
                // Crystallized fact: CLASS_BOND -> Green representation
                vertex.r = 0.1f; vertex.g = 0.9f; vertex.b = 0.1f;
            } else if (strcmp(val, "QUESTION_PENDING") == 0) {
                // Pending question: CLASS_SEED -> Blue/Yellow frontier representation
                vertex.r = 0.1f; vertex.g = 0.5f; vertex.b = 0.9f;
                vertex.strain = global_strain.boundary_gradient;
            } else if (strcmp(val, "") == 0 || strcmp(val, "UNBOUND") == 0) {
                // Retracted/Void: CLASS_VOID -> Red fracture representation
                vertex.r = 0.9f; vertex.g = 0.1f; vertex.b = 0.1f;
                vertex.strain = global_strain.boundary_gradient;
            }
        }

        vertices_out[written++] = vertex;
    }

    return written;
}
