#include <math.h>
#include "tsfi_parc_notetaker.h"

void tsfi_parc_notetaker_init(tsfi_parc_notetaker_fet_t *fet) {
    if (!fet) return;

    // Initialize Source, Gate, and Drain charge layout nodes
    fet->source_node.pos_x = 0.0f;
    fet->source_node.pos_y = 0.0f;
    fet->source_node.prev_x = 0.0f;
    fet->source_node.prev_y = 0.0f;
    fet->source_node.mass = 1.0f;
    fet->source_node.charge = 5.0f; // starting voltage level

    fet->gate_node.pos_x = 1.0f;
    fet->gate_node.pos_y = 0.0f;
    fet->gate_node.prev_x = 1.0f;
    fet->gate_node.prev_y = 0.0f;
    fet->gate_node.mass = 1.0f;
    fet->gate_node.charge = 5.0f;

    fet->drain_node.pos_x = 2.0f;
    fet->drain_node.pos_y = 0.0f;
    fet->drain_node.prev_x = 2.0f;
    fet->drain_node.prev_y = 0.0f;
    fet->drain_node.mass = 1.0f;
    fet->drain_node.charge = 5.0f;

    fet->depletion_factor = 0.0f;
}

int tsfi_parc_notetaker_step_discharge(tsfi_parc_notetaker_fet_t *fet, float dt) {
    if (!fet || dt <= 0.0f) return -1;

    // 1. Verlet Integration to solve charge node dynamics
    // Source Node
    float s_temp_x = fet->source_node.pos_x;
    float s_temp_y = fet->source_node.pos_y;
    float force_x = -0.1f * fet->source_node.pos_x; // spring return force
    fet->source_node.pos_x += (fet->source_node.pos_x - fet->source_node.prev_x) + (force_x / fet->source_node.mass) * dt * dt;
    fet->source_node.prev_x = s_temp_x;
    fet->source_node.prev_y = s_temp_y;

    // 2. Mass-spring coupling: Pull drain node charge layout towards source node
    float dx = fet->drain_node.pos_x - fet->source_node.pos_x;
    float dist = sqrtf(dx * dx);
    if (dist > 0.01f) {
        float rest_len = 2.0f;
        float spring_k = 0.5f;
        float displacement = dist - rest_len;
        float spring_force = -spring_k * displacement;
        
        // Disperse current during the discharge cycle
        fet->drain_node.pos_x += (spring_force / fet->drain_node.mass) * dt;
    }

    // 3. Discharge voltage dissipation curve
    fet->source_node.charge *= expf(-0.05f * dt);
    fet->gate_node.charge *= expf(-0.02f * dt);
    fet->drain_node.charge *= expf(-0.1f * dt);

    fet->depletion_factor = 1.0f - (fet->drain_node.charge / 5.0f);

    return 0;
}
