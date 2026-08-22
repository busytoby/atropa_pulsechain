#ifndef EVAS_SMART_RENDERMAN_PORCELAIN_GLAZE_THEOREMS_2676_2680_H
#define EVAS_SMART_RENDERMAN_PORCELAIN_GLAZE_THEOREMS_2676_2680_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TEAPOT_GLAZE_SURFACE_SAMPLES 64
#define TEAPOT_GLAZE_CANARY_GUARD 0x504F5243454C3636ULL /* "PORCEL66" */

typedef struct {
    uint32_t sample_id;
    float incident_ray_dir[3];
    float surface_normal[3];
    float microfacet_roughness_alpha;  /* alpha = 0.08 (Smooth specular glaze) */
    float specular_microfacet_brdf;    /* Cook-Torrance specular term */
    float subsurface_dipole_radiance[3];/* Translucent warm porcelain glow */
    bool is_energy_conserved;
} EvasTeapotGlazeSampleNode;

typedef struct {
    uint64_t head_guard;
    EvasTeapotGlazeSampleNode samples[TEAPOT_GLAZE_SURFACE_SAMPLES];
    float porcelain_refractive_index;  /* IOR = 1.54 */
    float glaze_absorption_sigma_a;    /* sigma_a = 0.002 mm^-1 */
    uint64_t evas_glaze_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_glaze_samples_shaded;
    uint32_t cook_torrance_eval_cycles;
    uint32_t cdc6600_60bit_glaze_words;
    uint32_t overflow_trapped_glaze_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_glaze_optics_lossless;
    bool is_evas_glaze_memory_safe;
} EvasSmartRenderManPorcelainGlazeContext;

typedef struct {
    float in_silicon_glaze_fidelity;
    float glaze_strategy_datbin_merkle_ratio;
    float glaze_eval_latency_ns;
    uint64_t verified_glaze_saat_clearances;

    bool evas_glaze_pipeline_verified;
    bool glaze_strategy_merkle_verified;
    bool glaze_submicro_latency_verified;
    bool glaze_lossless_saat_verified;
    bool glaze_cook_torrance_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartGlazeBeyond2675State;

int evas_smart_renderman_glaze_init(EvasSmartRenderManPorcelainGlazeContext *ctx);
int evas_smart_renderman_glaze_shade_sample(EvasSmartRenderManPorcelainGlazeContext *ctx, uint32_t sample_idx, float view_angle_rad, float out_radiance[3]);
bool evas_smart_renderman_glaze_assert_safety(EvasSmartRenderManPorcelainGlazeContext *ctx);

void evas_smart_glaze_beyond2675_init(EvasSmartGlazeBeyond2675State *state);
bool evas_smart_glaze_beyond2675_verify_theorems_2676_2680(EvasSmartGlazeBeyond2675State *state);
uint32_t evas_smart_glaze_beyond2675_compute_rule18(const EvasSmartGlazeBeyond2675State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_RENDERMAN_PORCELAIN_GLAZE_THEOREMS_2676_2680_H */
