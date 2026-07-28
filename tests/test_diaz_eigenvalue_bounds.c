#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#define MATRIX_DIM 3

// Power Iteration to estimate the maximum eigenvalue of stiffness matrix K
double estimate_max_eigenvalue(const double K[MATRIX_DIM][MATRIX_DIM], int iterations) {
    double v[MATRIX_DIM] = { 1.0, 1.0, 1.0 }; // Initial guess vector
    double next_v[MATRIX_DIM];

    for (int iter = 0; iter < iterations; iter++) {
        // Compute next_v = K * v
        for (int i = 0; i < MATRIX_DIM; i++) {
            next_v[i] = 0.0;
            for (int j = 0; j < MATRIX_DIM; j++) {
                next_v[i] += K[i][j] * v[j];
            }
        }

        // Normalize next_v
        double norm = 0.0;
        for (int i = 0; i < MATRIX_DIM; i++) {
            norm += next_v[i] * next_v[i];
        }
        norm = sqrt(norm);
        assert(norm > 1e-9);

        for (int i = 0; i < MATRIX_DIM; i++) {
            v[i] = next_v[i] / norm;
        }
    }

    // Rayleigh quotient: lambda = (v^T * K * v) / (v^T * v)
    double numerator = 0.0;
    double denominator = 0.0;
    for (int i = 0; i < MATRIX_DIM; i++) {
        double kv_i = 0.0;
        for (int j = 0; j < MATRIX_DIM; j++) {
            kv_i += K[i][j] * v[j];
        }
        numerator += v[i] * kv_i;
        denominator += v[i] * v[i];
    }

    return numerator / denominator;
}

// Diaz-Metcalf Stability Verification
bool verify_diaz_metcalf_stability(double lambda_max, double dt) {
    double stability_bound = 4.0 / (dt * dt);
    
    // Diaz-Metcalf stability condition: lambda_max * dt^2 <= 4
    printf("[STABILITY] Lambda_max: %.4f | Stability Bound (4/dt^2): %.4f\n", 
           lambda_max, stability_bound);
           
    return (lambda_max <= stability_bound);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT DIAZ-METCALF EIGENVALUE BOUNDS TEST SUITE\n");
    printf("=============================================================\n");

    // Construct stiffness matrix K for a 3-node spring assembly (Moth joint)
    // K is symmetric and positive semi-definite
    double K[MATRIX_DIM][MATRIX_DIM] = {
        {  40.0, -20.0,   0.0 },
        { -20.0,  50.0, -30.0 },
        {   0.0, -30.0,  30.0 }
    };

    printf("[TEST] Running Power Iteration to resolve maximum stiffness eigenvalue...\n");
    double lambda_max = estimate_max_eigenvalue(K, 30);
    printf("[TEST] Estimated Max Eigenvalue (lambda_max): %.6f\n", lambda_max);
    assert(lambda_max > 0.0);

    // 1. Test with stable time step (dt = 0.1s -> Bound = 400.0)
    double dt_stable = 0.1;
    printf("[TEST] Checking stable time-step (dt = %.4f)...\n", dt_stable);
    bool stable = verify_diaz_metcalf_stability(lambda_max, dt_stable);
    assert(stable == true);
    printf("[TEST] Stability bounds verified for dt = %.4f\n", dt_stable);

    // 2. Test with unstable time step (dt = 0.3s -> Bound = 44.44)
    double dt_unstable = 0.35;
    printf("[TEST] Checking unstable time-step (dt = %.4f)...\n", dt_unstable);
    bool unstable = verify_diaz_metcalf_stability(lambda_max, dt_unstable);
    assert(unstable == false); // Should violate Diaz-Metcalf stability bounds
    printf("[TEST] Instability bounds verified for dt = %.4f\n", dt_unstable);

    printf("=============================================================\n");
    printf("ALL DIAZ-METCALF EIGENVALUE BOUND TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}
