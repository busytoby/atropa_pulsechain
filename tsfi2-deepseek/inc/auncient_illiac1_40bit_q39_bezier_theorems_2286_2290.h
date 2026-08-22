#ifndef AUNCIENT_ILLIAC1_40BIT_Q39_BEZIER_THEOREMS_2286_2290_H
#define AUNCIENT_ILLIAC1_40BIT_Q39_BEZIER_THEOREMS_2286_2290_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ILLIAC1_WILLIAMS_TUBE_WORDS 1024
#define ILLIAC1_CANARY_GUARD 0x494C4C4941433130ULL /* "ILLIAC10" */

typedef struct {
    int64_t q39_x; /* 40-bit fixed-point (1 sign bit, 39 fractional bits) */
    int64_t q39_y;
    int64_t q39_z;
    int64_t q39_nx;
    int64_t q39_ny;
    int64_t q39_nz;
    bool is_patch_boundary_continuous;
} Illiac1Q39Vertex;

typedef struct {
    uint64_t head_guard;
    Illiac1Q39Vertex williams_crt_grid[ILLIAC1_WILLIAMS_TUBE_WORDS];
    uint64_t tail_guard;
    uint32_t total_q39_evaluations_completed;
    uint32_t patch_boundary_seams_verified;
    uint32_t cdc6600_60bit_expanded_words;
    uint32_t overflow_trapped_q39_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_q39_precision_lossless;
    bool is_illiac1_memory_safe;
} Illiac1Q39Context;

typedef struct {
    float in_silicon_illiac1_fidelity;
    float illiac1_strategy_datbin_merkle_ratio;
    float illiac1_evaluation_latency_ns;
    uint64_t verified_illiac1_saat_clearances;

    bool illiac1_pipeline_verified;
    bool illiac1_strategy_merkle_verified;
    bool illiac1_submicro_latency_verified;
    bool illiac1_lossless_saat_verified;
    bool sovereign_2290_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} Illiac1Q39Beyond2285State;

int cpm_tomie_illiac1_init(Illiac1Q39Context *ctx);
int cpm_tomie_illiac1_eval_bezier_q39(Illiac1Q39Context *ctx, uint32_t word_idx, double u, double v);
bool cpm_tomie_illiac1_assert_safety(Illiac1Q39Context *ctx);

void auncient_illiac1_q39_init(Illiac1Q39Beyond2285State *state);
bool auncient_illiac1_q39_verify_theorems_2286_2290(Illiac1Q39Beyond2285State *state);
uint32_t auncient_illiac1_q39_compute_rule18(const Illiac1Q39Beyond2285State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_ILLIAC1_40BIT_Q39_BEZIER_THEOREMS_2286_2290_H */
