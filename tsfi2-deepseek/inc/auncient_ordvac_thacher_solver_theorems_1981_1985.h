#ifndef AUNCIENT_ORDVAC_THACHER_SOLVER_THEOREMS_1981_1985_H
#define AUNCIENT_ORDVAC_THACHER_SOLVER_THEOREMS_1981_1985_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint64_t convergent_a; /* Numerator convergent modulo MotzkinPrime */
    uint64_t convergent_b; /* Denominator convergent modulo MotzkinPrime */
    uint32_t terms_evaluated;
    bool is_converged;
} OrdvacThacherSolverContext;

typedef struct {
    float in_silicon_ordvac_thacher_fidelity;
    float ordvac_thacher_strategy_datbin_merkle_ratio;
    float ordvac_thacher_eval_latency_ns;
    uint64_t verified_ordvac_thacher_saat_clearances;

    bool ordvac_thacher_solver_verified;
    bool ordvac_thacher_strategy_merkle_verified;
    bool ordvac_thacher_submicro_latency_verified;
    bool ordvac_thacher_lossless_saat_verified;
    bool grand_1985_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} OrdvacThacherBeyond1980State;

int cpm_tomie_ordvac_thacher_init(OrdvacThacherSolverContext *ctx);
int cpm_tomie_ordvac_thacher_evaluate(OrdvacThacherSolverContext *ctx, const uint64_t *a_coeffs, const uint64_t *b_coeffs, size_t order);
void auncient_ordvac_thacher_init(OrdvacThacherBeyond1980State *state);
bool auncient_ordvac_thacher_verify_theorems_1981_1985(OrdvacThacherBeyond1980State *state);
uint32_t auncient_ordvac_thacher_compute_rule18(const OrdvacThacherBeyond1980State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_ORDVAC_THACHER_SOLVER_THEOREMS_1981_1985_H */
