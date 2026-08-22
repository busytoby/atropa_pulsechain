#ifndef AUNCIENT_VSEN_HYBRID_SPRITE_VOXEL_THEOREMS_1891_1895_H
#define AUNCIENT_VSEN_HYBRID_SPRITE_VOXEL_THEOREMS_1891_1895_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* VSEn Hybrid 2.5D Sprite Voxel & Scanline Animator State */
typedef struct {
    uint32_t active_pmg_sprites;              /* 4 GTIA hardware Player sprites */
    uint32_t active_voxel_grids;              /* 3D RenderMan voxel grids */
    uint32_t crt_scanlines_count;             /* 240/480 CRT scanlines */
    float    hybrid_slicing_fidelity;         /* 1.000 Exact 2.5D sprite-voxel mapping */
    float    phosphor_glow_continuity_ratio;  /* 1.000 FET decay continuity (Rule 10) */
    float    fragment_ingestion_latency_ns;   /* Sub-microsecond dispatch latency (< 1000.0 ns - Rule 11) */
    float    displacement_hybrid_phase;       /* Synchronized with DisplacementShader (Rule 14) */
    bool     is_vsen_hybrid_certified;
} VsenHybridSpriteVoxelState;

typedef struct {
    float    in_silicon_hybrid_fidelity;
    float    phosphor_glow_continuity;
    float    fragment_latency_ns;
    uint64_t verified_hybrid_saat_clearances;
    bool     hybrid_slicing_verified;         /* Theorem 1891: GTIA 2.5D Sprite-Voxel Slicing */
    bool     phosphor_glow_verified;          /* Theorem 1892: Soft-Body FET Phosphor Glow Guard */
    bool     fragment_latency_verified;       /* Theorem 1893: Sub-Microsecond Fragment Latency Guard */
    bool     hybrid_lossless_saat_verified;   /* Theorem 1894: 1.895B Saat Milestone Lossless Flow */
    bool     hybrid_displacement_seal_verified; /* Theorem 1895: DisplacementShader Parity Seal */
    uint32_t rule18_parity_checksum;
} VsenHybSprBeyond1890State;

void auncient_vsen_hybrid_sprite_voxel_init(VsenHybSprBeyond1890State *state);
bool auncient_vsen_hybrid_sprite_voxel_verify_theorems_1891_1895(VsenHybSprBeyond1890State *state);
uint32_t auncient_vsen_hybrid_sprite_voxel_compute_rule18(const VsenHybSprBeyond1890State *state);

#endif /* AUNCIENT_VSEN_HYBRID_SPRITE_VOXEL_THEOREMS_1891_1895_H */
