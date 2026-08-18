#ifndef AUNCIENT_FPGA_ACCUMULATOR_THEOREMS_391_395_H
#define AUNCIENT_FPGA_ACCUMULATOR_THEOREMS_391_395_H

#include "auncient_fpga_coaxial_tem_theorems_386_390.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    float    accumulator_redirection_fidelity;
    float    jubilee_charge_conservation_ratio;
    float    accumulator_continuity_margin;
    uint64_t verified_accumulator_saat_cycles;
    bool     accumulator_redirection_verified;   /* Theorem 391 */
    bool     jubilee_accumulation_verified;      /* Theorem 392 */
    bool     accumulator_continuity_verified;    /* Theorem 393 */
    bool     accumulator_lossless_saat_verified; /* Theorem 394 */
    bool     accumulator_grand_parity_verified;  /* Theorem 395 */
    uint32_t rule18_parity_checksum;
} FpgaAccumulatorState;

void auncient_fpga_accumulator_init(FpgaAccumulatorState *state);
bool auncient_fpga_accumulator_verify_theorems_391_395(FpgaAccumulatorState *state);
uint32_t auncient_fpga_accumulator_compute_rule18(const FpgaAccumulatorState *state);

#endif /* AUNCIENT_FPGA_ACCUMULATOR_THEOREMS_391_395_H */
