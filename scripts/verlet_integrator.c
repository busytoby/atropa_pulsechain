#include "verlet_integrator.h"
#include <string.h>
#include <math.h>

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

    // Initialize Auncient Zener diode: Vz = 5.6V, temp_coeff = +0.003
    tsfi_zener_init(&system->zener, 5.6, 0.003, 1.0f / dt);
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
        
        // Zener physical clamping feedback on Y displacement
        double noise = 0.0;
        double vs = 0.08 * p->y; // Map position to virtual voltage
        double vd = tsfi_zener_tick(&system->zener, vs, 1000.0, &noise);
        double id = (vs - vd) / 1000.0;
        float restoring_ay = (float)(-id * 250000.0); // Saturated clamp acceleration
        
        // Calculate Diyat printing / breakdown tax and feed directly back to InteropRegistry
        if (system->registry && id > 0.0) {
            uint64_t diyat_fee = (uint64_t)(id * 50000.0); // Calibrate breakdown scale
            if (diyat_fee > 0) {
                // Accumulate accrued diyat fees into the active state monopole register
                system->registry->active_state.monopole += diyat_fee;
                
                // Modulate Barn heat parameter based on the physical Zener temperature junction drift
                system->registry->active_state.barn = (uint64_t)(system->zener.temp * 100.0);
            }
        }

        // 1. Integrate position (including Zener restoring force on Y axis)
        p->x = 2.0f * p->x - p->px + p->ax * dt_sq;
        p->y = 2.0f * p->y - p->py + (p->ay + restoring_ay) * dt_sq;
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
            if (system->registry) system->registry->boundary_collision_flags |= 1;
        } else if (p->x > system->boundary_max_x) {
            p->x = system->boundary_max_x;
            p->px = p->x + (temp_x - p->px) * cor;
            if (system->registry) system->registry->boundary_collision_flags |= 1;
        }
        
        // Y Boundaries
        if (p->y < system->boundary_min_y) {
            p->y = system->boundary_min_y;
            p->py = p->y + (temp_y - p->py) * cor;
            if (system->registry) system->registry->boundary_collision_flags |= 2;
        } else if (p->y > system->boundary_max_y) {
            p->y = system->boundary_max_y;
            p->py = p->y + (temp_y - p->py) * cor;
            if (system->registry) system->registry->boundary_collision_flags |= 2;
        }
        
        // Z Boundaries
        if (p->z < system->boundary_min_z) {
            p->z = system->boundary_min_z;
            p->pz = p->z + (temp_z - p->pz) * cor;
            if (system->registry) system->registry->boundary_collision_flags |= 4;
        } else if (p->z > system->boundary_max_z) {
            p->z = system->boundary_max_z;
            p->pz = p->z + (temp_z - p->pz) * cor;
            if (system->registry) system->registry->boundary_collision_flags |= 4;
        }
    }

    // 3. Resolve Spring Constraints (Hookean Relaxation Solver)
    // Runs multiple iterations to converge constraints cleanly
    for (int iter = 0; iter < 4; iter++) {
        for (int i = 0; i < MAX_CONSTRAINTS; i++) {
            Constraint *c = &system->constraints[i];
            if (!c->active) continue;

            Particle *p1 = &system->particles[c->p1];
            Particle *p2 = &system->particles[c->p2];
            if (!p1->active || !p2->active) continue;

            float dx = p2->x - p1->x;
            float dy = p2->y - p1->y;
            float dz = p2->z - p1->z;
            float dist_sq = dx*dx + dy*dy + dz*dz;
            if (dist_sq < 0.0001f) continue;

            // Fast inverse square root (Quake III algorithm) to optimize vector scaling
            float x2 = dist_sq * 0.5f;
            float y = dist_sq;
            uint32_t i_bits;
            memcpy(&i_bits, &y, sizeof(float));
            i_bits = 0x5f3759df - (i_bits >> 1);
            memcpy(&y, &i_bits, sizeof(float));
            y = y * (1.5f - (x2 * y * y)); // 1st iteration
            
            float inv_dist = y;
            float dist = dist_sq * inv_dist;

            float diff = c->rest_length - dist;
            float percent = (diff * inv_dist) * 0.5f * c->stiffness;
            float offset_x = dx * percent;
            float offset_y = dy * percent;
            float offset_z = dz * percent;

            p1->x -= offset_x;
            p1->y -= offset_y;
            p1->z -= offset_z;

            p2->x += offset_x;
            p2->y += offset_y;
            p2->z += offset_z;
        }
    }
}

#include <math.h>

bool verlet_system_link(VerletSystem *system, uint32_t p1, uint32_t p2, float stiffness) {
    if (!system || p1 >= MAX_PARTICLES || p2 >= MAX_PARTICLES) return false;
    
    // Find free constraint slot
    for (int i = 0; i < MAX_CONSTRAINTS; i++) {
        Constraint *c = &system->constraints[i];
        if (!c->active) {
            c->active = 1;
            c->p1 = p1;
            c->p2 = p2;
            c->stiffness = stiffness;
            
            // Calculate rest length dynamically based on current particle positions
            float dx = system->particles[p2].x - system->particles[p1].x;
            float dy = system->particles[p2].y - system->particles[p1].y;
            float dz = system->particles[p2].z - system->particles[p1].z;
            c->rest_length = (float)sqrt(dx*dx + dy*dy + dz*dz);
            return true;
        }
    }
    return false; // Constraints pool exhausted
}
