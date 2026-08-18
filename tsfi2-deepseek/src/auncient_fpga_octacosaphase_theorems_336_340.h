#ifndef AUNCIENT_FPGA_OCTACOSAPHASE_THEOREMS_336_340_H
#define AUNCIENT_FPGA_OCTACOSAPHASE_THEOREMS_336_340_H

#include "auncient_fpga_fourier_stator_theorems_331_335.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define OCTACOSAPHASE_CHANNELS 256

typedef struct {
    uint32_t active_hyper_fourier_phases;
    float    octacosa_spectral_flux_density;
    float    coaxial_cross_chatter_rejection_db;
    uint64_t verified_octacosa_clearances;
    bool     octacosaphase_256_basis_verified;    /* Theorem 336 */
    bool     spectral_flux_density_verified;      /* Theorem 337 */
    bool     cross_chatter_rejection_verified;    /* Theorem 338 */
    bool     octacosa_lossless_saat_verified;     /* Theorem 339 */
    bool     octacosaphase_grand_parity_verified; /* Theorem 340 */
    uint32_t rule18_parity_checksum;
} FpgaOctacosaphaseState;

void auncient_fpga_octacosa_init(FpgaOctacosaphaseState *state);
bool auncient_fpga_octacosa_verify_theorems_336_340(FpgaOctacosaphaseState *state);
uint32_t auncient_fpga_octacosa_compute_rule18(const FpgaOctacosaphaseState *state);

#endif /* AUNCIENT_FPGA_OCTACOSAPHASE_THEOREMS_336_340_H */
