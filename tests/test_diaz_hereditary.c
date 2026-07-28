#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#define HISTORY_DEPTH 5
#define VECTOR_DIM 3

// Structure representing a Verlet node with fractional-order hereditary memory
typedef struct {
    double pos[VECTOR_DIM];
    double pos_history[HISTORY_DEPTH][VECTOR_DIM]; // Cached history of past coordinate frames
    double force[VECTOR_DIM];
    double mass;
} HereditaryNode;

// Compute dot product of 3D vectors
double dot_product_3d(const double *v1, const double *v2) {
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

// Compute magnitude of 3D vector
double magnitude_3d(const double *v) {
    return sqrt(dot_product_3d(v, v));
}

// Update the coordinate history buffer
void push_history(HereditaryNode *node) {
    for (int i = HISTORY_DEPTH - 1; i > 0; i--) {
        for (int d = 0; d < VECTOR_DIM; d++) {
            node->pos_history[i][d] = node->pos_history[i - 1][d];
        }
    }
    for (int d = 0; d < VECTOR_DIM; d++) {
        node->pos_history[0][d] = node->pos[d];
    }
}

// J. B. Diaz Fractional-Order Hereditary Verlet Integrator
// Combines historical coordinate positions using memory weights
void update_hereditary_verlet(HereditaryNode *node, double dt, double memory_alpha) {
    double acceleration[VECTOR_DIM];
    for (int d = 0; d < VECTOR_DIM; d++) {
        acceleration[d] = node->force[d] / node->mass;
    }

    // Hereditary convolution weights (Grünwald-Letnikov fractional representation)
    // w[0] = 2.0 - alpha, w[1] = -(1.0 - alpha), etc.
    double w0 = 2.0 - memory_alpha;
    double w1 = -(1.0 - memory_alpha);
    double w_hist = -0.05; // Fading memory decay coefficient

    double next_pos[VECTOR_DIM];
    for (int d = 0; d < VECTOR_DIM; d++) {
        // Integrate current position with weighted historical trajectories
        next_pos[d] = w0 * node->pos[d] + 
                      w1 * node->pos_history[0][d] + 
                      w_hist * node->pos_history[1][d] + 
                      acceleration[d] * dt * dt;
    }

    push_history(node);
    for (int d = 0; d < VECTOR_DIM; d++) {
        node->pos[d] = next_pos[d];
    }
}

// Validate Diaz-Metcalf boundary transitions on coordinate dynamics
bool verify_diaz_metcalf_bounds(const double *velocity, const double *force, double a, double b) {
    double mag_f = magnitude_3d(force);
    if (mag_f < 1e-9) return true; // Stable state (no forces acting)

    double ratio = dot_product_3d(velocity, force) / (mag_f * mag_f);
    
    // Bounds check: if projection ratio slips outside [a, b], numerical instability is imminent
    return (ratio >= a && ratio <= b);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT DIAZ HEREDITARY TRANSITION VALIDATION SUITE\n");
    printf("=============================================================\n");

    // 1. Initialize Node with mass and position history
    HereditaryNode node = {
        .pos = { 0.0, 10.0, 0.0 },
        .mass = 1.0,
        .force = { 0.0, -9.8, 0.0 } // Downward gravity force
    };

    // Initialize history coordinates
    for (int i = 0; i < HISTORY_DEPTH; i++) {
        node.pos_history[i][0] = 0.0;
        node.pos_history[i][1] = 10.0;
        node.pos_history[i][2] = 0.0;
    }

    double dt = 0.01;
    double memory_alpha = 0.15; // Fractional integration damping scale
    double a = -0.5;            // Critical damping bound
    double b = 0.5;             // Elastic lock bound

    printf("[TEST] Simulating 10 steps of hereditary Verlet integration...\n");
    for (int step = 1; step <= 10; step++) {
        double prev_pos[VECTOR_DIM];
        memcpy(prev_pos, node.pos, VECTOR_DIM * sizeof(double));

        update_hereditary_verlet(&node, dt, memory_alpha);

        // Calculate velocity vector: v = (pos - prev_pos) / dt
        double velocity[VECTOR_DIM];
        for (int d = 0; d < VECTOR_DIM; d++) {
            velocity[d] = (node.pos[d] - prev_pos[d]) / dt;
        }

        // Verify that the dynamics respect Diaz-Metcalf bounds on every update step
        bool stable = verify_diaz_metcalf_bounds(velocity, node.force, a, b);
        printf("[STEP %02d] Y-Pos: %.4f | V-Mag: %.4f | Diaz-Metcalf Stable: %s\n", 
               step, node.pos[1], magnitude_3d(velocity), stable ? "YES" : "NO");

        assert(stable == true);
    }

    printf("=============================================================\n");
    printf("ALL DIAZ HEREDITARY TRANSITION TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}
