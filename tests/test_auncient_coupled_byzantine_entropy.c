#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Coupled simulator output data stream
typedef struct {
    double values[4]; // Displacement/charge values over 4 time steps
    gate_state_t simulation_gate;
    bool byzantine_fault_flagged;
} coupled_trajectory_t;

// -------------------------------------------------------------
// Wünsche Entropy Analysis on Coupled Simulator Trajectories
// -------------------------------------------------------------
double calculate_trajectory_entropy(const coupled_trajectory_t *traj) {
    // 1. Calculate sum for normalization
    double sum = 0.0;
    for (int i = 0; i < 4; i++) {
        sum += fabs(traj->values[i]);
    }
    if (sum == 0.0) return 0.0;

    // 2. Compute entropy over normalized state probabilities
    double entropy = 0.0;
    for (int i = 0; i < 4; i++) {
        double p = fabs(traj->values[i]) / sum;
        if (p > 0.0) {
            entropy -= p * (log(p) / log(2.0));
        }
    }
    return entropy;
}

bool evaluate_coupled_byzantine_gate(coupled_trajectory_t *traj, double max_entropy_threshold) {
    traj->simulation_gate = CUTOFF_STATE;
    traj->byzantine_fault_flagged = false;

    double H = calculate_trajectory_entropy(traj);

    // If entropy exceeds threshold, it implies a Byzantine fault / chaotic hardware error
    if (H > max_entropy_threshold) {
        traj->byzantine_fault_flagged = true;
        return false; // Byzantine fault flagged, simulation blocked
    }

    traj->simulation_gate = CONDUC_STATE;
    return true; // Stable physical simulation
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT COUPLED BYZANTINE ENTROPY VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Compliant Case: Stable physical trajectory (Low entropy) -> Should conduct
    coupled_trajectory_t traj_stable = {
        .values = { 10.0, 1.0, 0.0, 0.0 }, // Clear physical decaying trajectory
        .simulation_gate = CUTOFF_STATE,
        .byzantine_fault_flagged = false
    };

    printf("[TEST] Checking stable coupled simulator trajectory...\n");
    fflush(stdout);
    bool ok = evaluate_coupled_byzantine_gate(&traj_stable, 1.0);
    double H = calculate_trajectory_entropy(&traj_stable);
    printf("   Calculated Trajectory Entropy H = %.4f\n", H);
    assert(ok == true);
    assert(traj_stable.simulation_gate == CONDUC_STATE);
    assert(traj_stable.byzantine_fault_flagged == false);
    printf("   ✓ Physical trajectory approved. Conduction open.\n");
    fflush(stdout);

    // 2. Reject Case: Erratic / Byzantine trajectory (High entropy) -> Should flag & cutoff
    coupled_trajectory_t traj_byzantine = {
        .values = { 1.0, 50.0, -20.0, 100.0 }, // Wildly chaotic non-physical values (Byzantine)
        .simulation_gate = CUTOFF_STATE,
        .byzantine_fault_flagged = false
    };

    printf("[TEST] Checking chaotic coupled simulator trajectory (Byzantine trap)...\n");
    fflush(stdout);
    ok = evaluate_coupled_byzantine_gate(&traj_byzantine, 1.0);
    H = calculate_trajectory_entropy(&traj_byzantine);
    printf("   Calculated Trajectory Entropy H = %.4f\n", H);
    assert(ok == false);
    assert(traj_byzantine.simulation_gate == CUTOFF_STATE);
    assert(traj_byzantine.byzantine_fault_flagged == true);
    printf("   ✓ Trajectory entropy exceeded limits. Byzantine fault flagged successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("COUPLED BYZANTINE ENTROPY TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
