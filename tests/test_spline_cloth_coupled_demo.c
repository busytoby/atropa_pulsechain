#include "../src/auncient_timeline_autodin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define GRID_W 30
#define GRID_H 8

void render_ascii_grid(const ClothPoint *sq1, const ClothPoint *sq2, const SplinePhysNode *rope, int rope_count) {
    char grid[GRID_H][GRID_W];
    memset(grid, ' ', sizeof(grid));

    // Draw borders
    for (int x = 0; x < GRID_W; x++) {
        grid[0][x] = '-';
        grid[GRID_H - 1][x] = '-';
    }
    for (int y = 1; y < GRID_H - 1; y++) {
        grid[y][0] = '|';
        grid[y][GRID_W - 1] = '|';
    }

    // Rasterize Square 1 nodes into grid space (scale coordinates x: [0, 15] -> index)
    for (int i = 0; i < 4; i++) {
        int gx = (int)(sq1[i].x * 1.5f + 3.0f);
        int gy = (int)(sq1[i].y * 1.5f + 4.0f);
        if (gx >= 1 && gx < GRID_W - 1 && gy >= 1 && gy < GRID_H - 1) {
            grid[gy][gx] = '#';
        }
    }

    // Rasterize Square 2 nodes into grid space
    for (int i = 0; i < 4; i++) {
        int gx = (int)(sq2[i].x * 1.5f + 3.0f);
        int gy = (int)(sq2[i].y * 1.5f + 4.0f);
        if (gx >= 1 && gx < GRID_W - 1 && gy >= 1 && gy < GRID_H - 1) {
            grid[gy][gx] = '@';
        }
    }

    // Rasterize connecting Spline rope string nodes into grid space
    for (int i = 0; i < rope_count; i++) {
        int gx = (int)(rope[i].x * 1.5f + 3.0f);
        int gy = (int)(rope[i].y * 1.5f + 4.0f);
        if (gx >= 1 && gx < GRID_W - 1 && gy >= 1 && gy < GRID_H - 1) {
            // Do not overwrite squares
            if (grid[gy][gx] == ' ') {
                grid[gy][gx] = '~';
            }
        }
    }

    // Print grid out to console
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
    printf("AUNCIENT COUPLED SPLINE-VERLET STRINGS AND CLOTH SQUARES DEMO\n");
    printf("=============================================================\n");

    // 1. Initialize two small cloth squares (2x2 grid each)
    // Square 1: Centered around x = 0.0f
    ClothPoint square1[4] = {
        { .x = -1.0f, .y = 1.0f, .z = 0.0f, .px = -1.0f, .py = 1.0f, .pz = 0.0f, .is_anchored = 1 },
        { .x = 1.0f, .y = 1.0f, .z = 0.0f, .px = 1.0f, .py = 1.0f, .pz = 0.0f, .is_anchored = 1 },
        { .x = -1.0f, .y = -1.0f, .z = 0.0f, .px = -1.0f, .py = -1.0f, .pz = 0.0f, .is_anchored = 0 },
        { .x = 1.0f, .y = -1.0f, .z = 0.0f, .px = 1.0f, .py = -1.0f, .pz = 0.0f, .is_anchored = 0 }
    };

    // Square 2: Centered around x = 12.0f
    ClothPoint square2[4] = {
        { .x = 11.0f, .y = 1.0f, .z = 0.0f, .px = 11.0f, .py = 1.0f, .pz = 0.0f, .is_anchored = 0 },
        { .x = 13.0f, .y = 1.0f, .z = 0.0f, .px = 13.0f, .py = 1.0f, .pz = 0.0f, .is_anchored = 0 },
        { .x = 11.0f, .y = -1.0f, .z = 0.0f, .px = 11.0f, .py = -1.0f, .pz = 0.0f, .is_anchored = 0 },
        { .x = 13.0f, .y = -1.0f, .z = 0.0f, .px = 13.0f, .py = -1.0f, .pz = 0.0f, .is_anchored = 0 }
    };

    // 2. Initialize a spline string (5 nodes) connecting the squares
    SplinePhysNode string_rope[5];
    memset(string_rope, 0, sizeof(string_rope));
    for (int i = 0; i < 5; i++) {
        float t = (float)i / 4.0f;
        string_rope[i].x = 1.0f + t * 10.0f;
        string_rope[i].y = -1.0f + t * 2.0f;
        string_rope[i].z = 0.0f;
        string_rope[i].px = string_rope[i].x;
        string_rope[i].py = string_rope[i].y;
        string_rope[i].pz = string_rope[i].z;
    }

    printf("[INITIAL STATE]\n");
    render_ascii_grid(square1, square2, string_rope, 5);

    // 3. Run simulation loop for 5 frames under wind force
    float wind_x = 4.0f;
    float wind_y = -1.0f;
    float wind_z = 0.0f;

    printf("\n[RUNNING COUPLED PHYSICS SIMULATION]\n");
    for (int frame = 1; frame <= 5; frame++) {
        // Step square 1 unanchored nodes
        for (int i = 2; i < 4; i++) {
            float tx = square1[i].x;
            square1[i].x += (square1[i].x - square1[i].px) * 0.95f + wind_x * 0.01f;
            square1[i].px = tx;
        }

        // Couple spline string endpoints to the square corner vertices
        auncient_couple_spline_to_cloth(&string_rope[0], &square1[3]);
        auncient_couple_spline_to_cloth(&string_rope[4], &square2[0]);

        // Step spline Verlet physics
        auncient_spline_verlet_step(string_rope, 5, 0.1f, 0.95f, wind_x, wind_y, wind_z);

        printf("   Frame %d:\n", frame);
        render_ascii_grid(square1, square2, string_rope, 5);
    }

    printf("=============================================================\n");
    printf("AUNCIENT COUPLED PHYSICS DEMO COMPLETED\n");
    printf("=============================================================\n");
    return 0;
}
