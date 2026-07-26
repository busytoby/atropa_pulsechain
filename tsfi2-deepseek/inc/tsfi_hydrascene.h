#ifndef TSFI_HYDRASCENE_H
#define TSFI_HYDRASCENE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float points[8][3]; // Vertex coordinates
    int face_vertex_counts[6];
    int face_vertex_indices[24];
} HydraMeshTopology;

// Hydra Scene Delegate context
typedef struct {
    char delegate_id[64];
    HydraMeshTopology active_mesh;
} TSFiHydraScene;

// Initialize the Hydra Scene Delegate context
void tsfi_hydrascene_init(TSFiHydraScene *scene);

// Updates scene delegate geometry parameters based on custom scheme input
bool tsfi_hydrascene_update_mesh(TSFiHydraScene *scene, const float (*vertices)[3], int vertex_count);

// Queries primitive vertex coordinates from the delegate
bool tsfi_hydrascene_get_points(const TSFiHydraScene *scene, float (*out_points)[3]);

#endif // TSFI_HYDRASCENE_H
