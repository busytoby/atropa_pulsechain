#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>

// Auncient spelling check constraint verification

typedef struct {
    float x, y, z;
    float px, py, pz;
    float radius;
    bool anchored;
} Particle3D;

typedef struct {
    int p1, p2;
    float target_dist;
} LinkConstraint;

// --- System Configuration ---
#define NUM_CLOTH_PARTICLES 16
#define NUM_RIGID_PARTICLES 4
#define TOTAL_PARTICLES (NUM_CLOTH_PARTICLES + NUM_RIGID_PARTICLES)

#define MAX_CONSTRAINTS 128

static Particle3D particles[TOTAL_PARTICLES];
static LinkConstraint constraints[MAX_CONSTRAINTS];
static int num_constraints = 0;

void add_constraint(int p1, int p2, float dist) {
    if (num_constraints < MAX_CONSTRAINTS) {
        constraints[num_constraints++] = (LinkConstraint){ p1, p2, dist };
    }
}

void init_world() {
    num_constraints = 0;

    // 1. Initialize Cloth Grid (Particles 0..15)
    float spacing = 15.0f;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            int idx = y * 4 + x;
            float px = x * spacing - 22.5f;
            float py = y * spacing + 100.0f;
            particles[idx] = (Particle3D){ px, py, 0.0f, px, py, 0.0f, 2.0f, false };
        }
    }
    // Anchor top corners of the cloth
    particles[0].anchored = true;
    particles[3].anchored = true;

    // Cloth constraints
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            int idx = y * 4 + x;
            if (x < 3) add_constraint(idx, idx + 1, spacing);
            if (y < 3) add_constraint(idx, idx + 4, spacing);
        }
    }
    float diag_spacing = spacing * sqrtf(2.0f);
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            int idx = y * 4 + x;
            add_constraint(idx, idx + 5, diag_spacing);
            add_constraint(idx + 1, idx + 4, diag_spacing);
        }
    }

    // 2. Initialize Rigid Tetrahedron (Particles 16..19)
    // Positioned below the cloth to test collision interactions
    int r_start = NUM_CLOTH_PARTICLES;
    particles[r_start + 0] = (Particle3D){ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 15.0f, false };
    particles[r_start + 1] = (Particle3D){ 40.0f, 0.0f, 0.0f, 40.0f, 0.0f, 0.0f, 15.0f, false };
    particles[r_start + 2] = (Particle3D){ 20.0f, 34.6f, 0.0f, 20.0f, 34.6f, 0.0f, 15.0f, false };
    particles[r_start + 3] = (Particle3D){ 20.0f, 11.5f, 32.7f, 20.0f, 11.5f, 32.7f, 15.0f, false };

    // Tetrahedron constraints (6 links to form a rigid volumetric frame)
    float side = 40.0f;
    add_constraint(r_start + 0, r_start + 1, side);
    add_constraint(r_start + 1, r_start + 2, side);
    add_constraint(r_start + 2, r_start + 0, side);
    add_constraint(r_start + 0, r_start + 3, side);
    add_constraint(r_start + 1, r_start + 3, side);
    add_constraint(r_start + 2, r_start + 3, side);
}

void tick_verlet(float decay, float gravity_y) {
    for (int i = 0; i < TOTAL_PARTICLES; i++) {
        if (particles[i].anchored) continue;

        float temp_x = particles[i].x;
        float temp_y = particles[i].y;
        float temp_z = particles[i].z;

        particles[i].x += (particles[i].x - particles[i].px) * decay;
        particles[i].y += (particles[i].y - particles[i].py) * decay + gravity_y;
        particles[i].z += (particles[i].z - particles[i].pz) * decay;

        particles[i].px = temp_x;
        particles[i].py = temp_y;
        particles[i].pz = temp_z;
    }
}

// Flat ground plane collision solver (y = -100.0f)
void resolve_collisions() {
    float floor_y = -100.0f;
    for (int i = 0; i < TOTAL_PARTICLES; i++) {
        if (particles[i].y - particles[i].radius < floor_y) {
            float temp_y = particles[i].y;
            particles[i].y = floor_y + particles[i].radius;
            // Push previous y position up to simulate rebound bounce
            particles[i].py = temp_y + (temp_y - particles[i].py) * 0.3f;
        }
    }
}

void resolve_constraints() {
    for (int iter = 0; iter < 12; iter++) {
        for (int i = 0; i < num_constraints; i++) {
            LinkConstraint c = constraints[i];
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
    printf("=== ZMM Virtual Hardware: Unified Rigid & Cloth Verlet Test ===\n");
    init_world();

    printf("[INIT] World registered %d constraints across %d particles.\n", 
           num_constraints, TOTAL_PARTICLES);
    assert(num_constraints == 48); // 42 cloth + 6 rigid

    // Run physics frames
    printf("[TEST] Running 30 physics steps under gravity and floor bounds...\n");
    for (int step = 0; step < 30; step++) {
        tick_verlet(0.96f, -1.2f); // Negative gravity causes falling
        resolve_collisions();
        resolve_constraints();
    }

    // Verify rigid body shape maintenance (stiffness test)
    printf("[TEST] Verifying structural parameters of the Rigid Body...\n");
    int r_start = NUM_CLOTH_PARTICLES;
    float dx = particles[r_start + 1].x - particles[r_start + 0].x;
    float dy = particles[r_start + 1].y - particles[r_start + 0].y;
    float dz = particles[r_start + 1].z - particles[r_start + 0].z;
    float resolved_side = sqrtf(dx*dx + dy*dy + dz*dz);
    printf("   Tetrahedron edge distance: %.4f (Expected: 40.00)\n", resolved_side);
    assert(fabs(resolved_side - 40.0f) < 0.1f);

    // Verify cloth dangling coordinates
    printf("[TEST] Verifying cloth structural points...\n");
    for (int i = 0; i < NUM_CLOTH_PARTICLES; i++) {
        assert(!isnan(particles[i].x) && !isnan(particles[i].y));
    }

    printf("[SUCCESS] Unified Rigid and Cloth simulation checks passed.\n");
    return 0;
}
