#ifndef EVAS_SMART_RENDERMAN_TEAPOT_TK_POV_THEOREMS_2556_2560_H
#define EVAS_SMART_RENDERMAN_TEAPOT_TK_POV_THEOREMS_2556_2560_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RENDERMAN_TEAPOT_PATCH_COUNT 32
#define TEAPOT_TK_POV_CANARY_GUARD 0x544541504F543636ULL /* "TEAPOT66" */

typedef struct {
    uint32_t patch_id;
    float bicubic_control_points[16][3];
    float micropolygon_grid_resolution;
    float dipole_subsurface_radiance;
    float caustic_photon_intensity;
    float pov_retinal_integrated_lum;
    uint32_t yi_coordinate_leaf_hash;
    bool is_patch_diced_lossless;
} EvasTeapotBicubicPatchNode;

typedef struct {
    uint64_t head_guard;
    EvasTeapotBicubicPatchNode patches[RENDERMAN_TEAPOT_PATCH_COUNT];
    float camera_orbit_azimuth_rad;
    float camera_orbit_elevation_rad;
    float camera_focal_distance;
    uint64_t evas_teapot_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_patches_diced;
    uint32_t reyes_micropolygons_rasterized;
    uint32_t cdc6600_60bit_teapot_words;
    uint32_t overflow_trapped_teapot_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_teapot_reyes_pipeline_lossless;
    bool is_evas_teapot_tk_pov_memory_safe;
} EvasSmartRenderManTeapotTkPovContext;

typedef struct {
    float in_silicon_teapot_tk_pov_fidelity;
    float teapot_strategy_datbin_merkle_ratio;
    float teapot_dicing_latency_ns;
    uint64_t verified_teapot_saat_clearances;

    bool evas_teapot_tk_pov_pipeline_verified;
    bool teapot_strategy_merkle_verified;
    bool teapot_submicro_latency_verified;
    bool teapot_lossless_saat_verified;
    bool teapot_reyes_dicing_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartTeapotTkPovBeyond2555State;

int evas_smart_renderman_teapot_tk_pov_init(EvasSmartRenderManTeapotTkPovContext *ctx);
int evas_smart_renderman_teapot_dice_patch(EvasSmartRenderManTeapotTkPovContext *ctx, uint32_t patch_idx, float u_res, float v_res, float *out_micropolys);
bool evas_smart_renderman_teapot_tk_pov_assert_safety(EvasSmartRenderManTeapotTkPovContext *ctx);

void evas_smart_teapot_tk_pov_beyond2555_init(EvasSmartTeapotTkPovBeyond2555State *state);
bool evas_smart_teapot_tk_pov_beyond2555_verify_theorems_2556_2560(EvasSmartTeapotTkPovBeyond2555State *state);
uint32_t evas_smart_teapot_tk_pov_beyond2555_compute_rule18(const EvasSmartTeapotTkPovBeyond2555State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_RENDERMAN_TEAPOT_TK_POV_THEOREMS_2556_2560_H */
