#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "verlet_integrator.h"

int main() {
    printf("=== TESTING FLAT CACHE-ALIGNED VERLET INTEGRATOR ===\n");
    
    VerletSystem system;
    float dt = 0.016f; // ~60fps
    verlet_system_init(&system, dt);
    
    // Test Struct Alignment constraints
    printf("Size of Particle struct: %zu bytes (Expected: 64)\n", sizeof(Particle));
    assert(sizeof(Particle) == 64);
    
    // 1. Spawn a particle at (0, 100, 0) with a horizontal velocity of (10, 0, 0)
    assert(verlet_system_spawn(&system, 0.0f, 100.0f, 0.0f, 10.0f, 0.0f, 0.0f));
    
    Particle *p = &system.particles[0];
    assert(p->active == 1);
    assert(p->x == 0.0f);
    assert(p->y == 100.0f);
    assert(p->z == 0.0f);
    
    // 2. Perform 1 update frame
    verlet_system_update(&system);
    
    // Velocity update: x = x_prev_vel + horizontal_vel * dt = 0 + 10 * 0.016 = 0.16
    // Zener clamped gravity acceleration: y = 99.5421
    printf("After 1 update: x=%.4f, y=%.4f, z=%.4f\n", p->x, p->y, p->z);
    assert(fabs(p->x - 0.16f) < 0.001f);
    assert(fabs(p->y - 99.5421f) < 0.001f);
    
    // 3. Perform many updates to trigger a boundary bounce on floor (Y = -300)
    for (int frame = 0; frame < 300; frame++) {
        verlet_system_update(&system);
    }
    
    printf("After 301 updates: y=%.4f (Expected to remain inside boundaries: [-300, 300])\n", p->y);
    assert(p->y >= -300.0f && p->y <= 300.0f);
    
    printf("=== ALL VERLET INTEGRATOR TESTS PASSED ===\n");
    return 0;
}
