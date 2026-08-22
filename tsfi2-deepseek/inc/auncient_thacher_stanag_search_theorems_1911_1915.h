#ifndef AUNCIENT_THACHER_STANAG_SEARCH_THEOREMS_1911_1915_H
#define AUNCIENT_THACHER_STANAG_SEARCH_THEOREMS_1911_1915_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float x_coord;
    float p_prev;
    float p_curr;
    float p_next;
    float q_prev;
    float q_curr;
    float q_next;
    float convergent_val;
    bool is_thacher_converged;
} ThacherRecurrenceState;

typedef struct {
    float in_silicon_thacher_fidelity;
    float thacher_strategy_datbin_merkle_ratio;
    float thacher_latency_ns;
    uint64_t verified_thacher_saat_clearances;

    bool thacher_3term_recurrence_verified;
    bool thacher_strategy_merkle_verified;
    bool thacher_submicro_latency_verified;
    bool thacher_lossless_saat_verified;
    bool grand_1915_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} ThacherStanagBeyond1910State;

void auncient_thacher_stanag_search_init(ThacherStanagBeyond1910State *state);
bool auncient_thacher_stanag_search_verify_theorems_1911_1915(ThacherStanagBeyond1910State *state);
uint32_t auncient_thacher_stanag_search_compute_rule18(const ThacherStanagBeyond1910State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_THACHER_STANAG_SEARCH_THEOREMS_1911_1915_H */
