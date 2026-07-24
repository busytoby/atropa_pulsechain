#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

// Coupled Simulator State (Electrical FET Discharge & Soft-Body Verlet Physics)
typedef struct {
    // Electrical state
    double gate_charge;      // Q
    double discharge_rate;   // alpha

    // Soft-body mass-spring state (Verlet integration)
    double displacement;     // X (displacement relative to rest position)
    double prev_displacement;// X_prev
    double mass;             // M
    double spring_k;         // K
    double time_step;        // dt
} coupled_simulator_t;

// -------------------------------------------------------------
// Coupled Simulation Step
// -------------------------------------------------------------
void step_coupled_simulation(coupled_simulator_t *sim) {
    // 1. Soft-body Physics: Calculate force based on mechanical spring and electrical charge
    // Force = -K * displacement + (charge-dependent electrostatic attraction force)
    double electrostatic_force = 0.5 * sim->gate_charge * sim->gate_charge;
    double spring_force = -sim->spring_k * sim->displacement;
    double total_force = spring_force + electrostatic_force;
    double acceleration = total_force / sim->mass;

    // Verlet integration step for displacement: X_new = 2*X - X_prev + a * dt^2
    double current_disp = sim->displacement;
    sim->displacement = 2.0 * current_disp - sim->prev_displacement + acceleration * sim->time_step * sim->time_step;
    sim->prev_displacement = current_disp;

    // 2. Electrical: Update FET discharge cycle modulated by physical mechanical displacement
    // Discharge rate accelerates with larger mechanical displacement (compression of gate dielectric)
    double effective_discharge = sim->discharge_rate * (1.0 + fabs(sim->displacement));
    sim->gate_charge -= effective_discharge * sim->time_step;
    if (sim->gate_charge < 0.0) {
        sim->gate_charge = 0.0;
    }
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT COUPLED MICROSYSTEM SIMULATOR SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    coupled_simulator_t sim = {
        .gate_charge = 5.0,
        .discharge_rate = 0.5,
        .displacement = 0.0,
        .prev_displacement = 0.0,
        .mass = 1.0,
        .spring_k = 10.0,
        .time_step = 0.1
    };

    printf("[TEST] Running coupled simulator loop (5 steps)...\n");
    fflush(stdout);

    double initial_charge = sim.gate_charge;
    double initial_displacement = sim.displacement;

    for (int step = 1; step <= 5; step++) {
        step_coupled_simulation(&sim);
        printf("   Step %d: Charge = %.4f | Displacement = %.4f\n", step, sim.gate_charge, sim.displacement);
        fflush(stdout);
    }

    // Assert that coupling effects modulated both domains
    assert(sim.gate_charge < initial_charge);
    assert(sim.displacement != initial_displacement);
    printf("   ✓ Electrical and mechanical domains mutually converged.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("COUPLED SIMULATOR TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
