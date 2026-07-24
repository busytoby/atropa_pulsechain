#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// AUTOMATIC Storage Capacitor Simulation
typedef struct {
    double charge_v;      // Voltage/Charge position (V)
    double prev_charge_v; // Prior position for Verlet integration (V)
    double accel;         // Acceleration (V/s^2)
    gate_state_t allocation_gate;
} automatic_storage_t;

// -------------------------------------------------------------
// Verlet Solver Capacitor Discharge Step (FET physics simulation)
// -------------------------------------------------------------
void verlet_step_discharge(automatic_storage_t *storage, double dt, double k_decay) {
    // Restoring force pulling voltage to ground: F = -k * x
    storage->accel = -k_decay * storage->charge_v;

    // Verlet integration: x_new = 2*x - x_prev + a * dt^2
    double current_v = storage->charge_v;
    double next_v = (2.0 * current_v) - storage->prev_charge_v + (storage->accel * dt * dt);

    // Limit voltage to non-negative bounds
    if (next_v < 0.0) next_v = 0.0;

    storage->prev_charge_v = current_v;
    storage->charge_v = next_v;

    // NPN Conduction threshold: conducts only if voltage >= 0.7 V
    storage->allocation_gate = (storage->charge_v >= 0.7) ? CONDUC_STATE : CUTOFF_STATE;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT CAD VERLET CAPACITOR DISCHARGE VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initialize allocated automatic storage at 5.0 V (active-high conduction)
    automatic_storage_t storage = {
        .charge_v = 5.0,
        .prev_charge_v = 5.0,
        .accel = 0.0,
        .allocation_gate = CONDUC_STATE
    };

    // 1. Initial State: conducts (allocation is active)
    assert(storage.allocation_gate == CONDUC_STATE);
    printf("[TEST] Initial state: Voltage = %.2f V, Conduction Active.\n", storage.charge_v);
    fflush(stdout);

    // 2. Step Verlet discharge cycles
    printf("[TEST] Simulating block exit: Verlet capacitor discharge starting...\n");
    fflush(stdout);
    double dt = 0.1; // 100 ms step
    double k = 5.0;  // Spring restoring constant

    // Run 10 integration steps
    for (int i = 1; i <= 10; i++) {
        verlet_step_discharge(&storage, dt, k);
        const char *state_str = (storage.allocation_gate == CONDUC_STATE) ? "ALLOCATED" : "FREED";
        printf("   Step %d: Voltage = %.2f V -> Status: %s\n", i, storage.charge_v, state_str);
        fflush(stdout);
    }

    // After 10 steps, restoring pull should decay voltage below 0.7V, freeing storage
    assert(storage.allocation_gate == CUTOFF_STATE);
    assert(storage.charge_v < 0.7);
    printf("   ✓ Storage freed successfully via Verlet discharge curves.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("CAD VERLET CAPACITOR DISCHARGE TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
