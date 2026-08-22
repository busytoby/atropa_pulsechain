#ifndef AUNCIENT_RENDERMAN_CAUSTIC_ENERGY_BOUND_THEOREMS_2396_2400_H
#define AUNCIENT_RENDERMAN_CAUSTIC_ENERGY_BOUND_THEOREMS_2396_2400_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CAUSTIC_ACCUMULATOR_CELLS 64
#define CAUSTIC_CLAMP_MAX_LUMENS 100.0f
#define CAUSTIC_ENERGY_GUARD_CANARY 0x4341555353414636ULL /* "CAUSSAF6" */

typedef struct {
    uint32_t cell_id;
    float accumulated_flux_lumens;
    float non_preferential_clamp_ratio;
    bool is_cell_energy_bounded;
    bool is_hotspot_trapped;
} CausticSafetyCell;

typedef struct {
    uint64_t head_guard;
    CausticSafetyCell cells[CAUSTIC_ACCUMULATOR_CELLS];
    uint64_t energy_limiter_latch[64];
    uint64_t tail_guard;
    uint32_t total_flux_accumulations_evaluated;
    uint32_t hotspots_trapped_and_redirected;
    uint32_t cdc6600_60bit_caustic_words;
    uint32_t overflow_trapped_flux_bursts;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_global_energy_strictly_conserved;
    bool is_caustic_accumulator_memory_safe;
} RendermanCausticEnergySafetyContext;

typedef struct {
    float in_silicon_caustic_safety_fidelity;
    float caustic_safety_strategy_datbin_merkle_ratio;
    float caustic_safety_clamp_latency_ns;
    uint64_t verified_caustic_safety_saat_clearances;

    bool caustic_safety_pipeline_verified;
    bool caustic_safety_strategy_merkle_verified;
    bool caustic_safety_submicro_latency_verified;
    bool caustic_safety_lossless_saat_verified;
    bool sovereign_2400_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanCausticSafetyBeyond2395State;

int cpm_tomie_renderman_caustic_safety_init(RendermanCausticEnergySafetyContext *ctx);
int cpm_tomie_renderman_caustic_safety_accumulate(RendermanCausticEnergySafetyContext *ctx, uint32_t cell_idx, float input_flux);
bool cpm_tomie_renderman_caustic_safety_assert_safety(RendermanCausticEnergySafetyContext *ctx);

void auncient_renderman_caustic_safety_init(RendermanCausticSafetyBeyond2395State *state);
bool auncient_renderman_caustic_safety_verify_theorems_2396_2400(RendermanCausticSafetyBeyond2395State *state);
uint32_t auncient_renderman_caustic_safety_compute_rule18(const RendermanCausticSafetyBeyond2395State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_CAUSTIC_ENERGY_BOUND_THEOREMS_2396_2400_H */
