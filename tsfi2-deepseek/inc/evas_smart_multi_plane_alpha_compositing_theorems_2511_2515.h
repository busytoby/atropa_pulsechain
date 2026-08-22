#ifndef EVAS_SMART_MULTI_PLANE_ALPHA_COMPOSITING_THEOREMS_2511_2515_H
#define EVAS_SMART_MULTI_PLANE_ALPHA_COMPOSITING_THEOREMS_2511_2515_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EVAS_HUD_SPATIAL_PLANES 4
#define EVAS_PLANE_PIXEL_CHANNELS 64
#define EVAS_SMART_COMPOSITING_CANARY_GUARD 0x45564153434D5036ULL /* "EVASCMP6" */

typedef struct {
    uint32_t plane_index;
    float depth_z_layer;
    float plane_opacity_alpha;
    uint32_t blend_mode; /* 0: Porter-Duff Over, 1: Additive Luminous */
    bool is_plane_active;
} EvasSpatialCanvasPlane;

typedef struct {
    uint64_t head_guard;
    EvasSpatialCanvasPlane planes[EVAS_HUD_SPATIAL_PLANES];
    uint64_t evas_compositing_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_multi_plane_pixels_blended;
    uint32_t porter_duff_over_operations_solved;
    uint32_t cdc6600_60bit_compositing_words;
    uint32_t overflow_trapped_compositing_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_porter_duff_alpha_monotonic;
    bool is_evas_smart_compositing_memory_safe;
} EvasSmartMultiPlaneCompositingContext;

typedef struct {
    float in_silicon_compositing_fidelity;
    float compositing_strategy_datbin_merkle_ratio;
    float compositing_eval_latency_ns;
    uint64_t verified_compositing_saat_clearances;

    bool evas_compositing_pipeline_verified;
    bool compositing_strategy_merkle_verified;
    bool compositing_submicro_latency_verified;
    bool compositing_lossless_saat_verified;
    bool porter_duff_alpha_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartCompositingBeyond2510State;

int evas_smart_multi_plane_compositing_init(EvasSmartMultiPlaneCompositingContext *ctx);
int evas_smart_multi_plane_compositing_blend_pixel(EvasSmartMultiPlaneCompositingContext *ctx, const float layer_colors[EVAS_HUD_SPATIAL_PLANES][4], float out_rgba[4]);
bool evas_smart_multi_plane_compositing_assert_safety(EvasSmartMultiPlaneCompositingContext *ctx);

void evas_smart_compositing_beyond2510_init(EvasSmartCompositingBeyond2510State *state);
bool evas_smart_compositing_beyond2510_verify_theorems_2511_2515(EvasSmartCompositingBeyond2510State *state);
uint32_t evas_smart_compositing_beyond2510_compute_rule18(const EvasSmartCompositingBeyond2510State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_MULTI_PLANE_ALPHA_COMPOSITING_THEOREMS_2511_2515_H */
