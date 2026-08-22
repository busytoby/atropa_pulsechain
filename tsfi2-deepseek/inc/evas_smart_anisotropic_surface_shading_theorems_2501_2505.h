#ifndef EVAS_SMART_ANISOTROPIC_SURFACE_SHADING_THEOREMS_2501_2505_H
#define EVAS_SMART_ANISOTROPIC_SURFACE_SHADING_THEOREMS_2501_2505_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EVAS_SMART_SURFACE_PATCHES 32
#define EVAS_SMART_SHADING_CANARY_GUARD 0x4556415353484436ULL /* "EVASSHD6" */

typedef struct {
    uint32_t patch_index;
    float tangent_u[3];
    float bitangent_v[3];
    float surface_normal[3];
    float roughness_u;
    float roughness_v;
    float specular_reflectance;
    bool is_tangent_frame_orthonormal;
} EvasSmartSurfacePatchTangentFrame;

typedef struct {
    uint64_t head_guard;
    EvasSmartSurfacePatchTangentFrame frames[EVAS_SMART_SURFACE_PATCHES];
    uint64_t evas_shading_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_surface_patches_shaded;
    uint32_t brdf_microfacet_evaluations;
    uint32_t cdc6600_60bit_shading_words;
    uint32_t overflow_trapped_shading_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_anisotropic_brdf_energy_conserved;
    bool is_evas_smart_shading_memory_safe;
} EvasSmartAnisotropicShadingContext;

typedef struct {
    float in_silicon_anisotropic_fidelity;
    float anisotropic_strategy_datbin_merkle_ratio;
    float anisotropic_eval_latency_ns;
    uint64_t verified_shading_saat_clearances;

    bool evas_anisotropic_shading_pipeline_verified;
    bool anisotropic_strategy_merkle_verified;
    bool anisotropic_submicro_latency_verified;
    bool shading_lossless_saat_verified;
    bool anisotropic_tangent_orthonormal_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartAnisotropicBeyond2500State;

int evas_smart_anisotropic_shading_init(EvasSmartAnisotropicShadingContext *ctx);
int evas_smart_anisotropic_shading_set_tangent_frame(EvasSmartAnisotropicShadingContext *ctx, uint32_t patch_idx, const float tu[3], const float bv[3], const float n[3], float rough_u, float rough_v);
int evas_smart_anisotropic_shading_evaluate_brdf(EvasSmartAnisotropicShadingContext *ctx, uint32_t patch_idx, const float view_dir[3], const float light_dir[3], float *out_brdf);
bool evas_smart_anisotropic_shading_assert_safety(EvasSmartAnisotropicShadingContext *ctx);

void evas_smart_anisotropic_beyond2500_init(EvasSmartAnisotropicBeyond2500State *state);
bool evas_smart_anisotropic_beyond2500_verify_theorems_2501_2505(EvasSmartAnisotropicBeyond2500State *state);
uint32_t evas_smart_anisotropic_beyond2500_compute_rule18(const EvasSmartAnisotropicBeyond2500State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_ANISOTROPIC_SURFACE_SHADING_THEOREMS_2501_2505_H */
