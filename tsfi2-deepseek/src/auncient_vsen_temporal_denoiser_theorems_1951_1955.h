#ifndef AUNCIENT_VSEN_TEMPORAL_DENOISER_THEOREMS_1951_1955_H
#define AUNCIENT_VSEN_TEMPORAL_DENOISER_THEOREMS_1951_1955_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* VSEn Temporal Reprojected Path-Tracing Denoiser Animator State */
typedef struct {
    uint32_t active_reprojection_history_buffers; /* Multi-frame history accumulation buffers */
    uint32_t active_wavelet_filter_scales;       /* 22 discrete EDO-harmonic wavelet scales */
    float    motion_vector_slicing_fidelity;     /* 1.000 Exact reprojection mapping */
    float    wavelet_energy_conservation_ratio;  /* 1.000 Radiant flux conservation == 1.0 */
    float    disocclusion_relaxation_ratio;      /* 1.000 FET temporal decay continuity (Rule 10) */
    float    denoiser_dispatch_latency_ns;       /* Sub-microsecond dispatch latency (< 1000.0 ns - Rule 11) */
    float    displacement_denoiser_phase;        /* Synchronized with DisplacementShader (Rule 14) */
    bool     is_vsen_denoiser_certified;
} VsenTemporalDenoiserState;

typedef struct {
    float    in_silicon_reproject_fidelity;
    float    wavelet_energy_ratio;
    float    disocclusion_relaxation;
    float    denoiser_latency_ns;
    uint64_t verified_denoiser_saat_clearances;
    bool     motion_vector_verified;          /* Theorem 1951: Temporal Reprojection Invariance */
    bool     wavelet_energy_verified;         /* Theorem 1952: EDO-22 Wavelet Energy Guard */
    bool     disocclusion_verified;           /* Theorem 1953: Soft-Body FET Disocclusion Guard */
    bool     denoiser_lossless_saat_verified; /* Theorem 1954: 1.955B Saat Milestone Lossless Flow */
    bool     denoiser_displacement_seal_verified; /* Theorem 1955: DisplacementShader Parity Seal */
    uint32_t rule18_parity_checksum;
} VsenDenoiseBeyond1950State;

void auncient_vsen_temporal_denoiser_init(VsenDenoiseBeyond1950State *state);
bool auncient_vsen_temporal_denoiser_verify_theorems_1951_1955(VsenDenoiseBeyond1950State *state);
uint32_t auncient_vsen_temporal_denoiser_compute_rule18(const VsenDenoiseBeyond1950State *state);

#endif /* AUNCIENT_VSEN_TEMPORAL_DENOISER_THEOREMS_1951_1955_H */
