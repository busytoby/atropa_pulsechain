#ifndef EVAS_SMART_HAL1961_ANAMORPHIC_LENS_TEAPOT_THEOREMS_2616_2620_H
#define EVAS_SMART_HAL1961_ANAMORPHIC_LENS_TEAPOT_THEOREMS_2616_2620_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL1961_ANAMORPHIC_SAMPLES 64
#define HAL1961_LENS_CANARY_GUARD 0x48414C4C454E5336ULL /* "HALLENS6" */

typedef struct {
    uint32_t sample_id;
    float normalized_radius_r;
    float barrel_distortion_factor;
    float chromatic_aberration_shift_rb[2]; /* Red / Blue radial flare offset */
    float projected_screen_xy[2];
    bool is_lens_distortion_bounded;
} EvasHalAnamorphicLensSampleNode;

typedef struct {
    uint64_t head_guard;
    EvasHalAnamorphicLensSampleNode samples[HAL1961_ANAMORPHIC_SAMPLES];
    float lens_focal_length_mm;  /* 8.0 mm ultra-wide fisheye */
    float lens_k1_distortion;    /* k1 = -0.15 */
    float lens_k2_distortion;    /* k2 = 0.05 */
    uint64_t evas_hal_lens_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_lens_vertices_warped;
    uint32_t brown_conrady_transforms_executed;
    uint32_t cdc6600_60bit_hal_lens_words;
    uint32_t overflow_trapped_hal_lens_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_fisheye_optics_lossless;
    bool is_evas_hal_lens_memory_safe;
} EvasSmartHal1961AnamorphicLensContext;

typedef struct {
    float in_silicon_hal_lens_fidelity;
    float hal_lens_strategy_datbin_merkle_ratio;
    float hal_lens_warp_latency_ns;
    uint64_t verified_hal_lens_saat_clearances;

    bool evas_hal_lens_pipeline_verified;
    bool hal_lens_strategy_merkle_verified;
    bool hal_lens_submicro_latency_verified;
    bool hal_lossless_saat_verified;
    bool hal_lens_distortion_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartHalLensBeyond2615State;

int evas_smart_hal1961_lens_init(EvasSmartHal1961AnamorphicLensContext *ctx);
int evas_smart_hal1961_lens_warp_vertex(EvasSmartHal1961AnamorphicLensContext *ctx, uint32_t sample_idx, float norm_x, float norm_y, float out_warped_xy[2]);
bool evas_smart_hal1961_lens_assert_safety(EvasSmartHal1961AnamorphicLensContext *ctx);

void evas_smart_hal_lens_beyond2615_init(EvasSmartHalLensBeyond2615State *state);
bool evas_smart_hal_lens_beyond2615_verify_theorems_2616_2620(EvasSmartHalLensBeyond2615State *state);
uint32_t evas_smart_hal_lens_beyond2615_compute_rule18(const EvasSmartHalLensBeyond2615State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_HAL1961_ANAMORPHIC_LENS_TEAPOT_THEOREMS_2616_2620_H */
