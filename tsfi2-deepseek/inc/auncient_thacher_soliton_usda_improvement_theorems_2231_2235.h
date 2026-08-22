#ifndef AUNCIENT_THACHER_SOLITON_USDA_IMPROVEMENT_THEOREMS_2231_2235_H
#define AUNCIENT_THACHER_SOLITON_USDA_IMPROVEMENT_THEOREMS_2231_2235_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define THACHER_SOLITON_MAX_STAGES 16
#define THACHER_SOLITON_CANARY_GUARD 0x534F4C49544F4E36ULL /* "SOLITON6" */

typedef struct {
    double wave_amplitude;
    double phase_velocity;
    double dispersion_coeff;
    double usda_mesh_refinement_error;
    bool is_soliton_converged;
} ThacherSolitonState;

typedef struct {
    uint64_t head_guard;
    ThacherSolitonState stages[THACHER_SOLITON_MAX_STAGES];
    uint64_t tail_guard;
    uint32_t active_improvement_runs;
    uint32_t usda_vertices_optimized;
    uint32_t cdc6600_60bit_recurrence_words;
    uint32_t overflow_trapped_runs;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_run_over_run_monotonic;
    bool is_thacher_soliton_memory_safe;
} ThacherSolitonUsdaContext;

typedef struct {
    float in_silicon_soliton_fidelity;
    float soliton_strategy_datbin_merkle_ratio;
    float soliton_step_latency_ns;
    uint64_t verified_soliton_saat_clearances;

    bool thacher_soliton_pipeline_verified;
    bool soliton_strategy_merkle_verified;
    bool soliton_submicro_latency_verified;
    bool soliton_lossless_saat_verified;
    bool sovereign_2235_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} ThacherSolitonBeyond2230State;

int cpm_tomie_thacher_soliton_init(ThacherSolitonUsdaContext *ctx);
int cpm_tomie_thacher_soliton_step_improvement(ThacherSolitonUsdaContext *ctx, uint32_t run_idx, double initial_error);
bool cpm_tomie_thacher_soliton_assert_safety(ThacherSolitonUsdaContext *ctx);

void auncient_thacher_soliton_usda_init(ThacherSolitonBeyond2230State *state);
bool auncient_thacher_soliton_usda_verify_theorems_2231_2235(ThacherSolitonBeyond2230State *state);
uint32_t auncient_thacher_soliton_usda_compute_rule18(const ThacherSolitonBeyond2230State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_THACHER_SOLITON_USDA_IMPROVEMENT_THEOREMS_2231_2235_H */
