#include "../src/auncient_timeline_autodin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

    // Square 2: Centered around x = 10.0f
    ClothPoint square2[4] = {
        { .x = 9.0f, .y = 1.0f, .z = 0.0f, .px = 9.0f, .py = 1.0f, .pz = 0.0f, .is_anchored = 0 },
        { .x = 11.0f, .y = 1.0f, .z = 0.0f, .px = 11.0f, .py = 1.0f, .pz = 0.0f, .is_anchored = 0 },
        { .x = 9.0f, .y = -1.0f, .z = 0.0f, .px = 9.0f, .py = -1.0f, .pz = 0.0f, .is_anchored = 0 },
        { .x = 11.0f, .y = -1.0f, .z = 0.0f, .px = 11.0f, .py = -1.0f, .pz = 0.0f, .is_anchored = 0 }
    };

    // 2. Initialize a spline string (5 nodes) connecting the squares
    SplinePhysNode string_rope[5];
    memset(string_rope, 0, sizeof(string_rope));
    for (int i = 0; i < 5; i++) {
        // Interpolated positions between square 1 (right corner) and square 2 (left corner)
        float t = (float)i / 4.0f;
        string_rope[i].x = 1.0f + t * 8.0f;
        string_rope[i].y = -1.0f + t * 2.0f;
        string_rope[i].z = 0.0f;
        string_rope[i].px = string_rope[i].x;
        string_rope[i].py = string_rope[i].y;
        string_rope[i].pz = string_rope[i].z;
    }

    printf("[INITIAL STATE]\n");
    printf("   Square 1 Right-Bottom Anchor: (%f, %f, %f)\n", square1[3].x, square1[3].y, square1[3].z);
    printf("   Square 2 Left-Top Anchor:     (%f, %f, %f)\n", square2[0].x, square2[0].y, square2[0].z);
    printf("   Spline String Middle Node:    (%f, %f, %f)\n", string_rope[2].x, string_rope[2].y, string_rope[2].z);
    fflush(stdout);

    // 3. Run simulation loop for 5 frames under wind force
    float wind_x = 5.0f;
    float wind_y = -2.0f;
    float wind_z = 0.5f;

    printf("\n[RUNNING COUPLED PHYSICS SIMULATION]\n");
    for (int frame = 1; frame <= 5; frame++) {
        // Step square 1 (dummy Verlet motion for unanchored nodes)
        for (int i = 2; i < 4; i++) {
            float tx = square1[i].x;
            square1[i].x += (square1[i].x - square1[i].px) * 0.98f + wind_x * 0.0001f;
            square1[i].px = tx;
        }

        // Couple spline string endpoints to the square corner vertices
        auncient_couple_spline_to_cloth(&string_rope[0], &square1[3]);
        auncient_couple_spline_to_cloth(&string_rope[4], &square2[0]);

        // Step spline Verlet physics
        auncient_spline_verlet_step(string_rope, 5, 0.01f, 0.98f, wind_x, wind_y, wind_z);

        printf("   Frame %d:\n", frame);
        printf("      Rope Start Node:  (%f, %f, %f)\n", string_rope[0].x, string_rope[0].y, string_rope[0].z);
        printf("      Rope Middle Node: (%f, %f, %f)\n", string_rope[2].x, string_rope[2].y, string_rope[2].z);
        printf("      Rope End Node:    (%f, %f, %f)\n", string_rope[4].x, string_rope[4].y, string_rope[4].z);
        fflush(stdout);
    }

    printf("=============================================================\n");
    printf("AUNCIENT COUPLED PHYSICS DEMO COMPLETED\n");
    printf("=============================================================\n");
    return 0;
}
