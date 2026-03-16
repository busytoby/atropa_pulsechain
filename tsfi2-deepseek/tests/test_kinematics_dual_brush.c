#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// A conceptual test demonstrating the Dualistic Brush architecture for kinematics.
// This proves we can move objects in a persistent manifold using purely differential
// Matter/Void operations without executing a full screen clear.

#define W 64
#define H 64

typedef struct {
    float x0, y0;
    float x1, y1;
} BundleState;

typedef struct {
    uint8_t pixels[H][W];
} PersistentManifold;

// The basic drawing primitive (Bresenham's Line Algorithm)
void plot_line(PersistentManifold *m, int x0, int y0, int x1, int y1, uint8_t value) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        if (x0 >= 0 && x0 < W && y0 >= 0 && y0 < H) {
            m->pixels[y0][x0] = value;
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

// 1. The Origin Brush (The Void / Yin)
void brush_void(PersistentManifold *m, BundleState *state) {
    plot_line(m, (int)state->x0, (int)state->y0, (int)state->x1, (int)state->y1, 0); // Paint Black
}

// 2. The Destination Brush (The Matter / Yang)
void brush_matter(PersistentManifold *m, BundleState *state) {
    plot_line(m, (int)state->x0, (int)state->y0, (int)state->x1, (int)state->y1, 255); // Paint White
}

int main() {
    printf("[KINEMATICS] Initializing Persistent Manifold (Dualistic Brush Test)...\n");
    PersistentManifold *manifold = calloc(1, sizeof(PersistentManifold));
    
    // Initial State: A limb at rest
    BundleState old_limb = {32, 10, 32, 30};
    brush_matter(manifold, &old_limb);
    
    // Frame Loop
    for (int frame = 1; frame <= 5; frame++) {
        // Calculate new kinematics (swinging right)
        BundleState new_limb;
        new_limb.x0 = 32;          // Shoulder stays anchored
        new_limb.y0 = 10;
        new_limb.x1 = 32 + (frame * 5); // Wrist swings right
        new_limb.y1 = 30 - (frame * 2); // Wrist raises slightly
        
        printf("\n--- FRAME %d ---\n", frame);
        
        // EXECUTE DUALISTIC ANIMATION
        brush_void(manifold, &old_limb);   // Erase the past
        brush_matter(manifold, &new_limb); // Assert the present
        
        // Calculate the Kinematic Velocity Vector dynamically from the delta
        float vx = new_limb.x1 - old_limb.x1;
        float vy = new_limb.y1 - old_limb.y1;
        printf("Wrist Velocity Vector: Vx=%.1f, Vy=%.1f\n", vx, vy);
        
        // State progression
        old_limb = new_limb;
    }
    
    free(manifold);
    printf("\n[SUCCESS] Differential Kinematics executed without full-matrix wipes.\n");
    return 0;
}
