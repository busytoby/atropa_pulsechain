#ifndef AUNCIENT_RENDERMAN_REYES_EFL_EVAS_CANVAS_THEOREMS_2481_2485_H
#define AUNCIENT_RENDERMAN_REYES_EFL_EVAS_CANVAS_THEOREMS_2481_2485_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EFL_EVAS_OBJECT_SLOTS 64
#define EFL_EVAS_CANVAS_CANARY_GUARD 0x45464C4556415336ULL /* "EFLEVAS6" */

typedef struct {
    uint32_t object_id;
    uint32_t patch_source_id;
    float canvas_x;
    float canvas_y;
    float canvas_z_depth;
    float alpha_opacity;
    bool is_damage_rect_dirtied;
    bool is_evas_object_visible;
} EflEvasSmartObjectNode;

typedef struct {
    uint64_t head_guard;
    EflEvasSmartObjectNode objects[EFL_EVAS_OBJECT_SLOTS];
    uint64_t efl_rebar_blit_latch[64];
    uint64_t tail_guard;
    uint32_t total_evas_objects_composited;
    uint32_t damage_rect_blits_executed;
    uint32_t cdc6600_60bit_evas_words;
    uint32_t overflow_trapped_canvas_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_efl_reyes_canvas_lossless;
    bool is_efl_evas_canvas_memory_safe;
} RendermanReyesEflCanvasContext;

typedef struct {
    float in_silicon_efl_canvas_fidelity;
    float efl_canvas_strategy_datbin_merkle_ratio;
    float efl_canvas_blit_latency_ns;
    uint64_t verified_efl_canvas_saat_clearances;

    bool efl_reyes_canvas_pipeline_verified;
    bool efl_canvas_strategy_merkle_verified;
    bool efl_canvas_submicro_latency_verified;
    bool efl_canvas_lossless_saat_verified;
    bool sovereign_2485_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanEflCanvasBeyond2480State;

int cpm_tomie_efl_canvas_init(RendermanReyesEflCanvasContext *ctx);
int cpm_tomie_efl_canvas_bind_patch(RendermanReyesEflCanvasContext *ctx, uint32_t obj_idx, uint32_t patch_id, float x, float y, float z, float alpha);
int cpm_tomie_efl_canvas_render_damage(RendermanReyesEflCanvasContext *ctx, uint32_t obj_idx);
bool cpm_tomie_efl_canvas_assert_safety(RendermanReyesEflCanvasContext *ctx);

void auncient_efl_canvas_init(RendermanEflCanvasBeyond2480State *state);
bool auncient_efl_canvas_verify_theorems_2481_2485(RendermanEflCanvasBeyond2480State *state);
uint32_t auncient_efl_canvas_compute_rule18(const RendermanEflCanvasBeyond2480State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_REYES_EFL_EVAS_CANVAS_THEOREMS_2481_2485_H */
