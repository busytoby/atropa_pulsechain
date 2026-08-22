#ifndef AUNCIENT_UIUC_ORDVAC_MATH_THEOREMS_1976_1980_H
#define AUNCIENT_UIUC_ORDVAC_MATH_THEOREMS_1976_1980_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint64_t accumulator;
    uint64_t quotient_register;
    uint64_t memory_words[1024]; /* 1K 40-bit words stored in 64-bit unsigned words */
    uint32_t program_counter;
    bool is_arithmetic_overflow;
} UiucOrdvacMathContext;

typedef struct {
    float in_silicon_ordvac_fidelity;
    float ordvac_strategy_datbin_merkle_ratio;
    float ordvac_arithmetic_latency_ns;
    uint64_t verified_ordvac_saat_clearances;

    bool ordvac_40bit_alu_verified;
    bool ordvac_strategy_merkle_verified;
    bool ordvac_submicro_latency_verified;
    bool ordvac_lossless_saat_verified;
    bool grand_1980_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} UiucOrdvacBeyond1975State;

int cpm_tomie_uiuc_ordvac_init(UiucOrdvacMathContext *ctx);
int cpm_tomie_uiuc_ordvac_multiply_step(UiucOrdvacMathContext *ctx, uint64_t multiplier, uint64_t multiplicand);
void auncient_uiuc_ordvac_init(UiucOrdvacBeyond1975State *state);
bool auncient_uiuc_ordvac_verify_theorems_1976_1980(UiucOrdvacBeyond1975State *state);
uint32_t auncient_uiuc_ordvac_compute_rule18(const UiucOrdvacBeyond1975State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_UIUC_ORDVAC_MATH_THEOREMS_1976_1980_H */
