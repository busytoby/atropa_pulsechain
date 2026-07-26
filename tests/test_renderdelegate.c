#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../tsfi2-deepseek/inc/tsfi_renderdelegate.h"

int main(void) {
    printf("=== RUNNING AUNCIENT HYDRA RENDER DELEGATE (RENDERDELEGATE) TESTS ===\n");

    TSFiRenderDelegate render;
    tsfi_renderdelegate_init(&render);

    assert(strcmp(render.renderer_name, "AuncientVulkanRenderer") == 0);
    assert(render.drawn_primitive_count == 0);

    // Vertex points to draw
    float points[3][3] = {
        {0.0f, 0.0f, 0.0f},
        {1.0f, 2.0f, 3.0f},
        {-1.0f, 0.0f, 5.5f}
    };

    bool ok = tsfi_renderdelegate_draw(&render, points, 3);
    assert(ok == true);
    assert(render.drawn_primitive_count == 3);

    printf("   ✓ Render delegate drawing and primitive counting verified successfully.\n");
    printf("=== AUNCIENT HYDRA RENDER DELEGATE (RENDERDELEGATE) TESTS COMPLETE (PASS) ===\n");
    return 0;
}
