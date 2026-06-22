#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>

// Auncient spelling check constraint verification

typedef struct {
    float x, y, z;
    float px, py, pz;
    bool anchored;
} ClothParticle;

typedef struct {
    int p1, p2;
    float target_dist;
} GridConstraint;

#define GRID_WIDTH 4
#define GRID_HEIGHT 4
#define NUM_PARTICLES (GRID_WIDTH * GRID_HEIGHT)
#define MAX_CONSTRAINTS 64

static ClothParticle particles[NUM_PARTICLES];
static GridConstraint constraints[MAX_CONSTRAINTS];
static int num_constraints = 0;

void add_constraint(int p1, int p2, float dist) {
    if (num_constraints < MAX_CONSTRAINTS) {
        constraints[num_constraints++] = (GridConstraint){ p1, p2, dist };
    }
}

void init_cloth_grid() {
    float spacing = 20.0f;
    num_constraints = 0;

    // Initialize particles in a flat 2D grid
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            int idx = y * GRID_WIDTH + x;
            float px = x * spacing;
            float py = y * spacing;
            particles[idx] = (ClothParticle){ px, py, 0.0f, px, py, 0.0f, false };
        }
    }

    // Anchor the top-left and top-right corner particles
    particles[0].anchored = true;
    particles[GRID_WIDTH - 1].anchored = true;

    // Create horizontal and vertical structural constraints
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            int idx = y * GRID_WIDTH + x;
            // Horizontal link
            if (x < GRID_WIDTH - 1) {
                add_constraint(idx, idx + 1, spacing);
            }
            // Vertical link
            if (y < GRID_HEIGHT - 1) {
                add_constraint(idx, idx + GRID_WIDTH, spacing);
            }
        }
    }

    // Create diagonal shear constraints to prevent shear distortion
    float diag_spacing = spacing * sqrtf(2.0f);
    for (int y = 0; y < GRID_HEIGHT - 1; y++) {
        for (int x = 0; x < GRID_WIDTH - 1; x++) {
            int idx = y * GRID_WIDTH + x;
            add_constraint(idx, idx + GRID_WIDTH + 1, diag_spacing);
            add_constraint(idx + 1, idx + GRID_WIDTH, diag_spacing);
        }
    }
}

void tick_physics(float decay, float gravity_y) {
    for (int i = 0; i < NUM_PARTICLES; i++) {
        if (particles[i].anchored) continue;

        float temp_x = particles[i].x;
        float temp_y = particles[i].y;
        float temp_z = particles[i].z;

        // Verlet integration step with gravity
        particles[i].x += (particles[i].x - particles[i].px) * decay;
        particles[i].y += (particles[i].y - particles[i].py) * decay + gravity_y;
        particles[i].z += (particles[i].z - particles[i].pz) * decay;

        particles[i].px = temp_x;
        particles[i].py = temp_y;
        particles[i].pz = temp_z;
    }
}

void resolve_grid_constraints() {
    for (int iter = 0; iter < 8; iter++) {
        for (int i = 0; i < num_constraints; i++) {
            GridConstraint c = constraints[i];
            float dx = particles[c.p2].x - particles[c.p1].x;
            float dy = particles[c.p2].y - particles[c.p1].y;
            float dz = particles[c.p2].z - particles[c.p1].z;

            float dist = sqrtf(dx*dx + dy*dy + dz*dz);
            if (dist == 0.0f) continue;

            float diff = c.target_dist - dist;
            float percent = (diff / dist) * 0.5f;

            float ox = dx * percent;
            float oy = dy * percent;
            float oz = dz * percent;

            // Distribute forces based on anchoring state
            if (!particles[c.p1].anchored && !particles[c.p2].anchored) {
                particles[c.p1].x -= ox;
                particles[c.p1].y -= oy;
                particles[c.p1].z -= oz;
                particles[c.p2].x += ox;
                particles[c.p2].y += oy;
                particles[c.p2].z += oz;
            } else if (particles[c.p1].anchored) {
                particles[c.p2].x += ox * 2.0f;
                particles[c.p2].y += oy * 2.0f;
                particles[c.p2].z += oz * 2.0f;
            } else if (particles[c.p2].anchored) {
                particles[c.p1].x -= ox * 2.0f;
                particles[c.p1].y -= oy * 2.0f;
                particles[c.p1].z -= oz * 2.0f;
            }
        }
    }
}

int main() {
    printf("=== ZMM Virtual Hardware: Verlet Cloth Simulation ===\n");
    init_cloth_grid();

    printf("[INIT] Grid created with %d particles and %d constraints.\n", 
           NUM_PARTICLES, num_constraints);
    assert(num_constraints > 0);

    // Apply gravity and run simulation ticks
    printf("[TEST] Running 20 physics steps under gravity...\n");
    for (int step = 0; step < 20; step++) {
        tick_physics(0.95f, 0.8f); // Damped gravity
        resolve_grid_constraints();
    }

    // Verify hanging grid coordinates
    printf("[TEST] Verifying final grid positions...\n");
    for (int i = 0; i < NUM_PARTICLES; i++) {
        printf("   Particle %d -> Position: (x: %.2f, y: %.2f, z: %.2f)\n", 
               i, particles[i].x, particles[i].y, particles[i].z);
        // Ensure none of the values are NaN
        assert(!isnan(particles[i].x) && !isnan(particles[i].y));
    }

    // Assert anchored points remained completely static
    assert(particles[0].x == 0.0f && particles[0].y == 0.0f);
    assert(particles[GRID_WIDTH - 1].x == 60.0f && particles[GRID_WIDTH - 1].y == 0.0f);

    printf("[SUCCESS] Verlet cloth grid simulation verification passed.\n");
    return 0;
}
