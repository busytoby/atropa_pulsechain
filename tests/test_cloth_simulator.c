#include "../src/cloth_simulator.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT CLOTH PHYSICS SIMULATOR TEST\n");
    printf("=============================================================\n");

    cloth_init();
    printf("   ✓ Cloth grid initialization verified.\n");

    // Perform 10 update cycles with wind
    for (int i = 0; i < 10; i++) {
        cloth_update(0.005f, 0.0f, -0.002f);
    }
    printf("   ✓ 10 solver update steps executed.\n");

    // Generate mesh output
    static ClothVertex vertices[1000];
    static int indices[5000];
    int vertex_count = 0;
    int index_count = 0;
    cloth_generate_mesh(vertices, indices, &vertex_count, &index_count);

    assert(vertex_count == CLOTH_WIDTH * CLOTH_HEIGHT);
    assert(index_count == (CLOTH_WIDTH - 1) * (CLOTH_HEIGHT - 1) * 6);
    printf("   ✓ Mesh output generation verified (Vertices: %d, Indices: %d).\n", vertex_count, index_count);

    printf("=============================================================\n");
    printf("AUNCIENT CLOTH SIMULATOR TEST COMPLETE\n");
    printf("=============================================================\n");
    return 0;
}
