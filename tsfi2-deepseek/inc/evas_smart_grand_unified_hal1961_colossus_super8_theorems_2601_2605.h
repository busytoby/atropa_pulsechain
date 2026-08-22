#ifndef EVAS_SMART_GRAND_UNIFIED_HAL1961_COLOSSUS_SUPER8_THEOREMS_2601_2605_H
#define EVAS_SMART_GRAND_UNIFIED_HAL1961_COLOSSUS_SUPER8_THEOREMS_2601_2605_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GRAND_UNIFIED_COMPOSITE_PLANES 6
#define GRAND_UNIFIED_CANARY_GUARD 0x48414C434F4C3838ULL /* "HALCOL88" */

typedef struct {
    uint32_t plane_index;
    const char *plane_tag;
    float plane_alpha_weight;
    float plane_radiance_rgb[3];
    bool is_plane_active;
} EvasGrandUnifiedPlaneNode;

typedef struct {
    uint64_t head_guard;
    EvasGrandUnifiedPlaneNode planes[GRAND_UNIFIED_COMPOSITE_PLANES];
    float edje_base_membrane_potential_mv; /* -70.0 mV stable base */
    float colossus_p7_intensity;           /* Dual-exponential Permalight green trail */
    float hal_cyclops_breathing_lum;       /* f/1.4 glowing red optical eye */
    float super8_widescreen_aspect_ratio;  /* 1.85:1 (1920x1038) */
    uint32_t reyes_teapot_micropolygons;   /* 8,192 micropolygons */
    uint64_t evas_grand_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_grand_composite_frames;
    uint32_t six_plane_blits_executed;
    uint32_t cdc6600_60bit_grand_words;
    uint32_t overflow_trapped_grand_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_grand_unified_pipeline_lossless;
    bool is_evas_grand_memory_safe;
} EvasSmartGrandUnifiedHalColossusSuper8Context;

typedef struct {
    float in_silicon_grand_fidelity;
    float grand_strategy_datbin_merkle_ratio;
    float grand_composite_latency_ns;
    uint64_t verified_grand_saat_clearances;

    bool evas_grand_pipeline_verified;
    bool grand_strategy_merkle_verified;
    bool grand_submicro_latency_verified;
    bool grand_lossless_saat_verified;
    bool grand_composite_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartGrandBeyond2600State;

int evas_smart_grand_unified_init(EvasSmartGrandUnifiedHalColossusSuper8Context *ctx);
int evas_smart_grand_unified_render_frame(EvasSmartGrandUnifiedHalColossusSuper8Context *ctx, uint32_t frame_idx, float time_sec, float out_final_rgb[3]);
bool evas_smart_grand_unified_assert_safety(EvasSmartGrandUnifiedHalColossusSuper8Context *ctx);

void evas_smart_grand_beyond2600_init(EvasSmartGrandBeyond2600State *state);
bool evas_smart_grand_beyond2600_verify_theorems_2601_2605(EvasSmartGrandBeyond2600State *state);
uint32_t evas_smart_grand_beyond2600_compute_rule18(const EvasSmartGrandBeyond2600State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_GRAND_UNIFIED_HAL1961_COLOSSUS_SUPER8_THEOREMS_2601_2605_H */
