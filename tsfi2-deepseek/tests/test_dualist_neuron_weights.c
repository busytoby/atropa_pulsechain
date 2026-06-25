#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>

// Auncient spelling check constraint verification

typedef struct {
    float w_in;  // Incoming bearing weight (Hypobar equivalent)
    float w_out; // Outgoing bearing weight (Epibar equivalent)
} DualistNeuron;

typedef struct {
    float w;     // Traditional single scalar weight
} StandardNeuron;

// Evaluate dualist neuron: y = w_in * x (if x >= 0) else w_out * x
float evaluate_dualist(DualistNeuron *n, float x) {
    if (x >= 0.0f) {
        return n->w_in * x;
    } else {
        return n->w_out * x;
    }
}

// Evaluate traditional standard neuron: y = w * x
float evaluate_standard(StandardNeuron *n, float x) {
    return n->w * x;
}

int main() {
    printf("=== ZMM Virtual Hardware: Dualist Weights vs Traditional weights Unit Test ===\n");

    DualistNeuron dual_n = { 0.1f, 0.1f };
    StandardNeuron std_n = { 0.1f };

    // Asymmetric target function:
    // y = 3.0 * x for x >= 0 (incoming activation)
    // y = 0.5 * x for x < 0  (outgoing response)
    
    float training_inputs[10] = { 1.0f, -1.0f, 2.0f, -2.0f, 0.5f, -0.5f, 1.5f, -1.5f, 3.0f, -3.0f };
    float target_outputs[10] = { 3.0f, -0.5f, 6.0f, -1.0f, 1.5f, -0.25f, 4.5f, -0.75f, 9.0f, -1.5f };

    float lr = 0.1f;
    int epochs = 100;

    printf("[TRAINING] Training standard neuron (single weight) vs dualist neuron (incoming/outgoing weights)...\n");

    for (int epoch = 0; epoch < epochs; epoch++) {
        for (int i = 0; i < 10; i++) {
            float x = training_inputs[i];
            float target = target_outputs[i];

            // Train Standard Neuron
            float pred_std = evaluate_standard(&std_n, x);
            float error_std = target - pred_std;
            std_n.w += lr * error_std * x;

            // Train Dualist Neuron (Incoming / Outgoing updated independently based on input phase)
            float pred_dual = evaluate_dualist(&dual_n, x);
            float error_dual = target - pred_dual;
            if (x >= 0.0f) {
                dual_n.w_in += lr * error_dual * x;
            } else {
                dual_n.w_out += lr * error_dual * x;
            }
        }
    }

    // Evaluate final convergence errors
    printf("[RESULTS] Convergence evaluation:\n");
    printf("   Traditional Weight (Standard): w = %.4f\n", std_n.w);
    printf("   Dualist Weights (Incoming/Hypobar): w_in = %.4f, (Outgoing/Epibar): w_out = %.4f\n", dual_n.w_in, dual_n.w_out);

    float sum_err_std = 0.0f;
    float sum_err_dual = 0.0f;

    for (int i = 0; i < 10; i++) {
        float x = training_inputs[i];
        float target = target_outputs[i];

        float err_std = fabsf(target - evaluate_standard(&std_n, x));
        float err_dual = fabsf(target - evaluate_dualist(&dual_n, x));

        sum_err_std += err_std;
        sum_err_dual += err_dual;
    }

    printf("   Total Absolute Error (Traditional Standard): %.4f\n", sum_err_std);
    printf("   Total Absolute Error (Dualist Weights): %.4f\n", sum_err_dual);

    // Verify that the dualist weight model solved the asymmetric problem while standard weight failed
    assert(sum_err_dual < 0.001f); // Dual weights should resolve with zero error
    assert(sum_err_std > 1.0f);    // Single weight must fail to resolve the dual-target phases

    printf("[SUCCESS] Dualist weights successfully solved the asymmetric target function unit test.\n");
    return 0;
}
