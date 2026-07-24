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

typedef struct {
    uint32_t current_state;
    uint32_t checkpoint_state;
    uint32_t approvals;
    uint32_t total_nodes;
    bool proof_valid;
    bool rollback_triggered;
} tx_resolver_t;

// Coupled simulator output data stream
typedef struct {
    double values[4]; // Displacement/charge values over 4 time steps
    tx_resolver_t resolver;
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
    traj->resolver.rollback_triggered = false;

    // Ackerman-compatible Transaction Resolution:
    // If consensus is reached (majority approvals) and the transaction proof is valid,
    // we commit the state change ("ball rolling forward" proof).
    bool consensus_reached = (traj->resolver.approvals > traj->resolver.total_nodes / 2);
    if (consensus_reached && traj->resolver.proof_valid) {
        // Commit: The ball successfully rolls forward
        traj->resolver.checkpoint_state = traj->resolver.current_state;
        traj->simulation_gate = CONDUC_STATE;
        return true; // Bypass raw entropy checks as transaction trajectory is deterministic
    }

    // Rollback: If consensus fails, restore state from checkpoint
    if (!consensus_reached || !traj->resolver.proof_valid) {
        traj->resolver.current_state = traj->resolver.checkpoint_state;
        traj->resolver.rollback_triggered = true;
    }

    // Fallback: If no structured consensus state was processed, calculate raw trajectory entropy
    double H = calculate_trajectory_entropy(traj);
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
        .resolver = { .approvals = 0, .total_nodes = 3, .proof_valid = false },
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
        .resolver = { .approvals = 0, .total_nodes = 3, .proof_valid = false },
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

    // 3. Ackerman Commit Case: Ball rolling forward with valid consensus proof
    coupled_trajectory_t traj_commit = {
        .values = { 1.0, 50.0, -20.0, 100.0 }, // Would normally fail raw entropy check
        .resolver = {
            .current_state = 0xABC,
            .checkpoint_state = 0x111,
            .approvals = 2,
            .total_nodes = 3,
            .proof_valid = true
        },
        .simulation_gate = CUTOFF_STATE,
        .byzantine_fault_flagged = false
    };

    printf("[TEST] Checking Ackerman transactional commit (ball rolling forward proof)...\n");
    fflush(stdout);
    ok = evaluate_coupled_byzantine_gate(&traj_commit, 1.0);
    assert(ok == true);
    assert(traj_commit.simulation_gate == CONDUC_STATE);
    assert(traj_commit.resolver.checkpoint_state == 0xABC); // State updated
    assert(traj_commit.resolver.rollback_triggered == false);
    printf("   ✓ Transaction committed. State updated to 0xABC.\n");
    fflush(stdout);

    // 4. Ackerman Rollback Case: Fails consensus approvals -> Triggers rollback
    coupled_trajectory_t traj_rollback = {
        .values = { 1.0, 50.0, -20.0, 100.0 }, // Chaotic trajectory
        .resolver = {
            .current_state = 0xABC,
            .checkpoint_state = 0x111,
            .approvals = 1, // Fails majority approvals (1/3)
            .total_nodes = 3,
            .proof_valid = true
        },
        .simulation_gate = CUTOFF_STATE,
        .byzantine_fault_flagged = false
    };

    printf("[TEST] Checking Ackerman transactional rollback (insufficient approvals)...\n");
    fflush(stdout);
    ok = evaluate_coupled_byzantine_gate(&traj_rollback, 1.0);
    // Should fall back to raw entropy validation and fail
    assert(ok == false);
    assert(traj_rollback.resolver.current_state == 0x111); // Rolled back to checkpoint
    assert(traj_rollback.resolver.rollback_triggered == true);
    printf("   ✓ Transaction rolled back. State restored to 0x111.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("COUPLED BYZANTINE ENTROPY TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
