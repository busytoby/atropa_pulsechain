#ifndef EVAS_SMART_COLOSSUS_PERMALIGHT_HAL1961_SUPER8_THEOREMS_2586_2590_H
#define EVAS_SMART_COLOSSUS_PERMALIGHT_HAL1961_SUPER8_THEOREMS_2586_2590_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define COLOSSUS_PHOSPHOR_TRAIL_NODES 64
#define COLOSSUS_PERMALIGHT_CANARY_GUARD 0x434F4C4F53535553ULL /* "COLOSSUS" */

typedef struct {
    uint32_t node_id;
    float phosphor_intensity_p7;
    float phosphor_decay_fast_ms; /* tau_fast = 1.2 ms */
    float phosphor_decay_slow_ms; /* tau_slow = 145.0 ms */
    float colossus_green_rgb[3];  /* [0.05, 0.98, 0.15] */
    bool is_permalight_persistent;
} EvasColossusPhosphorTrailNode;

typedef struct {
    uint64_t head_guard;
    EvasColossusPhosphorTrailNode trails[COLOSSUS_PHOSPHOR_TRAIL_NODES];
    float hal_cyclops_red_rgb[3]; /* [0.98, 0.02, 0.01] */
    float widescreen_gate_185_xywh[4]; /* [0, 0, 1920, 1038] */
    uint64_t evas_colossus_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_colossus_frames_composited;
    uint32_t permalight_trails_evaluated;
    uint32_t cdc6600_60bit_colossus_words;
    uint32_t overflow_trapped_colossus_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_colossus_permalight_lossless;
    bool is_evas_colossus_memory_safe;
} EvasSmartColossusPermalightContext;

typedef struct {
    float in_silicon_colossus_fidelity;
    float colossus_strategy_datbin_merkle_ratio;
    float colossus_composite_latency_ns;
    uint64_t verified_colossus_saat_clearances;

    bool evas_colossus_pipeline_verified;
    bool colossus_strategy_merkle_verified;
    bool colossus_submicro_latency_verified;
    bool colossus_lossless_saat_verified;
    bool colossus_permalight_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartColossusBeyond2585State;

int evas_smart_colossus_permalight_init(EvasSmartColossusPermalightContext *ctx);
int evas_smart_colossus_permalight_step_decay(EvasSmartColossusPermalightContext *ctx, uint32_t trail_idx, float dt_ms, float initial_lum, float *out_decayed_lum);
bool evas_smart_colossus_permalight_assert_safety(EvasSmartColossusPermalightContext *ctx);

void evas_smart_colossus_beyond2585_init(EvasSmartColossusBeyond2585State *state);
bool evas_smart_colossus_beyond2585_verify_theorems_2586_2590(EvasSmartColossusBeyond2585State *state);
uint32_t evas_smart_colossus_beyond2585_compute_rule18(const EvasSmartColossusBeyond2585State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_COLOSSUS_PERMALIGHT_HAL1961_SUPER8_THEOREMS_2586_2590_H */
