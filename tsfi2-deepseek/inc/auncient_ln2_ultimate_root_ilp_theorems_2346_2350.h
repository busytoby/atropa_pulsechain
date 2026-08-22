#ifndef AUNCIENT_LN2_ULTIMATE_ROOT_ILP_THEOREMS_2346_2350_H
#define AUNCIENT_LN2_ULTIMATE_ROOT_ILP_THEOREMS_2346_2350_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LN2_Q39_TARGET 0x00000058B90BFC0ULL /* ln(2) ~ 0.693147180559945 in 40-bit Q39 */
#define LN2_ROOT_CANARY_GUARD 0x4C4E32524F4F5436ULL /* "LN2ROOT6" */

typedef struct {
    uint32_t step_id;
    int64_t q39_taylor_term;
    int64_t q39_accumulated_sum;
    int64_t q39_residual_to_ln2;
    bool is_term_exact;
} Ln2RootRecurrenceStep;

typedef struct {
    uint64_t head_guard;
    Ln2RootRecurrenceStep steps[64];
    uint64_t ln2_zmm_latch[64];
    uint64_t tail_guard;
    uint32_t total_ln2_evaluations;
    uint32_t exact_q39_terms_summed;
    uint32_t cdc6600_60bit_ln2_words;
    uint32_t overflow_trapped_evaluations;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_ln2_ultimate_root_exact;
    bool is_ln2_root_memory_safe;
} Ln2UltimateRootContext;

typedef struct {
    float in_silicon_ln2_fidelity;
    float ln2_strategy_datbin_merkle_ratio;
    float ln2_convergence_latency_ns;
    uint64_t verified_ln2_saat_clearances;

    bool ln2_pipeline_verified;
    bool ln2_strategy_merkle_verified;
    bool ln2_submicro_latency_verified;
    bool ln2_lossless_saat_verified;
    bool sovereign_2350_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} Ln2RootBeyond2345State;

int cpm_tomie_ln2_ultimate_root_init(Ln2UltimateRootContext *ctx);
int cpm_tomie_ln2_ultimate_root_step(Ln2UltimateRootContext *ctx, uint32_t num_terms);
bool cpm_tomie_ln2_ultimate_root_assert_safety(Ln2UltimateRootContext *ctx);

void auncient_ln2_ultimate_root_init(Ln2RootBeyond2345State *state);
bool auncient_ln2_ultimate_root_verify_theorems_2346_2350(Ln2RootBeyond2345State *state);
uint32_t auncient_ln2_ultimate_root_compute_rule18(const Ln2RootBeyond2345State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_LN2_ULTIMATE_ROOT_ILP_THEOREMS_2346_2350_H */
