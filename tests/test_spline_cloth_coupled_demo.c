#include "../src/auncient_timeline_autodin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define GRID_W 40
#define GRID_H 12

// Signed Distance Function (SDF) of the physical spline string (sphere-chain representation)
float spline_sdf(float x, float y, float z, const SplinePhysNode *nodes, int count) {
    float min_dist = 1e9f;
    float radius = 1.0f; // Radius of the spline tube

    for (int i = 0; i < count; i++) {
        float dx = x - nodes[i].x;
        float dy = y - nodes[i].y;
        float dz = z - nodes[i].z;
        float dist = sqrtf(dx * dx + dy * dy + dz * dz) - radius;
        if (dist < min_dist) {
            min_dist = dist;
        }
    }
    return min_dist;
}

// Ray marches the spline SDF and returns a character glyph based on hit depth
char ray_march_pixel(float rx, float ry, const SplinePhysNode *nodes, int count) {
    float ro_x = rx;
    float ro_y = ry;
    float ro_z = -10.0f; // Ray origin

    float rd_x = 0.0f;
    float rd_y = 0.0f;
    float rd_z = 1.0f;  // Ray direction (orthographic projection)

    float t = 0.0f;
    for (int step = 0; step < 15; step++) {
        float px = ro_x + rd_x * t;
        float py = ro_y + rd_y * t;
        float pz = ro_z + rd_z * t;

        float d = spline_sdf(px, py, pz, nodes, count);
        if (d < 0.1f) {
            // Hit! Shading based on depth t
            if (t < 8.0f) return '@';
            if (t < 10.0f) return 'o';
            if (t < 12.0f) return '*';
            return '.';
        }
        t += d;
        if (t > 20.0f) break;
    }
    return ' ';
}

void render_raymarched_scene(const ClothPoint *sq1, const ClothPoint *sq2, const SplinePhysNode *rope, int rope_count) {
    char grid[GRID_H][GRID_W];

    // Evaluate each pixel using the ray marcher
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            // Map character coordinates to 3D world space
            float rx = ((float)x / (GRID_W - 1)) * 18.0f - 3.0f;
            float ry = ((float)y / (GRID_H - 1)) * 8.0f - 4.0f;

            grid[y][x] = ray_march_pixel(rx, ry, rope, rope_count);
        }
    }

    // Rasterize Square 1 nodes into grid space (overlay)
    for (int i = 0; i < 4; i++) {
        int gx = (int)(sq1[i].x * 2.0f + 6.0f);
        int gy = (int)(sq1[i].y * 1.2f + 5.0f);
        if (gx >= 0 && gx < GRID_W && gy >= 0 && gy < GRID_H) {
            grid[gy][gx] = '#';
        }
    }

    // Rasterize Square 2 nodes into grid space (overlay)
    for (int i = 0; i < 4; i++) {
        int gx = (int)(sq2[i].x * 2.0f + 6.0f);
        int gy = (int)(sq2[i].y * 1.2f + 5.0f);
        if (gx >= 0 && gx < GRID_W && gy >= 0 && gy < GRID_H) {
            grid[gy][gx] = 'X';
        }
    }

    // Print frame out to console
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            putchar(grid[y][x]);
        }
        putchar('\n');
    }
    fflush(stdout);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT RAY-MARCHED SPLINE VERLET COAXIAL PHYSICS SCENE\n");
    printf("=============================================================\n");

    // 1. Initialize two small cloth squares (2x2 grid each)
    ClothPoint square1[4] = {
        { .x = -2.0f, .y = 2.0f, .z = 0.0f, .px = -2.0f, .py = 2.0f, .pz = 0.0f, .is_anchored = 1 },
        { .x = 0.0f, .y = 2.0f, .z = 0.0f, .px = 0.0f, .py = 2.0f, .pz = 0.0f, .is_anchored = 1 },
        { .x = -2.0f, .y = -2.0f, .z = 0.0f, .px = -2.0f, .py = -2.0f, .pz = 0.0f, .is_anchored = 0 },
        { .x = 0.0f, .y = -2.0f, .z = 0.0f, .px = 0.0f, .py = -2.0f, .pz = 0.0f, .is_anchored = 0 }
    };

    ClothPoint square2[4] = {
        { .x = 12.0f, .y = 2.0f, .z = 0.0f, .px = 12.0f, .py = 2.0f, .pz = 0.0f, .is_anchored = 0 },
        { .x = 14.0f, .y = 2.0f, .z = 0.0f, .px = 14.0f, .py = 2.0f, .pz = 0.0f, .is_anchored = 0 },
        { .x = 12.0f, .y = -2.0f, .z = 0.0f, .px = 12.0f, .py = -2.0f, .pz = 0.0f, .is_anchored = 0 },
        { .x = 14.0f, .y = -2.0f, .z = 0.0f, .px = 14.0f, .py = -2.0f, .pz = 0.0f, .is_anchored = 0 }
    };

    // 2. Initialize spline string connecting the squares
    SplinePhysNode string_rope[5];
    memset(string_rope, 0, sizeof(string_rope));
    for (int i = 0; i < 5; i++) {
        float t = (float)i / 4.0f;
        string_rope[i].x = 0.0f + t * 12.0f;
        string_rope[i].y = -2.0f + t * 4.0f;
        string_rope[i].z = 0.0f;
        string_rope[i].px = string_rope[i].x;
        string_rope[i].py = string_rope[i].y;
        string_rope[i].pz = string_rope[i].z;
    }

    printf("[INITIAL STATE]\n");
    render_raymarched_scene(square1, square2, string_rope, 5);

    // 3. Run simulation loop for 5 frames under wind force
    float wind_x = 6.0f;
    float wind_y = -2.0f;
    float wind_z = 1.0f;

    printf("\n[RUNNING RAY-MARCHED PHYSICS SIMULATION]\n");
    for (int frame = 1; frame <= 5; frame++) {
        // Step square 1 unanchored nodes
        for (int i = 2; i < 4; i++) {
            float tx = square1[i].x;
            square1[i].x += (square1[i].x - square1[i].px) * 0.95f + wind_x * 0.01f;
            square1[i].px = tx;
        }

        // Couple spline string endpoints
        auncient_couple_spline_to_cloth(&string_rope[0], &square1[3]);
        auncient_couple_spline_to_cloth(&string_rope[4], &square2[0]);

        // Step spline Verlet physics
        auncient_spline_verlet_step(string_rope, 5, 0.1f, 0.95f, wind_x, wind_y, wind_z);

        printf("   Frame %d:\n", frame);
        render_raymarched_scene(square1, square2, string_rope, 5);
    }

    printf("=============================================================\n");
    printf("AUNCIENT RAY-MARCHED PHYSICS DEMO COMPLETED\n");
    printf("=============================================================\n");
    return 0;
}
