#ifndef AUNCIENT_VAESENN_AFFECTIVE_BRDF_TEAPOT_THEOREMS_2401_2405_H
#define AUNCIENT_VAESENN_AFFECTIVE_BRDF_TEAPOT_THEOREMS_2401_2405_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AFFECTIVE_BRDF_MICROPOLYGONS 64
#define AFFECTIVE_BRDF_CANARY_GUARD 0x5641454252444636ULL /* "VAEBRDF6" */

typedef struct {
    uint32_t micropolygon_id;
    float specular_roughness;
    float diffuse_albedo;
    float fresnel_f0;
    float brdf_reflected_radiance;
    bool is_brdf_reciprocal;
} AffectiveMicropolygonBrdf;

typedef struct {
    uint64_t head_guard;
    AffectiveMicropolygonBrdf micropolygons[AFFECTIVE_BRDF_MICROPOLYGONS];
    uint64_t brdf_eval_latch[64];
    uint64_t tail_guard;
    uint32_t total_brdf_evaluations_completed;
    uint32_t reciprocity_checks_passed;
    uint32_t cdc6600_60bit_brdf_words;
    uint32_t overflow_trapped_brdf_passes;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_helmholtz_reciprocity_exact;
    bool is_affective_brdf_memory_safe;
} VaesennAffectiveBrdfContext;

typedef struct {
    float in_silicon_brdf_fidelity;
    float brdf_strategy_datbin_merkle_ratio;
    float brdf_shading_latency_ns;
    uint64_t verified_brdf_saat_clearances;

    bool affective_brdf_pipeline_verified;
    bool brdf_strategy_merkle_verified;
    bool brdf_submicro_latency_verified;
    bool brdf_lossless_saat_verified;
    bool sovereign_2405_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} VaesennBrdfBeyond2400State;

int cpm_tomie_vaesenn_brdf_init(VaesennAffectiveBrdfContext *ctx);
int cpm_tomie_vaesenn_brdf_evaluate(VaesennAffectiveBrdfContext *ctx, uint32_t poly_idx, float valence, float arousal);
bool cpm_tomie_vaesenn_brdf_assert_safety(VaesennAffectiveBrdfContext *ctx);

void auncient_vaesenn_brdf_init(VaesennBrdfBeyond2400State *state);
bool auncient_vaesenn_brdf_verify_theorems_2401_2405(VaesennBrdfBeyond2400State *state);
uint32_t auncient_vaesenn_brdf_compute_rule18(const VaesennBrdfBeyond2400State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_VAESENN_AFFECTIVE_BRDF_TEAPOT_THEOREMS_2401_2405_H */
