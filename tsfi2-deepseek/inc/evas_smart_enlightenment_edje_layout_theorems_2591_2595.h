#ifndef EVAS_SMART_ENLIGHTENMENT_EDJE_LAYOUT_THEOREMS_2591_2595_H
#define EVAS_SMART_ENLIGHTENMENT_EDJE_LAYOUT_THEOREMS_2591_2595_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EDJE_PART_ELEMENT_COUNT 64
#define EDJE_LAYOUT_CANARY_GUARD 0x45444A4536363636ULL /* "EDJE6666" */

typedef struct {
    uint32_t part_id;
    uint32_t part_type; /* 0: RECT, 1: IMAGE, 2: TEXT, 3: SWALLOW_CANVAS */
    float rel1_xy[2];   /* Relative anchor top-left [0.0, 1.0] */
    float rel2_xy[2];   /* Relative anchor bottom-right [0.0, 1.0] */
    float offset1_xy[2];/* Absolute pixel offset top-left */
    float offset2_xy[2];/* Absolute pixel offset bottom-right */
    float computed_box_xywh[4];
    bool is_part_resolved;
} EvasEdjeLayoutPartNode;

typedef struct {
    uint64_t head_guard;
    EvasEdjeLayoutPartNode parts[EDJE_PART_ELEMENT_COUNT];
    float parent_container_w;
    float parent_container_h;
    uint64_t evas_edje_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_edje_parts_resolved;
    uint32_t layout_recalculations_executed;
    uint32_t cdc6600_60bit_edje_words;
    uint32_t overflow_trapped_edje_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_edje_layout_constraint_exact;
    bool is_evas_edje_layout_memory_safe;
} EvasSmartEdjeLayoutContext;

typedef struct {
    float in_silicon_edje_fidelity;
    float edje_strategy_datbin_merkle_ratio;
    float edje_layout_latency_ns;
    uint64_t verified_edje_saat_clearances;

    bool evas_edje_pipeline_verified;
    bool edje_strategy_merkle_verified;
    bool edje_submicro_latency_verified;
    bool edje_lossless_saat_verified;
    bool edje_constraint_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartEdjeBeyond2590State;

int evas_smart_edje_layout_init(EvasSmartEdjeLayoutContext *ctx);
int evas_smart_edje_layout_resolve_part(EvasSmartEdjeLayoutContext *ctx, uint32_t part_idx, float rel1_x, float rel1_y, float rel2_x, float rel2_y, float out_box[4]);
bool evas_smart_edje_layout_assert_safety(EvasSmartEdjeLayoutContext *ctx);

void evas_smart_edje_beyond2590_init(EvasSmartEdjeBeyond2590State *state);
bool evas_smart_edje_beyond2590_verify_theorems_2591_2595(EvasSmartEdjeBeyond2590State *state);
uint32_t evas_smart_edje_beyond2590_compute_rule18(const EvasSmartEdjeBeyond2590State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_ENLIGHTENMENT_EDJE_LAYOUT_THEOREMS_2591_2595_H */
