#ifndef AUNCIENT_FPGA_HEXACONTAPHASE_THEOREMS_321_325_H
#define AUNCIENT_FPGA_HEXACONTAPHASE_THEOREMS_321_325_H

#include "auncient_fpga_octacontaphase_theorems_316_320.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define HEXACONTATETRAPHASE_CHANNELS 64

typedef struct {
    uint32_t active_yi_stator_phases;
    float    yi_64_orthogonality_flux_coherence;
    float    hyper_dimensional_torque_ripple_db;
    uint64_t verified_yi_hexagram_commutations;
    bool     yi_64phase_stator_manifold_verified;/* Theorem 321 */
    bool     yi_hexagram_spatial_orthogonality_verified;/* Theorem 322 */
    bool     hyper_torque_zero_ripple_verified;  /* Theorem 323 */
    bool     yi_canonical_lossless_saat_verified;/* Theorem 324 */
    bool     hexaconta_grand_parity_verified;    /* Theorem 325 */
    uint32_t rule18_parity_checksum;
} FpgaHexacontaphaseState;

void auncient_fpga_hexaconta_init(FpgaHexacontaphaseState *state);
bool auncient_fpga_hexaconta_verify_theorems_321_325(FpgaHexacontaphaseState *state);
uint32_t auncient_fpga_hexaconta_compute_rule18(const FpgaHexacontaphaseState *state);

#endif /* AUNCIENT_FPGA_HEXACONTAPHASE_THEOREMS_321_325_H */
