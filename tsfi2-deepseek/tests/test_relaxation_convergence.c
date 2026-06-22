#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>

// Auncient spelling check constraint verification

typedef struct {
    float x, y;
} Particle2D;

typedef struct {
    int p1, p2;
    float target_dist;
} SimpleConstraint;

#define NUM_PARTICLES 4
#define NUM_CONSTRAINTS 3

// Test a triangular structure with a central hub node (high-conflict configuration)
static Particle2D particles[NUM_PARTICLES] = {
    { 0.0f, 0.0f },     // 0: Central Hub Node
    { 100.0f, 0.0f },   // 1: Right Node
    { -50.0f, 86.6f },  // 2: Top-Left Node
    { -50.0f, -86.6f }  // 3: Bottom-Left Node
};

static SimpleConstraint constraints[NUM_CONSTRAINTS] = {
    { 0, 1, 100.0f },
    { 0, 2, 100.0f },
    { 0, 3, 100.0f }
};

// Calculate total constraint distance error in the system
float calculate_total_error() {
    float total_error = 0.0f;
    for (int i = 0; i < NUM_CONSTRAINTS; i++) {
        SimpleConstraint c = constraints[i];
        float dx = particles[c.p2].x - particles[c.p1].x;
        float dy = particles[c.p2].y - particles[c.p1].y;
        float dist = sqrtf(dx*dx + dy*dy);
        total_error += fabs(dist - c.target_dist);
    }
    return total_error;
}

// Perform a single pass of relaxation projection
void perform_relaxation_pass() {
    for (int i = 0; i < NUM_CONSTRAINTS; i++) {
        SimpleConstraint c = constraints[i];
        float dx = particles[c.p2].x - particles[c.p1].x;
        float dy = particles[c.p2].y - particles[c.p1].y;
        float dist = sqrtf(dx*dx + dy*dy);
        if (dist == 0.0f) continue;

        float diff = c.target_dist - dist;
        float percent = (diff / dist) * 0.5f;

        float ox = dx * percent;
        float oy = dy * percent;

        // Pull particles together or push apart proportionally
        particles[c.p1].x -= ox;
        particles[c.p1].y -= oy;
        particles[c.p2].x += ox;
        particles[c.p2].y += oy;
    }
}

int main() {
    printf("=== ZMM Virtual Hardware: Relaxation Solver Convergence Test ===\n");

    // Verify initial error is zero (system starts in equilibrium)
    float initial_error = calculate_total_error();
    printf("[INIT] Total initial system error: %.4f (Expected: 0.000)\n", initial_error);
    assert(initial_error < 0.1f);

    // Apply severe displacement to the hub node to create conflict
    printf("[TEST] Displacing central hub node to create constraint conflict...\n");
    particles[0].x = 30.0f;
    particles[0].y = -20.0f;

    float displaced_error = calculate_total_error();
    printf("   -> Error after displacement: %.4f\n", displaced_error);
    assert(displaced_error > 10.0f);

    // Track error decay over successive relaxation iterations
    float previous_error = displaced_error;
    for (int iter = 1; iter <= 4; iter++) {
        perform_relaxation_pass();
        float current_error = calculate_total_error();
        printf("   Iteration %d -> Current error: %.4f (Decay factor: %.2f)\n", 
               iter, current_error, current_error / previous_error);
        
        // Assert error strictly decreases during each iteration step
        assert(current_error < previous_error);
        previous_error = current_error;
    }

    printf("   Final system error after 4 iterations: %.4f\n", previous_error);
    assert(previous_error < 5.0f);

    printf("[SUCCESS] Relaxation solver convergence test suite passed.\n");
    return 0;
}
