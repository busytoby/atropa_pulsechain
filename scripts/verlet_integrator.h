#ifndef VERLET_INTEGRATOR_H
#define VERLET_INTEGRATOR_H

#include <stdint.h>
#include <stdbool.h>
#include "../tsfi2-deepseek/inc/tsfi_zener.h"

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
    uint32_t p1;          // Index of first particle
    uint32_t p2;          // Index of second particle
    float rest_length;    // Rest length of spring link
    float stiffness;      // Stiffness parameter (0.0 to 1.0)
    uint32_t active;      // Active constraint flag
} Constraint;

#define MAX_CONSTRAINTS 2048

// Spatial Partitioning Grid for O(N) neighbor lookup
#define GRID_CELL_SIZE 50.0f
#define GRID_DIM_X 20
#define GRID_DIM_Y 12
#define GRID_DIM_Z 20

typedef struct {
    int count;
    uint32_t particle_indices[32]; // Max particles per grid cell
} GridCell;

#include "libantigravity_interop.h"

typedef struct {
    Particle particles[MAX_PARTICLES];
    Constraint constraints[MAX_CONSTRAINTS];
    float dt;
    float boundary_min_x, boundary_max_x;
    float boundary_min_y, boundary_max_y;
    float boundary_min_z, boundary_max_z;
    TsfiZener zener;       // Electro-mechanical Zener clamp feedback
    InteropRegistry *registry; // Direct pointer to feed back Diyat tax values
    
    // Grid spatial partition
    GridCell grid[GRID_DIM_X][GRID_DIM_Y][GRID_DIM_Z];
    
    // Boid behavioral parameters for charge-carrier collective dynamics
    float max_accel;       // Acceleration allocation budget limit
    float max_speed;       // Speed limit for boid behavior
    float separation_dist; // Distance to maintain separation
    float neighbor_dist;   // Radius for alignment and cohesion
    float separation_w;    // Weight for separation behavior
    float alignment_w;     // Weight for alignment behavior
    float cohesion_w;      // Weight for cohesion behavior
} VerletSystem;

// Initialize the particle system
void verlet_system_init(VerletSystem *system, float dt);

// Establish spring constraint between two particles
bool verlet_system_link(VerletSystem *system, uint32_t p1, uint32_t p2, float stiffness);

// Spawn a particle at a specific location
bool verlet_system_spawn(VerletSystem *system, float x, float y, float z, float vx, float vy, float vz);

// Update all active particles using Verlet integration, spatial partitioning, prioritized budgeting, and steering behaviors
void verlet_system_update(VerletSystem *system);

#endif // VERLET_INTEGRATOR_H
