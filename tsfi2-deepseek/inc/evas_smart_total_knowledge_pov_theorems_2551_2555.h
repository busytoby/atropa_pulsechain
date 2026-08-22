#ifndef EVAS_SMART_TOTAL_KNOWLEDGE_POV_THEOREMS_2551_2555_H
#define EVAS_SMART_TOTAL_KNOWLEDGE_POV_THEOREMS_2551_2555_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define YI_HEXAGRAM_COORDINATES 64
#define TOTAL_KNOWLEDGE_POV_CANARY_GUARD 0x544B504F56363636ULL /* "TKPOV666" */

typedef struct {
    uint32_t yi_hexagram_index;
    float phase_angle_rad;
    float temporal_luminance_flux;
    float epistemic_retention_ratio;
    uint32_t ramac_merkle_leaf_hash;
    bool is_total_knowledge_retained;
} EvasYiTotalKnowledgePovNode;

typedef struct {
    uint64_t head_guard;
    EvasYiTotalKnowledgePovNode nodes[YI_HEXAGRAM_COORDINATES];
    uint64_t evas_tk_pov_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_yi_harmonics_evaluated;
    uint32_t epistemic_merkle_nodes_verified;
    uint32_t cdc6600_60bit_tk_words;
    uint32_t overflow_trapped_tk_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_total_knowledge_unbroken;
    bool is_evas_tk_pov_memory_safe;
} EvasSmartTotalKnowledgePovContext;

typedef struct {
    float in_silicon_tk_pov_fidelity;
    float tk_pov_strategy_datbin_merkle_ratio;
    float tk_pov_eval_latency_ns;
    uint64_t verified_tk_pov_saat_clearances;

    bool evas_tk_pov_pipeline_verified;
    bool tk_pov_strategy_merkle_verified;
    bool tk_pov_submicro_latency_verified;
    bool tk_pov_lossless_saat_verified;
    bool yi_harmonic_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartTkPovBeyond2550State;

int evas_smart_tk_pov_init(EvasSmartTotalKnowledgePovContext *ctx);
int evas_smart_tk_pov_evaluate_harmonic(EvasSmartTotalKnowledgePovContext *ctx, uint32_t hex_idx, float lum, float *out_phase);
bool evas_smart_tk_pov_assert_safety(EvasSmartTotalKnowledgePovContext *ctx);

void evas_smart_tk_pov_beyond2550_init(EvasSmartTkPovBeyond2550State *state);
bool evas_smart_tk_pov_beyond2550_verify_theorems_2551_2555(EvasSmartTkPovBeyond2550State *state);
uint32_t evas_smart_tk_pov_beyond2550_compute_rule18(const EvasSmartTkPovBeyond2550State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_TOTAL_KNOWLEDGE_POV_THEOREMS_2551_2555_H */
