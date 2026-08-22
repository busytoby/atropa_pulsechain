#ifndef EVAS_SMART_HAL1961_STAR_GATE_TRANSCENDENCE_THEOREMS_2661_2665_H
#define EVAS_SMART_HAL1961_STAR_GATE_TRANSCENDENCE_THEOREMS_2661_2665_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL1961_STAR_GATE_SLIT_RAYS 64
#define HAL1961_STAR_GATE_CANARY_GUARD 0x5354415247415436ULL /* "STARGAT6" */

typedef struct {
    uint32_t ray_id;
    float slit_scan_offset_u;          /* Slit-scan spatial warp coordinate u */
    float slit_scan_offset_v;          /* Slit-scan spatial warp coordinate v */
    float hyperspace_corridor_rgb[3];  /* Chromatic streak radiance */
    float star_child_coherence;        /* 1.0 = lossless Total Knowledge coherence */
    bool is_slit_scan_converged;
} EvasHalStarGateSlitRayNode;

typedef struct {
    uint64_t head_guard;
    EvasHalStarGateSlitRayNode rays[HAL1961_STAR_GATE_SLIT_RAYS];
    float monolith_aspect_ratio_149;   /* 1 : 4 : 9 Pythagoras volume ratio */
    float infinite_slit_scan_velocity; /* Hyperbolic Lorentz velocity parameter */
    uint64_t evas_stargate_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_transcendence_frames;
    uint32_t slit_scan_convolutions_executed;
    uint32_t cdc6600_60bit_stargate_words;
    uint32_t overflow_trapped_stargate_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_star_gate_corridor_lossless;
    bool is_evas_stargate_memory_safe;
} EvasSmartHal1961StarGateTranscendenceContext;

typedef struct {
    float in_silicon_stargate_fidelity;
    float stargate_strategy_datbin_merkle_ratio;
    float stargate_warp_latency_ns;
    uint64_t verified_stargate_saat_clearances;

    bool evas_stargate_pipeline_verified;
    bool stargate_strategy_merkle_verified;
    bool stargate_submicro_latency_verified;
    bool stargate_lossless_saat_verified;
    bool stargate_hyperspace_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartStarGateBeyond2660State;

int evas_smart_hal1961_stargate_init(EvasSmartHal1961StarGateTranscendenceContext *ctx);
int evas_smart_hal1961_stargate_warp_ray(EvasSmartHal1961StarGateTranscendenceContext *ctx, uint32_t ray_idx, float u, float v, float out_radiance[3]);
bool evas_smart_hal1961_stargate_assert_safety(EvasSmartHal1961StarGateTranscendenceContext *ctx);

void evas_smart_stargate_beyond2660_init(EvasSmartStarGateBeyond2660State *state);
bool evas_smart_stargate_beyond2660_verify_theorems_2661_2665(EvasSmartStarGateBeyond2660State *state);
uint32_t evas_smart_stargate_beyond2660_compute_rule18(const EvasSmartStarGateBeyond2660State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_HAL1961_STAR_GATE_TRANSCENDENCE_THEOREMS_2661_2665_H */
