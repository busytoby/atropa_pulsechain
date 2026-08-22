#ifndef AUNCIENT_RENDERMAN_SHADOW_MAP_PCF_THEOREMS_2201_2205_H
#define AUNCIENT_RENDERMAN_SHADOW_MAP_PCF_THEOREMS_2201_2205_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHADOW_MAP_DIM 512
#define PCF_KERNEL_TAPS 16

typedef struct {
    float depth_buffer[SHADOW_MAP_DIM][SHADOW_MAP_DIM];
    double light_view_proj[4][4];
    uint32_t total_pcf_evaluations;
    uint32_t penumbra_taps_filtered;
    bool is_shadow_map_rendered;
    bool is_pcf_filtering_converged;
} RendermanShadowPcfContext;

typedef struct {
    float in_silicon_shadow_fidelity;
    float shadow_strategy_datbin_merkle_ratio;
    float shadow_pcf_eval_latency_ns;
    uint64_t verified_shadow_saat_clearances;

    bool renderman_shadow_pcf_pipeline_verified;
    bool shadow_strategy_merkle_verified;
    bool shadow_submicro_latency_verified;
    bool shadow_lossless_saat_verified;
    bool sovereign_2205_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanShadowPcfBeyond2200State;

int cpm_tomie_renderman_shadow_pcf_init(RendermanShadowPcfContext *ctx);
int cpm_tomie_renderman_shadow_pcf_filter(RendermanShadowPcfContext *ctx, uint32_t query_points);
void auncient_renderman_shadow_pcf_init(RendermanShadowPcfBeyond2200State *state);
bool auncient_renderman_shadow_pcf_verify_theorems_2201_2205(RendermanShadowPcfBeyond2200State *state);
uint32_t auncient_renderman_shadow_pcf_compute_rule18(const RendermanShadowPcfBeyond2200State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_SHADOW_MAP_PCF_THEOREMS_2201_2205_H */
