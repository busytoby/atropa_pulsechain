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
    float epibar;  // Tension threshold (stretching)
    float hypobar; // Compression threshold (squeezing)
} AsymmetricConstraint;

// Function to resolve constraint asymmetrically using Epibar and Hypobar
void resolve_asymmetric_constraint(Particle *particles, AsymmetricConstraint *c) {
    float dx = particles[c->p2].x - particles[c->p1].x;
    float dy = particles[c->p2].y - particles[c->p1].y;
    float dz = particles[c->p2].z - particles[c->p1].z;
    float dw = particles[c->p2].w - particles[c->p1].w;

    float dist = sqrtf(dx*dx + dy*dy + dz*dz + dw*dw);
    if (dist == 0.0f) return;

    float diff = c->target_dist - dist;
    float correction_force = 0.0f;

    // Epibar vs. Hypobar asymmetric correction force calculations
    if (diff < 0.0f) {
        // Stretched beyond target_dist -> Apply Epibar severity
        correction_force = diff * (0.5f * c->epibar);
    } else {
        // Squeezed under target_dist -> Apply Hypobar severity
        correction_force = diff * (0.5f * c->hypobar);
    }

    float ox = (dx / dist) * correction_force;
    float oy = (dy / dist) * correction_force;
    float oz = (dz / dist) * correction_force;
    float ow = (dw / dist) * correction_force;

    particles[c->p1].x -= ox;
    particles[c->p1].y -= oy;
    particles[c->p1].z -= oz;
    particles[c->p1].w -= ow;

    particles[c->p2].x += ox;
    particles[c->p2].y += oy;
    particles[c->p2].z += oz;
    particles[c->p2].w += ow;
}

int main() {
    printf("=== ZMM Virtual Hardware: Epibar & Hypobar Asymmetric Verlet Constraints Test ===\n");

    // Initialize two test particles linked by a 100.0 unit constraint
    Particle particles[2] = {
        { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 10.0f },
        { 100.0f, 0.0f, 0.0f, 0.0f, 100.0f, 0.0f, 0.0f, 10.0f }
    };

    AsymmetricConstraint constraint = {
        .p1 = 0,
        .p2 = 1,
        .target_dist = 100.0f,
        .epibar = 1.0f,   // High tension severity
        .hypobar = 0.2f   // Low compression severity (floppy compression)
    };

    // Test Scenario 1: Tension / Stretching (Distance set to 120.0f)
    printf("[TEST] Testing stretching correction (Epibar = 1.0)...\n");
    particles[0].x = 0.0f;
    particles[1].x = 120.0f;
    resolve_asymmetric_constraint(particles, &constraint);
    
    float stretched_dist = fabs(particles[1].x - particles[0].x);
    printf("   -> Corrected distance under tension: %.4f (Expected: 100.00)\n", stretched_dist);
    assert(fabs(stretched_dist - 100.0f) < 0.001f);

    // Reset particles
    particles[0].x = 0.0f;
    particles[1].x = 100.0f;

    // Test Scenario 2: Compression / Squeezing (Distance set to 80.0f)
    printf("[TEST] Testing compression correction (Hypobar = 0.2)...\n");
    particles[0].x = 0.0f;
    particles[1].x = 80.0f;
    resolve_asymmetric_constraint(particles, &constraint);

    float compressed_dist = fabs(particles[1].x - particles[0].x);
    printf("   -> Corrected distance under compression: %.4f (Expected: 84.00)\n", compressed_dist);
    assert(fabs(compressed_dist - 84.0f) < 0.001f);

    printf("[SUCCESS] Epibar and Hypobar asymmetric constraint checks passed.\n");
    return 0;
}
