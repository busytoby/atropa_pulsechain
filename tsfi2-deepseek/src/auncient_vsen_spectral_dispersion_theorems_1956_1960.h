#ifndef AUNCIENT_VSEN_SPECTRAL_DISPERSION_THEOREMS_1956_1960_H
#define AUNCIENT_VSEN_SPECTRAL_DISPERSION_THEOREMS_1956_1960_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* VSEn Spectral Dispersion & Corneal Iridescence Animator State */
typedef struct {
    uint32_t active_spectral_interference_bands; /* 22 discrete Airy wave interference bands */
    uint32_t active_sellmeier_prism_rays;       /* Chromatic dispersion rays */
    float    thin_film_slicing_fidelity;        /* 1.000 Exact Airy wave mapping */
    float    dispersion_energy_conservation_ratio; /* 1.000 Optical flux conservation <= 1.0 */
    float    tear_film_relaxation_ratio;        /* 1.000 FET wetness decay continuity (Rule 10) */
    float    dispersion_dispatch_latency_ns;    /* Sub-microsecond dispatch latency (< 1000.0 ns - Rule 11) */
    float    displacement_dispersion_phase;     /* Synchronized with DisplacementShader (Rule 14) */
    bool     is_vsen_dispersion_certified;
} VsenSpectralDispersionState;

typedef struct {
    float    in_silicon_thin_film_fidelity;
    float    dispersion_energy_ratio;
    float    tear_film_relaxation;
    float    dispersion_latency_ns;
    uint64_t verified_dispersion_saat_clearances;
    bool     thin_film_verified;              /* Theorem 1956: 22-Tone EDO Thin-Film Invariance */
    bool     dispersion_energy_verified;      /* Theorem 1957: Sellmeier Dispersion Energy Guard */
    bool     tear_film_verified;              /* Theorem 1958: Soft-Body FET Tear Film Guard */
    bool     dispersion_lossless_saat_verified; /* Theorem 1959: 1.960B Saat Milestone Lossless Flow */
    bool     dispersion_displacement_seal_verified; /* Theorem 1960: DisplacementShader Parity Seal */
    uint32_t rule18_parity_checksum;
} VsenSpcDispBeyond1955State;

void auncient_vsen_spectral_dispersion_init(VsenSpcDispBeyond1955State *state);
bool auncient_vsen_spectral_dispersion_verify_theorems_1956_1960(VsenSpcDispBeyond1955State *state);
uint32_t auncient_vsen_spectral_dispersion_compute_rule18(const VsenSpcDispBeyond1955State *state);

#endif /* AUNCIENT_VSEN_SPECTRAL_DISPERSION_THEOREMS_1956_1960_H */
