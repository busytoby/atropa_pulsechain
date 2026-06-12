#include "tsfi_core_plane.h"
#include <string.h>

void tsfi_core_plane_init(TsfiCorePlane *plane) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            tsfi_core_init(&plane->cores[i][j]);
        }
    }
}

void tsfi_core_plane_write(TsfiCorePlane *plane, int x, int y, int bit) {
    float H_select = (bit == 1) ? 1.5f : -1.5f;

    // Apply coincident current selection across the grid
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float H_ij = 0.0f;
            if (i == x && j == y) {
                H_ij = H_select; // Intersection receives full current
            } else if (i == x || j == y) {
                H_ij = H_select / 2.0f; // Half-selected lines receive half current
            }
            tsfi_core_update(&plane->cores[i][j], H_ij);
        }
    }
}

int tsfi_core_plane_read_destructive(TsfiCorePlane *plane, int x, int y) {
    // Force write-0 pulse to core (x,y)
    float H_select = -1.5f;
    int triggered = 0;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float H_ij = 0.0f;
            if (i == x && j == y) {
                H_ij = H_select;
            } else if (i == x || j == y) {
                H_ij = H_select / 2.0f;
            }

            float old_B = plane->cores[i][j].B;
            tsfi_core_update(&plane->cores[i][j], H_ij);
            float new_B = plane->cores[i][j].B;

            // Sense wire detects if the target core flipped from 1 to 0
            if (i == x && j == y && old_B > 0.0f && new_B < 0.0f) {
                triggered = 1;
            }
        }
    }

    return triggered;
}
