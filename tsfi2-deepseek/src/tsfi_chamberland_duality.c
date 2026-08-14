#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi_chamberland_duality.h"

tsfi_chamberland_accumulator_t* tsfi_chamberland_accumulator_create(void) {
    tsfi_chamberland_accumulator_t *acc = (tsfi_chamberland_accumulator_t *)calloc(1, sizeof(tsfi_chamberland_accumulator_t));
    if (!acc) return NULL;
    acc->epibar_accumulator = 0.0f;
    acc->hypobar_accumulator = 0.0f;
    acc->dual_eigenvalue = 1.0f;
    acc->selection_pattern = CHAMBERLAND_PATTERN_BIJECTION_DUAL;
    acc->transaction_count = 0;
    return acc;
}

bool tsfi_chamberland_accumulator_update(
    tsfi_chamberland_accumulator_t *acc,
    const float *x,
    int dim,
    float epibar_delta,
    float hypobar_delta
) {
    if (!acc || !x || dim <= 0) return false;

    // Non-preferential additive accumulation into Epibar (\bar{\epsilon}) and Hypobar (\underline{\epsilon})
    float x_boundary_norm = fabsf(x[0]) + fabsf(x[dim - 1]);
    float x_interior_norm = 0.0f;
    for (int i = 1; i < dim - 1; i += 2) x_interior_norm += fabsf(x[i]);
    x_interior_norm /= (float)(dim / 2);

    acc->epibar_accumulator += epibar_delta + x_boundary_norm * 0.10f;
    acc->hypobar_accumulator += hypobar_delta + x_interior_norm * 0.10f;
    acc->transaction_count++;

    // Compute Marc Chamberland Duality Eigenvalue \lambda_{dual}
    float sum_acc = acc->epibar_accumulator + acc->hypobar_accumulator + 1e-6f;
    acc->dual_eigenvalue = (2.0f * acc->epibar_accumulator * acc->hypobar_accumulator) / (sum_acc * sum_acc);

    // Classify Tomographic Bijection Selection Pattern
    float diff = acc->epibar_accumulator - acc->hypobar_accumulator;
    if (fabsf(diff) < 0.15f * sum_acc) {
        acc->selection_pattern = CHAMBERLAND_PATTERN_BIJECTION_DUAL;
    } else if (diff > 0.0f) {
        acc->selection_pattern = CHAMBERLAND_PATTERN_HYPERTOMOGRAPHY;
    } else {
        acc->selection_pattern = CHAMBERLAND_PATTERN_HYPOTOMOGRAPHY;
    }

    return true;
}

bool tsfi_chamberland_eval_duality_bijection(
    const tsfi_chamberland_accumulator_t *acc,
    float *duality_metric_out,
    uint32_t *pattern_out
) {
    if (!acc) return false;
    if (duality_metric_out) *duality_metric_out = acc->dual_eigenvalue;
    if (pattern_out) *pattern_out = acc->selection_pattern;
    return (acc->dual_eigenvalue > 0.01f);
}

float tsfi_chamberland_eval_epibar_23tree_functional(const float *x, int dim) {
    if (!x || dim <= 0) return 0.0f;
    // 2-3 Tree Linear Functional Domain: Evaluates multi-way functional bounds across vector
    float func_val = 0.0f;
    for (int i = 0; i < dim; i += 3) {
        float node_val = x[i];
        if (i + 1 < dim) node_val += x[i + 1] * 0.5f;
        if (i + 2 < dim) node_val += x[i + 2] * 0.25f;
        func_val += fabsf(node_val);
    }
    return func_val / (float)(dim / 3 + 1);
}

float tsfi_chamberland_eval_hypobar_rb_classification(const float *cand_logits, int count, float top_score) {
    if (!cand_logits || count <= 0) return 0.0f;
    // Red-Black Tree Classification Domain: Evaluates logit classification variance
    float logit_sum = 0.0f;
    for (int i = 0; i < count; i++) logit_sum += cand_logits[i];
    float mean = logit_sum / (float)count;
    return fabsf(top_score - mean);
}

void tsfi_chamberland_accumulator_destroy(tsfi_chamberland_accumulator_t *acc) {
    if (acc) free(acc);
}
