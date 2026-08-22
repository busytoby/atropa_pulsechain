#ifndef AUNCIENT_RENDERMAN_REYES_DICING_MICROPOLY_THEOREMS_2476_2480_H
#define AUNCIENT_RENDERMAN_REYES_DICING_MICROPOLY_THEOREMS_2476_2480_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REYES_BICUBIC_PATCHES 32
#define REYES_DICING_MICROPOLYGONS 1024
#define REYES_DICING_CANARY_GUARD 0x5245594449434536ULL /* "REYDICE6" */

typedef struct {
    uint32_t patch_id;
    float screen_area_pixels;
    uint32_t u_dicing_rate;
    uint32_t v_dicing_rate;
    uint32_t total_subpixel_micropolygons;
    bool is_diced_to_subpixel_limit;
} ReyesBicubicBezierPatch;

typedef struct {
    uint64_t head_guard;
    ReyesBicubicBezierPatch patches[REYES_BICUBIC_PATCHES];
    uint64_t reyes_dicing_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_patches_diced;
    uint32_t total_micropolygons_rasterized;
    uint32_t cdc6600_60bit_dicing_words;
    uint32_t overflow_trapped_dicing_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_reyes_subpixel_dicing_lossless;
    bool is_reyes_dicing_memory_safe;
} RendermanReyesDicingContext;

typedef struct {
    float in_silicon_dicing_fidelity;
    float dicing_strategy_datbin_merkle_ratio;
    float dicing_eval_latency_ns;
    uint64_t verified_dicing_saat_clearances;

    bool reyes_dicing_pipeline_verified;
    bool dicing_strategy_merkle_verified;
    bool dicing_submicro_latency_verified;
    bool dicing_lossless_saat_verified;
    bool sovereign_2480_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} RendermanReyesDicingBeyond2475State;

int cpm_tomie_reyes_dicing_init(RendermanReyesDicingContext *ctx);
int cpm_tomie_reyes_dicing_evaluate_patch(RendermanReyesDicingContext *ctx, uint32_t patch_idx, float screen_area);
bool cpm_tomie_reyes_dicing_assert_safety(RendermanReyesDicingContext *ctx);

void auncient_reyes_dicing_init(RendermanReyesDicingBeyond2475State *state);
bool auncient_reyes_dicing_verify_theorems_2476_2480(RendermanReyesDicingBeyond2475State *state);
uint32_t auncient_reyes_dicing_compute_rule18(const RendermanReyesDicingBeyond2475State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_RENDERMAN_REYES_DICING_MICROPOLY_THEOREMS_2476_2480_H */
