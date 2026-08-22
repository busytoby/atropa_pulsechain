#ifndef EVAS_SMART_NCSA_MOSAIC_VIEWPORT_THEOREMS_2526_2530_H
#define EVAS_SMART_NCSA_MOSAIC_VIEWPORT_THEOREMS_2526_2530_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EVAS_MOSAIC_DOM_NODES 32
#define EVAS_MOSAIC_VIEWPORT_CANARY_GUARD 0x4D4F534149433636ULL /* "MOSAIC66" */

typedef struct {
    uint32_t node_id;
    uint32_t tag_type; /* 0: HTML, 1: HEAD, 2: TITLE, 3: BODY, 4: H1, 5: P, 6: PRE_VT100 */
    float layout_box_xywh[4]; /* x, y, width, height */
    uint32_t font_glyph_count;
    bool is_dom_node_rendered;
} EvasMosaicHtmlDomNode;

typedef struct {
    uint64_t head_guard;
    EvasMosaicHtmlDomNode nodes[EVAS_MOSAIC_DOM_NODES];
    uint64_t evas_mosaic_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_dom_nodes_parsed;
    uint32_t mosaic_layout_boxes_resolved;
    uint32_t cdc6600_60bit_mosaic_words;
    uint32_t overflow_trapped_dom_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_native_html5_dom_clean;
    bool is_evas_mosaic_viewport_memory_safe;
} EvasSmartMosaicViewportContext;

typedef struct {
    float in_silicon_mosaic_fidelity;
    float mosaic_strategy_datbin_merkle_ratio;
    float mosaic_layout_latency_ns;
    uint64_t verified_mosaic_saat_clearances;

    bool evas_mosaic_viewport_pipeline_verified;
    bool mosaic_strategy_merkle_verified;
    bool mosaic_submicro_latency_verified;
    bool mosaic_lossless_saat_verified;
    bool mosaic_dom_hierarchy_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartMosaicBeyond2525State;

int evas_smart_mosaic_viewport_init(EvasSmartMosaicViewportContext *ctx);
int evas_smart_mosaic_viewport_insert_node(EvasSmartMosaicViewportContext *ctx, uint32_t node_idx, uint32_t tag, float x, float y, float w, float h, uint32_t glyphs);
int evas_smart_mosaic_viewport_render_plane(EvasSmartMosaicViewportContext *ctx, uint32_t node_idx, float *out_area);
bool evas_smart_mosaic_viewport_assert_safety(EvasSmartMosaicViewportContext *ctx);

void evas_smart_mosaic_beyond2525_init(EvasSmartMosaicBeyond2525State *state);
bool evas_smart_mosaic_beyond2525_verify_theorems_2526_2530(EvasSmartMosaicBeyond2525State *state);
uint32_t evas_smart_mosaic_beyond2525_compute_rule18(const EvasSmartMosaicBeyond2525State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_NCSA_MOSAIC_VIEWPORT_THEOREMS_2526_2530_H */
