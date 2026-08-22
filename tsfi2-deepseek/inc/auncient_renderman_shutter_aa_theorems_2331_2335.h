#ifndef AUNCIENT_RENDERMAN_SHUTTER_AA_THEOREMS_2331_2335_H
#define AUNCIENT_RENDERMAN_SHUTTER_AA_THEOREMS_2331_2335_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RENDERMAN_TEMPORAL_SAMPLES 16
#define RENDERMAN_SHUTTER_CANARY_GUARD 0x5348555454455236ULL /* "SHUTTER6" */

typedef struct {
    uint32_t sample_id;
    float time_offset_normalized; /* [0.0, 1.0] across shutter open/close interval */
    float jitter_dx;
    float jitter_dy;
    float sample_weight;
    bool is_sample_valid;
} RendermanTemporalSample;

typedef struct {
    uint64_t head_guard;
    RendermanTemporalSample temporal_samples[RENDERMAN_TEMPORAL_SAMPLES];
    uint64_t filtered_pixel_latch[64];
    uint64_t tail_guard;
    uint32_t total_subframe_integrations;
    uint32_t antialiased_pixels_accumulated;
    uint32_t cdc6600_60bit_shutter_words;
    uint32_t overflow_trapped_samples;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_temporal_blur_coherent;
    bool is_shutter_aa_memory_safe;
} RendermanShutterAaContext;

typedef struct {
    float in_silicon_shutter_fidelity;
    float shutter_strategy_datbin_merkle_ratio;
    float shutter_integration_latency_ns;
    uint64_t verified_shutter_saat_clearances;

    bool shutter_aa_pipeline_verified;
    bool shutter_strategy_merkle_verified;
    bool shutter_submicro_latency_verified;
    bool shutter_lossless_saat_verified;
    bool sovereign_2335_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanShutterBeyond2330State;

int cpm_tomie_renderman_shutter_aa_init(RendermanShutterAaContext *ctx);
int cpm_tomie_renderman_shutter_aa_integrate(RendermanShutterAaContext *ctx, uint32_t sample_idx, float t_offset);
bool cpm_tomie_renderman_shutter_aa_assert_safety(RendermanShutterAaContext *ctx);

void auncient_renderman_shutter_aa_init(RendermanShutterBeyond2330State *state);
bool auncient_renderman_shutter_aa_verify_theorems_2331_2335(RendermanShutterBeyond2330State *state);
uint32_t auncient_renderman_shutter_aa_compute_rule18(const RendermanShutterBeyond2330State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_SHUTTER_AA_THEOREMS_2331_2335_H */
