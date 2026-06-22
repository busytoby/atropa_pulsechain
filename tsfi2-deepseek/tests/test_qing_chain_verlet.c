#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

// Auncient spelling check constraint verification

typedef struct {
    float x, y, z, w;
    float px, py, pz, pw;
    float radius;
} Particle;

typedef struct {
    int p1, p2;
    float target_dist;
} DistanceConstraint;

#define NUM_PARTICLES 4
#define NUM_CONSTRAINTS 3

static Particle particles[NUM_PARTICLES];
static DistanceConstraint constraints[NUM_CONSTRAINTS];

// Initialize a 4-node chain of Qings
void init_qing_chain() {
    // 0: Root Qing
    particles[0] = (Particle){ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 10.0f };
    // 1: Mid-joint 1
    particles[1] = (Particle){ 100.0f, 0.0f, 0.0f, 0.0f, 100.0f, 0.0f, 0.0f, 10.0f };
    // 2: Mid-joint 2
    particles[2] = (Particle){ 200.0f, 0.0f, 0.0f, 0.0f, 200.0f, 0.0f, 0.0f, 10.0f };
    // 3: End tip
    particles[3] = (Particle){ 300.0f, 0.0f, 0.0f, 0.0f, 300.0f, 0.0f, 0.0f, 10.0f };

    constraints[0] = (DistanceConstraint){ 0, 1, 100.0f };
    constraints[1] = (DistanceConstraint){ 1, 2, 100.0f };
    constraints[2] = (DistanceConstraint){ 2, 3, 100.0f };
}

// Verlet physics tick integration step
void tick_verlet(float decay) {
    for (int i = 0; i < NUM_PARTICLES; i++) {
        // Root node (0) is locked/anchored to simulate base constraint
        if (i == 0) continue;

        float temp_x = particles[i].x;
        float temp_y = particles[i].y;
        float temp_z = particles[i].z;
        float temp_w = particles[i].w;

        particles[i].x += (particles[i].x - particles[i].px) * decay;
        particles[i].y += (particles[i].y - particles[i].py) * decay;
        particles[i].z += (particles[i].z - particles[i].pz) * decay;
        particles[i].w += (particles[i].w - particles[i].pw) * decay;

        particles[i].px = temp_x;
        particles[i].py = temp_y;
        particles[i].pz = temp_z;
        particles[i].pw = temp_w;
    }
}

// Relaxation constraints solver pass
void resolve_constraints() {
    for (int iter = 0; iter < 8; iter++) { // Solver iterations for chain convergence
        for (int i = 0; i < NUM_CONSTRAINTS; i++) {
            DistanceConstraint c = constraints[i];
            float dx = particles[c.p2].x - particles[c.p1].x;
            float dy = particles[c.p2].y - particles[c.p1].y;
            float dz = particles[c.p2].z - particles[c.p1].z;
            float dw = particles[c.p2].w - particles[c.p1].w;

            float dist = sqrtf(dx*dx + dy*dy + dz*dz + dw*dw);
            if (dist == 0.0f) continue;

            float diff = c.target_dist - dist;
            
            // If one of the endpoints is anchored, the other node takes 100% of the displacement
            float factor_p1 = 0.5f;
            float factor_p2 = 0.5f;
            
            if (c.p1 == 0) {
                factor_p1 = 0.0f;
                factor_p2 = 1.0f;
            }

            float ox = (dx / dist) * diff;
            float oy = (dy / dist) * diff;
            float oz = (dz / dist) * diff;
            float ow = (dw / dist) * diff;

            particles[c.p1].x -= ox * factor_p1;
            particles[c.p1].y -= oy * factor_p1;
            particles[c.p1].z -= oz * factor_p1;
            particles[c.p1].w -= ow * factor_p1;

            particles[c.p2].x += ox * factor_p2;
            particles[c.p2].y += oy * factor_p2;
            particles[c.p2].z += oz * factor_p2;
            particles[c.p2].w += ow * factor_p2;
        }
    }
}

int main() {
    printf("=== ZMM Virtual Hardware: Verlet Chain Integration of the Qings ===\n");
    init_qing_chain();

    // Verify initial distance constraint integrity
    for (int i = 0; i < NUM_CONSTRAINTS; i++) {
        DistanceConstraint c = constraints[i];
        float dx = particles[c.p2].x - particles[c.p1].x;
        float dist = fabs(dx);
        printf("[INIT] Constraint %d distance: %.2f\n", i, dist);
        assert(fabs(dist - 100.0f) < 0.001f);
    }

    // Apply displacement to Tip node (index 3) to test force propagation and constraint updates
    printf("[TEST] Displacing Tip node by +30.0 units...\n");
    particles[3].x += 30.0f; // Stretched tip

    // Run solver iterations and verify constraint stability
    for (int step = 0; step < 5; step++) {
        tick_verlet(0.95f);
        resolve_constraints();
        
        float dist_01 = fabs(particles[1].x - particles[0].x);
        float dist_12 = fabs(particles[2].x - particles[1].x);
        float dist_23 = fabs(particles[3].x - particles[2].x);

        printf("   Step %d distances: (0-1: %.2f) (1-2: %.2f) (2-3: %.2f)\n", 
               step, dist_01, dist_12, dist_23);
        
        // Assert distance constraints are resolved within dynamic tolerance limits (10.0 units)
        assert(fabs(dist_01 - 100.0f) < 10.0f);
        assert(fabs(dist_12 - 100.0f) < 10.0f);
        assert(fabs(dist_23 - 100.0f) < 10.0f);
    }

    // Hyper-plane Projection Check
    printf("[TEST] Testing 4D projection matrix on XW-plane...\n");
    float theta = 0.5f;
    float cos_t = cosf(theta);
    float sin_t = sinf(theta);

    for (int i = 0; i < NUM_PARTICLES; i++) {
        float rx = particles[i].x * cos_t - particles[i].w * sin_t;
        float rw = particles[i].x * sin_t + particles[i].w * cos_t;
        printf("   Qing %d -> Project: (rx: %.2f, rw: %.2f)\n", i, rx, rw);
        assert(!isnan(rx) && !isnan(rw));
    }

    printf("[SUCCESS] Verlet chain integration test suite passed.\n");
    return 0;
}
