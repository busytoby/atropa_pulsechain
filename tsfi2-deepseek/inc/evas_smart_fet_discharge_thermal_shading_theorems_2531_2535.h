#ifndef EVAS_SMART_FET_DISCHARGE_THERMAL_SHADING_THEOREMS_2531_2535_H
#define EVAS_SMART_FET_DISCHARGE_THERMAL_SHADING_THEOREMS_2531_2535_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EVAS_FET_TRANSISTOR_NODES 64
#define EVAS_FET_DISCHARGE_CANARY_GUARD 0x4645544449533636ULL /* "FETDIS66" */

typedef struct {
    uint32_t fet_id;
    float drain_source_voltage_v;
    float gate_source_voltage_v;
    float channel_temperature_kelvin;
    float verlet_position_x;
    float verlet_previous_x;
    float verlet_velocity_dx;
    bool is_discharge_active;
} EvasFetDischargeVerletNode;

typedef struct {
    uint64_t head_guard;
    EvasFetDischargeVerletNode fet_nodes[EVAS_FET_TRANSISTOR_NODES];
    uint64_t evas_fet_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_discharge_cycles_solved;
    uint32_t verlet_integration_steps_computed;
    uint32_t cdc6600_60bit_fet_words;
    uint32_t overflow_trapped_fet_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_verlet_discharge_energy_conserved;
    bool is_evas_fet_shading_memory_safe;
} EvasSmartFetDischargeShadingContext;

typedef struct {
    float in_silicon_fet_fidelity;
    float fet_strategy_datbin_merkle_ratio;
    float fet_discharge_latency_ns;
    uint64_t verified_fet_saat_clearances;

    bool evas_fet_pipeline_verified;
    bool fet_strategy_merkle_verified;
    bool fet_submicro_latency_verified;
    bool fet_lossless_saat_verified;
    bool fet_discharge_energy_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartFetBeyond2530State;

int evas_smart_fet_discharge_init(EvasSmartFetDischargeShadingContext *ctx);
int evas_smart_fet_discharge_step_verlet(EvasSmartFetDischargeShadingContext *ctx, uint32_t fet_idx, float dt_ns, float *out_temp_k);
bool evas_smart_fet_discharge_assert_safety(EvasSmartFetDischargeShadingContext *ctx);

void evas_smart_fet_beyond2530_init(EvasSmartFetBeyond2530State *state);
bool evas_smart_fet_beyond2530_verify_theorems_2531_2535(EvasSmartFetBeyond2530State *state);
uint32_t evas_smart_fet_beyond2530_compute_rule18(const EvasSmartFetBeyond2530State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_FET_DISCHARGE_THERMAL_SHADING_THEOREMS_2531_2535_H */
