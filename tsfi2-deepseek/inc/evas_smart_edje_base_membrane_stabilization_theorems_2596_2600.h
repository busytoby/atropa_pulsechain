#ifndef EVAS_SMART_EDJE_BASE_MEMBRANE_STABILIZATION_THEOREMS_2596_2600_H
#define EVAS_SMART_EDJE_BASE_MEMBRANE_STABILIZATION_THEOREMS_2596_2600_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EDJE_BASE_MEMBRANE_ELEMENT_COUNT 64
#define EDJE_MEMBRANE_CANARY_GUARD 0x45444A454D454D36ULL /* "EDJEMEM6" */

typedef struct {
    uint32_t element_id;
    float relative_anchor_rel1_xy[2];
    float relative_anchor_rel2_xy[2];
    float resting_potential_mv;       /* Exact -70.0 mV baseline */
    float fluid_mosaic_elasticity;    /* 1.0 = lossless elastic continuity */
    float resolved_membrane_box_xywh[4];
    bool is_membrane_part_stabilized;
} EvasEdjeBaseMembraneNode;

typedef struct {
    uint64_t head_guard;
    EvasEdjeBaseMembraneNode elements[EDJE_BASE_MEMBRANE_ELEMENT_COUNT];
    float canvas_viewport_w;
    float canvas_viewport_h;
    uint64_t evas_edje_mem_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_membrane_parts_anchored;
    uint32_t relative_constraint_solves_executed;
    uint32_t cdc6600_60bit_edje_mem_words;
    uint32_t overflow_trapped_edje_mem_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_edje_base_membrane_lossless;
    bool is_evas_edje_mem_memory_safe;
} EvasSmartEdjeBaseMembraneContext;

typedef struct {
    float in_silicon_edje_mem_fidelity;
    float edje_mem_strategy_datbin_merkle_ratio;
    float edje_mem_eval_latency_ns;
    uint64_t verified_edje_mem_saat_clearances;

    bool evas_edje_mem_pipeline_verified;
    bool edje_mem_strategy_merkle_verified;
    bool edje_mem_submicro_latency_verified;
    bool edje_mem_lossless_saat_verified;
    bool edje_base_membrane_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartEdjeMemBeyond2595State;

int evas_smart_edje_base_membrane_init(EvasSmartEdjeBaseMembraneContext *ctx);
int evas_smart_edje_base_membrane_anchor_element(EvasSmartEdjeBaseMembraneContext *ctx, uint32_t elem_idx, float rel1_x, float rel1_y, float rel2_x, float rel2_y, float out_box[4]);
bool evas_smart_edje_base_membrane_assert_safety(EvasSmartEdjeBaseMembraneContext *ctx);

void evas_smart_edje_mem_beyond2595_init(EvasSmartEdjeMemBeyond2595State *state);
bool evas_smart_edje_mem_beyond2595_verify_theorems_2596_2600(EvasSmartEdjeMemBeyond2595State *state);
uint32_t evas_smart_edje_mem_beyond2595_compute_rule18(const EvasSmartEdjeMemBeyond2595State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_EDJE_BASE_MEMBRANE_STABILIZATION_THEOREMS_2596_2600_H */
