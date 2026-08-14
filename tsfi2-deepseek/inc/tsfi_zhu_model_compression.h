#ifndef TSFI_ZHU_MODEL_COMPRESSION_H
#define TSFI_ZHU_MODEL_COMPRESSION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "tsfi_totient_qing_tomography.h"

// Xunyu Zhu et al. (2023) Model Compression State Struct
typedef struct {
    float pruning_ratio;               // Percentage of structured pruned activation channels
    float outlier_ratio;               // Percentage of high-magnitude activation outliers (|x_i| > 3\sigma)
    float student_distillation_loss;   // Dynamic logit distillation KL-divergence metric
    size_t memory_bytes_reclaimed;     // RAM bytes saved via structured pruning
} tsfi_zhu_compression_state_t;

// Evaluates Xunyu Zhu et al. Outlier-Aware Isolation & Structured Pruning over activation vector x
bool tsfi_zhu_eval_outlier_pruning(
    float *x,
    int dim,
    float outlier_threshold_sigma,
    tsfi_zhu_compression_state_t *compression_out
);

// Evaluates Dynamic Logit Distillation & Student Calibration over candidate logit scores
bool tsfi_zhu_eval_student_distillation(
    const float *teacher_logits,
    const float *student_logits,
    int count,
    float *distillation_loss_out
);

// Yi Chen et al. (Cambricon-C) Efficient 4-Bit Matrix Unit via Primitivization API
typedef struct {
    float primitive_ops_throughput_gflops; // Effective GFLOPS from primitive 4-bit matrix units
    float quantization_efficiency;        // Efficiency of primitivized INT4 execution
    float energy_saving_ratio;             // Hardware energy reduction ratio via primitive decomposition
    float primitive_decomposition_gain;    // Speedup gain from decomposed 4-bit ALU primitives
    float reconfigurable_grid_utilization;// Dynamic utilization ratio of 4-bit ALU primitive grid
    float bit_slice_pipeline_latency_ns;  // Pipeline latency for bit-sliced 4-bit accumulation
    float systolic_array_efficiency;       // Systolic primitivized MAC array efficiency
    float non_uniform_quant_scale;         // Non-uniform 4-bit quantization scale factor
    float dynamic_precision_scale;        // Dynamic 4-bit/8-bit precision scaling factor
    float mixed_precision_accumulator_loss;// Accuracy loss metric for mixed-precision accumulation
    bool  primitivization_active;          // True if Cambricon-C 4-bit primitives are enabled
} tsfi_cambricon_c_state_t;

bool tsfi_cambricon_c_eval_primitive_matmul(
    const float *x,
    const float *w_q4,
    float *out,
    int dim,
    tsfi_cambricon_c_state_t *cambricon_out
);

bool tsfi_cambricon_c_decompose_primitives(
    const float *matrix_a,
    const float *matrix_b,
    float *matrix_out,
    int rows,
    int cols,
    tsfi_cambricon_c_state_t *cambricon_out
);

bool tsfi_cambricon_c_eval_bit_slice_pipeline(
    const float *x,
    int dim,
    uint8_t bit_slice_width,
    tsfi_cambricon_c_state_t *cambricon_out
);

bool tsfi_cambricon_c_eval_systolic_primitives(
    const float *x,
    int dim,
    int array_dim,
    tsfi_cambricon_c_state_t *cambricon_out
);

bool tsfi_cambricon_c_eval_mixed_precision_accumulator(
    const float *x,
    int dim,
    float precision_target,
    tsfi_cambricon_c_state_t *cambricon_out
);

// Mohamed S Abdelfattah et al. (DAC 2020) AutoML Hardware Co-Design Engine Struct
typedef struct {
    float optimal_accelerator_area_mm2;   // Co-designed hardware accelerator area
    float optimal_clock_frequency_mhz;   // Co-designed peak clock frequency
    float Pareto_efficiency_score;        // Pareto-optimal energy-accuracy trade-off score
    float latency_reduction_ratio;        // Co-designed latency reduction vs baseline
} tsfi_abdelfattah_automl_codesign_t;

bool tsfi_abdelfattah_eval_automl_codesign(
    const float *x,
    int dim,
    float target_latency_ms,
    tsfi_abdelfattah_automl_codesign_t *codesign_out
);

// Kazuki Egashira et al. (2024) GGUF Quantization Gap Defensive Audit Struct
typedef struct {
    float quantization_gap_error_norm;   // Deviation metric between FP16 reference and GGUF INT4 logits
    float outlier_perturbation_score;     // Quantization gap vulnerability perturbation score
    bool  quantization_gap_sanitized;     // True if quantization gap perturbations are mitigated
} tsfi_gguf_quantization_gap_defense_t;

bool tsfi_egashira_eval_gguf_quantization_gap_defense(
    const float *logits,
    int vocab_size,
    float gap_threshold,
    tsfi_gguf_quantization_gap_defense_t *defense_out
);

#endif // TSFI_ZHU_MODEL_COMPRESSION_H
