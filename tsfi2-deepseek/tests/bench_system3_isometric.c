#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

// Represents an entity in System 3's The Last Ninja 3D Isometric engine
typedef struct {
    int id;
    float x, y, z;      // 3D world coordinates
    float sx, sy;       // Projected 2D screen coordinates
    int z_depth;        // Depth value for rendering priority (Z-sorting)
    bool is_active;
} Entity;

// Standard Isometric projection formula used to map 3D coordinate space to 2D C64 display
static void project_isometric(Entity *e) {
    // Standard System 3 transformation matrices:
    // Screen X = (X - Y) * cos(30 degrees)
    // Screen Y = (X + Y) * sin(30 degrees) - Z
    // Approximating cos(30) = 0.866 and sin(30) = 0.5 for fast integer/fixed-point scaling
    e->sx = (e->x - e->y) * 0.866f;
    e->sy = (e->x + e->y) * 0.5f - e->z;
    e->z_depth = (int)(e->x + e->y + e->z);
}

// Compare function for Z-sorting rendering order
static int compare_depth(const void *a, const void *b) {
    const Entity *ea = (const Entity *)a;
    const Entity *eb = (const Entity *)b;
    return ea->z_depth - eb->z_depth;
}

int main(void) {
    printf("[BENCHMARK] Starting System 3 3D-Isometric Projection and Z-Sorting Benchmark...\n");

    const int NUM_ENTITIES = 1000;
    const int ITERATIONS = 20000;
    
    Entity *entities = malloc(sizeof(Entity) * NUM_ENTITIES);
    if (!entities) {
        perror("Failed to allocate entities memory");
        return 1;
    }

    // Initialize entities with random walk path values
    srand((unsigned int)time(NULL));
    for (int i = 0; i < NUM_ENTITIES; i++) {
        entities[i].id = i;
        entities[i].x = (float)(rand() % 320);
        entities[i].y = (float)(rand() % 200);
        entities[i].z = (float)(rand() % 16);
        entities[i].is_active = true;
    }

    clock_t start = clock();

    unsigned long long projection_count = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        // 1. Perform isometric projection mapping
        for (int i = 0; i < NUM_ENTITIES; i++) {
            project_isometric(&entities[i]);
            projection_count++;
        }
        
        // 2. Perform Z-sorting to simulate draw orders
        qsort(entities, NUM_ENTITIES, sizeof(Entity), compare_depth);
    }

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("[BENCHMARK] System 3 simulation statistics:\n");
    printf("  - Total iterations: %d\n", ITERATIONS);
    printf("  - Entities per frame: %d\n", NUM_ENTITIES);
    printf("  - Total projections calculated: %llu\n", projection_count);
    printf("  - Execution time: %.4f seconds\n", time_spent);
    if (time_spent > 0) {
        double thru = (double)projection_count / time_spent;
        printf("  - Throughput: %.2f projections/sec\n", thru);
    }

    free(entities);
    printf("[BENCHMARK] System 3 benchmark completed successfully!\n");
    return 0;
}
