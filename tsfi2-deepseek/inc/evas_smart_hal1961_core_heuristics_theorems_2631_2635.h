#ifndef EVAS_SMART_HAL1961_CORE_HEURISTICS_THEOREMS_2631_2635_H
#define EVAS_SMART_HAL1961_CORE_HEURISTICS_THEOREMS_2631_2635_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL1961_HEURISTIC_NODES 64
#define HAL1961_CORE_CANARY_GUARD 0x48414C434F524536ULL /* "HALCORE6" */

typedef struct {
    uint32_t heuristic_id;
    float confidence_weight;       /* Bounded [0.0, 1.0] */
    float telemetry_voltage_mv;    /* Invariant -70.0 mV stable baseline */
    float cognitive_entropy;       /* Zero empirical noise (Rule 21) */
    uint32_t merkle_proof_leaf_hash;
    bool is_heuristic_uncompromised;
} EvasHalHeuristicCoreNode;

typedef struct {
    uint64_t head_guard;
    EvasHalHeuristicCoreNode nodes[HAL1961_HEURISTIC_NODES];
    float global_system_fidelity;
    uint64_t evas_hal_core_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_inference_cycles_executed;
    uint32_t heuristic_state_evaluations;
    uint32_t cdc6600_60bit_hal_core_words;
    uint32_t overflow_trapped_hal_core_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_hal_cognitive_core_lossless;
    bool is_evas_hal_core_memory_safe;
} EvasSmartHal1961CoreHeuristicsContext;

typedef struct {
    float in_silicon_hal_core_fidelity;
    float hal_core_strategy_datbin_merkle_ratio;
    float hal_core_eval_latency_ns;
    uint64_t verified_hal_core_saat_clearances;

    bool evas_hal_core_pipeline_verified;
    bool hal_core_strategy_merkle_verified;
    bool hal_core_submicro_latency_verified;
    bool hal_lossless_saat_verified;
    bool hal_core_heuristic_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartHalCoreBeyond2630State;

int evas_smart_hal1961_core_init(EvasSmartHal1961CoreHeuristicsContext *ctx);
int evas_smart_hal1961_core_evaluate_heuristic(EvasSmartHal1961CoreHeuristicsContext *ctx, uint32_t node_idx, float stimulus, float *out_confidence);
bool evas_smart_hal1961_core_assert_safety(EvasSmartHal1961CoreHeuristicsContext *ctx);

void evas_smart_hal_core_beyond2630_init(EvasSmartHalCoreBeyond2630State *state);
bool evas_smart_hal_core_beyond2630_verify_theorems_2631_2635(EvasSmartHalCoreBeyond2630State *state);
uint32_t evas_smart_hal_core_beyond2630_compute_rule18(const EvasSmartHalCoreBeyond2630State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_HAL1961_CORE_HEURISTICS_THEOREMS_2631_2635_H */
