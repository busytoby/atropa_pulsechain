#ifndef AUNCIENT_COCKPIT_THACHER_PARAMETRIC_EVALUATOR_THEOREMS_2391_2395_H
#define AUNCIENT_COCKPIT_THACHER_PARAMETRIC_EVALUATOR_THEOREMS_2391_2395_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define THACHER_CYLINDER_SCALES 8
#define THACHER_SLIDE_RULE_CANARY_GUARD 0x5448414348455236ULL /* "THACHER6" */

typedef struct {
    uint32_t scale_id;
    float rotary_angle_radians;
    float slide_linear_offset;
    float log_scaled_result;
    bool is_scale_calibrated;
} ThacherParametricScale;

typedef struct {
    uint64_t head_guard;
    ThacherParametricScale scales[THACHER_CYLINDER_SCALES];
    uint64_t thacher_rotary_latch[64];
    uint64_t tail_guard;
    uint32_t total_parametric_sweeps_evaluated;
    uint32_t slide_rule_products_computed;
    uint32_t cdc6600_60bit_thacher_words;
    uint32_t overflow_trapped_sweeps;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_thacher_logarithmic_precision_exact;
    bool is_cockpit_thacher_memory_safe;
} CockpitThacherEvaluatorContext;

typedef struct {
    float in_silicon_thacher_fidelity;
    float thacher_strategy_datbin_merkle_ratio;
    float thacher_eval_latency_ns;
    uint64_t verified_thacher_saat_clearances;

    bool thacher_evaluator_pipeline_verified;
    bool thacher_strategy_merkle_verified;
    bool thacher_submicro_latency_verified;
    bool thacher_lossless_saat_verified;
    bool sovereign_2395_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} CockpitThacherBeyond2390State;

int cpm_tomie_cockpit_thacher_init(CockpitThacherEvaluatorContext *ctx);
int cpm_tomie_cockpit_thacher_sweep(CockpitThacherEvaluatorContext *ctx, uint32_t scale_idx, float theta, float offset);
bool cpm_tomie_cockpit_thacher_assert_safety(CockpitThacherEvaluatorContext *ctx);

void auncient_cockpit_thacher_init(CockpitThacherBeyond2390State *state);
bool auncient_cockpit_thacher_verify_theorems_2391_2395(CockpitThacherBeyond2390State *state);
uint32_t auncient_cockpit_thacher_compute_rule18(const CockpitThacherBeyond2390State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_COCKPIT_THACHER_PARAMETRIC_EVALUATOR_THEOREMS_2391_2395_H */
