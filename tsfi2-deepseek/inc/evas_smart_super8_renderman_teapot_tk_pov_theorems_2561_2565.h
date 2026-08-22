#ifndef EVAS_SMART_SUPER8_RENDERMAN_TEAPOT_TK_POV_THEOREMS_2561_2565_H
#define EVAS_SMART_SUPER8_RENDERMAN_TEAPOT_TK_POV_THEOREMS_2561_2565_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SUPER8_FILM_STRIP_FRAMES 24
#define SUPER8_ASPECT_RATIO_W 1.85f
#define SUPER8_CANARY_GUARD 0x5355504552383636ULL /* "SUPER866" */

typedef struct {
    uint32_t frame_index;
    float shutter_open_phase_rad;
    float silver_halide_emulsion_transmittance;
    float widescreen_viewport_xywh[4]; /* 1920 x 1038 (1.85:1 aspect) */
    uint32_t yi_emulsion_grain_hash;
    bool is_super8_shutter_flicker_free;
} EvasSuper8FilmFrameNode;

typedef struct {
    uint64_t head_guard;
    EvasSuper8FilmFrameNode frames[SUPER8_FILM_STRIP_FRAMES];
    float geneva_mechanism_pull_down_speed_fps;
    float rotary_shutter_blade_count;
    uint64_t evas_super8_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_super8_frames_projected;
    uint32_t geneva_intermittent_steps;
    uint32_t cdc6600_60bit_super8_words;
    uint32_t overflow_trapped_super8_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_super8_185_aspect_ratio_exact;
    bool is_evas_super8_memory_safe;
} EvasSmartSuper8TeapotTkPovContext;

typedef struct {
    float in_silicon_super8_fidelity;
    float super8_strategy_datbin_merkle_ratio;
    float super8_projection_latency_ns;
    uint64_t verified_super8_saat_clearances;

    bool evas_super8_pipeline_verified;
    bool super8_strategy_merkle_verified;
    bool super8_submicro_latency_verified;
    bool super8_lossless_saat_verified;
    bool super8_widescreen_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartSuper8Beyond2560State;

int evas_smart_super8_teapot_tk_pov_init(EvasSmartSuper8TeapotTkPovContext *ctx);
int evas_smart_super8_teapot_project_frame(EvasSmartSuper8TeapotTkPovContext *ctx, uint32_t frame_idx, float shutter_angle, float *out_transmittance);
bool evas_smart_super8_teapot_tk_pov_assert_safety(EvasSmartSuper8TeapotTkPovContext *ctx);

void evas_smart_super8_beyond2560_init(EvasSmartSuper8Beyond2560State *state);
bool evas_smart_super8_beyond2560_verify_theorems_2561_2565(EvasSmartSuper8Beyond2560State *state);
uint32_t evas_smart_super8_beyond2560_compute_rule18(const EvasSmartSuper8Beyond2560State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_SUPER8_RENDERMAN_TEAPOT_TK_POV_THEOREMS_2561_2565_H */
