#ifndef AUNCIENT_RENDERMAN_DEEP_SHADOW_MAP_THEOREMS_2471_2475_H
#define AUNCIENT_RENDERMAN_DEEP_SHADOW_MAP_THEOREMS_2471_2475_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEEP_SHADOW_DEPTH_LAYERS 8
#define DEEP_SHADOW_PIXEL_CHANNELS 64
#define DEEP_SHADOW_CANARY_GUARD 0x4445455053484136ULL /* "DEEPSHA6" */

typedef struct {
    uint32_t layer_idx;
    float depth_z_distance;
    float transmittance_tau;
    float volumetric_opacity;
    bool is_layer_monotonic;
} DeepShadowDepthLayer;

typedef struct {
    uint32_t pixel_id;
    DeepShadowDepthLayer layers[DEEP_SHADOW_DEPTH_LAYERS];
    float final_attenuated_radiance;
    bool is_pixel_shadow_resolved;
} DeepShadowPixelChannel;

typedef struct {
    uint64_t head_guard;
    DeepShadowPixelChannel channels[DEEP_SHADOW_PIXEL_CHANNELS];
    uint64_t deep_shadow_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_deep_shadow_pixels_evaluated;
    uint32_t volumetric_attenuations_solved;
    uint32_t cdc6600_60bit_shadow_words;
    uint32_t overflow_trapped_pixels;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_volumetric_shadow_monotonic;
    bool is_deep_shadow_memory_safe;
} RendermanDeepShadowContext;

typedef struct {
    float in_silicon_deep_shadow_fidelity;
    float deep_shadow_strategy_datbin_merkle_ratio;
    float deep_shadow_eval_latency_ns;
    uint64_t verified_deep_shadow_saat_clearances;

    bool deep_shadow_pipeline_verified;
    bool deep_shadow_strategy_merkle_verified;
    bool deep_shadow_submicro_latency_verified;
    bool deep_shadow_lossless_saat_verified;
    bool sovereign_2475_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanDeepShadowBeyond2470State;

int cpm_tomie_deep_shadow_init(RendermanDeepShadowContext *ctx);
int cpm_tomie_deep_shadow_evaluate(RendermanDeepShadowContext *ctx, uint32_t pix_idx, float init_radiance);
bool cpm_tomie_deep_shadow_assert_safety(RendermanDeepShadowContext *ctx);

void auncient_deep_shadow_init(RendermanDeepShadowBeyond2470State *state);
bool auncient_deep_shadow_verify_theorems_2471_2475(RendermanDeepShadowBeyond2470State *state);
uint32_t auncient_deep_shadow_compute_rule18(const RendermanDeepShadowBeyond2470State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_DEEP_SHADOW_MAP_THEOREMS_2471_2475_H */
