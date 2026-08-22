#ifndef AUNCIENT_RENDERMAN_MICROFACET_ANISOTROPY_THEOREMS_2416_2420_H
#define AUNCIENT_RENDERMAN_MICROFACET_ANISOTROPY_THEOREMS_2416_2420_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ANISOTROPY_TANGENT_PATCHES 64
#define ANISOTROPY_CANARY_GUARD 0x414E49534F545036ULL /* "ANISOTP6" */

typedef struct {
    uint32_t patch_id;
    float tangent_u_roughness;
    float bitangent_v_roughness;
    float anisotropy_ratio;
    float microfacet_specular_peak;
    bool is_patch_energy_bounded;
} AnisotropicMicrofacetPatch;

typedef struct {
    uint64_t head_guard;
    AnisotropicMicrofacetPatch patches[ANISOTROPY_TANGENT_PATCHES];
    uint64_t anisotropy_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_anisotropic_evaluations;
    uint32_t tangent_frames_oriented;
    uint32_t cdc6600_60bit_anisotropy_words;
    uint32_t overflow_trapped_evaluations;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_anisotropy_reciprocal_and_bounded;
    bool is_microfacet_anisotropy_memory_safe;
} RendermanMicrofacetAnisotropyContext;

typedef struct {
    float in_silicon_anisotropy_fidelity;
    float anisotropy_strategy_datbin_merkle_ratio;
    float anisotropy_eval_latency_ns;
    uint64_t verified_anisotropy_saat_clearances;

    bool anisotropy_pipeline_verified;
    bool anisotropy_strategy_merkle_verified;
    bool anisotropy_submicro_latency_verified;
    bool anisotropy_lossless_saat_verified;
    bool sovereign_2420_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanAnisotropyBeyond2415State;

int cpm_tomie_renderman_anisotropy_init(RendermanMicrofacetAnisotropyContext *ctx);
int cpm_tomie_renderman_anisotropy_evaluate(RendermanMicrofacetAnisotropyContext *ctx, uint32_t patch_idx, float r_u, float r_v);
bool cpm_tomie_renderman_anisotropy_assert_safety(RendermanMicrofacetAnisotropyContext *ctx);

void auncient_renderman_anisotropy_init(RendermanAnisotropyBeyond2415State *state);
bool auncient_renderman_anisotropy_verify_theorems_2416_2420(RendermanAnisotropyBeyond2415State *state);
uint32_t auncient_renderman_anisotropy_compute_rule18(const RendermanAnisotropyBeyond2415State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_MICROFACET_ANISOTROPY_THEOREMS_2416_2420_H */
