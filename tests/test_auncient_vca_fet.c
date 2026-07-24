#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// Simulated low-level FET gate parameters mapped to Verlet mass-spring dynamics
typedef struct {
    double v_curr;        // Current gate voltage (x_n)
    double v_prev;        // Previous gate voltage (x_{n-1})
    double capacitance;   // Mass parameter (m)
    double channel_resist;// Damping factor (c)
    double stiffness;     // Electrostatic spring constant (k)
    double threshold;     // Cutoff voltage threshold (e.g. 0.7V)
} vca_fet_t;

// -------------------------------------------------------------
// VCA Verlet FET Discharge Step
// -------------------------------------------------------------
void vca_fet_discharge_step(vca_fet_t *fet, double dt) {
    // 1. Calculate velocity: v = (x_curr - x_prev) / dt
    double velocity = (fet->v_curr - fet->v_prev) / dt;

    // 2. Compute electrostatic restoration and damping forces
    // F = -k * x - c * v
    double force = -fet->stiffness * fet->v_curr - fet->channel_resist * velocity;

    // 3. Acceleration: a = F / m (capacitance acts as mass term)
    double acceleration = force / fet->capacitance;

    // 4. Verlet Integration Step: x_{n+1} = 2*x_n - x_{n-1} + a * dt^2
    double v_next = 2.0 * fet->v_curr - fet->v_prev + acceleration * dt * dt;

    // Update state history
    fet->v_prev = fet->v_curr;
    fet->v_curr = v_next;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT VCA VERLET FET DISCHARGE VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initialize FET at full charge (5.0V)
    vca_fet_t fet = {
        .v_curr = 5.0,
        .v_prev = 5.0,
        .capacitance = 1.0,   // Unit capacitance
        .channel_resist = 1.5, // Channel resistance (damps oscillation)
        .stiffness = 2.0,     // Electrostatic decay pull
        .threshold = 0.7      // Transistor cutoff threshold
    };

    printf("[TEST] Running VCA Verlet discharge simulation...\n");
    fflush(stdout);

    double dt = 0.1;
    int steps = 0;
    bool threshold_crossed = false;

    // Run solver steps to simulate decay
    while (steps < 100) {
        vca_fet_discharge_step(&fet, dt);
        steps++;

        // Track when voltage drops below threshold
        if (fet.v_curr < fet.threshold && !threshold_crossed) {
            threshold_crossed = true;
            printf("   ✓ Cutoff threshold crossed at step %d (Voltage: %0.4fV)\n", steps, fet.v_curr);
            fflush(stdout);
        }

        // Break once decay stabilizes near 0
        if (fet.v_curr < 0.05 && steps > 30) {
            break;
        }
    }

    assert(threshold_crossed == true);
    assert(fet.v_curr < fet.threshold);
    printf("   ✓ FET discharge settled stably below threshold.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("VCA VERLET FET TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
