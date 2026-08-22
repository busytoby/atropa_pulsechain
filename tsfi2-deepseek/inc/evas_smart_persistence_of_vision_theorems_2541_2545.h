#ifndef EVAS_SMART_PERSISTENCE_OF_VISION_THEOREMS_2541_2545_H
#define EVAS_SMART_PERSISTENCE_OF_VISION_THEOREMS_2541_2545_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define POV_INTEGRATION_SAMPLES 64
#define POV_CANARY_GUARD 0x504F565343414E36ULL /* "POVSCAN6" */

typedef struct {
    uint32_t sample_id;
    float frame_timestamp_ms;
    float discrete_luminance_sample;
    float integrated_retinal_response;
    float temporal_flicker_amplitude;
    bool is_flicker_below_cff_threshold;
} EvasRetinalPovSampleNode;

typedef struct {
    uint64_t head_guard;
    EvasRetinalPovSampleNode samples[POV_INTEGRATION_SAMPLES];
    uint64_t evas_pov_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_pov_frames_integrated;
    uint32_t cff_fusion_checks_completed;
    uint32_t cdc6600_60bit_pov_words;
    uint32_t overflow_trapped_pov_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_retinal_motion_continuous;
    bool is_evas_pov_memory_safe;
} EvasSmartRetinalPovContext;

typedef struct {
    float in_silicon_pov_fidelity;
    float pov_strategy_datbin_merkle_ratio;
    float pov_eval_latency_ns;
    uint64_t verified_pov_saat_clearances;

    bool evas_pov_pipeline_verified;
    bool pov_strategy_merkle_verified;
    bool pov_submicro_latency_verified;
    bool pov_lossless_saat_verified;
    bool retinal_flicker_fusion_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartPovBeyond2540State;

int evas_smart_retinal_pov_init(EvasSmartRetinalPovContext *ctx);
int evas_smart_retinal_pov_integrate_frame(EvasSmartRetinalPovContext *ctx, uint32_t frame_idx, float timestamp_ms, float lum_val, float *out_response);
bool evas_smart_retinal_pov_assert_safety(EvasSmartRetinalPovContext *ctx);

void evas_smart_pov_beyond2540_init(EvasSmartPovBeyond2540State *state);
bool evas_smart_pov_beyond2540_verify_theorems_2541_2545(EvasSmartPovBeyond2540State *state);
uint32_t evas_smart_pov_beyond2540_compute_rule18(const EvasSmartPovBeyond2540State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_PERSISTENCE_OF_VISION_THEOREMS_2541_2545_H */
