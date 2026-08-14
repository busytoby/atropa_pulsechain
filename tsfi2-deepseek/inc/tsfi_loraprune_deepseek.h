#ifndef TSFI_LORAPRUNE_DEEPSEEK_H
#define TSFI_LORAPRUNE_DEEPSEEK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "tsfi_totient_qing_tomography.h"

// Zhang et al. (2023/2024) LoRAPrune State Struct
typedef struct {
    int   rank_r;                     // Low-rank adapter dimension r (8)
    float lora_alpha;                 // Scaling factor \alpha (16.0f)
    float lora_importance_score;      // \mathbf{I}_{LoRA}: Low-rank guided importance metric
    float channel_pruning_ratio;      // Percentage of pruned attention heads
    bool  adapter_merged;             // True if low-rank weights were merged into base tensor
} tsfi_loraprune_state_t;

// Evaluates LoRA-Guided Parameter Importance over low-rank matrices A and B
bool tsfi_loraprune_eval_importance(
    const float *matrix_a,
    const float *matrix_b,
    int dim,
    int rank_r,
    tsfi_loraprune_state_t *loraprune_out
);

// Merges low-rank adapter weights into base tensor W_0 zero-copy
bool tsfi_loraprune_merge_weights(
    float *base_weight,
    const float *matrix_a,
    const float *matrix_b,
    int dim,
    int rank_r,
    float lora_alpha
);

#endif // TSFI_LORAPRUNE_DEEPSEEK_H
