#ifndef AUNCIENT_RENDERMAN_TEAPOT_ILLIAC_PLL_THEOREMS_2296_2300_H
#define AUNCIENT_RENDERMAN_TEAPOT_ILLIAC_PLL_THEOREMS_2296_2300_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TEAPOT_BEZIER_PATCHES 32
#define TEAPOT_MICROPOLYGONS_PER_PATCH 256
#define TEAPOT_TOTAL_MICROPOLYGONS (TEAPOT_BEZIER_PATCHES * TEAPOT_MICROPOLYGONS_PER_PATCH) /* 8,192 */
#define TEAPOT_ILLIAC_PLL_CANARY_GUARD 0x54454150494C4C30ULL /* "TEAPILL0" */

typedef struct {
    uint32_t patch_id;
    uint32_t diced_micropolygons;
    float pll_phase_lock_freq_hz;
    float thacher_curvature_metric;
    bool is_dma_streamed;
    bool is_seam_continuous;
} TeapotPatchPllState;

typedef struct {
    uint64_t head_guard;
    TeapotPatchPllState patches[TEAPOT_BEZIER_PATCHES];
    uint64_t vram_rebar_latch[TEAPOT_TOTAL_MICROPOLYGONS];
    uint64_t tail_guard;
    uint32_t total_frames_rendered;
    uint32_t total_micropolygons_rasterized;
    uint32_t cdc6600_60bit_frame_words;
    uint32_t overflow_trapped_frames;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_reyes_rasterization_coherent;
    bool is_teapot_demo_memory_safe;
} RendermanTeapotIlliacPllContext;

typedef struct {
    float in_silicon_demo_fidelity;
    float demo_strategy_datbin_merkle_ratio;
    float demo_raster_latency_ns;
    uint64_t verified_demo_saat_clearances;

    bool teapot_demo_pipeline_verified;
    bool demo_strategy_merkle_verified;
    bool demo_submicro_latency_verified;
    bool demo_lossless_saat_verified;
    bool sovereign_2300_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanTeapotIlliacBeyond2295State;

int cpm_tomie_renderman_teapot_illiac_pll_init(RendermanTeapotIlliacPllContext *ctx);
int cpm_tomie_renderman_teapot_illiac_pll_render_frame(RendermanTeapotIlliacPllContext *ctx);
bool cpm_tomie_renderman_teapot_illiac_pll_assert_safety(RendermanTeapotIlliacPllContext *ctx);

void auncient_renderman_teapot_illiac_pll_init(RendermanTeapotIlliacBeyond2295State *state);
bool auncient_renderman_teapot_illiac_pll_verify_theorems_2296_2300(RendermanTeapotIlliacBeyond2295State *state);
uint32_t auncient_renderman_teapot_illiac_pll_compute_rule18(const RendermanTeapotIlliacBeyond2295State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_TEAPOT_ILLIAC_PLL_THEOREMS_2296_2300_H */
