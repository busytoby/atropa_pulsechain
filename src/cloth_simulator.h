#ifndef CLOTH_SIMULATOR_H
#define CLOTH_SIMULATOR_H

#include <stdint.h>

// Auncient Cloth Physics Mass-Spring Solver for TSFI2

typedef struct {
    float x, y, z;
    float px, py, pz; // Previous positions for Verlet integration
    float fx, fy, fz;
    float nx, ny, nz;
    int is_anchored;
    float anchor_x, anchor_y, anchor_z;
} ClothPoint;

typedef struct {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
    uint32_t color;
} ClothVertex;

#define CLOTH_WIDTH 15
#define CLOTH_HEIGHT 10

#ifdef __cplusplus
extern "C" {
#endif

void cloth_init(void);
void cloth_update(float wind_x, float wind_y, float wind_z);
void cloth_set_anchor(int x, int y, float tx, float ty, float tz);
void cloth_apply_sphere_collision(float cx, float cy, float cz, float radius);
void cloth_generate_mesh(ClothVertex *vertices, int *indices, int *vertex_count, int *index_count);

#ifdef __cplusplus
}
#endif

#endif // CLOTH_SIMULATOR_H
