#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tsfi_loraprune_deepseek.h"

int main(void) {
    printf("================ TEST ZHANG ET AL. (ACL 2024) LORAPRUNE ================\n");

    int dim = 16;
    int rank_r = 4;
    float *matrix_a = (float *)calloc((size_t)(rank_r * dim), sizeof(float));
    float *matrix_b = (float *)calloc((size_t)(dim * rank_r), sizeof(float));
    float *base_w   = (float *)calloc((size_t)dim, sizeof(float));

    for (int i = 0; i < rank_r * dim; i++) matrix_a[i] = 0.10f;
    for (int i = 0; i < dim * rank_r; i++) matrix_b[i] = 0.20f;
    for (int i = 0; i < dim; i++) base_w[i] = 1.00f;

    tsfi_loraprune_state_t lora_st;
    bool ok_eval = tsfi_loraprune_eval_importance(matrix_a, matrix_b, dim, rank_r, &lora_st);
    assert(ok_eval);
    assert(lora_st.lora_importance_score > 0.0f);
    printf("  [PASS] LoRA-Guided Importance I_LoRA evaluated (Score = %.4f, Pruning Ratio = %.2f%%).\n", lora_st.lora_importance_score, lora_st.channel_pruning_ratio * 100.0f);

    bool ok_merge = tsfi_loraprune_merge_weights(base_w, matrix_a, matrix_b, dim, rank_r, 16.0f);
    assert(ok_merge);
    assert(base_w[0] > 1.00f);
    printf("  [PASS] Zero-Copy LoRA Weight Merge verified (W_merged[0] = %.4f).\n", base_w[0]);

    free(matrix_a); free(matrix_b); free(base_w);
    printf("========================================================================\n");
    return 0;
}
