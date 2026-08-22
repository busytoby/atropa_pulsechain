#ifndef EVAS_SMART_ANAGLYPH_STEREO_PROJECTION_THEOREMS_2546_2550_H
#define EVAS_SMART_ANAGLYPH_STEREO_PROJECTION_THEOREMS_2546_2550_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EVAS_STEREO_SAMPLE_CHANNELS 64
#define EVAS_STEREO_CANARY_GUARD 0x53544552454F3636ULL /* "STEREO66" */

typedef struct {
    uint32_t channel_id;
    float left_eye_rgb[3];
    float right_eye_rgb[3];
    float anaglyph_red_cyan_rgb[3];
    float ocular_parallax_disparity;
    bool is_disparity_bounded;
} EvasAnaglyphStereoPixelNode;

typedef struct {
    uint64_t head_guard;
    EvasAnaglyphStereoPixelNode channels[EVAS_STEREO_SAMPLE_CHANNELS];
    float interpupillary_distance_ipd;
    float focal_plane_distance;
    uint64_t evas_stereo_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_stereo_pixels_projected;
    uint32_t red_cyan_matrix_multiplications;
    uint32_t cdc6600_60bit_stereo_words;
    uint32_t overflow_trapped_stereo_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_stereoscopic_disparity_lossless;
    bool is_evas_stereo_memory_safe;
} EvasSmartStereoProjectionContext;

typedef struct {
    float in_silicon_stereo_fidelity;
    float stereo_strategy_datbin_merkle_ratio;
    float stereo_eval_latency_ns;
    uint64_t verified_stereo_saat_clearances;

    bool evas_stereo_pipeline_verified;
    bool stereo_strategy_merkle_verified;
    bool stereo_submicro_latency_verified;
    bool stereo_lossless_saat_verified;
    bool stereo_anaglyph_matrix_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartStereoBeyond2545State;

int evas_smart_stereo_projection_init(EvasSmartStereoProjectionContext *ctx);
int evas_smart_stereo_projection_blend_anaglyph(EvasSmartStereoProjectionContext *ctx, uint32_t channel_idx, const float left_rgb[3], const float right_rgb[3], float parallax, float out_anaglyph[3]);
bool evas_smart_stereo_projection_assert_safety(EvasSmartStereoProjectionContext *ctx);

void evas_smart_stereo_beyond2545_init(EvasSmartStereoBeyond2545State *state);
bool evas_smart_stereo_beyond2545_verify_theorems_2546_2550(EvasSmartStereoBeyond2545State *state);
uint32_t evas_smart_stereo_beyond2545_compute_rule18(const EvasSmartStereoBeyond2545State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_ANAGLYPH_STEREO_PROJECTION_THEOREMS_2546_2550_H */
