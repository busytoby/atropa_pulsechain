#ifndef AUNCIENT_RENDERMAN_DEEP_SHADOW_MAP_THEOREMS_2376_2380_H
#define AUNCIENT_RENDERMAN_DEEP_SHADOW_MAP_THEOREMS_2376_2380_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEEP_SHADOW_LAYERS 8
#define DEEP_SHADOW_SAMPLES_PER_LAYER 32
#define DEEP_SHADOW_CANARY_GUARD 0x4445455053484144ULL /* "DEEPSHAD" */

typedef struct {
    uint32_t layer_id;
    float depth_z;
    float transmittance;
    float absorption_density;
    bool is_layer_monotonic;
} DeepShadowSampleLayer;

typedef struct {
    uint64_t head_guard;
    DeepShadowSampleLayer layers[DEEP_SHADOW_LAYERS];
    uint64_t shadow_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_shadow_integrations_evaluated;
    uint32_t volumetric_attenuations_computed;
    uint32_t cdc6600_60bit_shadow_words;
    uint32_t overflow_trapped_shadow_lookups;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_deep_shadow_transmittance_monotonic;
    bool is_deep_shadow_map_memory_safe;
} RendermanDeepShadowMapContext;

typedef struct {
    float in_silicon_shadow_fidelity;
    float shadow_strategy_datbin_merkle_ratio;
    float shadow_eval_latency_ns;
    uint64_t verified_shadow_saat_clearances;

    bool shadow_map_pipeline_verified;
    bool shadow_strategy_merkle_verified;
    bool shadow_submicro_latency_verified;
    bool shadow_lossless_saat_verified;
    bool sovereign_2380_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanDeepShadowBeyond2375State;

int cpm_tomie_renderman_deep_shadow_init(RendermanDeepShadowMapContext *ctx);
int cpm_tomie_renderman_deep_shadow_sample(RendermanDeepShadowMapContext *ctx, uint32_t layer_idx, float z, float absorption);
bool cpm_tomie_renderman_deep_shadow_assert_safety(RendermanDeepShadowMapContext *ctx);

void auncient_renderman_deep_shadow_init(RendermanDeepShadowBeyond2375State *state);
bool auncient_renderman_deep_shadow_verify_theorems_2376_2380(RendermanDeepShadowBeyond2375State *state);
uint32_t auncient_renderman_deep_shadow_compute_rule18(const RendermanDeepShadowBeyond2375State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_DEEP_SHADOW_MAP_THEOREMS_2376_2380_H */
