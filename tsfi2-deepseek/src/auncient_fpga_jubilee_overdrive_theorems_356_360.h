#ifndef AUNCIENT_FPGA_JUBILEE_OVERDRIVE_THEOREMS_356_360_H
#define AUNCIENT_FPGA_JUBILEE_OVERDRIVE_THEOREMS_356_360_H

#include "auncient_fpga_optic_resonator_theorems_351_355.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    float    word_coupling_rail_limit_volts;
    float    tanh_saturation_compression_ratio;
    float    germanium_diode_clamp_voltage;
    float    winchestermq_256byte_alignment_slack;
    uint64_t verified_jubilee_accumulations;
    bool     jubilee_analog_overdrive_verified;   /* Theorem 356 */
    bool     tanh_softknee_compression_verified;  /* Theorem 357 */
    bool     germanium_clamp_invariance_verified; /* Theorem 358 */
    bool     wmq_256byte_displacement_verified;   /* Theorem 359 */
    bool     jubilee_grand_master_parity_verified;/* Theorem 360 */
    uint32_t rule18_parity_checksum;
} FpgaJubileeOverdriveState;

void auncient_fpga_jubilee_init(FpgaJubileeOverdriveState *state);
bool auncient_fpga_jubilee_verify_theorems_356_360(FpgaJubileeOverdriveState *state);
uint32_t auncient_fpga_jubilee_compute_rule18(const FpgaJubileeOverdriveState *state);

#endif /* AUNCIENT_FPGA_JUBILEE_OVERDRIVE_THEOREMS_356_360_H */
