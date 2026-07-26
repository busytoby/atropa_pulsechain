#include "tsfi_hydrascene.h"
#include <string.h>

void tsfi_hydrascene_init(TSFiHydraScene *scene) {
    if (!scene) return;
    strcpy(scene->delegate_id, "/auncient/scene/delegate");
    memset(&scene->active_mesh, 0, sizeof(scene->active_mesh));
}

bool tsfi_hydrascene_update_mesh(TSFiHydraScene *scene, const float (*vertices)[3], int vertex_count) {
    if (!scene || !vertices || vertex_count > 8) return false;
    for (int i = 0; i < vertex_count; i++) {
        scene->active_mesh.points[i][0] = vertices[i][0];
        scene->active_mesh.points[i][1] = vertices[i][1];
        scene->active_mesh.points[i][2] = vertices[i][2];
    }
    return true;
}

bool tsfi_hydrascene_get_points(const TSFiHydraScene *scene, float (*out_points)[3]) {
    if (!scene || !out_points) return false;
    for (int i = 0; i < 8; i++) {
        out_points[i][0] = scene->active_mesh.points[i][0];
        out_points[i][1] = scene->active_mesh.points[i][1];
        out_points[i][2] = scene->active_mesh.points[i][2];
    }
    return true;
}
