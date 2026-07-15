#include "verlet_integrator.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

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
    
    // Initialize default charge carrier collective dynamics parameters
    system->max_accel = 300000.0f;
    system->max_speed = 5000.0f;
    system->separation_dist = 20.0f;
    system->neighbor_dist = 60.0f;
    system->separation_w = 1.8f;
    system->alignment_w = 1.0f;
    system->cohesion_w = 1.0f;
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

// Inline helper to accumulate acceleration under max budget
static inline void allocate_acceleration(float *acc_x, float *acc_y, float *acc_z, float *allocated_mag, float target_mag, float fx, float fy, float fz, float max_budget) {
    float mag = (float)sqrt(fx*fx + fy*fy + fz*fz);
    if (mag < 0.0001f) return;
    
    // Normalize force vector and scale to target magnitude
    float scale = target_mag / mag;
    float add_x = fx * scale;
    float add_y = fy * scale;
    float add_z = fz * scale;
    
    if (*allocated_mag + target_mag <= max_budget) {
        *acc_x += add_x;
        *acc_y += add_y;
        *acc_z += add_z;
        *allocated_mag += target_mag;
    } else {
        // Budget partially filled, trim excess
        float remaining = max_budget - *allocated_mag;
        if (remaining > 0.0f) {
            float trim = remaining / target_mag;
            *acc_x += add_x * trim;
            *acc_y += add_y * trim;
            *acc_z += add_z * trim;
            *allocated_mag = max_budget;
        }
    }
}

void verlet_system_update(VerletSystem *system) {
    if (!system) return;
    
    float dt_sq = system->dt * system->dt;
    float inv_dt = 1.0f / system->dt;
    
    // Copy current state to avoid in-place update artifacts during neighbor lookups
    Particle old_particles[MAX_PARTICLES];
    memcpy(old_particles, system->particles, sizeof(system->particles));
    
    // 1. Build Spatial Grid Partitioning Map
    memset(system->grid, 0, sizeof(system->grid));
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &old_particles[i];
        if (!p->active) continue;
        
        int gx = (int)((p->x - system->boundary_min_x) / GRID_CELL_SIZE);
        int gy = (int)((p->y - system->boundary_min_y) / GRID_CELL_SIZE);
        int gz = (int)((p->z - system->boundary_min_z) / GRID_CELL_SIZE);
        
        if (gx < 0) gx = 0; else if (gx >= GRID_DIM_X) gx = GRID_DIM_X - 1;
        if (gy < 0) gy = 0; else if (gy >= GRID_DIM_Y) gy = GRID_DIM_Y - 1;
        if (gz < 0) gz = 0; else if (gz >= GRID_DIM_Z) gz = GRID_DIM_Z - 1;
        
        GridCell *cell = &system->grid[gx][gy][gz];
        if (cell->count < 32) {
            cell->particle_indices[cell->count++] = i;
        }
    }
    
    // 2. Perform Integration and Boid Behaviors
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &system->particles[i];
        Particle *old_p = &old_particles[i];
        if (!p->active) continue;
        
        float temp_x = p->x;
        float temp_y = p->y;
        float temp_z = p->z;
        
        // Approximate current velocity
        float vel_x = (old_p->x - old_p->px) * inv_dt;
        float vel_y = (old_p->y - old_p->py) * inv_dt;
        float vel_z = (old_p->z - old_p->pz) * inv_dt;
        
        // --- 2a. Priority 1: Auncient Zener Clamping Physics (High Priority) ---
        double noise = 0.0;
        double vs = 0.08 * old_p->y;
        double vd = tsfi_zener_tick(&system->zener, vs, 1000.0, &noise);
        double id = (vs - vd) / 1000.0;
        float restoring_ay = (float)(-id * 250000.0);
        
        if (system->registry && id > 0.0) {
            uint64_t diyat_fee = (uint64_t)(id * 50000.0);
            if (diyat_fee > 0) {
                system->registry->active_state.monopole += diyat_fee;
                system->registry->active_state.barn = (uint64_t)(system->zener.temp * 100.0);
            }
        }
        
        // --- 2b. Priority 2: Predictive Steer-to-Avoid Boundary System ---
        float boundary_steer_x = 0.0f;
        float boundary_steer_y = 0.0f;
        float boundary_steer_z = 0.0f;
        float safety_margin = 35.0f;
        
        // Project position based on current velocity
        float proj_x = old_p->x + vel_x * 0.1f;
        float proj_y = old_p->y + vel_y * 0.1f;
        float proj_z = old_p->z + vel_z * 0.1f;
        
        if (proj_x < system->boundary_min_x + safety_margin) boundary_steer_x = (system->boundary_min_x + safety_margin) - proj_x;
        else if (proj_x > system->boundary_max_x - safety_margin) boundary_steer_x = (system->boundary_max_x - safety_margin) - proj_x;
        
        if (proj_y < system->boundary_min_y + safety_margin) boundary_steer_y = (system->boundary_min_y + safety_margin) - proj_y;
        else if (proj_y > system->boundary_max_y - safety_margin) boundary_steer_y = (system->boundary_max_y - safety_margin) - proj_y;
        
        if (proj_z < system->boundary_min_z + safety_margin) boundary_steer_z = (system->boundary_min_z + safety_margin) - proj_z;
        else if (proj_z > system->boundary_max_z - safety_margin) boundary_steer_z = (system->boundary_max_z - safety_margin) - proj_z;
        
        // --- 2c. Priority 3 & 4: Flocking Behaviors via Grid Neighbors ---
        float sep_x = 0.0f, sep_y = 0.0f, sep_z = 0.0f;
        float align_x = 0.0f, align_y = 0.0f, align_z = 0.0f;
        float coh_x = 0.0f, coh_y = 0.0f, coh_z = 0.0f;
        int sep_count = 0, neighbor_count = 0;
        
        int p_gx = (int)((old_p->x - system->boundary_min_x) / GRID_CELL_SIZE);
        int p_gy = (int)((old_p->y - system->boundary_min_y) / GRID_CELL_SIZE);
        int p_gz = (int)((old_p->z - system->boundary_min_z) / GRID_CELL_SIZE);
        
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                for (int dz = -1; dz <= 1; dz++) {
                    int n_gx = p_gx + dx;
                    int n_gy = p_gy + dy;
                    int n_gz = p_gz + dz;
                    
                    if (n_gx < 0 || n_gx >= GRID_DIM_X) continue;
                    if (n_gy < 0 || n_gy >= GRID_DIM_Y) continue;
                    if (n_gz < 0 || n_gz >= GRID_DIM_Z) continue;
                    
                    GridCell *cell = &system->grid[n_gx][n_gy][n_gz];
                    for (int n_idx = 0; n_idx < cell->count; n_idx++) {
                        uint32_t other_idx = cell->particle_indices[n_idx];
                        if (other_idx == (uint32_t)i) continue;
                        
                        Particle *other = &old_particles[other_idx];
                        float diff_x = old_p->x - other->x;
                        float diff_y = old_p->y - other->y;
                        float diff_z = old_p->z - other->z;
                        float dist = (float)sqrt(diff_x*diff_x + diff_y*diff_y + diff_z*diff_z);
                        if (dist < 0.001f) continue;
                        
                        if (dist < system->separation_dist) {
                            sep_x += diff_x / dist;
                            sep_y += diff_y / dist;
                            sep_z += diff_z / dist;
                            sep_count++;
                        }
                        
                        if (dist < system->neighbor_dist) {
                            float o_vel_x = (other->x - other->px) * inv_dt;
                            float o_vel_y = (other->y - other->py) * inv_dt;
                            float o_vel_z = (other->z - other->pz) * inv_dt;
                            
                            align_x += o_vel_x;
                            align_y += o_vel_y;
                            align_z += o_vel_z;
                            
                            coh_x += other->x;
                            coh_y += other->y;
                            coh_z += other->z;
                            
                            neighbor_count++;
                        }
                    }
                }
            }
        }
        
        // --- 2d. Budget-prioritized Acceleration Allocation ---
        float final_ax = 0.0f;
        float final_ay = p->ay; // Base gravity
        float final_az = 0.0f;
        float allocated_mag = 0.0f;
        
        // 1. Zener Diode Clamping Restoring acceleration (Priority 1)
        float zener_mag = (float)fabs(restoring_ay);
        if (zener_mag > 0.0001f) {
            allocate_acceleration(&final_ax, &final_ay, &final_az, &allocated_mag, zener_mag, 0.0f, restoring_ay, 0.0f, system->max_accel);
        }
        
        // 2. Predictive Steer-to-avoid Boundaries (Priority 2)
        float boundary_steer_mag = (float)sqrt(boundary_steer_x*boundary_steer_x + boundary_steer_y*boundary_steer_y + boundary_steer_z*boundary_steer_z);
        if (boundary_steer_mag > 0.0001f) {
            allocate_acceleration(&final_ax, &final_ay, &final_az, &allocated_mag, boundary_steer_mag * 12.0f, boundary_steer_x, boundary_steer_y, boundary_steer_z, system->max_accel);
        }
        
        // 3. Separation behavior (Priority 3)
        if (sep_count > 0) {
            float sep_mag = (float)sqrt(sep_x*sep_x + sep_y*sep_y + sep_z*sep_z);
            if (sep_mag > 0.0001f) {
                allocate_acceleration(&final_ax, &final_ay, &final_az, &allocated_mag, system->max_accel * 0.4f * system->separation_w, sep_x, sep_y, sep_z, system->max_accel);
            }
        }
        
        // 4. Alignment & Cohesion behaviors (Priority 4)
        if (neighbor_count > 0) {
            float avg_align_x = align_x / neighbor_count - vel_x;
            float avg_align_y = align_y / neighbor_count - vel_y;
            float avg_align_z = align_z / neighbor_count - vel_z;
            float align_mag = (float)sqrt(avg_align_x*avg_align_x + avg_align_y*avg_align_y + avg_align_z*avg_align_z);
            if (align_mag > 0.0001f) {
                allocate_acceleration(&final_ax, &final_ay, &final_az, &allocated_mag, system->max_accel * 0.15f * system->alignment_w, avg_align_x, avg_align_y, avg_align_z, system->max_accel);
            }
            
            float avg_coh_x = coh_x / neighbor_count - old_p->x;
            float avg_coh_y = coh_y / neighbor_count - old_p->y;
            float avg_coh_z = coh_z / neighbor_count - old_p->z;
            float coh_mag = (float)sqrt(avg_coh_x*avg_coh_x + avg_coh_y*avg_coh_y + avg_coh_z*avg_coh_z);
            if (coh_mag > 0.0001f) {
                allocate_acceleration(&final_ax, &final_ay, &final_az, &allocated_mag, system->max_accel * 0.1f * system->cohesion_w, avg_coh_x, avg_coh_y, avg_coh_z, system->max_accel);
            }
        }
        
        // 3. Integrate position (incorporating budget-prioritized accelerations)
        p->x = 2.0f * p->x - p->px + final_ax * dt_sq;
        p->y = 2.0f * p->y - p->py + final_ay * dt_sq;
        p->z = 2.0f * p->z - p->pz + final_az * dt_sq;
        
        p->px = temp_x;
        p->py = temp_y;
        p->pz = temp_z;
        
        // Speed limit / damping clamp
        float new_vel_x = (p->x - p->px) * inv_dt;
        float new_vel_y = (p->y - p->py) * inv_dt;
        float new_vel_z = (p->z - p->pz) * inv_dt;
        float speed = (float)sqrt(new_vel_x*new_vel_x + new_vel_y*new_vel_y + new_vel_z*new_vel_z);
        if (speed > system->max_speed) {
            float clamp_factor = system->max_speed / speed;
            p->x = p->px + new_vel_x * clamp_factor * system->dt;
            p->y = p->py + new_vel_y * clamp_factor * system->dt;
            p->z = p->pz + new_vel_z * clamp_factor * system->dt;
        }
        
        // 4. Handle Boundary Collisions (Coefficient of Restitution: 0.85)
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
