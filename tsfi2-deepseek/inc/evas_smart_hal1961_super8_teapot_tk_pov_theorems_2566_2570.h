#ifndef EVAS_SMART_HAL1961_SUPER8_TEAPOT_TK_POV_THEOREMS_2566_2570_H
#define EVAS_SMART_HAL1961_SUPER8_TEAPOT_TK_POV_THEOREMS_2566_2570_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL1961_ANNUNCIATOR_COUNT 16
#define HAL1961_COCKPIT_CANARY_GUARD 0x48414C3930303036ULL /* "HAL90006" */

typedef struct {
    uint32_t annunciator_id;
    float warning_luminance;
    float crt_vector_x;
    float crt_vector_y;
    bool is_annunciator_active;
} EvasHalAnnunciatorNode;

typedef struct {
    uint64_t head_guard;
    EvasHalAnnunciatorNode annunciators[HAL1961_ANNUNCIATOR_COUNT];
    float hal_cyclops_eye_rgb[3];   /* [0.98, 0.02, 0.01] */
    float hal_cyclops_pupil_radius; /* 12.0 mm */
    float widescreen_gate_xywh[4];  /* [0, 0, 1920, 1038] */
    uint64_t evas_hal_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_hal_telemetry_cycles;
    uint32_t composite_planes_blitted;
    uint32_t cdc6600_60bit_hal_words;
    uint32_t overflow_trapped_hal_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_hal_cyclops_composite_lossless;
    bool is_evas_hal1961_memory_safe;
} EvasSmartHal1961CockpitContext;

typedef struct {
    float in_silicon_hal_fidelity;
    float hal_strategy_datbin_merkle_ratio;
    float hal_composite_latency_ns;
    uint64_t verified_hal_saat_clearances;

    bool evas_hal_cockpit_pipeline_verified;
    bool hal_strategy_merkle_verified;
    bool hal_submicro_latency_verified;
    bool hal_lossless_saat_verified;
    bool hal_cyclops_eye_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartHalBeyond2565State;

int evas_smart_hal1961_cockpit_init(EvasSmartHal1961CockpitContext *ctx);
int evas_smart_hal1961_cockpit_composite_frame(EvasSmartHal1961CockpitContext *ctx, uint32_t frame_idx, float pulse_phase, float out_composite_rgb[3]);
bool evas_smart_hal1961_cockpit_assert_safety(EvasSmartHal1961CockpitContext *ctx);

void evas_smart_hal_beyond2565_init(EvasSmartHalBeyond2565State *state);
bool evas_smart_hal_beyond2565_verify_theorems_2566_2570(EvasSmartHalBeyond2565State *state);
uint32_t evas_smart_hal_beyond2565_compute_rule18(const EvasSmartHalBeyond2565State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_HAL1961_SUPER8_TEAPOT_TK_POV_THEOREMS_2566_2570_H */
