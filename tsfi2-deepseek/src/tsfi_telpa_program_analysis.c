#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi_telpa_program_analysis.h"

#define MAX_COUNTER_EXAMPLES 64

static uint32_t g_counter_examples[MAX_COUNTER_EXAMPLES];
static float    g_counter_penalties[MAX_COUNTER_EXAMPLES];
static size_t   g_counter_count = 0;

static float compute_euler_totient(uint32_t n) {
    uint32_t result = n;
    uint32_t p = 2;
    uint32_t temp = n;
    while (p * p <= temp) {
        if (temp % p == 0) {
            while (temp % p == 0) temp /= p;
            result -= result / p;
        }
        p++;
    }
    if (temp > 1) result -= result / temp;
    return (float)result * 0.125f;
}

bool tsfi_telpa_eval_totient_qing(
    const float *x,
    int dim,
    uint32_t n_totient,
    tsfi_telpa_state_t *telpa_out
) {
    if (!x || dim <= 0 || !telpa_out) return false;

    float r_qing = compute_euler_totient(n_totient > 0 ? n_totient : 10);
    telpa_out->totient_qing_radius = r_qing;

    float norm = 0.0f;
    for (int i = 0; i < dim; i++) norm += x[i] * x[i];
    norm = sqrtf(norm / (float)dim) + 1e-6f;

    // Totient Qing predicate distance: d(p, x) = || ||x|| - r_{qing} ||
    telpa_out->branch_predicate_distance = fabsf(norm - r_qing);
    telpa_out->dependency_score = 1.0f / (1.0f + telpa_out->branch_predicate_distance);
    telpa_out->counter_example_count = g_counter_count;
    telpa_out->hard_branch_covered = (telpa_out->branch_predicate_distance < 0.25f);

    return true;
}

bool tsfi_telpa_eval_branch_distance(
    const float *x,
    int dim,
    float target_value,
    tsfi_telpa_state_t *telpa_out
) {
    if (!x || dim <= 0 || !telpa_out) return false;

    float mean = 0.0f;
    for (int i = 0; i < dim; i++) mean += fabsf(x[i]);
    mean /= (float)dim;

    // Predicate distance: d(p, x) = |mean - target|
    telpa_out->branch_predicate_distance = fabsf(mean - target_value);
    telpa_out->dependency_score = 1.0f / (1.0f + telpa_out->branch_predicate_distance);
    telpa_out->totient_qing_radius = 1.0f;
    telpa_out->counter_example_count = g_counter_count;
    telpa_out->hard_branch_covered = (telpa_out->branch_predicate_distance < 0.25f);

    return true;
}

bool tsfi_telpa_register_counter_example(
    uint32_t token_id,
    float penalty_weight
) {
    if (g_counter_count >= MAX_COUNTER_EXAMPLES) {
        g_counter_count = 0; // Ring buffer wrap-around
    }
    g_counter_examples[g_counter_count] = token_id;
    g_counter_penalties[g_counter_count] = penalty_weight;
    g_counter_count++;
    return true;
}

float tsfi_telpa_evaluate_candidate_bonus(
    uint32_t token_id,
    const tsfi_telpa_state_t *telpa_state
) {
    if (!telpa_state) return 0.0f;

    // Penalty for tokens registered in counter-example ring buffer
    float penalty = 0.0f;
    for (size_t i = 0; i < g_counter_count; i++) {
        if (g_counter_examples[i] == token_id) {
            penalty += g_counter_penalties[i];
        }
    }

    float dependency_bonus = telpa_state->dependency_score * 0.10f;
    return dependency_bonus - penalty;
}

bool tsfi_telpa_eval_ast_predicate_distance(
    const float *x,
    int dim,
    int ast_depth,
    tsfi_telpa_state_t *telpa_out
) {
    if (!x || dim <= 0 || !telpa_out) return false;

    float base_d = 0.0f;
    for (int i = 0; i < dim; i++) base_d += fabsf(x[i]);
    base_d /= (float)dim;

    // AST-Aware predicate distance: d_{AST} = d_{base} + 0.15 * ast_depth
    telpa_out->branch_predicate_distance = base_d + 0.15f * (float)ast_depth;
    telpa_out->dependency_score = 1.0f / (1.0f + telpa_out->branch_predicate_distance);
    telpa_out->totient_qing_radius = 0.50f;
    telpa_out->counter_example_count = g_counter_count;
    telpa_out->hard_branch_covered = (telpa_out->branch_predicate_distance < 0.25f);
    return true;
}

bool tsfi_telpa_decay_counter_examples(float decay_factor) {
    float factor = (decay_factor > 0.0f && decay_factor < 1.0f) ? decay_factor : 0.85f;
    for (size_t i = 0; i < g_counter_count; i++) {
        g_counter_penalties[i] *= factor;
    }
    return true;
}

float tsfi_telpa_eval_header_interop_bonus(const char *token_str) {
    if (!token_str) return 0.0f;
    if (strcmp(token_str, "tsfi_zorse_eval_gguf_pure_c") == 0 || strcmp(token_str, "tsfi_helmholtz_ring_create") == 0 || strcmp(token_str, "tsfi_chamberland_accumulator_create") == 0) {
        return 5000.0f;
    }
    return 0.0f;
}
