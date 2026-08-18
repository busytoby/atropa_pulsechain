#ifndef AUNCIENT_FPGA_FOURIER_STATOR_THEOREMS_331_335_H
#define AUNCIENT_FPGA_FOURIER_STATOR_THEOREMS_331_335_H

#include "auncient_fpga_polyphase_torus_theorems_326_330.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define FOURIER_128PHASE_HARMONIC_BINS 128

typedef struct {
    uint32_t active_fourier_harmonic_bins;
    float    banach_hilbert_isometry_energy_ratio;
    float    fourier_gated_qfactor;
    uint64_t verified_fourier_pdl_clearances;
    bool     fourier_128phase_basis_verified;      /* Theorem 331 */
    bool     banach_hilbert_isometry_verified;     /* Theorem 332 */
    bool     fourier_gated_pdl_clearance_verified; /* Theorem 333 */
    bool     fourier_lossless_saat_verified;       /* Theorem 334 */
    bool     fourier_stator_grand_parity_verified; /* Theorem 335 */
    uint32_t rule18_parity_checksum;
} FpgaFourierStatorState;

void auncient_fpga_fourier_stator_init(FpgaFourierStatorState *state);
bool auncient_fpga_fourier_stator_verify_theorems_331_335(FpgaFourierStatorState *state);
uint32_t auncient_fpga_fourier_stator_compute_rule18(const FpgaFourierStatorState *state);

#endif /* AUNCIENT_FPGA_FOURIER_STATOR_THEOREMS_331_335_H */
