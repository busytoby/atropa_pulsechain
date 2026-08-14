#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_chamberland_duality.h"

int main(void) {
    printf("================ TEST MARC CHAMBERLAND TOMOGRAPHIC DUALITY BIJECTION ================\n");

    tsfi_chamberland_accumulator_t *acc = tsfi_chamberland_accumulator_create();
    assert(acc != NULL);
    assert(acc->selection_pattern == CHAMBERLAND_PATTERN_BIJECTION_DUAL);

    float x_sample[16];
    for (int i = 0; i < 16; i++) x_sample[i] = (float)(i + 1) * 0.25f;

    // Test Epibar (\bar{\epsilon}) Hypertomography Accumulation
    bool u1 = tsfi_chamberland_accumulator_update(acc, x_sample, 16, 5.0f, 0.5f);
    assert(u1);
    assert(acc->selection_pattern == CHAMBERLAND_PATTERN_HYPERTOMOGRAPHY);
    printf("  [PASS] Epibar Hypertomography Accumulation verified (Pattern = 0x%08X).\n", acc->selection_pattern);

    // Test Hypobar (\underline{\epsilon}) Hypotomography Accumulation
    bool u2 = tsfi_chamberland_accumulator_update(acc, x_sample, 16, 0.5f, 10.0f);
    assert(u2);
    assert(acc->selection_pattern == CHAMBERLAND_PATTERN_HYPOTOMOGRAPHY);
    printf("  [PASS] Hypobar Hypotomography Accumulation verified (Pattern = 0x%08X).\n", acc->selection_pattern);

    // Test Bijective Equilibrium Dual State
    bool u3 = tsfi_chamberland_accumulator_update(acc, x_sample, 16, 4.5f, 0.0f);
    assert(u3);
    assert(acc->selection_pattern == CHAMBERLAND_PATTERN_BIJECTION_DUAL);
    printf("  [PASS] Bijective Equilibrium Dual State verified (Pattern = 0x%08X).\n", acc->selection_pattern);

    float duality_metric = 0.0f;
    uint32_t pattern = 0;
    bool ok_eval = tsfi_chamberland_eval_duality_bijection(acc, &duality_metric, &pattern);
    assert(ok_eval);
    assert(pattern == CHAMBERLAND_PATTERN_BIJECTION_DUAL);
    printf("  [PASS] Marc Chamberland Duality Metric verified (lambda_dual = %.4f).\n", duality_metric);

    // Test Epibar 2-3 Tree Linear Functional Domain Evaluation
    float epibar_func = tsfi_chamberland_eval_epibar_23tree_functional(x_sample, 16);
    assert(epibar_func > 0.0f);
    printf("  [PASS] Epibar 2-3 Tree Linear Functional Domain verified (val = %.4f).\n", epibar_func);

    // Test Hypobar Red-Black Tree Classification Domain Evaluation
    float cands[8] = {1.0f, 2.5f, 0.5f, 4.0f, 3.2f, 1.8f, 2.1f, 3.5f};
    float hypobar_class = tsfi_chamberland_eval_hypobar_rb_classification(cands, 8, 4.0f);
    assert(hypobar_class > 0.0f);
    printf("  [PASS] Hypobar Red-Black Tree Classification Domain verified (val = %.4f).\n", hypobar_class);

    tsfi_chamberland_accumulator_destroy(acc);
    printf("====================================================================================\n");
    return 0;
}
