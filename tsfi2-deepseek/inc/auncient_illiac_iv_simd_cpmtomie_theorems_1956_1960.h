#ifndef AUNCIENT_ILLIAC_IV_SIMD_CPMTOMIE_THEOREMS_1956_1960_H
#define AUNCIENT_ILLIAC_IV_SIMD_CPMTOMIE_THEOREMS_1956_1960_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ILLIAC_PE_COUNT 64

typedef struct {
    uint32_t pe_registers[ILLIAC_PE_COUNT];
    uint32_t recurrence_alpha[ILLIAC_PE_COUNT];
    uint32_t recurrence_beta[ILLIAC_PE_COUNT];
    uint32_t recurrence_gamma[ILLIAC_PE_COUNT];
    uint32_t active_pe_mask;
    uint32_t cycles_executed;
    bool is_simd_synchronized;
} IlliacIvSimdContext;

typedef struct {
    float in_silicon_illiac_fidelity;
    float illiac_strategy_datbin_merkle_ratio;
    float illiac_simd_latency_ns;
    uint64_t verified_illiac_saat_clearances;

    bool illiac_64pe_simd_verified;
    bool illiac_strategy_merkle_verified;
    bool illiac_submicro_latency_verified;
    bool illiac_lossless_saat_verified;
    bool grand_1960_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} IlliacIvBeyond1955State;

int cpm_tomie_illiac_init(IlliacIvSimdContext *ctx);
int cpm_tomie_illiac_execute_3term_step(IlliacIvSimdContext *ctx);
void auncient_illiac_iv_init(IlliacIvBeyond1955State *state);
bool auncient_illiac_iv_verify_theorems_1956_1960(IlliacIvBeyond1955State *state);
uint32_t auncient_illiac_iv_compute_rule18(const IlliacIvBeyond1955State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_ILLIAC_IV_SIMD_CPMTOMIE_THEOREMS_1956_1960_H */
