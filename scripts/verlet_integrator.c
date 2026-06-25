#include "verlet_integrator.h"
#include <string.h>

void verlet_system_init(VerletSystem *system, float dt) {
    if (!system) return;
    memset(system, 0, sizeof(VerletSystem));
    system->dt = dt;
    
    // Default boundary bounds
    system->boundary_min_x = -500.0f;
    system->boundary_max_x =  500.0f;
    system->boundary_min_y = -300.0f;
    system->boundary_max_y =  300.0f;
    system->boundary_min_z = -500.0f;
    system->boundary_max_z =  500.0f;
}

bool verlet_system_spawn(VerletSystem *system, float x, float y, float z, float vx, float vy, float vz) {
    if (!system) return false;
    
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &system->particles[i];
        if (!p->active) {
            p->active = 1;
            p->x = x;
            p->y = y;
            p->z = z;
            
            // Previous position derived from starting velocity
            p->px = x - vx * system->dt;
            p->py = y - vy * system->dt;
            p->pz = z - vz * system->dt;
            
            p->ax = 0.0f;
            p->ay = -9.81f; // Gravity force vector
            p->az = 0.0f;
            return true;
        }
    }
    return false; // Pool exhausted
}

void verlet_system_update(VerletSystem *system) {
    if (!system) return;
    
    float dt_sq = system->dt * system->dt;
    
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &system->particles[i];
        if (!p->active) continue;
        
        float temp_x = p->x;
        float temp_y = p->y;
        float temp_z = p->z;
        
        // 1. Integrate position
        p->x = 2.0f * p->x - p->px + p->ax * dt_sq;
        p->y = 2.0f * p->y - p->py + p->ay * dt_sq;
        p->z = 2.0f * p->z - p->pz + p->az * dt_sq;
        
        p->px = temp_x;
        p->py = temp_y;
        p->pz = temp_z;
        
        // 2. Handle Boundary Collisions (Coefficient of Restitution: 0.85)
        float cor = 0.85f;
        
        // X Boundaries
        if (p->x < system->boundary_min_x) {
            p->x = system->boundary_min_x;
            p->px = p->x + (temp_x - p->px) * cor;
        } else if (p->x > system->boundary_max_x) {
            p->x = system->boundary_max_x;
            p->px = p->x + (temp_x - p->px) * cor;
        }
        
        // Y Boundaries
        if (p->y < system->boundary_min_y) {
            p->y = system->boundary_min_y;
            p->py = p->y + (temp_y - p->py) * cor;
        } else if (p->y > system->boundary_max_y) {
            p->y = system->boundary_max_y;
            p->py = p->y + (temp_y - p->py) * cor;
        }
        
        // Z Boundaries
        if (p->z < system->boundary_min_z) {
            p->z = system->boundary_min_z;
            p->pz = p->z + (temp_z - p->pz) * cor;
        } else if (p->z > system->boundary_max_z) {
            p->z = system->boundary_max_z;
            p->pz = p->z + (temp_z - p->pz) * cor;
        }
    }
}
