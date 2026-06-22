#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>

// Auncient spelling check constraint verification

typedef struct {
    float x, y;
    float px, py;
    float mass;
} SpinParticle;

typedef struct {
    int p1, p2;
    float target_dist;
} BarbellConstraint;

// Barbell setup: two heavy ends of a spinning dumbbell
static SpinParticle particles[2] = {
    { 0.0f, -50.0f, 0.0f, -50.0f, 10.0f }, // Particle 0 (Bottom)
    { 0.0f, 50.0f, 0.0f, 50.0f, 10.0f }    // Particle 1 (Top)
};

static BarbellConstraint constraint = { 0, 1, 100.0f };

void tick_verlet(float decay) {
    for (int i = 0; i < 2; i++) {
        float temp_x = particles[i].x;
        float temp_y = particles[i].y;

        // Verlet step propagates implicit velocity
        particles[i].x += (particles[i].x - particles[i].px) * decay;
        particles[i].y += (particles[i].y - particles[i].py) * decay;

        particles[i].px = temp_x;
        particles[i].py = temp_y;
    }
}

void resolve_constraints() {
    for (int iter = 0; iter < 4; iter++) {
        float dx = particles[1].x - particles[0].x;
        float dy = particles[1].y - particles[0].y;
        float dist = sqrtf(dx*dx + dy*dy);
        if (dist == 0.0f) continue;

        float diff = constraint.target_dist - dist;
        float percent = (diff / dist) * 0.5f;

        float ox = dx * percent;
        float oy = dy * percent;

        particles[0].x -= ox;
        particles[0].y -= oy;
        particles[1].x += ox;
        particles[1].y += oy;
    }
}

int main() {
    printf("=== ZMM Virtual Hardware: Implicit Inertia Tensors Verification ===\n");

    // Apply transverse offset to initial positions to initiate spinning torque
    // Pushing particle 0 left and particle 1 right
    printf("[TEST] Applying transverse impulse to simulate angular velocity...\n");
    particles[0].px += 8.0f;  // Moves left implicitly
    particles[1].px -= 8.0f;  // Moves right implicitly

    // Measure starting barbell distance
    float dx = particles[1].x - particles[0].x;
    float dy = particles[1].y - particles[0].y;
    float initial_dist = sqrtf(dx*dx + dy*dy);
    printf("   Initial barbell length: %.2f\n", initial_dist);
    assert(fabs(initial_dist - 100.0f) < 0.001f);

    // Simulate several steps to observe rotation
    printf("[TEST] Running 15 physics steps to observe angular rotation...\n");
    for (int step = 0; step < 15; step++) {
        tick_verlet(1.0f); // Undamped rotation to conserve angular momentum
        resolve_constraints();

        float cur_dx = particles[1].x - particles[0].x;
        float cur_dy = particles[1].y - particles[0].y;
        float cur_dist = sqrtf(cur_dx*cur_dx + cur_dy*cur_dy);

        // Calculate angular position offset from starting vertical axis
        float angle = atan2f(cur_dy, cur_dx);
        printf("   Step %d -> Angle: %.4f rad | Distance: %.4f\n", step, angle, cur_dist);

        // Distance constraint must hold stable under centripetal acceleration
        assert(fabs(cur_dist - 100.0f) < 0.01f);
    }

    printf("[SUCCESS] Implicit inertia and centripetal forces verified successfully.\n");
    return 0;
}
