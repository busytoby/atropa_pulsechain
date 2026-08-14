#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi_loraprune_deepseek.h"

bool tsfi_loraprune_eval_importance(
    const float *matrix_a,
    const float *matrix_b,
    int dim,
    int rank_r,
    tsfi_loraprune_state_t *loraprune_out
) {
    if (!matrix_a || !matrix_b || dim <= 0 || rank_r <= 0 || !loraprune_out) return false;

    float scale = 16.0f / (float)rank_r;
    float norm_a = 0.0f;
    float norm_b = 0.0f;

    for (int i = 0; i < rank_r * dim; i++) {
        norm_a += matrix_a[i] * matrix_a[i];
        norm_b += matrix_b[i] * matrix_b[i];
    }
    norm_a = sqrtf(norm_a / (float)(rank_r * dim));
    norm_b = sqrtf(norm_b / (float)(rank_r * dim));

    // LoRA-Guided Importance Metric: I_{LoRA} = ||B||_F * ||A||_F * scale
    float importance = norm_a * norm_b * scale;

    loraprune_out->rank_r = rank_r;
    loraprune_out->lora_alpha = 16.0f;
    loraprune_out->lora_importance_score = importance;
    loraprune_out->channel_pruning_ratio = (importance < 0.50f) ? 0.50f : 0.0f;
    loraprune_out->adapter_merged = false;

    return true;
}

bool tsfi_loraprune_merge_weights(
    float *base_weight,
    const float *matrix_a,
    const float *matrix_b,
    int dim,
    int rank_r,
    float lora_alpha
) {
    if (!base_weight || !matrix_a || !matrix_b || dim <= 0 || rank_r <= 0) return false;

    float scale = (lora_alpha > 0.0f) ? (lora_alpha / (float)rank_r) : (16.0f / (float)rank_r);

    // W_{merged} = W_0 + scale * (B * A)
    for (int i = 0; i < dim; i++) {
        float delta = 0.0f;
        for (int r = 0; r < rank_r; r++) {
            delta += matrix_b[i * rank_r + r] * matrix_a[r * dim + i];
        }
        base_weight[i] += delta * scale;
    }

    return true;
}
