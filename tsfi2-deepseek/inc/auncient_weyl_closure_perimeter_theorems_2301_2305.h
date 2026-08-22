#ifndef AUNCIENT_WEYL_CLOSURE_PERIMETER_THEOREMS_2301_2305_H
#define AUNCIENT_WEYL_CLOSURE_PERIMETER_THEOREMS_2301_2305_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WEYL_CHAMBER_ROOTS 8
#define WEYL_CLOSURE_CANARY_GUARD 0x5745594C434C4F53ULL /* "WEYLCLOS" */

typedef struct {
    uint32_t root_id;
    int64_t root_vector_x; /* 40-bit Q39 fixed-point */
    int64_t root_vector_y;
    int64_t root_vector_z;
    float reflection_phase_rad;
    bool is_root_orthogonal;
} WeylChamberRoot;

typedef struct {
    uint64_t head_guard;
    WeylChamberRoot roots[WEYL_CHAMBER_ROOTS];
    uint64_t perimeter_reflection_latch[64];
    uint64_t tail_guard;
    uint32_t total_weyl_reflections_evaluated;
    uint32_t chamber_invariants_bounded;
    uint32_t cdc6600_60bit_weyl_words;
    uint32_t overflow_trapped_reflections;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_weyl_group_closure_symmetric;
    bool is_weyl_closure_memory_safe;
} WeylClosureContext;

typedef struct {
    float in_silicon_weyl_fidelity;
    float weyl_strategy_datbin_merkle_ratio;
    float weyl_reflection_latency_ns;
    uint64_t verified_weyl_saat_clearances;

    bool weyl_closure_pipeline_verified;
    bool weyl_strategy_merkle_verified;
    bool weyl_submicro_latency_verified;
    bool weyl_lossless_saat_verified;
    bool sovereign_2305_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} WeylClosureBeyond2300State;

int cpm_tomie_weyl_closure_init(WeylClosureContext *ctx);
int cpm_tomie_weyl_reflect_root(WeylClosureContext *ctx, uint32_t root_idx, float angle_rad);
bool cpm_tomie_weyl_closure_assert_safety(WeylClosureContext *ctx);

void auncient_weyl_closure_init(WeylClosureBeyond2300State *state);
bool auncient_weyl_closure_verify_theorems_2301_2305(WeylClosureBeyond2300State *state);
uint32_t auncient_weyl_closure_compute_rule18(const WeylClosureBeyond2300State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_WEYL_CLOSURE_PERIMETER_THEOREMS_2301_2305_H */
