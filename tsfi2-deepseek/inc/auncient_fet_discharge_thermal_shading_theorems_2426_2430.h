#ifndef AUNCIENT_FET_DISCHARGE_THERMAL_SHADING_THEOREMS_2426_2430_H
#define AUNCIENT_FET_DISCHARGE_THERMAL_SHADING_THEOREMS_2426_2430_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FET_DISCHARGE_NODES 64
#define FET_VOLTAGE_SUPPLY_V 3.3f
#define FET_THERMAL_CANARY_GUARD 0x4645545448455236ULL /* "FETTHER6" */

typedef struct {
    uint32_t node_id;
    float gate_voltage_v;
    float drain_current_ma;
    float verlet_discharge_power_watts;
    float thermal_emission_lumens;
    bool is_discharge_bounded;
} FetVerletThermalNode;

typedef struct {
    uint64_t head_guard;
    FetVerletThermalNode nodes[FET_DISCHARGE_NODES];
    uint64_t thermal_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_discharge_cycles_solved;
    uint32_t verlet_mass_spring_steps;
    uint32_t cdc6600_60bit_thermal_words;
    uint32_t overflow_trapped_discharges;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_lowpower_thermal_lossless;
    bool is_fet_discharge_shading_memory_safe;
} FetDischargeThermalContext;

typedef struct {
    float in_silicon_thermal_fidelity;
    float thermal_strategy_datbin_merkle_ratio;
    float thermal_solve_latency_ns;
    uint64_t verified_thermal_saat_clearances;

    bool fet_thermal_pipeline_verified;
    bool thermal_strategy_merkle_verified;
    bool thermal_submicro_latency_verified;
    bool thermal_lossless_saat_verified;
    bool sovereign_2430_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} FetThermalBeyond2425State;

int cpm_tomie_fet_thermal_init(FetDischargeThermalContext *ctx);
int cpm_tomie_fet_thermal_step(FetDischargeThermalContext *ctx, uint32_t node_idx, float v_gate);
bool cpm_tomie_fet_thermal_assert_safety(FetDischargeThermalContext *ctx);

void auncient_fet_thermal_init(FetThermalBeyond2425State *state);
bool auncient_fet_thermal_verify_theorems_2426_2430(FetThermalBeyond2425State *state);
uint32_t auncient_fet_thermal_compute_rule18(const FetThermalBeyond2425State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_FET_DISCHARGE_THERMAL_SHADING_THEOREMS_2426_2430_H */
