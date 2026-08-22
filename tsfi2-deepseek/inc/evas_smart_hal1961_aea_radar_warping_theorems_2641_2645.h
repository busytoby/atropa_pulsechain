#ifndef EVAS_SMART_HAL1961_AEA_RADAR_WARPING_THEOREMS_2641_2645_H
#define EVAS_SMART_HAL1961_AEA_RADAR_WARPING_THEOREMS_2641_2645_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL1961_RADAR_SWEEP_RAYS 64
#define HAL1961_RADAR_CANARY_GUARD 0x4145415241444136ULL /* "AEARADA6" */

typedef struct {
    uint32_t ray_id;
    float azimuth_angle_rad;
    float elevation_angle_rad;
    float echo_attenuation_db;
    float antenna_boresight_vector[3];
    bool is_radar_echo_locked;
} EvasHalAeaRadarRayNode;

typedef struct {
    uint64_t head_guard;
    EvasHalAeaRadarRayNode rays[HAL1961_RADAR_SWEEP_RAYS];
    float aea_frequency_ghz;           /* 14.2 GHz Ku-band AE-35 radar link */
    float radar_sweep_rpm;             /* 60.0 RPM planar sweep */
    uint64_t evas_hal_radar_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_radar_sweeps_completed;
    uint32_t beamforming_transform_cycles;
    uint32_t cdc6600_60bit_radar_words;
    uint32_t overflow_trapped_radar_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_ae35_pointing_lossless;
    bool is_evas_hal_radar_memory_safe;
} EvasSmartHal1961AeaRadarContext;

typedef struct {
    float in_silicon_hal_radar_fidelity;
    float hal_radar_strategy_datbin_merkle_ratio;
    float hal_radar_sweep_latency_ns;
    uint64_t verified_hal_radar_saat_clearances;

    bool evas_hal_radar_pipeline_verified;
    bool hal_radar_strategy_merkle_verified;
    bool hal_radar_submicro_latency_verified;
    bool hal_lossless_saat_verified;
    bool hal_radar_echo_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartHalRadarBeyond2640State;

int evas_smart_hal1961_radar_init(EvasSmartHal1961AeaRadarContext *ctx);
int evas_smart_hal1961_radar_sweep_ray(EvasSmartHal1961AeaRadarContext *ctx, uint32_t ray_idx, float az_rad, float el_rad, float out_boresight[3]);
bool evas_smart_hal1961_radar_assert_safety(EvasSmartHal1961AeaRadarContext *ctx);

void evas_smart_hal_radar_beyond2640_init(EvasSmartHalRadarBeyond2640State *state);
bool evas_smart_hal_radar_beyond2640_verify_theorems_2641_2645(EvasSmartHalRadarBeyond2640State *state);
uint32_t evas_smart_hal_radar_beyond2640_compute_rule18(const EvasSmartHalRadarBeyond2640State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_HAL1961_AEA_RADAR_WARPING_THEOREMS_2641_2645_H */
