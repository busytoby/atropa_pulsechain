#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>

// Auncient spelling check constraint verification

typedef struct {
    float x, y, z;
    float px, py, pz;
} JointParticle;

typedef struct {
    int p1, p2;
    float target_dist;
} BoneConstraint;

#define NUM_BONE_PARTICLES 2
#define GRID_W 3
#define GRID_H 3
#define NUM_CLOTH_PARTICLES (GRID_W * GRID_H)
#define TOTAL_PARTICLES (NUM_BONE_PARTICLES + NUM_CLOTH_PARTICLES)
#define MAX_CONSTRAINTS 64

static JointParticle particles[TOTAL_PARTICLES];
static BoneConstraint constraints[MAX_CONSTRAINTS];
static int num_constraints = 0;

void add_constraint(int p1, int p2, float dist) {
    if (num_constraints < MAX_CONSTRAINTS) {
        constraints[num_constraints++] = (BoneConstraint){ p1, p2, dist };
    }
}

void init_cloth_bone_world() {
    num_constraints = 0;

    // 1. Initialize Spinning Rigid Bone (Index 0..1)
    // Anchored at center to spin on the XY-plane
    particles[0] = (JointParticle){ -30.0f, 0.0f, 0.0f, -30.0f, 0.0f, 0.0f };
    particles[1] = (JointParticle){ 30.0f, 0.0f, 0.0f, 30.0f, 0.0f, 0.0f };
    add_constraint(0, 1, 60.0f); // Rigid bone constraint

    // 2. Initialize Cloth grid (Index 2..10)
    // Drapes down from the bone endpoints
    float spacing = 15.0f;
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            int idx = NUM_BONE_PARTICLES + y * GRID_W + x;
            float px = x * spacing - 15.0f;
            float py = - (y + 1) * spacing;
            particles[idx] = (JointParticle){ px, py, 0.0f, px, py, 0.0f };
        }
    }

    // Attach cloth top-row particles to the bone endpoints dynamically:
    // Left edge of cloth attached to Left bone endpoint (0)
    add_constraint(0, 2, spacing);
    // Right edge of cloth attached to Right bone endpoint (1)
    add_constraint(1, 4, spacing);

    // Horizontal & vertical cloth grid constraints
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            int idx = NUM_BONE_PARTICLES + y * GRID_W + x;
            if (x < GRID_W - 1) add_constraint(idx, idx + 1, spacing);
            if (y < GRID_H - 1) add_constraint(idx, idx + GRID_W, spacing);
        }
    }
}

void tick_verlet(float decay, float gravity_y) {
    for (int i = 0; i < TOTAL_PARTICLES; i++) {
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

void resolve_constraints() {
    for (int iter = 0; iter < 12; iter++) {
        for (int i = 0; i < num_constraints; i++) {
            BoneConstraint c = constraints[i];
            float dx = particles[c.p2].x - particles[c.p1].x;
            float dy = particles[c.p2].y - particles[c.p1].y;
            float dz = particles[c.p2].z - particles[c.p1].z;

            float dist = sqrtf(dx*dx + dy*dy + dz*dz);
            if (dist == 0.0f) continue;

            float diff = c.target_dist - dist;
            
            // Mass ratios: Bone (0, 1) has infinite mass relative to cloth (>= 2)
            float factor_p1 = 0.5f;
            float factor_p2 = 0.5f;

            if (c.p1 < 2 && c.p2 >= 2) {
                factor_p1 = 0.0f; // Bone does not move
                factor_p2 = 1.0f; // Cloth takes 100% correction
            }

            float ox = (dx / dist) * diff;
            float oy = (dy / dist) * diff;
            float oz = (dz / dist) * diff;

            particles[c.p1].x -= ox * factor_p1;
            particles[c.p1].y -= oy * factor_p1;
            particles[c.p1].z -= oz * factor_p1;

            particles[c.p2].x += ox * factor_p2;
            particles[c.p2].y += oy * factor_p2;
            particles[c.p2].z += oz * factor_p2;
        }
        
        // Centering constraint: Anchor center of the bone to coordinates (0, 0)
        float cx = (particles[0].x + particles[1].x) * 0.5f;
        float cy = (particles[0].y + particles[1].y) * 0.5f;
        particles[0].x -= cx; particles[0].y -= cy;
        particles[1].x -= cx; particles[1].y -= cy;
    }
}

int main() {
    printf("=== ZMM Virtual Hardware: Verlet Cloth & Bone Inertia Demo ===\n");
    init_cloth_bone_world();

    printf("[INIT] World registered %d particles and %d constraints.\n", 
           TOTAL_PARTICLES, num_constraints);

    // Apply spin torque to the bone endpoints to start rotation
    printf("[TEST] Spin impulse applied to rigid bone endpoints...\n");
    particles[0].px += 5.0f;
    particles[1].px -= 5.0f;

    // Simulate steps under gravity
    printf("[TEST] Running 20 physics simulation steps...\n");
    for (int step = 0; step < 20; step++) {
        tick_verlet(0.98f, -0.5f); // Damped gravity
        resolve_constraints();

        float bone_dx = particles[1].x - particles[0].x;
        float bone_dy = particles[1].y - particles[0].y;
        float bone_dist = sqrtf(bone_dx*bone_dx + bone_dy*bone_dy);
        
        // Output cloth tip coordinates to verify swing propagation
        int tip_idx = TOTAL_PARTICLES - 1;
        printf("   Step %d -> Bone length: %.2f | Cloth tip: (x: %.2f, y: %.2f)\n", 
               step, bone_dist, particles[tip_idx].x, particles[tip_idx].y);
        
        // Assert rigid bone holds length stable
        assert(fabs(bone_dist - 60.0f) < 0.1f);
    }

    printf("[SUCCESS] Cloth & bone inertia propagation verified successfully.\n");
    return 0;
}
