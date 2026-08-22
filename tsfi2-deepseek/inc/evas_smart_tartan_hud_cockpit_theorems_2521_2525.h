#ifndef EVAS_SMART_TARTAN_HUD_COCKPIT_THEOREMS_2521_2525_H
#define EVAS_SMART_TARTAN_HUD_COCKPIT_THEOREMS_2521_2525_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EVAS_TARTAN_GAUGE_ELEMENTS 16
#define EVAS_TARTAN_HUD_CANARY_GUARD 0x54415254414E3636ULL /* "TARTAN66" */

typedef struct {
    uint32_t gauge_id;
    float needle_angle_rad;
    float dial_value_scale;
    float screen_coord_xy[2];
    bool is_gauge_dirtied;
} EvasTartanHudGaugeElement;

typedef struct {
    uint64_t head_guard;
    EvasTartanHudGaugeElement gauges[EVAS_TARTAN_GAUGE_ELEMENTS];
    uint64_t evas_tartan_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_gauge_ticks_rendered;
    uint32_t damage_blits_executed;
    uint32_t cdc6600_60bit_hud_words;
    uint32_t overflow_trapped_hud_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_tartan_gauge_rendering_lossless;
    bool is_evas_tartan_hud_memory_safe;
} EvasSmartTartanHudContext;

typedef struct {
    float in_silicon_tartan_fidelity;
    float tartan_strategy_datbin_merkle_ratio;
    float tartan_render_latency_ns;
    uint64_t verified_tartan_saat_clearances;

    bool evas_tartan_hud_pipeline_verified;
    bool tartan_strategy_merkle_verified;
    bool tartan_submicro_latency_verified;
    bool tartan_lossless_saat_verified;
    bool tartan_dial_angle_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartTartanBeyond2520State;

int evas_smart_tartan_hud_init(EvasSmartTartanHudContext *ctx);
int evas_smart_tartan_hud_update_gauge(EvasSmartTartanHudContext *ctx, uint32_t gauge_idx, float value, float *out_angle);
bool evas_smart_tartan_hud_assert_safety(EvasSmartTartanHudContext *ctx);

void evas_smart_tartan_beyond2520_init(EvasSmartTartanBeyond2520State *state);
bool evas_smart_tartan_beyond2520_verify_theorems_2521_2525(EvasSmartTartanBeyond2520State *state);
uint32_t evas_smart_tartan_beyond2520_compute_rule18(const EvasSmartTartanBeyond2520State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_TARTAN_HUD_COCKPIT_THEOREMS_2521_2525_H */
