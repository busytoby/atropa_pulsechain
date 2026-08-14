#ifndef TSFI_CODE_REVIEW_H
#define TSFI_CODE_REVIEW_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Code Review Classification Verdict (Watanabe et al., June 2024)
typedef enum {
    REVIEW_VERDICT_ACCEPT                 = 0x41434350, // "ACCP" Accepted by developer rules
    REVIEW_VERDICT_REJECT_NO_BENEFIT      = 0x524A424E, // "RJBN" Rejected: No tangible benefit
    REVIEW_VERDICT_REJECT_BUG_FOUND       = 0x524A4247, // "RJBG" Rejected: Bug detected
    REVIEW_VERDICT_REJECT_STYLE_MISMATCH  = 0x524A5354  // "RJST" Rejected: Style mismatch
} tsfi_review_verdict_t;

// Code Review Audit State Struct
typedef struct {
    tsfi_review_verdict_t verdict;
    float benefit_ratio;        // \mathcal{B} / \mathcal{E}: Structural benefit score
    float soundness_score;      // \mathcal{S}_{bug}: Code soundness score (1.0 = bug-free)
    float style_alignment;      // \mathcal{A}_{style}: Project rule alignment
    float acceptance_score;     // \mathcal{R}_{accept}: Composite developer acceptance score
} tsfi_code_review_state_t;

// Audits a generated C code stub against Watanabe et al. Developer Acceptance Criteria
bool tsfi_code_review_audit_stub(
    const char *code_stub,
    tsfi_code_review_state_t *review_out
);

// Classifies developer prompt intent (Outsourcing vs. Reference)
bool tsfi_code_review_classify_intent(
    const char *prompt,
    bool *is_outsourcing_intent_out
);

// Auto-corrects syntax or style defects in generated code stub
bool tsfi_code_review_rewrite_correction(
    char *code_stub,
    size_t max_len,
    const tsfi_code_review_state_t *review_state
);

// Yangxiao Cai et al. (September 2025) LLM Open-Source Project Issue Classification
typedef enum {
    LLM_ISSUE_TYPE_MODEL_CONVERGENCE_DRIFT  = 0x4D434452, // "MCDR" Loss spikes, precision underflow, divergence
    LLM_ISSUE_TYPE_DEPENDENCY_COMPATIBILITY = 0x44455043, // "DEPC" CUDA/driver/kernel runtime mismatches
    LLM_ISSUE_TYPE_RESOURCE_OOM_CONTENTION  = 0x4F4F4D43, // "OOMC" VRAM fragmentation, context length blowout
    LLM_ISSUE_TYPE_DATA_PIPELINE_CORRUPTION = 0x44504950, // "DPIP" Tokenizer alignment, corrupted binary shards
    LLM_ISSUE_TYPE_API_SPEC_DESYNC          = 0x41504953  // "APIS" Prompt template & grammar mismatch
} tsfi_llm_issue_type_t;

typedef struct {
    tsfi_llm_issue_type_t root_cause_type;
    float failure_probability;   // Likelihood of project runtime fault
    float mitigation_confidence; // Resolution confidence using recommended solution
    char recommended_solution[128]; // Prescribed architectural solution
} tsfi_cai_issue_diagnosis_t;

bool tsfi_cai_diagnose_project_issues(
    const float *activation_tensor,
    int dim,
    float memory_pressure,
    tsfi_cai_issue_diagnosis_t *diagnosis_out
);

// Yangxiao Cai et al. (September 2025) 3-Dimensional LLM Issue Triage Framework: Symptom x Cause x Solution
typedef enum {
    CAI_SYMPTOM_CRASH_HANG               = 0x53594348, // "SYCH" Execution crash or thread deadlock
    CAI_SYMPTOM_DEGRADED_ACCURACY        = 0x53594441, // "SYDA" Perplexity degradation / gibberish outputs
    CAI_SYMPTOM_PERFORMANCE_SLOWDOWN     = 0x53595053, // "SYPS" Latency budget breach (>500ms)
    CAI_SYMPTOM_BUILD_PACKAGING_FAIL     = 0x53594246  // "SYBF" Compilation / linking failure
} tsfi_cai_symptom_t;

typedef enum {
    CAI_SOLUTION_KERNEL_FUSION           = 0x534C4B46, // "SLKF" FlashAttention / SwiGLU fused kernels
    CAI_SOLUTION_DYNAMIC_QUANTIZATION    = 0x534C4451, // "SLDQ" Q4_K block dequantization fallback
    CAI_SOLUTION_GRAMMAR_CONSTRAINED_GEN = 0x534C4743, // "SLGC" Red-Black tree grammar pruning
    CAI_SOLUTION_BINARY_WAL_RECOVERY     = 0x534C4257  // "SLBW" Rule 13 binary WAL ledger replay
} tsfi_cai_solution_strategy_t;

typedef struct {
    tsfi_cai_symptom_t observed_symptom;
    tsfi_llm_issue_type_t root_cause;
    tsfi_cai_solution_strategy_t strategy;
    float time_to_mitigate_ms;
    bool auto_remediated;
} tsfi_cai_triaged_issue_t;

bool tsfi_cai_triage_runtime_defect(
    double latency_ms,
    float perplexity_delta,
    float memory_pressure,
    tsfi_cai_triaged_issue_t *triage_out
);

bool tsfi_cai_apply_auto_remediation(
    const tsfi_cai_triaged_issue_t *triage,
    float *activation_tensor,
    int dim
);

// Yangxiao Cai et al. (September 2025) Issue Resolution Lifecycle Struct
typedef struct {
    uint32_t total_detected_defects;
    uint32_t successfully_localized;
    uint32_t patches_verified;
    float resolution_efficiency_ratio; // (patches_verified / total_detected)
    float regression_guard_margin;     // Distance from failure threshold
} tsfi_cai_lifecycle_state_t;

bool tsfi_cai_eval_resolution_lifecycle(
    const tsfi_cai_triaged_issue_t *triage,
    float post_patch_perplexity,
    double post_patch_latency_ms,
    tsfi_cai_lifecycle_state_t *lifecycle_out
);

#endif // TSFI_CODE_REVIEW_H
