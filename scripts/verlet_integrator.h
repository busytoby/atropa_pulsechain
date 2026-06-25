#ifndef VERLET_INTEGRATOR_H
#define VERLET_INTEGRATOR_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_PARTICLES 1024

// Cache-line aligned (64 bytes) to prevent false sharing during multi-threaded updates
typedef struct __attribute__((aligned(64))) {
    float x, y, z;         // Current position coordinates
    float px, py, pz;      // Previous position coordinates
    float ax, ay, az;      // Acceleration vector values
    uint32_t active;       // Active state flag
    uint32_t padding[6];   // Padding to exactly fill 64 bytes
} Particle;

typedef struct {
    Particle particles[MAX_PARTICLES];
    float dt;
    float boundary_min_x, boundary_max_x;
    float boundary_min_y, boundary_max_y;
    float boundary_min_z, boundary_max_z;
} VerletSystem;

// Initialize the particle system
void verlet_system_init(VerletSystem *system, float dt);

// Spawn a particle at a specific location
bool verlet_system_spawn(VerletSystem *system, float x, float y, float z, float vx, float vy, float vz);

// Update all active particles using Verlet integration and boundary constraints
void verlet_system_update(VerletSystem *system);

#endif // VERLET_INTEGRATOR_H
