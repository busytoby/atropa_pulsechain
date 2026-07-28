#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#define MATRIX_DIM 3
#define HISTORY_DEPTH 4

typedef struct {
    double pos[MATRIX_DIM];
    double pos_history[HISTORY_DEPTH][MATRIX_DIM];
    double force[MATRIX_DIM];
    double mass;
} MetamorphosisJoint;

// Power Iteration to estimate the maximum eigenvalue of the stiffness matrix
double get_max_eigenvalue(const double K[MATRIX_DIM][MATRIX_DIM], int iterations) {
    double v[MATRIX_DIM] = { 1.0, 1.0, 1.0 };
    double next_v[MATRIX_DIM];

    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < MATRIX_DIM; i++) {
            next_v[i] = 0.0;
            for (int j = 0; j < MATRIX_DIM; j++) {
                next_v[i] += K[i][j] * v[j];
            }
        }
        double norm = 0.0;
        for (int i = 0; i < MATRIX_DIM; i++) norm += next_v[i] * next_v[i];
        norm = sqrt(norm);
        assert(norm > 1e-9);

        for (int i = 0; i < MATRIX_DIM; i++) v[i] = next_v[i] / norm;
    }

    double numerator = 0.0;
    double denominator = 0.0;
    for (int i = 0; i < MATRIX_DIM; i++) {
        double kv_i = 0.0;
        for (int j = 0; j < MATRIX_DIM; j++) kv_i += K[i][j] * v[j];
        numerator += v[i] * kv_i;
        denominator += v[i] * v[i];
    }
    return numerator / denominator;
}

// Update joint coordinate history
void push_joint_history(MetamorphosisJoint *joint) {
    for (int i = HISTORY_DEPTH - 1; i > 0; i--) {
        memcpy(joint->pos_history[i], joint->pos_history[i - 1], MATRIX_DIM * sizeof(double));
    }
    memcpy(joint->pos_history[0], joint->pos, MATRIX_DIM * sizeof(double));
}

// Hereditary fractional Verlet update
void integrate_joint_hereditary(MetamorphosisJoint *joint, double dt, double memory_alpha) {
    double w0 = 2.0 - memory_alpha;
    double w1 = -(1.0 - memory_alpha);
    double w_hist = -0.05;

    double next_pos[MATRIX_DIM];
    for (int d = 0; d < MATRIX_DIM; d++) {
        double acc = joint->force[d] / joint->mass;
        next_pos[d] = w0 * joint->pos[d] + 
                      w1 * joint->pos_history[0][d] + 
                      w_hist * joint->pos_history[1][d] + 
                      acc * dt * dt;
    }

    push_joint_history(joint);
    memcpy(joint->pos, next_pos, MATRIX_DIM * sizeof(double));
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT INTEGRATED METAMORPHOSIS PHYSICS TEST SUITE\n");
    printf("=============================================================\n");

    MetamorphosisJoint joint = {
        .pos = { 0.0, 1.0, 0.0 },
        .mass = 1.0,
        .force = { 0.0, -9.8, 0.0 }
    };

    // Initialize history
    for (int i = 0; i < HISTORY_DEPTH; i++) {
        memcpy(joint.pos_history[i], joint.pos, MATRIX_DIM * sizeof(double));
    }

    double base_K[MATRIX_DIM][MATRIX_DIM] = {
        {  4.0, -2.0,  0.0 },
        { -2.0,  5.0, -3.0 },
        {  0.0, -3.0,  3.0 }
    };

    double memory_alpha = 0.12;

    printf("[TEST] Running 15-frame Metamorphosis Simulation...\n");
    for (int frame = 1; frame <= 15; frame++) {
        // Metamorphosis: Stiffness increases over time (hardening)
        double stiffness_multiplier = 1.0 + (double)frame * 0.5;
        double current_K[MATRIX_DIM][MATRIX_DIM];
        for (int i = 0; i < MATRIX_DIM; i++) {
            for (int j = 0; j < MATRIX_DIM; j++) {
                current_K[i][j] = base_K[i][j] * stiffness_multiplier;
            }
        }

        // 1. Solve maximum eigenvalue for current stiffness state
        double lambda_max = get_max_eigenvalue(current_K, 20);

        // 2. Adaptively calculate safe time-step dt based on Diaz-Metcalf stability bounds
        double dt_safe = sqrt(4.0 / lambda_max) * 0.90; // 10% safety margin

        // 3. Integrate dynamics using fractional-order Verlet solver
        integrate_joint_hereditary(&joint, dt_safe, memory_alpha);

        // 4. Verify stability (coordinates should remain bounded)
        double dist = sqrt(joint.pos[0]*joint.pos[0] + joint.pos[1]*joint.pos[1] + joint.pos[2]*joint.pos[2]);
        printf("[FRAME %02d] Rigidity: %.2fx | Lambda_max: %.3f | Safe dt: %.4fs | Node Pos: (%.3f, %.3f, %.3f)\n",
               frame, stiffness_multiplier, lambda_max, dt_safe, joint.pos[0], joint.pos[1], joint.pos[2]);

        assert(!isnan(dist));
        assert(dist < 50.0); // Coordinate explosion check
    }

    printf("=============================================================\n");
    printf("ALL INTEGRATED METAMORPHOSIS TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}
