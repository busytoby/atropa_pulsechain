#ifndef TSFI_GEOMSUBSET_H
#define TSFI_GEOMSUBSET_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_MAX_SUBSET_FACES 32

typedef struct {
    char subset_path[64];
    int face_indices[TSFI_MAX_SUBSET_FACES];
    int face_count;
    char material_binding_path[64];
} TSFiGeomSubset;

// Initialize the GeomSubset context
void tsfi_geomsubset_init(TSFiGeomSubset *subset, const char *path);

// Assigns a list of face indices to the subset
bool tsfi_geomsubset_set_faces(TSFiGeomSubset *subset, const int indices[], int count);

// Binds a material path to the subset
void tsfi_geomsubset_bind_material(TSFiGeomSubset *subset, const char *material_path);

// Checks if a specific face index is included in the subset
bool tsfi_geomsubset_has_face(const TSFiGeomSubset *subset, int face_idx);

#endif // TSFI_GEOMSUBSET_H
