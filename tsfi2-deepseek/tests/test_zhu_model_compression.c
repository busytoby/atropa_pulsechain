#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_zhu_model_compression.h"

int main(void) {
    printf("================ TEST XUNYU ZHU ET AL. (2023) MODEL COMPRESSION ================\n");

    float x_sample[16];
    for (int i = 0; i < 16; i++) {
        x_sample[i] = (i == 7) ? 10.0f : (float)i * 0.01f;
    }

    tsfi_zhu_compression_state_t comp;
    bool ok_prune = tsfi_zhu_eval_outlier_pruning(x_sample, 16, 3.0f, &comp);
    assert(ok_prune);
    assert(comp.outlier_ratio > 0.0f);
    printf("  [PASS] Outlier Isolation & Structured Pruning verified (Outlier Ratio = %.2f%%, Reclaimed = %zu bytes).\n", comp.outlier_ratio * 100.0f, comp.memory_bytes_reclaimed);

    float teacher[8] = {1.0f, 2.0f, 0.5f, 0.2f, 0.1f, 0.0f, 0.3f, 0.4f};
    float student[8] = {0.9f, 2.1f, 0.4f, 0.25f, 0.12f, 0.05f, 0.28f, 0.38f};
    float kl_loss = 0.0f;
    bool ok_distill = tsfi_zhu_eval_student_distillation(teacher, student, 8, &kl_loss);
    assert(ok_distill);
    printf("  [PASS] Student Logit Distillation KL-Loss verified (KL-Loss = %.4f).\n", kl_loss);

    printf("=================================================================================\n");
    return 0;
}
