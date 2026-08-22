#ifndef AUNCIENT_RENDERMAN_SSS_PORCELAIN_TEAPOT_THEOREMS_2411_2415_H
#define AUNCIENT_RENDERMAN_SSS_PORCELAIN_TEAPOT_THEOREMS_2411_2415_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SSS_DIFFUSION_OCTAVES 8
#define SSS_RADIAL_SAMPLES 32
#define SSS_PORCELAIN_CANARY_GUARD 0x504F524353535336ULL /* "PORCSSS6" */

typedef struct {
    uint32_t octave_id;
    float mean_free_path_mm;
    float diffusion_weight;
    float radial_irradiance_lumens;
    bool is_octave_normalized;
} PorcelainSssDiffusionOctave;

typedef struct {
    uint64_t head_guard;
    PorcelainSssDiffusionOctave octaves[SSS_DIFFUSION_OCTAVES];
    uint64_t sss_radiance_latch[64];
    uint64_t tail_guard;
    uint32_t total_sss_integrations_completed;
    uint32_t radial_diffusion_profiles_computed;
    uint32_t cdc6600_60bit_sss_words;
    uint32_t overflow_trapped_integrations;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_sss_energy_conserved;
    bool is_sss_porcelain_memory_safe;
} RendermanSssPorcelainContext;

typedef struct {
    float in_silicon_sss_fidelity;
    float sss_strategy_datbin_merkle_ratio;
    float sss_diffusion_latency_ns;
    uint64_t verified_sss_saat_clearances;

    bool sss_porcelain_pipeline_verified;
    bool sss_strategy_merkle_verified;
    bool sss_submicro_latency_verified;
    bool sss_lossless_saat_verified;
    bool sovereign_2415_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanSssBeyond2410State;

int cpm_tomie_renderman_sss_init(RendermanSssPorcelainContext *ctx);
int cpm_tomie_renderman_sss_evaluate_profile(RendermanSssPorcelainContext *ctx, uint32_t octave_idx, float mfp, float weight);
bool cpm_tomie_renderman_sss_assert_safety(RendermanSssPorcelainContext *ctx);

void auncient_renderman_sss_init(RendermanSssBeyond2410State *state);
bool auncient_renderman_sss_verify_theorems_2411_2415(RendermanSssBeyond2410State *state);
uint32_t auncient_renderman_sss_compute_rule18(const RendermanSssBeyond2410State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_SSS_PORCELAIN_TEAPOT_THEOREMS_2411_2415_H */
