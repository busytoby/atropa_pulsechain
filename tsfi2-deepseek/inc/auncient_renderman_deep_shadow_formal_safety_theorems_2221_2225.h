#ifndef AUNCIENT_RENDERMAN_DEEP_SHADOW_FORMAL_SAFETY_THEOREMS_2221_2225_H
#define AUNCIENT_RENDERMAN_DEEP_SHADOW_FORMAL_SAFETY_THEOREMS_2221_2225_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEEP_SHADOW_CELLS_X 64
#define DEEP_SHADOW_CELLS_Y 64
#define DEEP_SHADOW_MAX_NODES_PER_PIXEL 16
#define DEEP_SHADOW_CANARY_GUARD 0x534841444F573636ULL /* "SHADOW66" */

typedef struct {
    float depth;
    float transmittance;
} DeepShadowNode;

typedef struct {
    uint32_t node_count;
    DeepShadowNode nodes[DEEP_SHADOW_MAX_NODES_PER_PIXEL];
} DeepShadowPixel;

typedef struct {
    uint64_t head_guard;
    DeepShadowPixel pixels[DEEP_SHADOW_CELLS_Y][DEEP_SHADOW_CELLS_X];
    uint64_t tail_guard;
    uint32_t total_transmittance_nodes_allocated;
    uint32_t overflow_trapped_samples;
    uint32_t cdc6600_60bit_words_filtered;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_deep_shadow_memory_safe;
} RendermanDeepShadowSafetyContext;

typedef struct {
    float in_silicon_deep_shadow_fidelity;
    float deep_shadow_strategy_datbin_merkle_ratio;
    float deep_shadow_filter_latency_ns;
    uint64_t verified_deep_shadow_saat_clearances;

    bool deep_shadow_safety_pipeline_verified;
    bool deep_shadow_strategy_merkle_verified;
    bool deep_shadow_submicro_latency_verified;
    bool deep_shadow_lossless_saat_verified;
    bool sovereign_2225_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanDeepShadowBeyond2220State;

int cpm_tomie_renderman_deep_shadow_safety_init(RendermanDeepShadowSafetyContext *ctx);
int cpm_tomie_renderman_deep_shadow_insert_sample(RendermanDeepShadowSafetyContext *ctx, uint32_t x, uint32_t y, float depth, float trans);
bool cpm_tomie_renderman_deep_shadow_assert_safety(RendermanDeepShadowSafetyContext *ctx);

void auncient_renderman_deep_shadow_safety_init(RendermanDeepShadowBeyond2220State *state);
bool auncient_renderman_deep_shadow_safety_verify_theorems_2221_2225(RendermanDeepShadowBeyond2220State *state);
uint32_t auncient_renderman_deep_shadow_safety_compute_rule18(const RendermanDeepShadowBeyond2220State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_DEEP_SHADOW_FORMAL_SAFETY_THEOREMS_2221_2225_H */
