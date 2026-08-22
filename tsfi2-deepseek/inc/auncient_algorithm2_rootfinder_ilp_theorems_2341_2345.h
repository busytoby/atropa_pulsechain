#ifndef AUNCIENT_ALGORITHM2_ROOTFINDER_ILP_THEOREMS_2341_2345_H
#define AUNCIENT_ALGORITHM2_ROOTFINDER_ILP_THEOREMS_2341_2345_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ROOTFINDER_MAX_DEGREE 8
#define ROOTFINDER_CANARY_GUARD 0x524F4F54414C4732ULL /* "ROOTALG2" */

typedef struct {
    uint32_t root_id;
    double root_real;
    double root_imag;
    double residual_norm;
    uint32_t ilp_iterations_taken;
    bool is_root_converged;
} Algorithm2RootResult;

typedef struct {
    uint64_t head_guard;
    double polynomial_coeffs[ROOTFINDER_MAX_DEGREE + 1];
    Algorithm2RootResult roots[ROOTFINDER_MAX_DEGREE];
    uint64_t rootfinder_zmm_latch[64];
    uint64_t tail_guard;
    uint32_t total_rootfinding_runs;
    uint32_t isolated_subdivisions;
    uint32_t cdc6600_60bit_root_words;
    uint32_t overflow_trapped_root_requests;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_rootfinder_unconditionally_convergent;
    bool is_rootfinder_memory_safe;
} Algorithm2RootfinderContext;

typedef struct {
    float in_silicon_rootfinder_fidelity;
    float rootfinder_strategy_datbin_merkle_ratio;
    float rootfinder_convergence_latency_ns;
    uint64_t verified_rootfinder_saat_clearances;

    bool rootfinder_pipeline_verified;
    bool rootfinder_strategy_merkle_verified;
    bool rootfinder_submicro_latency_verified;
    bool rootfinder_lossless_saat_verified;
    bool sovereign_2345_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} Algorithm2RootfinderBeyond2340State;

int cpm_tomie_algorithm2_rootfinder_init(Algorithm2RootfinderContext *ctx);
int cpm_tomie_algorithm2_rootfinder_solve(Algorithm2RootfinderContext *ctx, const double *coeffs, uint32_t degree);
bool cpm_tomie_algorithm2_rootfinder_assert_safety(Algorithm2RootfinderContext *ctx);

void auncient_algorithm2_rootfinder_init(Algorithm2RootfinderBeyond2340State *state);
bool auncient_algorithm2_rootfinder_verify_theorems_2341_2345(Algorithm2RootfinderBeyond2340State *state);
uint32_t auncient_algorithm2_rootfinder_compute_rule18(const Algorithm2RootfinderBeyond2340State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_ALGORITHM2_ROOTFINDER_ILP_THEOREMS_2341_2345_H */
