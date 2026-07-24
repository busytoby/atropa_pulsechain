#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// Moon Ki Kim (MKK) Elastic Network Model for a 2-Gate Coupled FET Array
typedef struct {
    double v_curr[2];          // Current gate voltages (x_curr)
    double v_prev[2];          // Previous gate voltages (x_prev)
    double capacitance;        // Gate mass parameter (m)
    double damping;            // Channel resistance (c)
    double self_stiffness;     // Self-discharge spring constant (k)
    double coupling_stiffness; // Inter-gate coupling spring constant (k_c)
    double threshold;          // Parasitic turn-on threshold (0.7V)
} mkk_fet_array_t;

// -------------------------------------------------------------
// MKK Verlet Coupled FET Array Step
// -------------------------------------------------------------
void mkk_fet_array_step(mkk_fet_array_t *arr, double dt) {
    double v_next[2];

    for (int i = 0; i < 2; i++) {
        int neighbor = 1 - i;

        // 1. Calculate velocity
        double velocity = (arr->v_curr[i] - arr->v_prev[i]) / dt;

        // 2. Compute forces (Self-discharge + Elastic coupling force to neighbor)
        double self_force = -arr->self_stiffness * arr->v_curr[i] - arr->damping * velocity;
        double coupling_force = -arr->coupling_stiffness * (arr->v_curr[i] - arr->v_curr[neighbor]);
        double total_force = self_force + coupling_force;

        // 3. Acceleration (a = F / m)
        double accel = total_force / arr->capacitance;

        // 4. Verlet Integration Step
        v_next[i] = 2.0 * arr->v_curr[i] - arr->v_prev[i] + accel * dt * dt;
    }

    // Update state history
    for (int i = 0; i < 2; i++) {
        arr->v_prev[i] = arr->v_curr[i];
        arr->v_curr[i] = v_next[i];
    }
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT MKK ELASTIC NETWORK FET VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initialize FET 0 at full charge (5.0V), FET 1 at 0.0V (inactive neighbor)
    mkk_fet_array_t arr = {
        .v_curr = { 5.0, 0.0 },
        .v_prev = { 5.0, 0.0 },
        .capacitance = 1.0,
        .damping = 1.6,
        .self_stiffness = 2.0,
        .coupling_stiffness = 0.3, // Mild capacitive coupling
        .threshold = 0.7
    };

    printf("[TEST] Running MKK Elastic Network FET array simulation...\n");
    fflush(stdout);

    double dt = 0.1;
    double max_neighbor_spike = 0.0;
    int steps = 0;

    while (steps < 50) {
        mkk_fet_array_step(&arr, dt);
        steps++;

        // Track max parasitic spike on the inactive neighboring FET 1
        if (arr.v_curr[1] > max_neighbor_spike) {
            max_neighbor_spike = arr.v_curr[1];
        }
    }

    printf("   ✓ FET 0 final voltage: %0.4fV\n", arr.v_curr[0]);
    printf("   ✓ Neighbor FET 1 maximum parasitic spike: %0.4fV\n", max_neighbor_spike);
    fflush(stdout);

    // Asserts: Active FET 0 must decay past threshold; neighbor must not turn on parasitically
    assert(arr.v_curr[0] < arr.threshold);
    assert(max_neighbor_spike < arr.threshold);

    printf("   ✓ No parasitic gate turn-on detected on neighboring channels.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("MKK ELASTIC NETWORK FET TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
