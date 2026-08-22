#ifndef EVAS_SMART_RENDERMAN_REYES_DICING_TEAPOT_THEOREMS_2666_2670_H
#define EVAS_SMART_RENDERMAN_REYES_DICING_TEAPOT_THEOREMS_2666_2670_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RENDERMAN_BEZIER_PATCH_COUNT 32
#define RENDERMAN_REYES_MICROPOLYGONS 64
#define RENDERMAN_REYES_CANARY_GUARD 0x5245594553313936ULL /* "REYES196" */

typedef struct {
    uint32_t patch_id;
    float control_points_4x4[16][3];
    float u_bound[2];
    float v_bound[2];
    bool is_patch_diced_lossless;
} EvasRenderManBezierPatchNode;

typedef struct {
    uint32_t micropolygon_id;
    float vertex_corners_xyz[4][3];
    float surface_normal[3];
    float bssrdf_dipole_radiance_rgb[3];
    float screen_projected_area_pixels; /* Sub-pixel dicing threshold <= 1.0 pixel */
    bool is_micropolygon_shaded;
} EvasRenderManMicropolygonNode;

typedef struct {
    uint64_t head_guard;
    EvasRenderManBezierPatchNode patches[RENDERMAN_BEZIER_PATCH_COUNT];
    EvasRenderManMicropolygonNode micropolygons[RENDERMAN_REYES_MICROPOLYGONS];
    float dicing_rate_pixels;          /* Target: 1.0 pixel grid dicing rate */
    float porcelain_scatter_mean_free_path_mm; /* 0.45 mm dipole subsurface scattering */
    uint64_t evas_reyes_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_patches_split;
    uint32_t total_micropolygons_rasterized;
    uint32_t cdc6600_60bit_reyes_words;
    uint32_t overflow_trapped_reyes_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_reyes_pipeline_lossless;
    bool is_evas_reyes_memory_safe;
} EvasSmartRenderManReyesTeapotContext;

typedef struct {
    float in_silicon_reyes_fidelity;
    float reyes_strategy_datbin_merkle_ratio;
    float reyes_dicing_latency_ns;
    uint64_t verified_reyes_saat_clearances;

    bool evas_reyes_pipeline_verified;
    bool reyes_strategy_merkle_verified;
    bool reyes_submicro_latency_verified;
    bool reyes_lossless_saat_verified;
    bool reyes_micropolygon_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartReyesBeyond2665State;

int evas_smart_renderman_reyes_init(EvasSmartRenderManReyesTeapotContext *ctx);
int evas_smart_renderman_reyes_dice_patch(EvasSmartRenderManReyesTeapotContext *ctx, uint32_t patch_idx, uint32_t micropolygon_idx, float u, float v, float out_radiance[3]);
bool evas_smart_renderman_reyes_assert_safety(EvasSmartRenderManReyesTeapotContext *ctx);

void evas_smart_reyes_beyond2665_init(EvasSmartReyesBeyond2665State *state);
bool evas_smart_reyes_beyond2665_verify_theorems_2666_2670(EvasSmartReyesBeyond2665State *state);
uint32_t evas_smart_reyes_beyond2665_compute_rule18(const EvasSmartReyesBeyond2665State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_RENDERMAN_REYES_DICING_TEAPOT_THEOREMS_2666_2670_H */
