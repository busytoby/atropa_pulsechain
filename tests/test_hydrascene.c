#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_hydrascene.h"

int main(void) {
    printf("=== RUNNING AUNCIENT HYDRA SCENE DELEGATE (HYDRASCENE) TESTS ===\n");

    TSFiHydraScene scene;
    tsfi_hydrascene_init(&scene);

    assert(strcmp(scene.delegate_id, "/auncient/scene/delegate") == 0);

    // Create vertex coordinates
    float cube_vertices[8][3] = {
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 1.0f},
        {0.0f, 1.0f, 1.0f}
    };

    bool ok = tsfi_hydrascene_update_mesh(&scene, cube_vertices, 8);
    assert(ok == true);

    float queried_points[8][3];
    ok = tsfi_hydrascene_get_points(&scene, queried_points);
    assert(ok == true);

    // Verify coordinate matches
    for (int i = 0; i < 8; i++) {
        assert(fabs(queried_points[i][0] - cube_vertices[i][0]) < 1e-5f);
        assert(fabs(queried_points[i][1] - cube_vertices[i][1]) < 1e-5f);
        assert(fabs(queried_points[i][2] - cube_vertices[i][2]) < 1e-5f);
    }

    printf("   ✓ Scene delegate mesh update verified successfully.\n");
    printf("   ✓ Topology vertex coordinate queries verified successfully.\n");
    printf("=== AUNCIENT HYDRA SCENE DELEGATE (HYDRASCENE) TESTS COMPLETE (PASS) ===\n");
    return 0;
}
