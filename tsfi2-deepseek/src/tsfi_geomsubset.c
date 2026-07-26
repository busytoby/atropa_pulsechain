#include "tsfi_geomsubset.h"
#include <string.h>

void tsfi_geomsubset_init(TSFiGeomSubset *subset, const char *path) {
    if (!subset) return;
    
    if (path) {
        strncpy(subset->subset_path, path, sizeof(subset->subset_path) - 1);
    } else {
        strcpy(subset->subset_path, "/auncient/mesh/subset");
    }
    
    subset->face_count = 0;
    memset(subset->face_indices, 0, sizeof(subset->face_indices));
    memset(subset->material_binding_path, 0, sizeof(subset->material_binding_path));
}

bool tsfi_geomsubset_set_faces(TSFiGeomSubset *subset, const int indices[], int count) {
    if (!subset || !indices || count <= 0 || count > TSFI_MAX_SUBSET_FACES) return false;
    
    for (int i = 0; i < count; i++) {
        subset->face_indices[i] = indices[i];
    }
    subset->face_count = count;
    return true;
}

void tsfi_geomsubset_bind_material(TSFiGeomSubset *subset, const char *material_path) {
    if (!subset) return;
    
    if (material_path) {
        strncpy(subset->material_binding_path, material_path, sizeof(subset->material_binding_path) - 1);
    } else {
        memset(subset->material_binding_path, 0, sizeof(subset->material_binding_path));
    }
}

bool tsfi_geomsubset_has_face(const TSFiGeomSubset *subset, int face_idx) {
    if (!subset) return false;
    
    for (int i = 0; i < subset->face_count; i++) {
        if (subset->face_indices[i] == face_idx) {
            return true;
        }
    }
    return false;
}
