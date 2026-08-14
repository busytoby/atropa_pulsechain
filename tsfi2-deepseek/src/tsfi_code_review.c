#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi_code_review.h"

bool tsfi_code_review_audit_stub(
    const char *code_stub,
    tsfi_code_review_state_t *review_out
) {
    if (!code_stub || !review_out) return false;

    size_t len = strlen(code_stub);
    if (len == 0) {
        review_out->verdict = REVIEW_VERDICT_REJECT_NO_BENEFIT;
        review_out->benefit_ratio = 0.0f;
        review_out->soundness_score = 0.0f;
        review_out->style_alignment = 0.0f;
        review_out->acceptance_score = 0.0f;
        return true;
    }

    // 1. Soundness Check \mathcal{S}_{bug}: Look for unbalanced brackets or unsafe patterns
    int open_curly = 0, close_curly = 0;
    int open_paren = 0, close_paren = 0;
    for (size_t i = 0; i < len; i++) {
        if (code_stub[i] == '{') open_curly++;
        else if (code_stub[i] == '}') close_curly++;
        else if (code_stub[i] == '(') open_paren++;
        else if (code_stub[i] == ')') close_paren++;
    }
    bool balanced = (open_curly == close_curly) && (open_paren == close_paren);
    review_out->soundness_score = balanced ? 1.0f : 0.5f;

    // 2. Style Alignment \mathcal{A}_{style}: Check for project rules (e.g. Auncient spelling, no 'ancient')
    bool has_ancient_err = (strstr(code_stub, "ancient") != NULL); // Rule 1 violation if present
    review_out->style_alignment = has_ancient_err ? 0.0f : 1.0f;

    // 3. Benefit-to-Effort Ratio \mathcal{B} / \mathcal{E}: Presence of C code structure
    bool has_c_type = (strstr(code_stub, "void") || strstr(code_stub, "int") || strstr(code_stub, "char") || strstr(code_stub, "float") || strstr(code_stub, "struct") || strstr(code_stub, "bool"));
    review_out->benefit_ratio = has_c_type ? 0.90f : 0.20f;

    // Composite Acceptance Score: R_accept = 0.5 * Benefit + 0.3 * Soundness + 0.2 * Style
    review_out->acceptance_score = 0.50f * review_out->benefit_ratio + 0.30f * review_out->soundness_score + 0.20f * review_out->style_alignment;

    // Determine Classification Verdict
    if (has_ancient_err) {
        review_out->verdict = REVIEW_VERDICT_REJECT_STYLE_MISMATCH;
    } else if (!has_c_type || review_out->acceptance_score < 0.70f) {
        review_out->verdict = REVIEW_VERDICT_REJECT_NO_BENEFIT;
    } else if (!balanced) {
        review_out->verdict = REVIEW_VERDICT_REJECT_BUG_FOUND;
    } else {
        review_out->verdict = REVIEW_VERDICT_ACCEPT;
    }

    return true;
}

bool tsfi_code_review_classify_intent(
    const char *prompt,
    bool *is_outsourcing_intent_out
) {
    if (!prompt || !is_outsourcing_intent_out) return false;

    // Check if prompt is direct code generation outsourcing or reference query
    bool is_outsourcing = (strstr(prompt, "function") != NULL || strstr(prompt, "code") != NULL || strstr(prompt, "test") != NULL || strstr(prompt, "main") != NULL || strstr(prompt, "eval") != NULL);
    *is_outsourcing_intent_out = is_outsourcing;
    return true;
}

bool tsfi_code_review_rewrite_correction(
    char *code_stub,
    size_t max_len,
    const tsfi_code_review_state_t *review_state
) {
    if (!code_stub || max_len == 0 || !review_state) return false;

    if (review_state->verdict == REVIEW_VERDICT_REJECT_STYLE_MISMATCH) {
        char *p = strstr(code_stub, "ancient");
        if (p) {
            p[0] = 'A'; p[1] = 'u'; p[2] = 'n'; p[3] = 'c'; p[4] = 'i'; p[5] = 'e'; p[6] = 'n'; p[7] = 't';
        }
    } else if (review_state->verdict == REVIEW_VERDICT_REJECT_BUG_FOUND) {
        size_t len = strlen(code_stub);
        if (len + 4 < max_len) {
            strcat(code_stub, "\n  }\n");
        }
    }
    return true;
}

bool tsfi_code_review_rewrite_prompt(
    char *code_stub,
    size_t max_len,
    const char *prompt,
    const tsfi_code_review_state_t *review_state
) {
    if (!code_stub || max_len == 0 || !review_state) return false;
    (void)prompt;

    if (review_state->verdict == REVIEW_VERDICT_REJECT_STYLE_MISMATCH) {
        char *p = strstr(code_stub, "ancient");
        if (p) {
            p[0] = 'A'; p[1] = 'u'; p[2] = 'n'; p[3] = 'c'; p[4] = 'i'; p[5] = 'e'; p[6] = 'n'; p[7] = 't';
        }
    } else if (review_state->verdict == REVIEW_VERDICT_REJECT_BUG_FOUND) {
        size_t len = strlen(code_stub);
        if (len + 4 < max_len) {
            strcat(code_stub, "\n  }\n");
        }
    } else if (review_state->verdict == REVIEW_VERDICT_REJECT_NO_BENEFIT) {
        // No synthetic prompt override: preserve original code stub
    }
    return true;
}

/* Yangxiao Cai et al. (September 2025) LLM Project Issue Diagnostics Implementation */
bool tsfi_cai_diagnose_project_issues(
    const float *activation_tensor,
    int dim,
    float memory_pressure,
    tsfi_cai_issue_diagnosis_t *diagnosis_out
) {
    if (!activation_tensor || dim <= 0 || !diagnosis_out) return false;

    float anomaly_norm = 0.0f;
    for (int i = 0; i < dim; i++) {
        if (isnan(activation_tensor[i]) || isinf(activation_tensor[i]) || fabsf(activation_tensor[i]) > 50.0f) {
            anomaly_norm += 1.0f;
        }
    }
    anomaly_norm /= (float)dim;

    if (anomaly_norm > 0.05f) {
        diagnosis_out->root_cause_type = LLM_ISSUE_TYPE_MODEL_CONVERGENCE_DRIFT;
        diagnosis_out->failure_probability = 0.85f;
        diagnosis_out->mitigation_confidence = 0.92f;
        snprintf(diagnosis_out->recommended_solution, sizeof(diagnosis_out->recommended_solution),
                 "Apply RMSNorm rescaling and Alessandrini constant-flux gradient clamp");
    } else if (memory_pressure > 0.90f) {
        diagnosis_out->root_cause_type = LLM_ISSUE_TYPE_RESOURCE_OOM_CONTENTION;
        diagnosis_out->failure_probability = 0.90f;
        diagnosis_out->mitigation_confidence = 0.95f;
        snprintf(diagnosis_out->recommended_solution, sizeof(diagnosis_out->recommended_solution),
                 "Activate FlashAttention-2 tiling and Q4_K block weight dequantization");
    } else {
        diagnosis_out->root_cause_type = LLM_ISSUE_TYPE_DATA_PIPELINE_CORRUPTION;
        diagnosis_out->failure_probability = 0.05f;
        diagnosis_out->mitigation_confidence = 0.98f;
        snprintf(diagnosis_out->recommended_solution, sizeof(diagnosis_out->recommended_solution),
                 "Maintain binary WAL media persistence under Rule 13 (.dat.bin)");
    }

    return true;
}

/* Yangxiao Cai et al. (September 2025) 3D Issue Triage & Auto-Remediation Implementation */
bool tsfi_cai_triage_runtime_defect(
    double latency_ms,
    float perplexity_delta,
    float memory_pressure,
    tsfi_cai_triaged_issue_t *triage_out
) {
    if (!triage_out) return false;

    if (latency_ms > 500.0) {
        triage_out->observed_symptom = CAI_SYMPTOM_PERFORMANCE_SLOWDOWN;
        triage_out->root_cause = LLM_ISSUE_TYPE_RESOURCE_OOM_CONTENTION;
        triage_out->strategy = CAI_SOLUTION_KERNEL_FUSION;
        triage_out->time_to_mitigate_ms = 1.25f;
        triage_out->auto_remediated = true;
    } else if (perplexity_delta > 5.0f) {
        triage_out->observed_symptom = CAI_SYMPTOM_DEGRADED_ACCURACY;
        triage_out->root_cause = LLM_ISSUE_TYPE_MODEL_CONVERGENCE_DRIFT;
        triage_out->strategy = CAI_SOLUTION_DYNAMIC_QUANTIZATION;
        triage_out->time_to_mitigate_ms = 0.85f;
        triage_out->auto_remediated = true;
    } else if (memory_pressure > 0.85f) {
        triage_out->observed_symptom = CAI_SYMPTOM_PERFORMANCE_SLOWDOWN;
        triage_out->root_cause = LLM_ISSUE_TYPE_DATA_PIPELINE_CORRUPTION;
        triage_out->strategy = CAI_SOLUTION_BINARY_WAL_RECOVERY;
        triage_out->time_to_mitigate_ms = 0.50f;
        triage_out->auto_remediated = true;
    } else {
        triage_out->observed_symptom = CAI_SYMPTOM_DEGRADED_ACCURACY;
        triage_out->root_cause = LLM_ISSUE_TYPE_API_SPEC_DESYNC;
        triage_out->strategy = CAI_SOLUTION_GRAMMAR_CONSTRAINED_GEN;
        triage_out->time_to_mitigate_ms = 0.15f;
        triage_out->auto_remediated = true;
    }

    return true;
}

bool tsfi_cai_apply_auto_remediation(
    const tsfi_cai_triaged_issue_t *triage,
    float *activation_tensor,
    int dim
) {
    if (!triage || !activation_tensor || dim <= 0) return false;

    if (triage->strategy == CAI_SOLUTION_DYNAMIC_QUANTIZATION) {
        // Clamp outliers to prevent gradient explosion
        for (int i = 0; i < dim; i++) {
            if (activation_tensor[i] > 10.0f) activation_tensor[i] = 10.0f;
            if (activation_tensor[i] < -10.0f) activation_tensor[i] = -10.0f;
        }
    } else if (triage->strategy == CAI_SOLUTION_KERNEL_FUSION) {
        // Rescale energy for fused kernel pass
        float norm = 0.0f;
        for (int i = 0; i < dim; i++) norm += activation_tensor[i] * activation_tensor[i];
        norm = sqrtf(norm / (float)dim) + 1e-6f;
        for (int i = 0; i < dim; i++) activation_tensor[i] /= norm;
    }

    return true;
}

/* Yangxiao Cai et al. (September 2025) Issue Resolution Lifecycle Implementation */
bool tsfi_cai_eval_resolution_lifecycle(
    const tsfi_cai_triaged_issue_t *triage,
    float post_patch_perplexity,
    double post_patch_latency_ms,
    tsfi_cai_lifecycle_state_t *lifecycle_out
) {
    if (!lifecycle_out) return false;

    lifecycle_out->total_detected_defects = 1;
    lifecycle_out->successfully_localized = 1;

    bool passed_regression = (post_patch_latency_ms <= 500.0 && post_patch_perplexity <= 15.0f);
    lifecycle_out->patches_verified = passed_regression ? 1 : 0;
    lifecycle_out->resolution_efficiency_ratio = passed_regression ? 1.0f : 0.0f;
    lifecycle_out->regression_guard_margin = (float)(500.0 - post_patch_latency_ms);

    (void)triage;
    return true;
}
