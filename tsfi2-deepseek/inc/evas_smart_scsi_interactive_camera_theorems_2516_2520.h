#ifndef EVAS_SMART_SCSI_INTERACTIVE_CAMERA_THEOREMS_2516_2520_H
#define EVAS_SMART_SCSI_INTERACTIVE_CAMERA_THEOREMS_2516_2520_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EVAS_SCSI_KEY_BUFFER_SIZE 32
#define EVAS_SCSI_CAMERA_CANARY_GUARD 0x45565343414D3636ULL /* "EVSCAM66" */

typedef struct {
    uint32_t keycode;
    float azimuth_delta_rad;
    float elevation_delta_rad;
    float zoom_delta_units;
    bool is_key_event_handled;
} EvasScsiKeyEventMapping;

typedef struct {
    uint64_t head_guard;
    EvasScsiKeyEventMapping key_events[EVAS_SCSI_KEY_BUFFER_SIZE];
    float camera_orbit_matrix[4][4];
    float camera_azimuth_rad;
    float camera_elevation_rad;
    float camera_distance_units;
    uint64_t evas_camera_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_scsi_events_dispatched;
    uint32_t matrix_updates_computed;
    uint32_t cdc6600_60bit_camera_words;
    uint32_t overflow_trapped_events;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_camera_matrix_orthonormal;
    bool is_evas_scsi_camera_memory_safe;
} EvasSmartScsiCameraContext;

typedef struct {
    float in_silicon_camera_fidelity;
    float camera_strategy_datbin_merkle_ratio;
    float camera_transform_latency_ns;
    uint64_t verified_camera_saat_clearances;

    bool evas_scsi_camera_pipeline_verified;
    bool camera_strategy_merkle_verified;
    bool camera_submicro_latency_verified;
    bool camera_lossless_saat_verified;
    bool camera_matrix_orthonormality_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartCameraBeyond2515State;

int evas_smart_scsi_camera_init(EvasSmartScsiCameraContext *ctx);
int evas_smart_scsi_camera_dispatch_key(EvasSmartScsiCameraContext *ctx, uint32_t keycode, float *out_azimuth, float *out_elevation);
bool evas_smart_scsi_camera_assert_safety(EvasSmartScsiCameraContext *ctx);

void evas_smart_camera_beyond2515_init(EvasSmartCameraBeyond2515State *state);
bool evas_smart_camera_beyond2515_verify_theorems_2516_2520(EvasSmartCameraBeyond2515State *state);
uint32_t evas_smart_camera_beyond2515_compute_rule18(const EvasSmartCameraBeyond2515State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_SCSI_INTERACTIVE_CAMERA_THEOREMS_2516_2520_H */
