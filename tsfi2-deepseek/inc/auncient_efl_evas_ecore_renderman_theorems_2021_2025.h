#ifndef AUNCIENT_EFL_EVAS_ECORE_RENDERMAN_THEOREMS_2021_2025_H
#define AUNCIENT_EFL_EVAS_ECORE_RENDERMAN_THEOREMS_2021_2025_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EFL_MAX_CANVAS_OBJECTS 64
#define EFL_DIRTY_REGION_COUNT 16

typedef struct {
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
    bool is_dirty;
} EflEvasRect;

typedef struct {
    uint32_t object_id;
    uint32_t layer;
    EflEvasRect geometry;
    bool visible;
    uint32_t render_pass_count;
} EflEvasObject;

typedef struct {
    EflEvasObject objects[EFL_MAX_CANVAS_OBJECTS];
    uint32_t active_object_count;
    uint32_t ecore_events_dispatched;
    uint32_t ecore_timer_ticks;
    uint32_t cdc6600_ppu_rendered_regions;
    bool is_evas_canvas_rendered;
    bool is_ecore_loop_stable;
} EflEvasEcoreRenderManContext;

typedef struct {
    float in_silicon_efl_renderman_fidelity;
    float efl_strategy_datbin_merkle_ratio;
    float efl_event_dispatch_latency_ns;
    uint64_t verified_efl_saat_clearances;

    bool efl_evas_reyes_verified;
    bool efl_strategy_merkle_verified;
    bool efl_submicro_latency_verified;
    bool efl_lossless_saat_verified;
    bool grand_2025_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} EflEvasEcoreBeyond2020State;

int cpm_tomie_efl_evas_ecore_init(EflEvasEcoreRenderManContext *ctx);
int cpm_tomie_efl_evas_add_object(EflEvasEcoreRenderManContext *ctx, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t layer);
int cpm_tomie_efl_ecore_main_loop_iterate(EflEvasEcoreRenderManContext *ctx);
void auncient_efl_evas_ecore_init(EflEvasEcoreBeyond2020State *state);
bool auncient_efl_evas_ecore_verify_theorems_2021_2025(EflEvasEcoreBeyond2020State *state);
uint32_t auncient_efl_evas_ecore_compute_rule18(const EflEvasEcoreBeyond2020State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_EFL_EVAS_ECORE_RENDERMAN_THEOREMS_2021_2025_H */
